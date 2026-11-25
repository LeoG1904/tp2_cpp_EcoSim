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

L'application fonctionne sous Windows et utilise Win32/GDI pour l'affichage. La compilation et les tests ont été vérifiés avec ce rendu.

## Utilisation

| Action | Commande |
| --- | --- |
| Lancer une simulation depuis l'accueil | Bouton **Démarrer** |
| Vitesse x1, x2, x5 ou x10 | Touches `1`, `2`, `3` ou `4` |
| Passer à la vitesse suivante | **Tab** (x1, x2, x5, x10, puis retour à x1) |
| Mettre en pause ou reprendre | **Espace**, ou bouton **Pause** / **Reprendre** sur la carte et les courbes |
| Avancer d'une génération | Flèche **droite** ou **haut**, ou bouton **Génération +1**, pendant la pause |
| Afficher l'évolution des populations | Bouton **Courbes**, en haut à droite de la carte |
| Revenir à la carte | Bouton **Retour**, sur l'écran des courbes |
| Redémarrer avec une nouvelle carte et de nouveaux animaux | Bouton **Recommencer**, sur la carte et les courbes |
| Revenir à l'accueil, quitter ou annuler | `Échap` pendant la simulation, puis choisir **Menu**, **Quitter** ou **Annuler** |
| Quitter depuis l'accueil | `Échap` |

L'application s'ouvre sur l'accueil ; aucune simulation ne démarre avant le clic sur **Démarrer**. La simulation continue pendant la consultation du graphique tant qu'elle n'est pas en pause. Pendant la pause, **Génération +1** fait avancer le monde et les courbes d'une seule génération. La courbe jaune représente les herbivores et la courbe rouge les prédateurs. L'axe horizontal indique les générations ; une ligne marque la génération actuelle.

La touche `Échap` fige la simulation et affiche un choix : **Menu** revient à l'accueil, **Quitter** ferme l'application, **Annuler** reprend l'affichage. Un second appui sur `Échap` annule aussi le choix.

Quand il ne reste plus aucun animal, la simulation s'arrête à la dernière génération. La carte et les courbes restent accessibles pour consulter le résultat final.

Le bouton **Recommencer** repart à la génération 0, réinitialise la vitesse à x1 et affiche la nouvelle carte.

## Règles de la simulation

À chaque génération :

1. La végétation repousse sur les prairies.
2. Les herbivores rejoignent une case accessible riche en végétation et s'y nourrissent.
3. Les prédateurs cherchent des proies sur leur case ou sur une case voisine accessible ; chacun peut capturer au plus un herbivore.
4. Les survivants vieillissent et perdent de l'énergie. Les individus suffisamment âgés et nourris peuvent se reproduire.
5. Les animaux morts sont retirés ; les effectifs des deux populations sont enregistrés pour le graphique.

La carte et les populations initiales sont générées au démarrage d'une simulation et à chaque redémarrage. Des graines fixes peuvent être utilisées dans les tests pour reproduire un scénario.

## Organisation du code

Les fichiers `.ixx` exposent les modules C++ ; leurs implémentations sont dans les fichiers `.cpp`. Le moteur ne dépend d'aucune bibliothèque graphique : le rendu lit son état et son historique.

| Emplacement | Responsabilité |
| --- | --- |
| `src/world/` | Cellules, terrains, accès à la grille et génération du monde. |
| `src/animals/` | Classe de base `Animal`, comportements de `Herbivore` et de `Predator`. |
| `src/simulation/clock/` | Module `ecosim.clock` : classe `SimulationClock`, vitesses et pause. |
| `src/simulation/simulation/` | Module `ecosim.simulation` : classe `Simulation`, cycle des générations, végétation et historique des populations. |
| `src/render/render.ixx` et `render.cpp` | Interface graphique, fenêtre, entrées et choix de l'écran. |
| `src/render/render_home.cpp` | Affichage de l'écran d'accueil. |
| `src/render/render_exit_dialog.cpp` | Confirmation pour revenir au menu ou quitter. |
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
