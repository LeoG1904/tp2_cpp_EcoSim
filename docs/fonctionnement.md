# Comment fonctionne la simulation {#fonctionnement}

## Le monde

La carte est une **grille**, dont la largeur et la hauteur se choisissent à l'accueil (10 × 10 au
minimum). Chaque case a un terrain, une quantité d'herbe de 0 à 100 et une hauteur de 0 à 4
paliers. Les populations de départ suivent la surface (ecosim::ResizeMap).

| Terrain | Les animaux peuvent y aller ? | L'herbe y repousse ? |
| --- | --- | --- |
| Prairie | oui | oui |
| Eau | non | non |
| Obstacle | non | non |

Les animaux, eux, **ne sont pas enfermés dans les cases** : ils ont une position libre. La case
d'un animal est simplement celle qui se trouve sous lui. Plusieurs animaux peuvent partager
une case.

## La carte naît de deux reliefs

ecosim::GenerateRandomWorld() ne tire pas chaque case séparément : elle construit des reliefs
**continus**, par « bruit de valeur ».

```
  1. Des valeurs au sort, tous les 8 cases, puis tous les 4
            |
  2. Lissage entre ces valeurs : les voisines se ressemblent
            |
  3. Altitude : les creux deviennent de l'eau, les sommets des obstacles
     Humidité : elle fixe l'herbe de départ
            |
  4. Les prairies enfermées sont comblées
            |
  5. La terre est découpée en paliers selon l'altitude
```

La part d'eau et d'obstacles suit les réglages. Pour qu'aucun animal ne naisse enfermé, seule la
plus grande zone de prairies est gardée. Quand une carte est trop coupée, d'autres reliefs sont
essayés avant de combler.

## Le relief

La même altitude découpe la terre en paliers de surfaces égales, jusqu'au réglage **Relief** (0
donne une carte plate). L'eau reste en bas. Un animal garde une position à deux coordonnées : sa
hauteur est celle de la case située sous lui. Elle ne joue que sur sa vitesse : un palier plus
haut à moins de ecosim::Animal::ClimbReach case devant lui ne lui en laisse que la part
ecosim::Animal::ClimbPace, la moitié.

## La soif

Quand la soif est active (ecosim::Simulation::HasThirst), chaque animal a de l'eau, de 0 à
ecosim::Animal::MaxHydration. Il en perd à chaque génération et meurt à 0. À
ecosim::Animal::ThirstThreshold ou moins, il part boire et ne peut plus se reproduire. Passer sur une **berge**, une prairie
qui touche l'eau, remplit la jauge.

```
  À la création : depuis toutes les berges à la fois, on compte les pas vers chaque prairie
            |
       3   2   1   0 | eau
       4   #   2   1 | 0        # : obstacle
            |
  Un animal assoiffé vise la case voisine qui a un pas de moins
```

ecosim::WaterRoutes fait ce calcul **une seule fois**. La carte ne gardant qu'une zone de
prairies reliées, il suffit d'une berge pour que tout le monde puisse boire. Sans aucune berge,
la soif reste active : personne ne peut boire.

## Une génération

ecosim::Simulation::Step() applique toujours les mêmes étapes, dans cet ordre.

```
  1. Repousse
     chaque prairie gagne de l'herbe
            |
  2. Dix petits pas, et à chacun :
     - les cerfs et les lapins avancent
     - puis les loups et les renards avancent et chassent
     (les herbivores mangent au dernier pas)
     (un animal qui passe sur une berge boit)
     - les animaux qui se superposent sont écartés
     - enfin, les couples se forment
            |
  3. Métabolisme
     chaque survivant vieillit, perd de l'énergie et, avec la soif, de l'eau
            |
  4. Reproduction
     les femelles en couple peuvent avoir une portée
            |
  5. Statistiques
     effectifs, herbe, naissances et morts
```

Les morts sont retirés et les nouveau-nés ajoutés **après** le parcours des animaux : un
nouveau-né n'agit qu'à la génération suivante, et personne n'est mis à jour deux fois.

## Les collisions

Chaque animal a un **corps rond** : le lapin est le plus petit, le loup le plus gros. Après
chaque petit pas, ecosim::GridHabitat::Separate() écarte ceux qui se superposent.

- C'est une collision **souple** : on se pousse, sans jamais rester bloqué net. Un petit
  chevauchement peut subsister dans une foule.
- Le plus petit recule davantage. Personne n'est poussé hors des prairies.
- Les captures passent avant : une poussée ne sauve pas une proie déjà atteinte.

## Pourquoi 10 petits pas ?

Avec un seul déplacement par génération, les animaux sautaient d'un point à un autre. En
10 petits pas, chacun peut réagir en route : il **tourne progressivement**, contourne les
obstacles et change de cible. Le trajet est enregistré (ecosim::Path) ; l'écran le rejoue
ensuite de façon fluide.

## Les comportements

**Cerfs et lapins** (ecosim::Herbivore, la base commune des herbivores) :

