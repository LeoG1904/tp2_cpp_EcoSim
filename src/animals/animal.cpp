module ecosim.animal;

import std;

namespace ecosim {
    Animal::Animal(const std::uint64_t id, const Position position, const int energy,
                   const AnimalTraits& traits)
        : id{id}, position{position}, previousPosition{position}, energy{energy}, traits{traits} {
        if (energy <= 0 || energy > MaxEnergy) {
            throw std::invalid_argument{"L'énergie initiale doit être entre 1 et 100"};
        }
    }

    std::uint64_t Animal::GetId() const { return id; }
    std::optional<std::uint64_t> Animal::GetParentId() const { return parentId; }
    Position Animal::GetPosition() const { return position; }
    int Animal::GetAge() const { return age; }
    int Animal::GetEnergy() const { return energy; }
    const AnimalTraits& Animal::GetTraits() const { return traits; }
    bool Animal::IsAlive() const { return energy > 0 && age < traits.maxAge; }

    Position Animal::GetPreviousPosition() const { return previousPosition; }
    void Animal::MoveTo(const Position destination) {
        previousPosition = position;
        position = destination;
    }

    void Animal::SpendGeneration() {
        ++age;
        energy = std::max(0, energy - traits.energyLoss);
    }
    void Animal::Die() { energy = 0; }

    bool Animal::CanReproduce() const { return IsAlive() && age >= 5 && energy >= traits.reproductionEnergy; }
    void Animal::PayReproductionCost() { energy -= 30; }
    void Animal::GainEnergy(const int amount) { energy = std::min(MaxEnergy, energy + amount); }

    bool Animal::IsPrey() const { return false; }
    bool Animal::IsHunter() const { return false; }
    void Animal::ReceiveMeal(const int amount) { GainEnergy(amount); }

    int Animal::NearestDistance(const World& world, const Position from, const int radius,
                                const std::function<bool(Position)>& found) {
        // On explore des anneaux de plus en plus larges et on s'arrête au premier succès.
        for (int distance = 0; distance <= radius; ++distance) {
            for (int dx = -distance; dx <= distance; ++dx) {
                const int dy = distance - std::abs(dx);
                const Position above{from.x + dx, from.y - dy};
                const Position below{from.x + dx, from.y + dy};
                if (world.IsInside(above) && found(above)) return distance;
                if (dy != 0 && world.IsInside(below) && found(below)) return distance;
            }
        }
        return radius + 1;
    }

    std::unique_ptr<Animal> Animal::CreateOffspring(const std::uint64_t childId) const {
        auto child = MakeOffspring(childId);
        child->parentId = id;
        return child;
    }

    Position Animal::BestAccessiblePosition(const World& world,
                                            const std::function<int(Position)>& score,
                                            std::mt19937& rng) const {
        Position best = position;
        int bestScore = -1;
        int ties = 0;
        const auto consider = [&](const Position candidate) {
            if (!world.GetCell(candidate).IsAccessible()) return;
            const int value = score(candidate);
            if (value > bestScore) {
                bestScore = value;
                best = candidate;
                ties = 1;
            } else if (value == bestScore) {
                std::uniform_int_distribution<int> pick{1, ++ties};
                if (pick(rng) == 1) best = candidate;
            }
        };
        consider(position);
        for (const Position neighbor : world.Neighbors(position)) consider(neighbor);
        return best;
    }
}
