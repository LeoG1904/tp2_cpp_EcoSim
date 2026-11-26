/**
 * @file
 * @brief Ce qui est propre au renard.
 */
module ecosim.fox;

import std;

namespace ecosim {
    Fox::Fox(const std::uint64_t id, const Position cell, const int energy,
             const AnimalOptions& options)
        : Predator{id, cell, energy, Traits, options, Sight} {}

    AnimalKind Fox::GetKind() const { return AnimalKind::Fox; }
    bool Fox::Eats(const AnimalKind kind) const { return kind == AnimalKind::Rabbit; }

    std::unique_ptr<Animal> Fox::MakeOffspring(const std::uint64_t id) const {
        return std::make_unique<Fox>(id, GetPosition(), 30, GetOptions());
    }
}
