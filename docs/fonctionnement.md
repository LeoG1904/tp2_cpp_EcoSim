# Comment fonctionne la simulation {#fonctionnement}

## Le monde

La carte est une **grille**. Chaque case a un terrain et une quantité d'herbe de 0 à 100.

| Terrain | Les animaux peuvent y aller ? | L'herbe y repousse ? |
| --- | --- | --- |
| Prairie | oui | oui |
| Eau | non | non |
| Obstacle | non | non |

Les animaux, eux, **ne sont pas enfermés dans les cases** : ils ont une position libre. La case
d'un animal est simplement celle qui se trouve sous lui. Plusieurs animaux peuvent partager
une case.

## Une génération

ecosim::Simulation::Step() applique toujours les mêmes étapes, dans cet ordre.

```
  1. Repousse
     chaque prairie gagne de l'herbe
            |
  2. Dix petits pas, et à chacun :
     - les herbivores avancent
     - puis les prédateurs avancent et chassent
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

**Herbivore** (ecosim::Herbivore) :

- s'il est sur ses gardes et qu'un prédateur est à 2 cases ou moins, il **fuit** ;
- sinon, si l'herbe sous lui est suffisante, il **flâne** ;
- sinon, il **cherche** une herbe plus riche autour de lui, avec un peu d'hésitation.

**Prédateur** (ecosim::Predator) :

- il voit à 3 cases et **fonce** sur la proie la plus proche ;
- il capture une proie qu'il approche à moins de 0,45 case, **une fois par génération** au plus ;
- sans proie en vue, il **erre**.

## Les deux instincts réglables

| Instinct | Effet |
| --- | --- |
| Chance de fuir (herbivores) | Tirée à chaque génération : l'herbivore est sur ses gardes, ou non. |
| Esprit de meute (prédateurs) | Voit plus loin, vise les côtés de la proie quand un congénère est proche, rejoint le groupe et partage son repas. |

La valeur 0 désactive l'instinct.

## Naître et mourir

| Règle | Valeur |
| --- | --- |
| Âge minimal pour se reproduire | 5 générations |
| Énergie minimale pour se reproduire | réglable par espèce |
| Coût d'une naissance pour le parent | 30 |
| Énergie du nouveau-né | 30 |
| Causes de mort | faim (énergie à 0), vieillesse (âge maximum), capture |

## Pourquoi les résultats changent-ils ?

La carte, les positions et certaines décisions sont tirées au hasard avec `std::mt19937`.
**Une même graine redonne exactement la même simulation** : c'est ce qui rend les tests
reproductibles. L'application choisit une nouvelle graine à chaque démarrage.
