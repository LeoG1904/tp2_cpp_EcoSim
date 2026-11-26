# EcoSim

EcoSim est une simulation d'écosystème cellulaire en **C++23**. Une grille de prairies, d'eau et d'obstacles porte la végétation ; des herbivores et des prédateurs s'y déplacent **librement**, dans toutes les directions. Plusieurs animaux peuvent se trouver sur la même case. La simulation évolue par générations et son moteur fonctionne indépendamment de l'affichage.

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
| **Équilibre** | Herbivores : vitesse 120, chance de naissance 30 %, fuite 50 %. Prédateurs : vitesse 80, perte 3, âge maximum 60, énergie par proie 25, seuil 70, chance 15 %, meute 1 | Les deux espèces sont encore présentes à 1000 générations sur 30 graines sur 30 |
| **Famine** | Aucune repousse, perte des herbivores 3, un seul prédateur, de vitesse 80 | Herbivores éteints vers la génération 75, environ 85 % de leurs morts étant dues à la faim (100 graines) |
| **Invasion** | Beaucoup plus de prédateurs au départ (une case sur onze, 200 au maximum), aucune fuite, meute 3 | Tous les herbivores sont mangés en une dizaine de générations (100 graines) |

Ces résultats sont des tendances : la carte et les animaux restent tirés au hasard.

| Colonne | Réglages |
| --- | --- |
| Terrain | Pourcentage d'eau (0 à 50) et d'obstacles (0 à 40), repousse de la végétation par génération (0 à 20) |
| Herbivores et prédateurs | Nombre au départ, vitesse (20 à 300 % de case par génération), énergie de départ, énergie perdue par génération, âge maximum, seuil d'énergie et chance de reproduction |
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

L'application s'ouvre sur l'accueil ; aucune simulation ne démarre avant le clic sur **Démarrer**. La simulation continue pendant la consultation du graphique tant qu'elle n'est pas en pause. Pendant la pause, **Génération +1** fait avancer le monde et les courbes d'une seule génération. Sur la carte, chaque animal suit son trajet réel pendant la durée d'une génération, tourné dans le sens de sa marche. La pause fige le mouvement, et un pas manuel est animé en un quart de seconde. La case choisie est encadrée. Son panneau indique le terrain, la végétation et les animaux présents (espèce, numéro, âge et énergie, sept au maximum, les autres étant comptés). Il se met à jour à chaque génération et se place du côté opposé à la case. La sélection est conservée en passant par les courbes et effacée par **Recommencer** ou le retour au menu. Un animal est compté sur la case où il se trouvera à la fin du trajet affiché. L'écran **Courbes** présente chaque statistique dans son propre graphique, avec sa propre échelle : effectifs, naissances, morts de faim, morts de vieillesse, herbivores mangés, énergie moyenne et végétation des prairies. Les cases du haut affichent ou masquent chaque graphique ; les cases **Herbivores** et **Prédateurs** filtrent les courbes par espèce, et le bouton **Événements** passe du total depuis le début aux valeurs de chaque génération. La légende de chaque graphique donne la valeur actuelle ; en survolant un graphique, elle donne les valeurs de la génération pointée sur tous les graphiques. Les choix sont conservés après **Recommencer** et le retour au menu. La courbe jaune représente les herbivores, la rouge les prédateurs et la verte la végétation. La courbe d'énergie s'arrête quand une espèce n'a plus d'animaux. Un animal à la fois épuisé et trop âgé est compté comme mort de faim.

La touche `Échap` fige la simulation et affiche un choix : **Menu** revient à l'accueil, **Quitter** ferme l'application, **Annuler** reprend l'affichage. Un second appui sur `Échap` annule aussi le choix.

Quand il ne reste plus aucun animal, la simulation s'arrête à la dernière génération. La carte et les courbes restent accessibles pour consulter le résultat final.

Le bouton **Recommencer** repart à la génération 0, réinitialise la vitesse à x1 et affiche la nouvelle carte.

