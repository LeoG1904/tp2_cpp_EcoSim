module ecosim.animal;

import std;

namespace ecosim {
    Animal::Animal(const std::uint64_t id, const Position position, const int energy)
        : id{id}, position{position}, energy{energy} {
        if (energy <= 0 || energy > MaxEnergy) {
            throw std::invalid_argument{"L'énergie initiale doit être entre 1 et 100"};
        }
    }

    std::uint64_t Animal::GetId() const { return id; }
    Position Animal::GetPosition() const { return position; }
    int Animal::GetAge() const { return age; }
    int Animal::GetEnergy() const { return energy; }
    bool Animal::IsAlive() const { return energy > 0 && age < MaxAge; }

    void Animal::MoveTo(const Position destination) { position = destination; }

    void Animal::SpendGeneration() {
        ++age;
        energy = std::max(0, energy - 2);
    }

    bool Animal::CanReproduce() const { return IsAlive() && age >= 5 && energy >= 80; }
    void Animal::PayReproductionCost() { energy -= 30; }
    void Animal::GainEnergy(const int amount) { energy = std::min(MaxEnergy, energy + amount); }
}
