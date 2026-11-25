module ecosim.herbivore;

import std;

namespace ecosim {
    Herbivore::Herbivore(const std::uint64_t id, const Position position, const int energy,
                         const AnimalTraits& traits)
        : Animal{id, position, energy, traits} {}

    AnimalKind Herbivore::GetKind() const { return AnimalKind::Herbivore; }

    TurnPhase Herbivore::Phase() const { return TurnPhase::Grazing; }
    bool Herbivore::IsPrey() const { return true; }

    Position Herbivore::ChooseDestination(const Habitat& habitat, std::mt19937& rng) const {
        const World& world = habitat.GetWorld();
        return BestAccessiblePosition(world, [&world](const Position position) {
            return world.GetCell(position).GetVegetation();
        }, rng);
    }

    void Herbivore::Feed(Habitat& habitat) {
        Cell& cell = habitat.GetWorld().GetCell(GetPosition());
        if (!cell.IsAccessible()) return;
        const int consumed = std::min({GetTraits().foodEnergy, cell.GetVegetation(), MaxEnergy - GetEnergy()});
        cell.SetVegetation(cell.GetVegetation() - consumed);
        GainEnergy(consumed);
    }

    std::unique_ptr<Animal> Herbivore::MakeOffspring(const std::uint64_t id) const {
        return std::make_unique<Herbivore>(id, GetPosition(), 30, GetTraits());
    }
}
