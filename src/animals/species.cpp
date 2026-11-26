/**
 * @file
 * @brief Contenu du catalogue des espèces.
 */
module ecosim.species;

import std;

namespace {
    /// @brief Crée un animal de la classe demandée ; sert de fabrique au catalogue.
    /// @tparam Species Classe dérivée d'Animal.
    template <typename Species>
    std::unique_ptr<ecosim::Animal> Create(const std::uint64_t id, const ecosim::Position position,
                                           const int energy, const ecosim::AnimalOptions& options) {
        return std::make_unique<Species>(id, position, energy, options);
    }
}

namespace ecosim {
    const std::vector<SpeciesInfo>& SpeciesCatalog() {
        static const std::vector<SpeciesInfo> catalog{
            {AnimalKind::Deer, "Cerf", "Cerfs", Deer::DefaultBirthPercent, 15, 12, true, 25,
             &Create<Deer>},
            {AnimalKind::Rabbit, "Lapin", "Lapins", Rabbit::DefaultBirthPercent, 15, 12, true, 25,
             &Create<Rabbit>},
            {AnimalKind::Wolf, "Loup", "Loups", Wolf::DefaultBirthPercent, 150, 3, false, 2,
             &Create<Wolf>},
            {AnimalKind::Fox, "Renard", "Renards", Fox::DefaultBirthPercent, 150, 3, false, 1,
             &Create<Fox>},
        };
        return catalog;
    }

    const SpeciesInfo& SpeciesOf(const AnimalKind kind) {
        return SpeciesCatalog().at(static_cast<std::size_t>(kind));
    }
}
