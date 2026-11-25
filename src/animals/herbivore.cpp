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
        const auto hunterThere = [&habitat](const Position other) {
            return habitat.HunterCount(other) > 0;
        };
        // La fuite n'est pas automatique : un chasseur repéré n'est évité qu'avec une certaine
        // chance. Sinon les proies, qui jouent les premières, ne seraient jamais rattrapées.
        bool fleeing = false;
        if (const int reflex = GetTraits().instinct;
            reflex > 0 && NearestDistance(world, GetPosition(), ThreatSight, hunterThere) <= ThreatSight) {
            std::bernoulli_distribution react{std::min(reflex, 100) / 100.0};
            fleeing = react(rng);
        }
        return BestAccessiblePosition(world, [&](const Position position) {
            const int food = world.GetCell(position).GetVegetation();
            if (!fleeing) return food;
            // En fuite, s'éloigner passe avant la nourriture.
            const int safety = NearestDistance(world, position, ThreatSight, hunterThere);
            return safety * (Cell::MaxVegetation + 1) + food;
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