## Déplacements

La grille sert au décor (terrain et végétation) ; les animaux, eux, ont une position libre. La case d'un animal est celle située sous lui.

Une génération est découpée en **10 petits pas**. À chacun, un animal choisit une direction et une allure, puis :

- il tourne **progressivement** vers cette direction (0,65 radian au plus par petit pas) ;
- il avance selon sa **vitesse** : 100 correspond à une case par génération à pleine allure ;
- devant l'eau, un obstacle ou le bord, il essaie des directions de plus en plus écartées, et ne passe pas entre deux obstacles qui se touchent par un coin.

| Espèce | Comportement |
| --- | --- |
| **Herbivores** | Sur une herbe suffisante (30 ou plus), ils flânent à petite allure. Sinon, ils cherchent l'herbe la plus riche jusqu'à 2 cases, en préférant ce qui est proche et devant eux, avec un peu d'hésitation. Ils mangent une fois par génération, là où ils se trouvent à la fin. |
| **Prédateurs** | Ils voient à 3 cases et foncent sur la proie la plus proche. Sans proie, ils errent. Une proie à moins de 0,45 case est capturée, au plus une fois par génération ; ils flânent ensuite. |

Le vieillissement, la perte d'énergie, les naissances et la repousse ont toujours lieu une fois par génération. Le trajet de la dernière génération est enregistré : l'affichage le rejoue sans recalculer la simulation. La **vitesse ne coûte pas d'énergie** pour l'instant.

## Fuite et meutes

Chaque espèce a un **instinct**, réglable à l'accueil (dernière ligne de sa colonne) ; **0** le désactive.

| Réglage | Comportement | Valeur de départ |
| --- | --- | --- |
| **Chance de fuir (%)** (herbivores) | À chaque génération, l'herbivore a cette chance d'être sur ses gardes. Il s'éloigne alors à pleine allure des prédateurs situés à 2 cases ou moins, surtout du plus proche. | 25 |
| **Esprit de meute (vue +)** (prédateurs) | La valeur s'ajoute à la distance de vue. Près d'un congénère, chacun vise un côté de la proie pour lui couper la route ; sans proie, ils se regroupent. Le repas est partagé à parts égales avec les prédateurs à 1,5 case au plus, sans créer d'énergie. | 2 |

Les obstacles ne bloquent pas la vue.

## Vue 3D

Sur la carte, le bouton **Vue 3D** change l'affichage sans toucher à la simulation ; **Vue 2D** revient à la carte à plat. Les commandes de temps, les courbes, la confirmation Échap et le panneau d'une case fonctionnent dans les deux vues.

| Action en 3D | Commande |
| --- | --- |
| Tourner autour de la carte et changer l'inclinaison | Maintenir le **clic droit** et glisser |
| Se rapprocher ou s'éloigner | **Molette** |
| Revenir à la vue d'ensemble | Bouton **Recentrer** |
| Voir le détail d'une case | **Clic gauche** sur le sol de la case |

