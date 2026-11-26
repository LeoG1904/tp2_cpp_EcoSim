# EcoSim

TP noté (ESIMED) : simulateur d'écosystème en **C++23** avec modules, sous Windows.
Sujet : `EcoSim_Dossier_Projet_TP_CPP_V2.pdf`. Utilisation et règles : `README.md`.
Explications et diagrammes : `docs/`.

## Façon de travailler

- Répondre en **français**, simplement.
- **Ne pas commiter ni pousser** sans demande.
- La documentation décrit le projet et sa technique, **sans conseils pour le présenter**.

## Règles obligatoires

Une tâche n'est terminée que si ces deux règles sont respectées.

### 1. Tout ajout est documenté pour Doxygen

- Chaque nouveau fichier, classe, structure, énumération, fonction, constante et membre,
  **privés compris**, reçoit un commentaire Doxygen en français.
- Fichier : bloc `/** @file ... @brief ... */` en tête.
- Déclaration : `/// @brief ...`, avec `@param`, `@return` et `@throws` quand ils s'appliquent.
- Classes, structures et fonctions libres publiques : ajouter `@ingroup moteur`, `interface`
  ou `affichage`.
- Un changement de comportement met aussi à jour les commentaires et les pages de `docs/`.
- Les valeurs d'énumération sont écrites une par ligne, avec leur commentaire `///<`.
- Les deux contrôles doivent produire **zéro avertissement** :
  `doxygen Doxyfile` (site, journal `docs/doxygen-warnings.log`) et
  `doxygen Doxyfile.verification` (privés et aides internes compris, journal
  `docs/doxygen-verification.log`).
  Si la commande est introuvable : `C:\Program Files\doxygen\bin\doxygen.exe`.

### 2. Les diagrammes de classes sont toujours à jour

- Rien ne se lance tout seul : après tout changement de déclaration ou d'import, exécuter
  `python docs/generate_diagrams.py`. Ne pas modifier les SVG de `docs/images/` à la main.
- Les diagrammes détaillés, des données et des modules sont **lus dans le code** par
  `docs/uml_reader.py` et `docs/uml_details.py`. Vérifier qu'une nouvelle écriture C++ est
  bien lue (le lecteur ne couvre que celles du projet).
- Les deux **vues d'ensemble** sont décrites à la main à la fin de
  `docs/generate_diagrams.py` : les modifier dès qu'une classe ou interface est ajoutée,
  supprimée ou renommée, ou qu'un héritage, une possession, une référence, une dépendance
  (y compris dans le corps d'une fonction) ou une méthode affichée change.
- Contrôler les images à l'écran : texte non coupé, traits lisibles, flèches reliées.
  Si ce contrôle n'est pas possible, le dire.

## Commandes

CLion utilise `cmake-build-debug`. **Ne pas compiler dans ce dossier** (conflits avec CLion) :
utiliser `build/claude`, ignoré par Git. Compilateur : Clang (llvm-mingw).

```powershell
cmake -S . -B build/claude -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=C:/llvm-mingw/bin/clang++.exe -DECOSIM_BUILD_TESTS=ON
cmake --build build/claude
.\build\claude\tests.exe
doxygen Doxyfile
doxygen Doxyfile.verification
```

La première configuration télécharge Catch2. `EcoSim.exe` ouvre une fenêtre en plein écran.

## Architecture

| Couche | Dossiers | Cible CMake |
| --- | --- | --- |
| Moteur | `src/world`, `src/animals`, `src/simulation` | `ecosim_core` |
| Interface, C++ pur | `src/ui` | `ecosim_ui` |
| Application Windows | `src/render`, `src/app` | `EcoSim` |

- Le moteur ignore l'interface et l'affichage ; l'interface ignore Windows.
- `windows.h` n'est autorisé que dans `src/render`.
- Les tests ne compilent aucun code de `src/render`.

## Conventions de code

- Ni `std::span` ni `std::string_view`.
- Textes en `std::string` UTF-8 ; collections passées par référence à `std::vector`.
- `std::wstring` et littéraux `L"…"` réservés aux appels Windows ; la conversion des textes
  se fait uniquement dans `src/render/gdi/canvas.cpp`.
- Aucun `new` ni `delete`.
- Commentaires en français, noms du code principalement en anglais.

## Pièges connus

- Ne jamais compiler `main.cpp` seul : les modules passent par CMake.
- Dans un module, `windows.h` se place dans le fragment global (`module;`), et `import std;`
  **après** la déclaration du module.
- Après l'ajout ou le déplacement d'un fichier, recharger le projet CMake dans CLion.
- `docs/doxygen_filter.py` retire la syntaxe des modules pour Doxygen ; il nécessite Python.

## Choix assumés (ne pas annuler sans demander)

- La **simplicité** prime sur un découpage SOLID poussé : ne pas redécouper `UiController`
  ni recréer des hiérarchies pour les dessinateurs.
- Dans `src/ui`, pas de dossier par module.
- Écarts avec le sujet : Win32/GDI au lieu de Raylib, C++23 au lieu de C++20, déplacement
  libre au lieu du choix d'une cellule voisine.

## Avant de dire que c'est terminé

- Compiler et lancer les tests ; ajouter des tests pour tout nouveau comportement.
- Respecter les deux règles obligatoires ci-dessus.
- Pour un changement visuel, regarder l'écran, ou dire que cela n'a pas été fait.
- Si les règles de simulation changent, **remesurer les préréglages** sur de nombreuses graines
  et mettre à jour les résultats annoncés dans le README.
- Pour une simplification ou un déplacement de code, vérifier que le comportement est inchangé.
- Dire clairement ce qui n'a pas été vérifié.