- s'il est sur ses gardes et qu'un chasseur **qui le mange** est à 2 cases ou moins, il **fuit** ;
- sinon, s'il a soif, il **va boire** ;
- sinon, s'il est prêt à se reproduire et repère un partenaire, il **le rejoint** ;
- sinon, si l'herbe sous lui est suffisante, il **flâne** ;
- sinon, il **cherche** une herbe plus riche autour de lui, avec un peu d'hésitation.

Les deux espèces d'herbivores partagent ces règles, mais pas leurs atouts.

| Caractéristique fixe | Cerf (ecosim::Deer) | Lapin (ecosim::Rabbit) |
| --- | --- | --- |
| Reste brouter si l'herbe sous lui atteint | 30 | 5 : il se contente d'une herbe rase |
| Herbe qu'il ne peut pas brouter | les 5 derniers points | aucune |
| Herbe mangée au maximum par génération | 12 | 5 |
| Vitesse (% de case par génération) | 130 | 100 |
| Énergie perdue par génération | 2 | 1 |
| Eau perdue par génération | 3 | 2 |
| Âge maximum | 80 | 70 |
| Énergie pour se reproduire | 80 | 50 |
| Repas offert au chasseur | entier | la moitié |

L'herbe au ras du sol est donc **réservée aux lapins** : les cerfs ne peuvent pas leur retirer
toute leur nourriture.

Ces caractéristiques (ecosim::SpeciesTraits) **ne se règlent pas** : ce sont elles qui font
l'espèce. Les chasseurs ont aussi les leurs.

**Loups et renards** (ecosim::Predator, la base commune des chasseurs) :

- s'il a soif, il **va boire** ;
- sinon, prêt à se reproduire et un partenaire en vue, il **le rejoint** ;
- sinon, chacun **fonce** sur la proie la plus proche **parmi celles qu'il mange** ;
- il capture une proie qu'il approche à moins de 0,45 case, **une fois par génération** au plus ;
- sans proie en vue, il **erre**.

| Caractéristique fixe | Loup (ecosim::Wolf) | Renard (ecosim::Fox) |
| --- | --- | --- |
| Proies | cerfs et lapins | lapins seulement |
| Distance de vue, avant l'instinct | 3 cases | 1,5 case |
| Vitesse (% de case par génération) | 80 | 90 |
| Énergie par proie entière | 25 | 35 |
| Énergie perdue par génération | 2 | 2 |
| Eau perdue par génération | 2 | 2 |
| Âge maximum | 80 | 140 |
| Énergie pour se reproduire | 80 | 70 |
| En groupe | peut chasser en meute | toujours seul |

Un lapin ne vaut qu'une demi-proie : un renard en tire 17 points d'énergie, un loup 12.

## Ce qui se règle

Chaque espèce n'a que trois réglages : son **nombre au départ**, sa **chance de naissance** et
son **instinct** (ces deux derniers forment ecosim::AnimalOptions).

| Instinct | Effet |
| --- | --- |
| Chance de fuir (cerfs et lapins) | Tirée à chaque génération : l'herbivore est sur ses gardes, ou non. |
| Esprit de meute (loups) | Voit plus loin, vise les côtés de la proie quand un autre loup est proche, rejoint le groupe et partage son repas. |
| Flair (renards) | La valeur s'ajoute à la distance de vue. |

La valeur 0 désactive l'instinct.

## Naître et mourir

| Règle | Valeur |
| --- | --- |
| Parents | une femelle et un mâle de la même espèce |
| Soif | un animal assoiffé ne se reproduit pas |
| Âge minimal pour se reproduire | 5 générations |
| Énergie minimale pour se reproduire | fixée par l'espèce |
| Distance pour repérer un partenaire | 6 cases |
| Distance pour former un couple | 0,6 case |
| Petits par portée | cerf 1 à 2, lapin 2 à 5, loup 2 à 4, renard 1 à 3 |
| Repos après une portée | cerf 8, lapin 5, loup 12, renard 10 générations |
| Coût d'une portée pour chaque parent | 20 |
| Énergie du nouveau-né | 30 |
| Eau du nouveau-né | 50 (ecosim::Animal::NewbornHydration) : il naît assoiffé |
| Causes de mort | faim (énergie à 0), soif (eau à 0), vieillesse (âge maximum), capture |

À l'écran, le mâle est plus foncé et la femelle plus claire. Le cerf mâle porte des bois, le
loup une crinière, le renard un bout de queue blanc ; les oreilles du lapin sont bleues, celles
de la lapine roses.

Avant 5 générations, un animal est dessiné **jeune** : plus petit, avec une grosse tête, et le
même aspect pour les deux sexes (faon tacheté, lapereau et louveteau à petites oreilles,
renardeau à queue courte). Seul le dessin change : son corps compte autant dans les collisions.

## Pourquoi les résultats changent-ils ?

La carte, les positions et certaines décisions sont tirées au hasard avec `std::mt19937`.
**Une même graine redonne exactement la même simulation** : c'est ce qui rend les tests
reproductibles. L'application choisit une nouvelle graine à chaque démarrage.
