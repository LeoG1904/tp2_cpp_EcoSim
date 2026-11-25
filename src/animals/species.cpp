module ecosim.species;

import std;

namespace {
    template <typename Species>
    std::unique_ptr<ecosim::Animal> Create(const std::uint64_t id, const ecosim::Position position,
                                           const int energy, const ecosim::AnimalTraits& traits) {
        return std::make_unique<Species>(id, position, energy, traits);
    }
}

namespace ecosim {
    const std::vector<SpeciesInfo>& SpeciesCatalog() {
        static const std::vector<SpeciesInfo> catalog{
            {AnimalKind::Herbivore, "Herbivore", "Herbivores", AnimalTraits{}, 15, 12, true, 25,
             &Create<Herbivore>},
            {AnimalKind::Predator, "Prédateur", "Prédateurs", Predator::DefaultTraits, 150, 3, false, 2,
             &Create<Predator>},
        };
        return catalog;
    }

    const SpeciesInfo& SpeciesOf(const AnimalKind kind) {
        return SpeciesCatalog().at(static_cast<std::size_t>(kind));
    }
}
