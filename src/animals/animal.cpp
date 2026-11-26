module ecosim.animal;

import std;

namespace {
    // Distance gardée devant soi : les animaux ne se dessinent pas dans les murs.
    constexpr double BodyMargin{0.12};

    // Un pas en diagonale ne passe pas entre deux obstacles qui se touchent par un coin.
    bool CanCross(const ecosim::World& world, const ecosim::Point from, const ecosim::Point to) {
        const ecosim::Position a = ecosim::CellAt(from);
        const ecosim::Position b = ecosim::CellAt(to);
        if (a.x == b.x || a.y == b.y) return true;
        const auto open = [&world](const ecosim::Position cell) {
            return world.IsInside(cell) && world.GetCell(cell).IsAccessible();
        };
        return open({a.x, b.y}) || open({b.x, a.y});
    }
}

namespace ecosim {
    Animal::Animal(const std::uint64_t id, const Position cell, const int energy,
                   const AnimalTraits& traits)
        : id{id}, location{CenterOf(cell)},
          // Des directions de départ différentes sans consommer de hasard.
          heading{std::remainder(static_cast<double>(id) * 2.399963, 2.0 * std::numbers::pi)},
          energy{energy}, traits{traits} {
        if (energy <= 0 || energy > MaxEnergy) {
            throw std::invalid_argument{"L'énergie initiale doit être entre 1 et 100"};
        }
        path.fill(location);
    }

    std::uint64_t Animal::GetId() const { return id; }
    std::optional<std::uint64_t> Animal::GetParentId() const { return parentId; }
    Point Animal::GetLocation() const { return location; }
    Position Animal::GetPosition() const { return CellAt(location); }
    double Animal::GetHeading() const { return heading; }
    const Path& Animal::GetPath() const { return path; }
    int Animal::GetAge() const { return age; }
    int Animal::GetEnergy() const { return energy; }
    const AnimalTraits& Animal::GetTraits() const { return traits; }
    bool Animal::IsAlive() const { return energy > 0 && age < traits.maxAge; }

    void Animal::Place(const Point point, const double direction) {
        location = point;
        heading = direction;
        path.fill(location);
    }

    void Animal::StartPath() { path.fill(location); }

    void Animal::Advance(const World& world, const Steering steering, const int tick) {
        const double turn = std::remainder(steering.heading - heading, 2.0 * std::numbers::pi);
        heading += std::clamp(turn, -MaxTurnPerTick, MaxTurnPerTick);
        const double length = traits.speed / 100.0 / TicksPerGeneration
                            * std::clamp(steering.pace, 0.0, 1.0);
        if (length > 0.0) {
            // Devant un obstacle, on essaie des directions de plus en plus écartées.
            constexpr std::array<double, 10> detours{0.0, 0.6, -0.6, 1.2, -1.2, 1.9, -1.9, 2.6, -2.6,
                                                     std::numbers::pi};
            for (const double detour : detours) {
                const double angle = heading + detour;
                const Point next{location.x + std::cos(angle) * length,
                                 location.y + std::sin(angle) * length};
                const Point ahead{next.x + std::cos(angle) * BodyMargin,
                                  next.y + std::sin(angle) * BodyMargin};
                if (!world.IsWalkable(next) || !world.IsWalkable(ahead)
                    || !CanCross(world, location, next)) {
                    continue;
                }
                location = next;
                heading = angle;
                break;
            }
        }
        // Le reste du trajet suit la position atteinte.
        for (std::size_t i = static_cast<std::size_t>(tick) + 1; i < path.size(); ++i) path[i] = location;
    }

    void Animal::SpendGeneration() {
        ++age;
        energy = std::max(0, energy - traits.energyLoss);
    }
    void Animal::Die() { energy = 0; }

    bool Animal::CanReproduce() const { return IsAlive() && age >= 5 && energy >= traits.reproductionEnergy; }
    void Animal::PayReproductionCost() { energy -= 30; }
    void Animal::ReceiveMeal(const int amount) { energy = std::min(MaxEnergy, energy + amount); }

    bool Animal::IsPrey() const { return false; }
    void Animal::BeginGeneration(std::mt19937&) {}

    double Animal::HeadingTo(const Point from, const Point to) {
        return std::atan2(to.y - from.y, to.x - from.x);
    }

    double Animal::Jitter(std::mt19937& rng, const double spread) {
        std::uniform_real_distribution<double> angle{-spread, spread};
        return angle(rng);
    }

    std::unique_ptr<Animal> Animal::CreateOffspring(const std::uint64_t childId) const {
        auto child = MakeOffspring(childId);
        child->parentId = id;
        // Le nouveau-né apparaît auprès de son parent et part d'un autre côté.
        child->Place(location, std::remainder(heading + 2.4, 2.0 * std::numbers::pi));
        return child;
    }
}
