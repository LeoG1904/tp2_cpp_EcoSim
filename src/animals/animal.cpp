module ecosim.animal;

import std;

namespace ecosim {
    Animal::Animal(const std::uint64_t id, const Position position, const int energy,
                   const AnimalTraits& traits)
        : id{id}, position{position}, energy{energy}, traits{traits} {
        if (energy <= 0 || energy > MaxEnergy) {
            throw std::invalid_argument{"L'énergie initiale doit être entre 1 et 100"};
        }
    }

    std::uint64_t Animal::GetId() const { return id; }
    Position Animal::GetPosition() const { return position; }
    int Animal::GetAge() const { return age; }
    int Animal::GetEnergy() const { return energy; }
    const AnimalTraits& Animal::GetTraits() const { return traits; }
    bool Animal::IsAlive() const { return energy > 0 && age < traits.maxAge; }

    void Animal::MoveTo(const Position destination) { position = destination; }

    void Animal::SpendGeneration() {
        ++age;
        energy = std::max(0, energy - traits.energyLoss);
    }
    void Animal::Die() { energy = 0; }

    bool Animal::CanReproduce() const { return IsAlive() && age >= 5 && energy >= traits.reproductionEnergy; }
    void Animal::PayReproductionCost() { energy -= 30; }
    void Animal::GainEnergy(const int amount) { energy = std::min(MaxEnergy, energy + amount); }
}
