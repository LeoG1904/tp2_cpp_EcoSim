# EcoSim

EcoSim est une simulation d'écosystème cellulaire en **C++23**. Une grille de prairies, d'eau et d'obstacles accueille de la végétation, des herbivores et des prédateurs. Plusieurs animaux peuvent occuper la même case. La simulation évolue par générations et son moteur fonctionne indépendamment de l'affichage.

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
| **Équilibre** | Herbivores : âge maximum 120, seuil de reproduction 70, chance 40 %. Prédateurs : perte 3, âge maximum 60, énergie par proie 15, seuil 70, chance 5 % | Les deux espèces sont encore présentes à 1000 générations sur 20 graines sur 20 (18 sur 20 en 32 × 18) |
| **Famine** | Aucune repousse, perte des herbivores 3, trois fois moins de prédateurs au départ | Herbivores éteints vers la génération 70, avec bien plus de morts de faim que de captures |
| **Invasion** | Beaucoup plus de prédateurs au départ (une case sur sept, 200 au maximum) | Herbivores mangés vers la génération 30 |

Ces résultats sont des tendances : la carte et les animaux restent tirés au hasard.

| Colonne | Réglages |
| --- | --- |
| Terrain | Pourcentage d'eau (0 à 50) et d'obstacles (0 à 40), repousse de la végétation par génération (0 à 20) |
| Herbivores et prédateurs | Nombre au départ, énergie de départ, énergie perdue par génération, âge maximum, seuil d'énergie et chance de reproduction |
| Herbivores | Végétation mangée au maximum par génération |
| Prédateurs | Énergie gagnée par proie |

Les nouveau-nés gardent une énergie de 30, la reproduction coûte 30 et demande un âge d'au moins 5 générations. Le nombre d'animaux au départ dépend par défaut de la taille de l'écran. Les bornes sont définies dans `src/ui/presentation/settings_form.cpp` et, pour ce qui dépend de l'espèce, dans `src/ui/presentation/species_style.cpp`.

## Utilisation

| Action | Commande |
| --- | --- |
| Lancer une simulation depuis l'accueil | Bouton **Démarrer** |
| Vitesse x1, x2, x5 ou x10 | Touches `1`, `2`, `3` ou `4` |
| Passer à la vitesse suivante | **Tab** (x1, x2, x5, x10, puis retour à x1) |
| Mettre en pause ou reprendre | **Espace**, ou bouton **Pause** / **Reprendre** sur la carte et les courbes |
| Avancer d'une génération | Flèche **droite** ou **haut**, ou bouton **Génération +1**, pendant la pause |
| Voir le détail d'une case | Clic sur la case ; second clic ou bouton **X** pour fermer |
| Afficher l'évolution des populations | Bouton **Courbes**, en haut à droite de la carte |
| Revenir à la carte | Bouton **Retour**, sur l'écran des courbes |
| Redémarrer avec une nouvelle carte et de nouveaux animaux | Bouton **Recommencer**, sur la carte et les courbes |
| Revenir à l'accueil, quitter ou annuler | `Échap` pendant la simulation, puis choisir **Menu**, **Quitter** ou **Annuler** |
| Quitter depuis l'accueil | `Échap` |

L'application s'ouvre sur l'accueil ; aucune simulation ne démarre avant le clic sur **Démarrer**. La simulation continue pendant la consultation du graphique tant qu'elle n'est pas en pause. Pendant la pause, **Génération +1** fait avancer le monde et les courbes d'une seule génération. Sur la carte, les animaux glissent de leur ancienne case vers la nouvelle pendant la durée d'une génération. Seul l'affichage est animé : le moteur reste case par case. La pause fige le mouvement, et un pas manuel est animé en un quart de seconde. La case choisie est encadrée. Son panneau indique le terrain, la végétation et les animaux présents (espèce, numéro, âge et énergie, sept au maximum, les autres étant comptés). Il se met à jour à chaque génération et se place du côté opposé à la case. La sélection est conservée en passant par les courbes et effacée par **Recommencer** ou le retour au menu. Pendant un déplacement, un animal est déjà compté sur sa case d'arrivée. L'écran **Courbes** présente chaque statistique dans son propre graphique, avec sa propre échelle : effectifs, naissances, morts de faim, morts de vieillesse, herbivores mangés, énergie moyenne et végétation des prairies. Les cases du haut affichent ou masquent chaque graphique ; les cases **Herbivores** et **Prédateurs** filtrent les courbes par espèce, et le bouton **Événements** passe du total depuis le début aux valeurs de chaque génération. La légende de chaque graphique donne la valeur actuelle ; en survolant un graphique, elle donne les valeurs de la génération pointée sur tous les graphiques. Les choix sont conservés après **Recommencer** et le retour au menu. La courbe jaune représente les herbivores, la rouge les prédateurs et la verte la végétation. La courbe d'énergie s'arrête quand une espèce n'a plus d'animaux. Un animal à la fois épuisé et trop âgé est compté comme mort de faim.

