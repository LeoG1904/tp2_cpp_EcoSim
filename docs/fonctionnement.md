# Comment fonctionne la simulation {#fonctionnement}

## Le monde

La carte est une **grille**, dont la largeur et la hauteur se choisissent à l'accueil (10 × 10 au
minimum). Chaque case a un terrain et une quantité d'herbe de 0 à 100. Les populations de
départ suivent la surface (ecosim::ResizeMap).

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
```

La part d'eau et d'obstacles suit les réglages. Pour qu'aucun animal ne naisse enfermé, seule la
plus grande zone de prairies est gardée. Quand une carte est trop coupée, d'autres reliefs sont
essayés avant de combler.

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
            |
  3. Métabolisme
     chaque survivant vieillit et perd de l'énergie
            |
  4. Reproduction
     les animaux âgés et nourris peuvent avoir un petit
            |
  5. Statistiques
     effectifs, herbe, naissances et morts
```

Les morts sont retirés et les nouveau-nés ajoutés **après** le parcours des animaux : un
nouveau-né n'agit qu'à la génération suivante, et personne n'est mis à jour deux fois.

## Pourquoi 10 petits pas ?

Avec un seul déplacement par génération, les animaux sautaient d'un point à un autre. En
10 petits pas, chacun peut réagir en route : il **tourne progressivement**, contourne les
obstacles et change de cible. Le trajet est enregistré (ecosim::Path) ; l'écran le rejoue
ensuite de façon fluide.

## Les comportements

**Cerfs et lapins** (ecosim::Herbivore, la base commune des herbivores) :

- s'il est sur ses gardes et qu'un chasseur **qui le mange** est à 2 cases ou moins, il **fuit** ;
- sinon, si l'herbe sous lui est suffisante, il **flâne** ;
- sinon, il **cherche** une herbe plus riche autour de lui, avec un peu d'hésitation.

Les deux espèces d'herbivores partagent ces règles, mais pas leurs atouts.

| Caractéristique fixe | Cerf (ecosim::Deer) | Lapin (ecosim::Rabbit) |
| --- | --- | --- |
| Reste brouter si l'herbe sous lui atteint | 30 | 5 : il se contente d'une herbe rase |
| Herbe qu'il ne peut pas brouter | les 5 derniers points | aucune |
| Herbe mangée au maximum par génération | 12 | 5 |
| Vitesse (% de case par génération) | 120 | 100 |
| Énergie perdue par génération | 2 | 1 |
| Âge maximum | 80 | 60 |
| Énergie pour se reproduire | 80 | 50 |
| Repas offert au chasseur | entier | la moitié |

L'herbe au ras du sol est donc **réservée aux lapins** : les cerfs ne peuvent pas leur retirer
toute leur nourriture.

Ces caractéristiques (ecosim::SpeciesTraits) **ne se règlent pas** : ce sont elles qui font
l'espèce. Les chasseurs ont aussi les leurs.

**Loups et renards** (ecosim::Predator, la base commune des chasseurs) :

- chacun **fonce** sur la proie la plus proche **parmi celles qu'il mange** ;
- il capture une proie qu'il approche à moins de 0,45 case, **une fois par génération** au plus ;
- sans proie en vue, il **erre**.

| Caractéristique fixe | Loup (ecosim::Wolf) | Renard (ecosim::Fox) |
| --- | --- | --- |
| Proies | cerfs et lapins | lapins seulement |
| Distance de vue, avant l'instinct | 3 cases | 1,5 case |
| Vitesse (% de case par génération) | 80 | 110 |
| Énergie par proie entière | 25 | 40 |
| Énergie perdue par génération | 2 | 2 |
| Âge maximum | 80 | 100 |
| Énergie pour se reproduire | 80 | 70 |
| En groupe | peut chasser en meute | toujours seul |

Un lapin ne vaut qu'une demi-proie : un renard en tire 20 points d'énergie, un loup 12.

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
| Âge minimal pour se reproduire | 5 générations |
| Énergie minimale pour se reproduire | fixée par l'espèce |
| Coût d'une naissance pour le parent | 30 |
| Énergie du nouveau-né | 30 |
| Causes de mort | faim (énergie à 0), vieillesse (âge maximum), capture |

## Pourquoi les résultats changent-ils ?

La carte, les positions et certaines décisions sont tirées au hasard avec `std::mt19937`.
**Une même graine redonne exactement la même simulation** : c'est ce qui rend les tests
reproductibles. L'application choisit une nouvelle graine à chaque démarrage.
