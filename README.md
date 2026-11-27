# EcoSim

EcoSim est une simulation d'écosystème cellulaire en **C++23**. Une grille de prairies, d'eau et d'obstacles porte la végétation ; des cerfs, des lapins, des loups et des renards s'y déplacent **librement**, dans toutes les directions. Plusieurs animaux peuvent se trouver sur la même case. La simulation évolue par générations et son moteur fonctionne indépendamment de l'affichage.

L'application affiche la carte en direct, en **2D** ou dans une **première version 3D**, et permet de consulter l'évolution des statistiques sur des graphiques.

## Démarrage rapide sous Windows

Prérequis : **CMake 3.29 ou plus**, **Ninja** et un compilateur capable de construire les modules C++23 et `import std`. La configuration utilisée pour ce projet est **Clang 22 avec libc++ (llvm-mingw)**. Lancez les commandes suivantes depuis la racine du dépôt :

```powershell
cmake -S . -B cmake-build-debug -G Ninja -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug -DECOSIM_BUILD_TESTS=ON
cmake --build cmake-build-debug --parallel
.\cmake-build-debug\EcoSim.exe
```

Le premier lancement de CMake avec les tests activés récupère **Catch2** depuis GitHub et nécessite donc une connexion Internet. Pour compiler sans les tests, remplacez `-DECOSIM_BUILD_TESTS=ON` par `-DECOSIM_BUILD_TESTS=OFF`.

Si `cmake-build-debug` est déjà configuré, la commande `cmake --build cmake-build-debug --parallel` suffit pour reconstruire l'application dans **le même répertoire**.

L'application fonctionne sous Windows et utilise Win32/GDI pour l'affichage. La compilation et les tests ont été vérifiés avec ce rendu.

## Réglages de l'accueil

Avant de démarrer, cliquez sur une case et tapez la nouvelle valeur : le premier chiffre remplace l'ancienne. **Entrée** ou un clic ailleurs valide, **Tab** valide et passe à la case suivante, **Échap** annule la saisie. Une valeur hors limites est ramenée à la limite la plus proche ; une case laissée vide garde l'ancienne valeur. Les limites de la case active sont affichées en bas de l'écran. **Valeurs par défaut** réinitialise tous les réglages. Les réglages sont conservés au retour au menu et avec **Recommencer**, mais pas après la fermeture de l'application.

### Préréglages

Les boutons situés sous les colonnes remplacent tous les réglages. Chaque valeur reste modifiable ensuite, et le bouton choisi n'est plus surligné dès qu'une valeur change. **Valeurs par défaut** revient aux réglages d'origine.

| Préréglage | Ce qui change par rapport aux valeurs par défaut | Comportement observé (carte 42 × 26) |
| --- | --- | --- |
| **Équilibre** | Fuite des cerfs et des lapins 50 %, ni meute ni flair | Sur 400 cartes, les quatre espèces sont encore présentes à 400 générations dans 91 % des cas (de 86 à 95 % selon la série de 100), et à 1000 générations dans 86 % (de 81 à 91 %) |
| **Famine** | Aucune repousse, un seul loup et un seul renard | Plus aucun cerf ni lapin à 150 générations (la moitié des extinctions avant la génération 80 environ). Sur 100 cartes, la faim cause 93 % des morts chez les cerfs et 68 % chez les lapins |
| **Invasion** | Beaucoup plus de loups au départ (une case sur onze, 200 au maximum), aucune fuite, meute 3 | Sur 100 cartes, la moitié des extinctions de cerfs et de lapins a lieu avant la génération 12 environ, par capture. À la génération 100, il ne reste ni cerf ni lapin dans les 100 simulations |

Ces résultats sont des tendances : la carte et les animaux restent tirés au hasard. Les cartes mesurées n'ont pas servi à choisir les valeurs des espèces.

**Les préréglages dépendent de la taille.** « Équilibre » a été réglé pour une carte d'environ 42 × 26 :

