/**
 * @file
 * @brief Ce qui est propre au lapin.
 */
module ecosim.rabbit;

import std;

namespace ecosim {
    Rabbit::Rabbit(const std::uint64_t id, const Position cell, const int energy,
                   const AnimalOptions& options)
        : Herbivore{id, cell, energy, Traits, options, GrazingThreshold, 0} {}

    AnimalKind Rabbit::GetKind() const { return AnimalKind::Rabbit; }

    std::unique_ptr<Animal> Rabbit::MakeOffspring(const std::uint64_t id) const {
        return std::make_unique<Rabbit>(id, GetPosition(), NewbornEnergy, GetOptions());
    }
}
