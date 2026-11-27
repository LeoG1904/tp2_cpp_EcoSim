/**
 * @file
 * @brief Ce qui est propre au cerf.
 */
module ecosim.deer;

import std;

namespace ecosim {
    Deer::Deer(const std::uint64_t id, const Position cell, const int energy,
               const AnimalOptions& options)
        : Herbivore{id, cell, energy, Traits, options, GrazingThreshold, UngrazableVegetation} {}

    AnimalKind Deer::GetKind() const { return AnimalKind::Deer; }

    std::unique_ptr<Animal> Deer::MakeOffspring(const std::uint64_t id) const {
        return std::make_unique<Deer>(id, GetPosition(), NewbornEnergy, GetOptions());
    }
}