| Taille | Les quatre espèces à 400 générations | Plus aucun cerf ni lapin |
| --- | --- | --- |
| 10 × 10 | 0 % (200 cartes) | 84 % |
| 20 × 14 | 7 % (200 cartes) | 8 % |
| 30 × 20 | 59 % (200 cartes) | 1 % |
| 42 × 26 | 90 % (200 cartes) | 0 % |
| 48 × 27 | 96 % (200 cartes) | 0 % |
| 80 × 45 | 100 % (60 cartes) | 0 % |

« Famine » et « Invasion » gardent leur effet aux tailles mesurées (10 × 10, 20 × 14, 48 × 27 et 80 × 45) : à la fin de l'observation, il ne reste ni cerf ni lapin dans presque toutes les simulations (au moins 39 sur 40). Au-delà de 80 × 45, aucun préréglage n'a été mesuré.

| Colonne | Réglages |
| --- | --- |
| Terrain | Part de la carte couverte d'eau (0 à 50 %) et d'obstacles (0 à 40 %), repousse de la végétation par génération (0 à 20), largeur et hauteur (10 au minimum, voir [Taille de la carte](#taille-de-la-carte)) |
| Cerfs et lapins | Nombre au départ (0 à 999 999), chance de naissance (0 à 100 %), chance de fuir (0 à 100 %) |
| Loups | Nombre au départ (0 à 999 999), chance de naissance (0 à 100 %), esprit de meute (0 à 4) |
| Renards | Nombre au départ (0 à 999 999), chance de naissance (0 à 100 %), flair (0 à 4) |

Chaque espèce n'a que **trois réglages**. Ses autres caractéristiques sont fixes (voir [Les quatre espèces](#les-quatre-espèces)) : ce sont elles qui la définissent. Tous les animaux placés au départ ont 50 d'énergie. Les nouveau-nés en ont 30, la reproduction coûte 30 et demande un âge d'au moins 5 générations. Le nombre d'animaux au départ dépend par défaut de la taille de l'écran. Les bornes sont définies dans `src/ui/presentation/settings_form.cpp` et, pour ce qui dépend de l'espèce, dans `src/ui/presentation/species_style.cpp`.

## Utilisation

| Action | Commande |
| --- | --- |
| Lancer une simulation depuis l'accueil | Bouton **Démarrer** |
| Vitesse x1, x2, x5 ou x10 | Touches `1`, `2`, `3` ou `4` |
| Passer à la vitesse suivante | **Tab** (x1, x2, x5, x10, puis retour à x1) |
| Mettre en pause ou reprendre | **Espace**, ou bouton **Pause** / **Reprendre** sur la carte et les courbes |
| Avancer d'une génération | Flèche **droite** ou **haut**, ou bouton **Génération +1**, pendant la pause |
| Voir le détail d'une case | Clic sur la case ; second clic ou bouton **X** pour fermer |
| Zoomer sur la carte 2D | **Molette**, vers l'endroit sous la souris |
| Déplacer la carte 2D zoomée | Maintenir le **clic droit** et glisser |
| Revoir toute la carte | Bouton **Recentrer** |
| Afficher l'évolution des populations | Bouton **Courbes**, en haut à droite de la carte |
| Revenir à la carte | Bouton **Retour**, sur l'écran des courbes |
| Redémarrer avec une nouvelle carte et de nouveaux animaux | Bouton **Recommencer**, sur la carte et les courbes |
| Revenir à l'accueil, quitter ou annuler | `Échap` pendant la simulation, puis choisir **Menu**, **Quitter** ou **Annuler** |
| Quitter depuis l'accueil | `Échap` |

L'application s'ouvre sur l'accueil ; aucune simulation ne démarre avant le clic sur **Démarrer**. La simulation continue pendant la consultation du graphique tant qu'elle n'est pas en pause. Pendant la pause, **Génération +1** fait avancer le monde et les courbes d'une seule génération. Sur la carte, chaque animal suit son trajet réel pendant la durée d'une génération, tourné dans le sens de sa marche. La pause fige le mouvement, et un pas manuel est animé en un quart de seconde. La case choisie est encadrée. Son panneau indique le terrain, la végétation et les animaux présents (espèce, numéro, âge et énergie, sept au maximum, les autres étant comptés). Il se met à jour à chaque génération et se place du côté opposé à la case. La sélection est conservée en passant par les courbes et effacée par **Recommencer** ou le retour au menu. Un animal est compté sur la case où il se trouvera à la fin du trajet affiché. L'écran **Courbes** présente chaque statistique dans son propre graphique, avec sa propre échelle : effectifs, naissances, morts de faim, morts de vieillesse, cerfs et lapins mangés, énergie moyenne et végétation des prairies. Les cases du haut affichent ou masquent chaque graphique ; les cases **Cerfs**, **Lapins**, **Loups** et **Renards** filtrent les courbes par espèce, et le bouton **Événements** passe du total depuis le début aux valeurs de chaque génération. La légende de chaque graphique donne la valeur actuelle ; en survolant un graphique, elle donne les valeurs de la génération pointée sur tous les graphiques. Les choix sont conservés après **Recommencer** et le retour au menu. La courbe jaune représente les cerfs, la blanche les lapins, la rouge les loups, la violette les renards et la verte la végétation. Sur un graphique étroit, la légende n'affiche que les valeurs, repérées par la couleur. La courbe d'énergie s'arrête quand une espèce n'a plus d'animaux. Un animal à la fois épuisé et trop âgé est compté comme mort de faim.

La touche `Échap` fige la simulation et affiche un choix : **Menu** revient à l'accueil, **Quitter** ferme l'application, **Annuler** reprend l'affichage. Un second appui sur `Échap` annule aussi le choix.

Quand il ne reste plus aucun animal, la simulation s'arrête à la dernière génération. La carte et les courbes restent accessibles pour consulter le résultat final.

Le bouton **Recommencer** repart à la génération 0, réinitialise la vitesse à x1 et affiche la nouvelle carte.

## Taille de la carte

La largeur (X) et la hauteur (Y) se saisissent à l'accueil, dans la colonne **Terrain**. Le minimum est de **10 × 10**. Aucun plafond de taille n'est imposé : la saisie accepte jusqu'à 999 999 par côté, bien au-delà de ce qu'un ordinateur peut calculer. La taille proposée au départ est celle qui remplit l'écran avec des cases de 40 pixels.

- **Populations :** quand la taille change, les nombres d'animaux au départ sont recalculés pour garder la même densité (un cerf et un lapin pour 15 cases, un loup pour 150, un renard pour 135, avec un minimum). Les autres réglages sont conservés. Un préréglage encore intact reste actif, recalculé pour la nouvelle taille.
- **Grande carte :** à partir de 20 000 cases, l'accueil prévient que la simulation et la 3D risquent d'être lentes.
- **Mémoire insuffisante :** si la réservation de mémoire échoue, l'application reste sur l'accueil avec un message. Une taille très grande mais réservable peut aussi rendre l'ordinateur très lent avant tout message.
- **Lenteur :** la simulation est calculée dans son propre thread : la fenêtre continue de répondre pendant une génération longue. Quand le calcul ne suit pas la vitesse demandée, les générations en retard sont abandonnées et la simulation ralentit. Le dessin, lui, reste fait par le thread principal : une grande carte en 3D peut toujours rendre la fenêtre saccadée. Revenir au menu, recommencer ou quitter attend la fin de la génération en cours.
- **Mémoire :** l'écran reçoit des copies de la simulation. Une grande carte occupe donc plus de mémoire que la simulation seule.

### Réactivité

Mesures en version `Release`, **sans le dessin**, à partir du démarrage et pendant 6 secondes :

| Carte et vitesse demandée | Générations calculées par seconde | Plus longue image | Retour au menu |
| --- | --- | --- | --- |
| 48 × 27, x10 | 9,8 | 0,1 ms | 5 ms |
| 100 × 100, x10 | 9,8 | 0,2 ms | 39 ms |
| 200 × 200, x10 | 6,2 | 0,03 ms | 170 ms |
| 500 × 500, x1 | 0,8 | 0,03 ms | 210 ms |

La première génération arrive après le temps d'une génération à la vitesse choisie, d'où un peu moins que la vitesse demandée sur une durée courte.

### Vitesse mesurée

Mesures sur un ordinateur de développement, en version **optimisée** (`Release`). En version `Debug`, utilisée par défaut dans CLion, tout est nettement plus lent. Les populations sont celles proposées au départ.

| Carte | Animaux (pic sur 40 générations) | Une génération (moyenne) | Image 2D entière | Image 3D |
| --- | --- | --- | --- | --- |
| 48 × 27 | 1 000 | 3 ms | 6 ms | 14 ms |
| 100 × 100 | 8 000 | 27 ms | 23 ms | 79 ms |
| 200 × 200 | 32 000 | 130 ms | 91 ms | 290 ms |
| 500 × 500 | 200 000 | 1,4 s | 100 ms | 1,7 s |
| 1000 × 1000 | non mesuré | non mesuré | 280 ms | non mesurée |

Les images sont mesurées avec la population de départ, avant que les animaux se multiplient : elles sont ensuite plus lentes.

## Génération de la carte

La carte est construite par **bruit de valeur** : des valeurs sont tirées au sort sur une grille espacée, puis lissées entre elles, avec de grandes formes (tous les 8 cases) et des détails (tous les 4). Les cases voisines se ressemblent ainsi.

| Relief | Ce qu'il décide |
| --- | --- |
| **Altitude** | Les cases les plus basses deviennent de l'**eau**, les plus hautes des **obstacles**, selon les parts réglées à l'accueil. |
| **Humidité** | La **végétation de départ** : des régions riches et des régions pauvres. Toutes les quantités de 0 à 100 sont réparties à parts égales. |

**Aucune prairie n'est isolée.** Seule la plus grande zone de prairies reliées par un côté est gardée ; les autres poches deviennent de l'eau (si elles sont basses) ou des obstacles. Si la plus grande zone réunit moins de 95 % des prairies, d'autres reliefs sont essayés, 20 au maximum, et le moins morcelé est gardé. Les parts finales peuvent donc dépasser un peu les réglages : sur 1000 cartes de 42 × 26, la correction a retiré au plus 4 % de la carte pour les réglages (10 %, 10 %), (25 %, 20 %) et (30 %, 0 %), et au plus 8 % pour (50 %, 40 %), où il reste alors très peu de prairie. Un terrain réglé à 0 % n'est jamais ajouté.

La même graine redonne la même carte.

## Vue 2D

Au départ, la carte tient entière à l'écran, centrée, avec des cases carrées aussi grandes que possible. Des bandes sombres apparaissent si ses proportions diffèrent de celles de l'écran. La molette zoome jusqu'à des cases de 80 pixels, ou davantage si la vue d'ensemble les montre déjà plus grandes. Quand les cases mesurent moins de 4 pixels, le sol est dessiné pixel par pixel, sans quadrillage, et les animaux les plus petits deviennent des points. Le zoom est conservé avec **Recommencer** tant que la taille ne change pas.

## Déplacements

La grille sert au décor (terrain et végétation) ; les animaux, eux, ont une position libre. La case d'un animal est celle située sous lui.

Une génération est découpée en **10 petits pas**. À chacun, un animal choisit une direction et une allure, puis :

- il tourne **progressivement** vers cette direction (0,65 radian au plus par petit pas) ;
- il avance selon sa **vitesse** : 100 correspond à une case par génération à pleine allure ;
- devant l'eau, un obstacle ou le bord, il essaie des directions de plus en plus écartées, et ne passe pas entre deux obstacles qui se touchent par un coin.

| Espèce | Comportement |
| --- | --- |
| **Cerfs et lapins** | Sur une herbe suffisante (30 ou plus pour un cerf, 5 ou plus pour un lapin), ils flânent à petite allure. Sinon, ils cherchent l'herbe la plus riche jusqu'à 2 cases, en préférant ce qui est proche et devant eux, avec un peu d'hésitation. Ils mangent une fois par génération, là où ils se trouvent à la fin. Un cerf ne peut pas brouter les 5 derniers points d'herbe d'une case : ils restent aux lapins. |
| **Loups et renards** | Les loups voient à 3 cases, les renards à 1,5. Ils foncent sur la proie la plus proche parmi celles qu'ils mangent : cerfs et lapins pour un loup, lapins seulement pour un renard. Sans proie, ils errent. Une proie à moins de 0,45 case est capturée, au plus une fois par génération ; ils flânent ensuite. |

Le vieillissement, la perte d'énergie, les naissances et la repousse ont toujours lieu une fois par génération. Le trajet de la dernière génération est enregistré : l'affichage le rejoue sans recalculer la simulation. La **vitesse ne coûte pas d'énergie** pour l'instant.

### Les quatre espèces

| Caractéristique fixe | Cerf | Lapin | Loup | Renard |
| --- | --- | --- | --- | --- |
| Vitesse (% de case par génération) | 120 | 100 | 80 | 110 |
| Énergie perdue par génération | 2 | 1 | 2 | 2 |
| Âge maximum | 80 | 60 | 80 | 100 |
| Herbe mangée au maximum, ou énergie par proie | 12 | 5 | 25 | 40 |
| Énergie pour se reproduire | 80 | 50 | 80 | 70 |
| Reste brouter sur une herbe d'au moins | 30 | 5 | | |
| Herbe qu'il ne peut pas brouter | 5 | 0 | | |
| Repas offert au chasseur | 100 % | 50 % | | |
| Distance de vue avant l'instinct (cases) | | | 3 | 1,5 |
| Proies | | | cerfs et lapins | lapins |

| Réglage proposé au départ | Cerf | Lapin | Loup | Renard |
| --- | --- | --- | --- | --- |
| Chance de naissance | 25 % | 70 % | 6 % | 6 % |
| Instinct | fuite 25 % | fuite 25 % | meute 2 | flair 1 |

Chaque caractéristique fixe est écrite à un seul endroit, dans la classe de l'espèce (`Deer::Traits`, `Rabbit::Traits`, `Wolf::Traits`, `Fox::Traits`, le seuil d'herbe `GrazingThreshold`, la réserve `Deer::UngrazableVegetation`, la vue `Sight` et le régime `Eats`).

## Fuite et meutes

Chaque espèce a un **instinct**, réglable à l'accueil (dernière ligne de sa colonne) ; **0** le désactive.

| Réglage | Comportement | Valeur de départ |
| --- | --- | --- |
| **Chance de fuir (%)** (cerfs et lapins) | À chaque génération, l'herbivore a cette chance d'être sur ses gardes. Il s'éloigne alors à pleine allure des chasseurs qui le mangent, situés à 2 cases ou moins, surtout du plus proche. | 25 |
| **Esprit de meute (vue +)** (loups) | La valeur s'ajoute à la distance de vue. Près d'un congénère, chacun vise un côté de la proie pour lui couper la route ; sans proie, ils se regroupent. Le repas est partagé à parts égales avec les loups à 1,5 case au plus, sans créer d'énergie. | 2 |
| **Flair (vue +)** (renards) | La valeur s'ajoute à la distance de vue. | 1 |

Les obstacles ne bloquent pas la vue.

## Vue 3D

Sur la carte, le bouton **Vue 3D** change l'affichage sans toucher à la simulation ; **Vue 2D** revient à la carte à plat. Les commandes de temps, les courbes, la confirmation Échap et le panneau d'une case fonctionnent dans les deux vues.

| Action en 3D | Commande |
| --- | --- |
| Tourner autour de la carte et changer l'inclinaison | Maintenir le **clic droit** et glisser |
| Se rapprocher ou s'éloigner | **Molette** |
| Revenir à la vue d'ensemble | Bouton **Recentrer** |
| Voir le détail d'une case | **Clic gauche** sur le sol de la case |

Les obstacles sont des blocs, les cerfs des modèles jaunes, les lapins de petits modèles blancs les loups des modèles rouges plus hauts et les renards de petits modèles violets, orientés dans le sens du déplacement. La couleur des prairies suit la végétation. Des touffes d'herbe y poussent : une case rase (végétation inférieure à 5) n'en a pas, puis leur nombre (jusqu'à cinq) et leur hauteur augmentent avec la végétation. Elles raccourcissent quand un cerf ou un lapin broute et repoussent au fil des générations. Avec la repousse par défaut de 1 par génération, la croissance est progressive ; le réglage **Repousse végétation** l'accélère. La vue choisie est conservée après **Recommencer** et le retour au menu ; la position de la caméra aussi, tant que la taille de la carte ne change pas.

Limites de cette première version : le rendu est calculé par le processeur et dessiné avec Win32/GDI, sans carte graphique, textures ni éclairage dynamique. Les animaux proches les uns des autres peuvent se chevaucher à l'écran ; le panneau d'une case donne les effectifs réels. La caméra vise toujours le centre de la carte. La sélection vise le sol : cliquer sur le haut d'un obstacle peut choisir la case située derrière.

## Règles de la simulation

À chaque génération :

1. La végétation repousse sur les prairies.
2. Pendant 10 petits pas, les cerfs et les lapins, puis les loups et les renards, se déplacent ; les chasseurs capturent les proies qu'ils atteignent (au plus une chacun).
3. Les cerfs et les lapins encore vivants mangent la végétation de la case où ils se trouvent.
4. Les survivants vieillissent et perdent de l'énergie. Les individus suffisamment âgés et nourris peuvent se reproduire.
5. Les animaux morts sont retirés ; les effectifs, la végétation totale et les événements de la génération sont enregistrés par `SimulationStats`.

Ce modèle s'écarte du sujet, qui prévoit le choix d'une cellule voisine à chaque génération.

La carte et les populations initiales sont générées au démarrage d'une simulation et à chaque redémarrage. Des graines fixes peuvent être utilisées dans les tests pour reproduire un scénario.

## Organisation du code

Les fichiers `.ixx` exposent les modules C++ ; leurs implémentations sont dans les fichiers `.cpp`.

| Emplacement | Responsabilité |
| --- | --- |
| `src/world/` | Cellules, terrains, positions et génération du monde. |
| `src/animals/` | Ce qui est commun : classe de base `Animal`, interface `Habitat` et catalogue des espèces. |
| `src/animals/herbivores/` | La famille `Herbivore` et ses espèces, `Deer` et `Rabbit`. |
| `src/animals/predators/` | La famille `Predator` et ses espèces, `Wolf` et `Fox`. |
| `src/simulation/simulation/` | Classe `Simulation` : cycle des générations, déplacements et repousse. |
| `src/simulation/habitat/` | Classe `GridHabitat` : voisinage des animaux, captures et partage des repas. |
| `src/simulation/stats/` | Classe `SimulationStats` : historique et événements. |
| `src/simulation/settings/` | Réglages, valeurs par défaut et préréglages. |
| `src/simulation/clock/` | Classe `SimulationClock` : vitesses et pause. |
| `src/ui/application/` | Déroulement de l'application, interfaces `InputSource` et `View`, et classe `SimulationRunner` : le thread de calcul. |
| `src/ui/navigation/` | Classe `UiController` : clavier, souris et changement d'écran. |
| `src/ui/map/` | Caméra et scène 3D, lecture des trajets et texte du panneau d'une case. |
| `src/ui/presentation/` | Formulaire des réglages, apparence des espèces, graphiques et placement. |
| `src/render/platform/` | Fenêtre Windows, clavier et souris. |
| `src/render/renderer/` | Classe `Win32Renderer` : choisit ce qu'il faut dessiner. |
| `src/render/painters/` | Une fonction de dessin par écran ou élément. |
| `src/render/gdi/` | Classe `Canvas` : image en mémoire, fonds, textes, boutons et polices. |
| `src/app/main.cpp` | Assemblage des objets et boucle principale. |
| `tests/` | Tests du monde, des animaux, de la simulation et de la logique d'interface. |

### Trois couches

| Cible CMake | Contenu | Dépend de |
| --- | --- | --- |
| `ecosim_core` | Moteur : monde, animaux, simulation, horloge | bibliothèque standard |
| `ecosim_ui` | Logique d'interface en C++ pur | `ecosim_core` |
| `EcoSim` | Fenêtre, entrées et dessin Win32/GDI | `ecosim_ui`, `user32`, `gdi32` |

Les tests ne compilent aucun code Windows. Le moteur peut être construit seul avec `cmake --build cmake-build-debug --target ecosim_core`. Un autre rendu doit implémenter `View` et `InputSource`.

Le découpage privilégie la **simplicité** : `UiController` regroupe la navigation de tous les écrans, et la 2D comme la 3D sont choisies par une simple condition. Ajouter une vue demande de modifier le contrôleur et le rendu.

### Ajouter une espèce

1. Créer une classe dérivée de `Herbivore` si elle broute, de `Predator` si elle chasse (en précisant son régime avec `Eats`), ou d'`Animal` sinon, avec ses caractéristiques fixes (`SpeciesTraits`) et sa chance de naissance de départ.
2. Ajouter sa valeur à `AnimalKind` et augmenter `SpeciesCount` (`animal.ixx`).
3. Ajouter son entrée au catalogue (`src/animals/species.cpp`) : noms, population de départ, réglages proposés et classe.
4. Ajouter son apparence (`src/ui/presentation/species_style.cpp`) : couleur, silhouette, nom de l'instinct et limites de saisie.

Les réglages, les statistiques, les graphiques, le panneau d'une case et les cartes parcourent le catalogue. Une colonne s'ajoute à l'accueil, dont la lisibilité est alors à vérifier. Les préréglages nomment les espèces qu'ils modifient. Les proies (`IsPrey`) jouent en premier ; chacune n'est chassée que par les espèces dont `Eats` la désigne.

## Documentation

La documentation est générée avec **Doxygen** (Python est aussi nécessaire, pour le filtre des modules). Depuis la racine du projet :

```powershell
doxygen Doxyfile
```

Ouvrez ensuite `docs/doxygen/html/index.html`. Les pages d'explication se trouvent dans `docs/` : vue d'ensemble, fonctionnement, architecture, diagrammes détaillés et notions de C++. Les diagrammes sont régénérés avec `python docs/generate_diagrams.py`. Les commentaires des fichiers `.ixx` décrivent chaque classe et fonction visible ; les éléments oubliés sont signalés dans `docs/doxygen-warnings.log`.

Les membres privés et les aides internes sont eux aussi commentés, sans apparaître dans le site. Un second contrôle les vérifie ; son journal, `docs/doxygen-verification.log`, doit rester vide :

```powershell
doxygen Doxyfile.verification
```

## Conventions de code

Pour garder le code facile à lire dans le cadre du TP, on évite `std::span` et `std::string_view`. On utilise `std::string` pour les textes et des références à `std::vector` pour accéder aux collections sans les copier : `const std::vector<T>&` en lecture, `std::vector<T>&` si une modification est nécessaire.

Les textes sont en UTF-8. Seule exception : les fonctions Windows attendent des caractères larges. La conversion en `std::wstring` est faite uniquement dans `src/render/gdi/canvas.cpp` ; les noms de police et de classe de fenêtre, passés directement à Windows, restent des littéraux `L"…"`.

## Lancer les tests

Après configuration avec `-DECOSIM_BUILD_TESTS=ON` :

```powershell
cmake --build cmake-build-debug --parallel
ctest --test-dir cmake-build-debug --output-on-failure
```

Sous Windows, CTest exécute toute la suite Catch2 dans un test nommé `EcoSimSuite`. Pour voir le nombre de cas et d'assertions exécutés :

```powershell
.\cmake-build-debug\tests.exe --reporter compact
```
