// Catalogue des espèces : seul endroit du moteur qui les énumère avec leurs classes.
export module ecosim.species;

import std;
export import ecosim.herbivore;
export import ecosim.predator;

export namespace ecosim {
    struct SpeciesInfo {
        AnimalKind kind;
        std::string name;
        std::string pluralName;
        AnimalTraits defaultTraits;
        // Population de départ : une pour tant de cases, avec un minimum.
        int cellsPerAnimal;
        int minimumAtStart;
        bool canBeEaten;
        // Instinct activé dans l'application : fuite (%) ou rayon de meute.
        int defaultInstinct;
        std::unique_ptr<Animal> (*create)(std::uint64_t id, Position position, int energy,
                                          const AnimalTraits& traits);
    };

    // Une entrée par valeur d'AnimalKind, dans le même ordre.
    const std::vector<SpeciesInfo>& SpeciesCatalog();
    const SpeciesInfo& SpeciesOf(AnimalKind kind);
}
