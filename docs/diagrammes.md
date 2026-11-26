# Diagrammes détaillés {#diagrammes}

Ces images sont construites automatiquement à partir du code. Elles complètent les deux vues
d'ensemble de la page [architecture](@ref architecture), qui montrent les liens entre classes.

Elles contiennent toutes les classes, structures, énumérations et alias de `src/`, y compris
les types propres à un fichier `.cpp`, avec leurs membres publics, protégés et privés. Elles
n'indiquent ni les valeurs initiales des attributs ordinaires, ni les valeurs par défaut des
paramètres. Les fonctions d'aide internes à un fichier `.cpp` n'y figurent pas.

## Les classes et leurs membres

### Moteur

@image html detail_classes_moteur.svg "Classes du moteur" width=100%

<a href="detail_classes_moteur.svg" target="_blank">Ouvrir ce diagramme en grand</a>

### Interface

@image html detail_classes_interface.svg "Classes de l'interface" width=100%

<a href="detail_classes_interface.svg" target="_blank">Ouvrir ce diagramme en grand</a>

### Affichage

@image html detail_classes_affichage.svg "Classes de l'affichage" width=100%

<a href="detail_classes_affichage.svg" target="_blank">Ouvrir ce diagramme en grand</a>

## Les données

### Moteur

@image html donnees_moteur.svg "Données du moteur" width=100%

<a href="donnees_moteur.svg" target="_blank">Ouvrir ce diagramme en grand</a>

### Interface et affichage

@image html donnees_interface_affichage.svg "Données de l'interface et de l'affichage" width=100%

<a href="donnees_interface_affichage.svg" target="_blank">Ouvrir ce diagramme en grand</a>

## Les modules et les fonctions libres

Chaque boîte donne les modules importés (sans le préfixe `ecosim.`), puis les fonctions et
constantes déclarées hors de toute classe.

@image html modules.svg "Modules du projet" width=100%

<a href="modules.svg" target="_blank">Ouvrir ce diagramme en grand</a>
