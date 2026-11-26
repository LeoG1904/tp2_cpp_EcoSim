# Les notions de C++ utilisées {#notions}

| Notion | En clair | Exemple dans le projet |
| --- | --- | --- |
| Module (`.ixx`, `import`) | Remplace les en-têtes : un module déclare ce qu'il rend visible avec `export`. | `export module ecosim.world;` |
| `enum class` | Une liste de valeurs nommées, non mélangeables avec des entiers. | ecosim::Terrain, ecosim::AnimalKind |
| `std::vector` | Un tableau dont la taille peut changer. | les cases de ecosim::World |
| `std::array` | Un tableau de taille fixe. | ecosim::Path : 11 points |
| `std::optional` | Une valeur qui peut être absente. | ecosim::Habitat::NearestPrey : il peut n'y avoir aucune proie |
| `std::unique_ptr` | Possède un objet et le détruit automatiquement : aucun `delete` à écrire. | la liste des animaux de ecosim::Simulation |
| RAII | Une ressource est prise à la création d'un objet et rendue à sa destruction. | la fenêtre et l'image en mémoire du rendu |
| Méthode virtuelle | La version appelée dépend du type réel de l'objet. | ecosim::Animal::Steer |
| Interface | Une classe qui ne décrit que ce qu'il faut savoir faire. | ecosim::View, ecosim::InputSource, ecosim::Habitat |
| `const` | Promet de ne pas modifier ; le compilateur le vérifie. | le rendu lit une `const Simulation&` |
| Template | Un code écrit une fois pour plusieurs types. | ecosim::PerSpecies |
| `std::mt19937` | Un générateur pseudo-aléatoire : même graine, même suite. | la carte et les déplacements |
