module ecosim.predator;

import std;

namespace ecosim {
    Predator::Predator(const std::uint64_t id, const Position position, const int energy,
                       const AnimalTraits& traits)
        : Animal{id, position, energy, traits} {}

    AnimalKind Predator::GetKind() const { return AnimalKind::Predator; }

    TurnPhase Predator::Phase() const { return TurnPhase::Hunting; }

    Position Predator::ChooseDestination(const Habitat& habitat, std::mt19937& rng) const {
        return BestAccessiblePosition(habitat.GetWorld(), [&habitat](const Position position) {
            return habitat.PreyCount(position);
        }, rng);
    }

    void Predator::Feed(Habitat& habitat) {
        if (habitat.CatchPrey(GetPosition())) GainEnergy(GetTraits().foodEnergy);
    }

    std::unique_ptr<Animal> Predator::MakeOffspring(const std::uint64_t id) const {
        return std::make_unique<Predator>(id, GetPosition(), 30, GetTraits());
    }
}
