module ecosim.predator;

import std;

namespace ecosim {
    Predator::Predator(const std::uint64_t id, const Position position, const int energy,
                       const AnimalTraits& traits)
        : Animal{id, position, energy, traits} {}

    AnimalKind Predator::GetKind() const { return AnimalKind::Predator; }

    TurnPhase Predator::Phase() const { return TurnPhase::Hunting; }
    bool Predator::IsHunter() const { return true; }

    Position Predator::ChooseDestination(const Habitat& habitat, std::mt19937& rng) const {
        const World& world = habitat.GetWorld();
        const int range = GetTraits().instinct;
        const Position origin = GetPosition();
        return BestAccessiblePosition(world, [&](const Position position) {
            const int prey = habitat.PreyCount(position);
            if (range == 0) return prey;
            // En meute : d'abord les proies à portée, puis leur piste, puis rester groupés.
            const int toPrey = NearestDistance(world, position, range, [&habitat](const Position other) {
                return habitat.PreyCount(other) > 0;
            });
            const int toMate = NearestDistance(world, position, range, [&](const Position other) {
                // Le prédateur ne se compte pas lui-même.
                return habitat.HunterCount(other) > (other == origin ? 1 : 0);
            });
            return prey * 10000 + (range + 1 - toPrey) * 100 + (range + 1 - toMate);
        }, rng);
    }

    void Predator::Feed(Habitat& habitat) {
        if (!habitat.CatchPrey(GetPosition())) return;
        const int meal = GetTraits().foodEnergy;
        GainEnergy(GetTraits().instinct > 0 ? habitat.ShareMeal(*this, meal) : meal);
    }

    std::unique_ptr<Animal> Predator::MakeOffspring(const std::uint64_t id) const {
        return std::make_unique<Predator>(id, GetPosition(), 30, GetTraits());
    }
}