Les obstacles sont des blocs, les herbivores de petits modèles jaunes et les prédateurs des modèles rouges plus hauts, orientés dans le sens du déplacement. La couleur des prairies suit la végétation. Des touffes d'herbe y poussent : une case rase (végétation inférieure à 5) n'en a pas, puis leur nombre (jusqu'à cinq) et leur hauteur augmentent avec la végétation. Elles raccourcissent quand un herbivore broute et repoussent au fil des générations. Avec la repousse par défaut de 1 par génération, la croissance est progressive ; le réglage **Repousse végétation** l'accélère. La vue choisie et la position de la caméra sont conservées après **Recommencer** et le retour au menu.

Limites de cette première version : le rendu est calculé par le processeur et dessiné avec Win32/GDI, sans carte graphique, textures ni éclairage dynamique. Les animaux proches les uns des autres peuvent se chevaucher à l'écran ; le panneau d'une case donne les effectifs réels. La caméra vise toujours le centre de la carte. La sélection vise le sol : cliquer sur le haut d'un obstacle peut choisir la case située derrière.

## Règles de la simulation

À chaque génération :

1. La végétation repousse sur les prairies.
2. Pendant 10 petits pas, les herbivores puis les prédateurs se déplacent ; les prédateurs capturent les proies qu'ils atteignent (au plus une chacun).
3. Les herbivores encore vivants mangent la végétation de la case où ils se trouvent.
4. Les survivants vieillissent et perdent de l'énergie. Les individus suffisamment âgés et nourris peuvent se reproduire.
5. Les animaux morts sont retirés ; les effectifs, la végétation totale et les événements de la génération sont enregistrés par `SimulationStats`.

Ce modèle s'écarte du sujet, qui prévoit le choix d'une cellule voisine à chaque génération.

La carte et les populations initiales sont générées au démarrage d'une simulation et à chaque redémarrage. Des graines fixes peuvent être utilisées dans les tests pour reproduire un scénario.

## Organisation du code

Les fichiers `.ixx` exposent les modules C++ ; leurs implémentations sont dans les fichiers `.cpp`.

| Emplacement | Responsabilité |
| --- | --- |
| `src/world/` | Cellules, terrains, positions et génération du monde. |
| `src/animals/` | Classe de base `Animal`, interface `Habitat`, `Herbivore`, `Predator` et catalogue des espèces. |
| `src/simulation/simulation/` | Classe `Simulation` : cycle des générations, déplacements, voisinage et repousse. |
| `src/simulation/stats/` | Classe `SimulationStats` : historique et événements. |
| `src/simulation/settings/` | Réglages, valeurs par défaut et préréglages. |
| `src/simulation/clock/` | Classe `SimulationClock` : vitesses et pause. |
| `src/ui/application/` | Déroulement de l'application et interfaces `InputSource` et `View`. |
| `src/ui/navigation/` | Classe `UiController` : clavier, souris et changement d'écran. |
| `src/ui/map/` | Caméra et scène 3D, lecture des trajets et texte du panneau d'une case. |
| `src/ui/presentation/` | Formulaire des réglages, apparence des espèces, graphiques et placement. |
| `src/render/platform/` | Fenêtre Windows, clavier et souris. |
| `src/render/renderer/` | Classe `Win32Renderer` : choisit ce qu'il faut dessiner. |
| `src/render/painters/` | Une fonction de dessin par écran ou élément. |
| `src/render/gdi/` | Image en mémoire, texte et aides GDI. |
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

1. Créer une classe dérivée d'`Animal` (direction, action, descendance).
2. Ajouter sa valeur à `AnimalKind` et augmenter `SpeciesCount` (`animal.ixx`).
3. Ajouter son entrée au catalogue (`src/animals/species.cpp`) : noms, statistiques d'origine, population de départ et classe.
4. Ajouter son apparence (`src/ui/presentation/species_style.cpp`) : couleur, silhouette et limites de saisie.

Les réglages, les statistiques, les graphiques, le panneau d'une case et les cartes parcourent le catalogue. Une colonne s'ajoute à l'accueil, dont la lisibilité est alors à vérifier. Les préréglages ne règlent que les herbivores et les prédateurs. Les proies (`IsPrey`) jouent en premier et sont chassées par les prédateurs ; un régime alimentaire plus fin demanderait d'étendre `Habitat`.

## Documentation

La documentation est générée avec **Doxygen** (Python est aussi nécessaire, pour le filtre des modules). Depuis la racine du projet :

```powershell
doxygen Doxyfile
```

Ouvrez ensuite `docs/doxygen/html/index.html`. Les pages d'explication se trouvent dans `docs/` : vue d'ensemble, fonctionnement, architecture et notions de C++. Les commentaires des fichiers `.ixx` décrivent chaque classe et fonction visible ; les éléments oubliés sont signalés dans `docs/doxygen-warnings.log`.

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