La touche `Échap` fige la simulation et affiche un choix : **Menu** revient à l'accueil, **Quitter** ferme l'application, **Annuler** reprend l'affichage. Un second appui sur `Échap` annule aussi le choix.

Quand il ne reste plus aucun animal, la simulation s'arrête à la dernière génération. La carte et les courbes restent accessibles pour consulter le résultat final.

Le bouton **Recommencer** repart à la génération 0, réinitialise la vitesse à x1 et affiche la nouvelle carte.

## Vue 3D

Sur la carte, le bouton **Vue 3D** change l'affichage sans toucher à la simulation ; **Vue 2D** revient à la carte à plat. Les commandes de temps, les courbes, la confirmation Échap et le panneau d'une case fonctionnent dans les deux vues.

| Action en 3D | Commande |
| --- | --- |
| Tourner autour de la carte et changer l'inclinaison | Maintenir le **clic droit** et glisser |
| Se rapprocher ou s'éloigner | **Molette** |
| Revenir à la vue d'ensemble | Bouton **Recentrer** |
| Voir le détail d'une case | **Clic gauche** sur le sol de la case |

Les obstacles sont des blocs, les herbivores de petits modèles jaunes et les prédateurs des modèles rouges plus hauts, la tête tournée dans le sens du déplacement. La couleur des prairies suit la végétation. La vue choisie et la position de la caméra sont conservées après **Recommencer** et le retour au menu.

Limites de cette première version : le rendu est calculé par le processeur et dessiné avec Win32/GDI, sans carte graphique, textures ni éclairage dynamique. Au plus quatre modèles par espèce sont dessinés sur une même case ; le panneau donne les effectifs réels. La caméra vise toujours le centre de la carte. La sélection vise le sol : cliquer sur le haut d'un obstacle peut choisir la case située derrière.

## Règles de la simulation

À chaque génération :

1. La végétation repousse sur les prairies.
2. Les herbivores rejoignent une case accessible riche en végétation et s'y nourrissent.
3. Les prédateurs cherchent des proies sur leur case ou sur une case voisine accessible ; chacun peut capturer au plus un herbivore.
4. Les survivants vieillissent et perdent de l'énergie. Les individus suffisamment âgés et nourris peuvent se reproduire.
5. Les animaux morts sont retirés ; les effectifs, la végétation totale et les événements de la génération sont enregistrés par `SimulationStats`.

La carte et les populations initiales sont générées au démarrage d'une simulation et à chaque redémarrage. Des graines fixes peuvent être utilisées dans les tests pour reproduire un scénario.

## Organisation du code

Les fichiers `.ixx` exposent les modules C++ ; leurs implémentations sont dans les fichiers `.cpp`. Le moteur ne dépend d'aucune bibliothèque graphique : le rendu lit son état et son historique.

