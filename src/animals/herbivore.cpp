module ecosim.herbivore;

import std;

namespace ecosim {
    Herbivore::Herbivore(const std::uint64_t id, const Position position, const int energy)
        : Animal{id, position, energy} {}

    AnimalKind Herbivore::GetKind() const { return AnimalKind::Herbivore; }

    void Herbivore::Feed(World& world, std::span<const std::unique_ptr<Animal>>) {
        Cell& cell = world.GetCell(GetPosition());
        if (!cell.IsAccessible()) return;
        const int consumed = std::min({10, cell.GetVegetation(), MaxEnergy - GetEnergy()});
        cell.SetVegetation(cell.GetVegetation() - consumed);
        GainEnergy(consumed);
    }

    std::unique_ptr<Animal> Herbivore::CreateOffspring(const std::uint64_t id) const {
        return std::make_unique<Herbivore>(id, GetPosition(), 30);
    }
}
