module ecosim.predator;

import std;

namespace ecosim {
    Predator::Predator(const std::uint64_t id, const Position position, const int energy)
        : Animal{id, position, energy} {}

    AnimalKind Predator::GetKind() const { return AnimalKind::Predator; }

    void Predator::Hunt(std::vector<std::unique_ptr<Animal>>& animals) {
        for (const auto& prey : animals) {
            if (prey->GetKind() == AnimalKind::Herbivore && prey->IsAlive()
                && prey->GetPosition() == GetPosition()) {
                prey->Die();
                GainEnergy(EnergyFromPrey);
                return;
            }
        }
    }

    std::unique_ptr<Animal> Predator::CreateOffspring(const std::uint64_t id) const {
        return std::make_unique<Predator>(id, GetPosition(), 30);
    }
}