| Emplacement | Responsabilité |
| --- | --- |
| `src/world/` | Cellules, terrains, accès à la grille et génération du monde. |
| `src/animals/` | Classe de base `Animal`, interface `Habitat`, comportements de `Herbivore` et de `Predator`, catalogue des espèces. |
| `src/simulation/simulation/` | Module `ecosim.simulation` : classe `Simulation`, cycle des générations, végétation et historique des populations. |
| `src/simulation/stats/` | Module `ecosim.stats` : classe `SimulationStats`, historique et événements. |
| `src/simulation/settings/` | Module `ecosim.settings` : réglages d'une simulation et valeurs par défaut. |
| `src/simulation/clock/` | Module `ecosim.clock` : classe `SimulationClock`, vitesses et pause. |
| `src/core/` | Module `ecosim` : entrée publique du moteur. |
| `src/ui/application/` | Déroulement de l'application et interfaces `InputSource` et `View`. |
| `src/ui/navigation/` | Entrées, aiguillage et un rôle par module : accueil, confirmation, statistiques, carte et sélection. |
| `src/ui/map/` | Vues de la carte (`MapView`), caméra et scène 3D, animations et texte du panneau d'une case. |
| `src/ui/presentation/` | Contenu et placement : formulaire des réglages, apparence des espèces, graphiques et disposition. |
| `src/render/platform/window.*` | Fenêtre Windows et messages. |
| `src/render/platform/win32_input.*` | Lecture du clavier et de la souris. |
| `src/render/renderer/` | `Win32Renderer` : assemble les dessinateurs et choisit l'écran. |
| `src/render/painters/` | Un dessinateur par écran ou élément ; les cartes 2D et 3D suivent l'interface `MapPainter`. |
| `src/render/gdi/` | Image en mémoire, texte et aides GDI. |
| `src/app/main.cpp` | Assemblage des objets et boucle principale. |
| `tests/` | Tests du monde, des animaux, de la simulation, de l'horloge et de la logique d'interface. |

### Trois couches

| Cible CMake | Contenu | Dépend de |
| --- | --- | --- |
| `ecosim_core` | Moteur : monde, animaux, simulation, horloge | bibliothèque standard |
| `ecosim_ui` | Logique d'interface en C++ pur | `ecosim_core` |
| `EcoSim` | Fenêtre, entrées et dessin Win32/GDI | `ecosim_ui`, `user32`, `gdi32` |

Les tests ne compilent aucun code Windows. Le moteur peut être construit seul avec `cmake --build cmake-build-debug --target ecosim_core`. Un autre rendu doit implémenter `View` et `InputSource`, sans modifier `Application` ni le moteur.

### Ajouter une espèce

1. Créer une classe dérivée d'`Animal` (phase du tour, destination, nourriture, descendance).
2. Ajouter sa valeur à `AnimalKind` et augmenter `SpeciesCount` (`animal.ixx`).
3. Ajouter son entrée au catalogue (`src/animals/species.cpp`) : noms, statistiques d'origine, population de départ et classe.
4. Ajouter son apparence (`src/ui/presentation/species_style.cpp`) : couleur, silhouette et limites de saisie.

Les réglages, les statistiques, les graphiques, le panneau d'une case et les cartes parcourent le catalogue et n'ont pas à être modifiés. Une colonne s'ajoute à l'accueil, qui se resserre : sa lisibilité est à vérifier à l'écran avec trois espèces ou plus. Les préréglages ne règlent que les herbivores et les prédateurs. Une nouvelle phase de tour doit être ajoutée à `TurnPhases`. Seules les espèces marquées comme proies (`IsPrey`) sont chassées, par tout chasseur : un régime alimentaire plus fin demanderait d'étendre `Habitat`.

### Ajouter une vue de la carte

Créer une classe `MapView` (navigation) et une classe `MapPainter` (dessin), ajouter une valeur à `MapViewKind` et augmenter `MapViewCount`, puis les enregistrer dans `MapNavigator` et `Win32Renderer`. Les vues existantes ne changent pas.

## Conventions de code

Pour garder le code facile à lire dans le cadre du TP, on évite `std::span` et `std::string_view`. On utilise `std::string` pour les textes et des références à `std::vector` pour accéder aux collections sans les copier : `const std::vector<T>&` en lecture, `std::vector<T>&` si une modification est nécessaire.

Les textes sont en UTF-8. Seule exception : les fonctions Windows attendent des caractères larges. La conversion en `std::wstring` est faite uniquement dans `src/render/gdi/text.cpp` ; les noms de police et de classe de fenêtre, passés directement à Windows, restent des littéraux `L"…"`.

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
