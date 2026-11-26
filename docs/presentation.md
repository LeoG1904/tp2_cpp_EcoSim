# EcoSim en bref {#mainpage}

EcoSim simule un petit écosystème : **l'herbe pousse**, des **cerfs** et des **lapins** la
mangent et des **prédateurs** les chassent. Chaque animal a un âge et une énergie ; il se déplace, se nourrit,
se reproduit et meurt. Personne ne dirige les populations : leur évolution vient des règles
appliquées à chaque individu.

## Organisation générale

Le programme est découpé en **trois couches**. Une couche ne connaît que celles situées en dessous.

```
  +------------------------------------------------------------+
  |  AFFICHAGE (Windows)        src/render, src/app            |
  |  fenêtre, clavier, souris, dessin 2D et 3D                 |
  +------------------------------------------------------------+
                              |  utilise
  +------------------------------------------------------------+
  |  INTERFACE (C++ pur)        src/ui                         |
  |  écrans, boutons, caméra, graphiques                       |
  +------------------------------------------------------------+
                              |  utilise
  +------------------------------------------------------------+
  |  MOTEUR (C++ pur)           src/world, src/animals,        |
  |  monde, animaux, règles     src/simulation                 |
  +------------------------------------------------------------+
```

Le **moteur fonctionne sans affichage** : c'est ce qui permet de le tester, et ce qui permettrait
de lui donner un autre rendu.

## Les classes principales

| Classe | Son rôle en une phrase |
| --- | --- |
| ecosim::World | La carte : une grille de cases avec leur terrain et leur herbe. |
| ecosim::Animal | Ce que tout animal sait faire ; ecosim::Deer, ecosim::Rabbit et ecosim::Predator précisent comment. |
| ecosim::Simulation | Fait avancer le monde d'une génération. |
| ecosim::Application | Relie les commandes de l'utilisateur, la simulation et l'affichage. |
| ecosim::Win32Renderer | Dessine l'écran à partir de l'état de la simulation. |

## Contenu de la documentation

1. @ref fonctionnement : les règles du jeu, sans code.
2. @ref architecture : les diagrammes de classes et le trajet d'une image, du clic à l'écran.
3. @ref notions : les notions de C++ utilisées.

Le détail de chaque classe se trouve ensuite dans l'onglet **Rubriques** (par couche) ou
**Classes** (par ordre alphabétique).
