# Comprendre l'architecture {#architecture}

## Le trajet d'une image

La boucle principale (`src/app/main.cpp`) répète ces étapes tant que la fenêtre est ouverte. Les
générations sont calculées par un **second thread**.

```
  clavier et souris
        |
        v
  Win32Input       lit les touches et les clics
        |
        v
  UiController     choisit une action : Pause...
        |
        v
  Application      applique l'action et demande          THREAD DE CALCUL
        |          à l'horloge combien simuler
        |  -------- demande de générations --------->  SimulationRunner
        |                                             fait avancer Simulation
        |  <------- dernière photo publiée ----------  SimulationSnapshot
        v
  Win32Renderer    dessine la photo
```

Le calcul ne dépend donc pas du dessin : à la vitesse x10, ecosim::SimulationClock demande
simplement dix fois plus de générations par seconde.

## Les deux threads

| Thread | Son travail |
| --- | --- |
| Principal | clavier, souris, horloge et dessin |
| De calcul | ecosim::SimulationRunner : fait avancer la simulation |

- Après sa création, **seul le thread de calcul touche à ecosim::Simulation**.
- Les autres lui **demandent** des générations (ecosim::SimulationRunner::Request) ou l'ajout
  d'un animal (ecosim::SimulationRunner::Add), qu'il traite entre deux générations.
- Il publie des **photos** (ecosim::SimulationSnapshot) : des copies qui ne changent plus. Le
  thread principal les lit sans risque pendant que le calcul continue.
- Un `std::mutex` protège seulement l'échange : les demandes, le pointeur vers la
  dernière photo et l'état du thread. La copie elle-même se fait hors du verrou.
- Le thread dort tant que rien n'est demandé (`std::condition_variable_any`). C'est un
  `std::jthread` : sa destruction lui demande de s'arrêter et attend la fin de la génération en
  cours.

La fenêtre répond ainsi pendant le calcul. **Une génération n'est pas calculée plus vite** : la
copie ajoute même un travail, et le dessin reste fait par le thread principal.

## Les liens entre les classes

Les diagrammes suivent la notation UML ; leur légende est en bas de chaque image. Les deux
vues d'ensemble ci-dessous montrent les classes, quelques structures importantes et leurs
liens. Une classe qui implémente une interface a aussi les liens de cette interface : ils ne
sont pas redessinés. Les types et les membres au complet sont dans les
[diagrammes détaillés](@ref diagrammes).

### De l'application au moteur

@image html classes_application.svg "De l'application au moteur" width=100%

<a href="classes_application.svg" target="_blank">Ouvrir ce diagramme en grand</a>

Les flèches vont toujours du haut vers le bas ou restent dans leur couche : le moteur ne
connaît ni l'interface ni l'affichage. ecosim::Application ne parle à Windows qu'à travers
ecosim::InputSource et ecosim::View.

À chaque image, ecosim::Win32Renderer crée un ecosim::Canvas. Les fonctions de dessin y
travaillent en mémoire, puis le résultat est copié d'un coup dans la fenêtre, ce qui évite
le scintillement.

### Le moteur

@image html classes_moteur.svg "Le moteur de simulation" width=100%

<a href="classes_moteur.svg" target="_blank">Ouvrir ce diagramme en grand</a>

ecosim::Simulation possède la carte, les animaux et les statistiques. Les animaux ne la
connaissent pas : ils perçoivent leur entourage par ecosim::Habitat, implémentée par
ecosim::GridHabitat, que la simulation crée à chaque génération. Les chemins vers l'eau
(ecosim::WaterRoutes) sont au contraire calculés une fois, à la création de la simulation, qui
les prête à l'habitat quand la soif est active.

Les espèces sont décrites dans un catalogue, ecosim::SpeciesCatalog() : une fiche
ecosim::SpeciesInfo par espèce, avec son nom, ses valeurs de départ et la fonction
qui crée ses animaux. Les réglages et la simulation parcourent ce catalogue au lieu de
nommer chaque classe.

## Trois choix de conception

### 1. L'héritage pour les espèces

