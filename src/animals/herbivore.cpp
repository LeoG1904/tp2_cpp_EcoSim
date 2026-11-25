module ecosim.herbivore;

import std;

namespace ecosim {
    Herbivore::Herbivore(const std::uint64_t id, const Position position, const int energy,
                         const AnimalTraits& traits)
        : Animal{id, position, energy, traits} {}

    AnimalKind Herbivore::GetKind() const { return AnimalKind::Herbivore; }

    void Herbivore::Feed(World& world) {
        Cell& cell = world.GetCell(GetPosition());
        if (!cell.IsAccessible()) return;
        const int consumed = std::min({GetTraits().foodEnergy, cell.GetVegetation(), MaxEnergy - GetEnergy()});
        cell.SetVegetation(cell.GetVegetation() - consumed);
        GainEnergy(consumed);
    }

    std::unique_ptr<Animal> Herbivore::CreateOffspring(const std::uint64_t id) const {
        return std::make_unique<Herbivore>(id, GetPosition(), 30, GetTraits());
    }
}
