# EcoSim

EcoSim est une simulation d'écosystème cellulaire en **C++23**. Une grille de prairies, d'eau et d'obstacles accueille de la végétation, des herbivores et des prédateurs. Plusieurs animaux peuvent occuper la même case. La simulation évolue par générations et son moteur fonctionne indépendamment de l'affichage.

Le projet est un prototype **2D** : il affiche la carte en direct et permet de consulter l'évolution des populations sur un graphique.

## Démarrage rapide sous Windows

Prérequis : **CMake 3.29 ou plus**, **Ninja** et un compilateur capable de construire les modules C++23 et `import std`. La configuration utilisée pour ce projet est **Clang 22 avec libc++ (llvm-mingw)**. Lancez les commandes suivantes depuis la racine du dépôt :

```powershell
cmake -S . -B cmake-build-debug -G Ninja -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug -DECOSIM_BUILD_TESTS=ON
cmake --build cmake-build-debug --parallel
.\cmake-build-debug\EcoSim.exe
```

Le premier lancement de CMake avec les tests activés récupère **Catch2** depuis GitHub et nécessite donc une connexion Internet. Pour compiler sans les tests, remplacez `-DECOSIM_BUILD_TESTS=ON` par `-DECOSIM_BUILD_TESTS=OFF`.

Si `cmake-build-debug` est déjà configuré, la commande `cmake --build cmake-build-debug --parallel` suffit pour reconstruire l'application dans **le même répertoire**.

Sur Windows, le rendu utilise Win32/GDI par défaut. Sur les autres plateformes, CMake active Raylib 5.5 et le télécharge lors de la configuration. L'option `-DECOSIM_USE_RAYLIB=ON` permet aussi de choisir Raylib sur Windows. La compilation et les tests ont été vérifiés avec le rendu Win32/GDI.

## Utilisation

| Action | Commande |
| --- | --- |
| Vitesse x1, x2, x5 ou x10 | Touches `1`, `2`, `3` ou `4` |
| Afficher l'évolution des populations | Bouton **Courbes**, en haut à droite de la carte |
| Revenir à la carte | Bouton **Retour**, sur l'écran des courbes |
| Quitter | `Échap` |

La simulation continue pendant la consultation du graphique. La courbe jaune représente les herbivores et la courbe rouge les prédateurs. L'axe horizontal indique les générations ; une ligne marque la génération actuelle.

Quand il ne reste plus aucun animal, la simulation s'arrête à la dernière génération. La carte et les courbes restent accessibles pour consulter le résultat final.

## Règles de la simulation

À chaque génération :

1. La végétation repousse sur les prairies.
2. Les herbivores rejoignent une case accessible riche en végétation et s'y nourrissent.
3. Les prédateurs cherchent des proies sur leur case ou sur une case voisine accessible ; chacun peut capturer au plus un herbivore.
4. Les survivants vieillissent et perdent de l'énergie. Les individus suffisamment âgés et nourris peuvent se reproduire.
5. Les animaux morts sont retirés ; les effectifs des deux populations sont enregistrés pour le graphique.

La carte et les populations initiales sont générées à chaque lancement. Des graines fixes peuvent être utilisées dans les tests pour reproduire un scénario.

## Organisation du code

Les fichiers `.ixx` exposent les modules C++ ; leurs implémentations sont dans les fichiers `.cpp`. Le moteur ne dépend d'aucune bibliothèque graphique : le rendu lit son état et son historique.

| Emplacement | Responsabilité |
| --- | --- |
| `src/world/` | Cellules, terrains, accès à la grille et génération du monde. |
| `src/animals/` | Classe de base `Animal`, comportements de `Herbivore` et de `Predator`. |
| `src/simulation/clock/` | Module `ecosim.clock` : classe `SimulationClock` et vitesses. |
| `src/simulation/simulation/` | Module `ecosim.simulation` : classe `Simulation`, cycle des générations, végétation et historique des populations. |
| `src/render/render.ixx` et `render.cpp` | Interface graphique, fenêtre, entrées et choix de l'écran. |
| `src/render/render_world.cpp` | Affichage de la carte et des animaux. |
| `src/render/render_chart.cpp` et `layout.hpp` | Graphique des populations et placement des éléments. |
| `src/app/main.cpp` | Initialisation du monde et boucle de l'application. |
| `tests/` | Tests des règles du monde, des animaux, de l'horloge et de l'historique. |

## Conventions de code

Pour garder le code facile à lire dans le cadre du TP, on évite `std::span` et `std::string_view`. On utilise `std::string` pour les textes et des références à `std::vector` pour accéder aux collections sans les copier : `const std::vector<T>&` en lecture, `std::vector<T>&` si une modification est nécessaire.

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