ecosim::Simulation manipule des ecosim::Animal sans savoir s'il s'agit d'un cerf, d'un lapin,
d'un loup ou d'un renard. Chaque animal choisit sa direction (ecosim::Animal::Steer) et agit
(ecosim::Animal::Act) à sa façon. C'est le **polymorphisme**.

L'héritage a deux niveaux, et deux familles :

- ecosim::Herbivore réunit ce que les herbivores font de la même manière, fuir et brouter ;
  ecosim::Deer et ecosim::Rabbit ne gardent que leurs différences ;
- ecosim::Predator réunit la poursuite et la capture ; ecosim::Wolf y ajoute la meute, et
  ecosim::Fox son régime.

Chaque chasseur dit ce qu'il mange avec ecosim::Animal::Eats. La simulation s'en sert pour
trouver ses proies, et pour qu'un herbivore ne fuie que ceux qui le mangent.

Ce qui définit une espèce est rangé dans sa classe, en constantes (ecosim::SpeciesTraits). Ce
que l'utilisateur règle est à part (ecosim::AnimalOptions), et chaque petit en hérite.

### 2. L'interface Habitat

Un animal a besoin de savoir ce qui l'entoure, mais pas de connaître toute la simulation. Il
passe par ecosim::Habitat. On peut ainsi tester une espèce seule avec un faux habitat.

En interne, la simulation range les animaux **par case** : pour trouver ses voisins, un animal
ne regarde que les cases proches au lieu de parcourir toute la population. Ce rangement sert
aussi aux collisions, qui écartent les corps superposés (ecosim::GridHabitat::Separate), et à la
formation des couples (ecosim::GridHabitat::FormCouples).

### 3. Les interfaces InputSource et View

ecosim::Application ne connaît ni Windows ni le dessin : seulement ecosim::InputSource et
ecosim::View. Les tests les remplacent par de fausses versions et vérifient la pause ou le
redémarrage **sans ouvrir de fenêtre**.

## La navigation entre les écrans

ecosim::UiController regroupe la navigation de tous les écrans. Une version précédente la
découpait en plusieurs classes, plus proche des principes SOLID mais plus longue à suivre.
Le projet a privilégié la simplicité : ajouter un écran demande de modifier ce contrôleur et
le rendu.

## La 2D

ecosim::FlatView calcule où la carte apparaît à l'écran : vue d'ensemble, zoom et déplacement.
La même vue sert au dessin et aux clics, qui restent ainsi toujours d'accord.

## La 3D

La 3D n'utilise pas la carte graphique. ecosim::OrbitCamera projette chaque point de l'espace
sur l'écran, ecosim::BuildScene produit une liste de polygones, et ceux-ci sont dessinés **du
plus loin au plus proche** (algorithme du peintre) : les objets proches recouvrent les autres.

Sur une carte plate, le sol ne cache rien : il est dessiné en premier, puis les volumes. Avec du
relief, les cases sont rangées de la plus lointaine à la plus proche, et chacune est dessinée
avec ses flancs puis ce qu'elle porte. L'écart à la caméra compté en colonnes plus lignes donne
un ordre sûr : une case n'en cache jamais une qui est dessinée après elle. Un animal est rattaché
à la plus proche des cases qu'il recouvre sans être plus haute que la sienne.

Pour viser une case, ecosim::OrbitCamera::PickCell descend le rayon de la souris palier par
palier : ecosim::InputSource lui transmet donc la carte affichée.

## Limites techniques

- Le rendu utilise Win32/GDI : il ne fonctionne que sous Windows.
- La vitesse d'un animal ne coûte pas d'énergie.
- Les obstacles ne bloquent pas la vue des animaux.
- La 3D n'a ni textures ni éclairage dynamique.
- Les grandes cartes sont lentes, surtout en 3D, qui dessine toutes les cases.
- Les préréglages sont conçus pour les quatre espèces actuelles.

## Compilation

CMake construit trois cibles qui correspondent aux couches.

| Cible | Contenu | Dépend de |
| --- | --- | --- |
| `ecosim_core` | moteur | bibliothèque standard |
| `ecosim_ui` | interface | `ecosim_core` |
| `EcoSim` | affichage Windows | `ecosim_ui`, `user32`, `gdi32` |

Les tests ne compilent aucun code Windows. Ils sont dans le dossier `tests/`, organisé comme
`src/`.
