# Comprendre l'architecture {#architecture}

## Le trajet d'une image

La boucle principale (`src/app/main.cpp`) répète ces étapes tant que la fenêtre est ouverte.

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
  Application      applique l'action et demande
        |          à l'horloge combien simuler
        v
  Simulation       calcule ces générations
        |
        v
  Win32Renderer    dessine l'écran demandé
```

Le calcul ne dépend donc pas du dessin : à la vitesse x10, ecosim::SimulationClock demande
simplement dix fois plus de générations par seconde.

## Les liens entre les classes

Les diagrammes suivent la notation UML ; leur légende est en bas de chaque image. Ils montrent
les classes principales, pas les petites structures de données.

### De l'application au moteur

@image html classes_application.svg "De l'application au moteur" width=100%

<a href="classes_application.svg" target="_blank">Ouvrir ce diagramme en grand</a>

Les flèches vont toujours du haut vers le bas ou restent dans leur couche : le moteur ne
connaît ni l'interface ni l'affichage. ecosim::Application ne parle à Windows qu'à travers
ecosim::InputSource et ecosim::View.

### Le moteur

@image html classes_moteur.svg "Le moteur de simulation" width=100%

<a href="classes_moteur.svg" target="_blank">Ouvrir ce diagramme en grand</a>

ecosim::Simulation possède la carte, les animaux et les statistiques. Les animaux ne la
connaissent pas : ils perçoivent leur entourage par ecosim::Habitat, implémentée par une
classe interne créée à chaque génération.

## Trois choix de conception

### 1. L'héritage pour les espèces

ecosim::Simulation manipule des ecosim::Animal sans savoir s'il s'agit d'un herbivore ou d'un
prédateur. Chaque espèce redéfinit sa façon de choisir une direction
(ecosim::Animal::Steer) et d'agir (ecosim::Animal::Act). C'est le **polymorphisme**.

### 2. L'interface Habitat

Un animal a besoin de savoir ce qui l'entoure, mais pas de connaître toute la simulation. Il
passe par ecosim::Habitat. On peut ainsi tester une espèce seule avec un faux habitat.

En interne, la simulation range les animaux **par case** : pour trouver ses voisins, un animal
ne regarde que les cases proches au lieu de parcourir toute la population.

### 3. Les interfaces InputSource et View

ecosim::Application ne connaît ni Windows ni le dessin : seulement ecosim::InputSource et
ecosim::View. Les tests les remplacent par de fausses versions et vérifient la pause ou le
redémarrage **sans ouvrir de fenêtre**.

## La navigation entre les écrans

ecosim::UiController regroupe la navigation de tous les écrans. Une version précédente la
découpait en plusieurs classes, plus proche des principes SOLID mais plus longue à suivre.
Le projet a privilégié la simplicité : ajouter un écran demande de modifier ce contrôleur et
le rendu.

## La 3D

La 3D n'utilise pas la carte graphique. ecosim::OrbitCamera projette chaque point de l'espace
sur l'écran, ecosim::BuildScene produit une liste de polygones, et ceux-ci sont dessinés **du
plus loin au plus proche** (algorithme du peintre) : les objets proches recouvrent les autres.

## Limites techniques

- Le rendu utilise Win32/GDI : il ne fonctionne que sous Windows.
- La vitesse d'un animal ne coûte pas d'énergie.
- Les obstacles ne bloquent pas la vue des animaux.
- La 3D n'a ni textures ni éclairage dynamique.
- Les préréglages sont conçus pour les deux espèces actuelles.

## Compilation

CMake construit trois cibles qui correspondent aux couches.

| Cible | Contenu | Dépend de |
| --- | --- | --- |
| `ecosim_core` | moteur | bibliothèque standard |
| `ecosim_ui` | interface | `ecosim_core` |
| `EcoSim` | affichage Windows | `ecosim_ui`, `user32`, `gdi32` |

Les tests ne compilent aucun code Windows. Ils sont dans le dossier `tests/`, organisé comme
`src/`.
