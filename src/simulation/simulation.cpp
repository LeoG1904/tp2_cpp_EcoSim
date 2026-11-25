module ecosim.simulation;

import std;

namespace ecosim {
    void SimulationClock::SetSpeed(const int multiplier) {
        if (std::ranges::find(Speeds, multiplier) == Speeds.end()) {
            throw std::invalid_argument{"Vitesse non supportée"};
        }
        speed = multiplier;
    }

    int SimulationClock::GetSpeed() const { return speed; }

    int SimulationClock::Advance(const double realSeconds) {
        accumulator += realSeconds * speed * BaseGenerationsPerSecond;
        const int steps = static_cast<int>(accumulator);
        accumulator -= steps;
        return steps;
    }

    void GrowVegetation(World& world) {
        constexpr int growthPerGeneration{1};
        for (int y = 0; y < world.GetHeight(); ++y) {
            for (int x = 0; x < world.GetWidth(); ++x) {
                Cell& cell = world.GetCell(Position{x, y});
                if (cell.GetTerrain() == Terrain::Prairie) {
                    cell.SetVegetation(cell.GetVegetation() + growthPerGeneration);
                }
            }
        }
    }

    Simulation::Simulation(World world, std::mt19937 rng)
        : world{std::move(world)}, rng{std::move(rng)} {}

    const World& Simulation::GetWorld() const { return world; }
    int Simulation::GetGeneration() const { return generation; }

    std::size_t Simulation::HerbivoreCount() const {
        return static_cast<std::size_t>(std::ranges::count_if(animals, [](const auto& animal) {
            return animal->GetKind() == AnimalKind::Herbivore;
        }));
    }

    std::vector<AnimalSnapshot> Simulation::Animals() const {
        std::vector<AnimalSnapshot> result;
        result.reserve(animals.size());
        for (const auto& animal : animals) {
            result.push_back({animal->GetId(), animal->GetPosition(), animal->GetKind(),
                              animal->GetAge(), animal->GetEnergy()});
        }
        return result;
    }

    void Simulation::AddHerbivore(const Position position, const int energy) {
        if (!world.IsInside(position) || !world.GetCell(position).IsAccessible()) {
            throw std::invalid_argument{"Un herbivore doit naître sur une prairie"};
        }
        animals.push_back(std::make_unique<Herbivore>(nextId, position, energy));
        ++nextId;
    }

    void Simulation::SeedHerbivores(const int count) {
        if (count < 0) throw std::invalid_argument{"Population initiale négative"};
        if (count == 0) return;

        std::vector<Position> accessible;
        for (int y = 0; y < world.GetHeight(); ++y) {
            for (int x = 0; x < world.GetWidth(); ++x) {
                const Position position{x, y};
                if (world.GetCell(position).IsAccessible()) accessible.push_back(position);
            }
        }
        if (accessible.empty()) throw std::runtime_error{"Aucune prairie pour les herbivores"};

        std::uniform_int_distribution<std::size_t> pick{0, accessible.size() - 1};
        for (int i = 0; i < count; ++i) AddHerbivore(accessible[pick(rng)]);
    }

    Position Simulation::ChooseDestination(const Position origin) {
        Position best = origin;
        int bestVegetation = world.GetCell(origin).GetVegetation();
        int ties = 1;
        for (const Position neighbor : world.Neighbors(origin)) {
            const Cell& cell = world.GetCell(neighbor);
            if (!cell.IsAccessible()) continue;
            if (cell.GetVegetation() > bestVegetation) {
                bestVegetation = cell.GetVegetation();
                best = neighbor;
                ties = 1;
            } else if (cell.GetVegetation() == bestVegetation) {
                std::uniform_int_distribution<int> pick{1, ++ties};
                if (pick(rng) == 1) best = neighbor;
            }
        }
        return best;
    }

    void Simulation::Step() {
        GrowVegetation(world);
        std::vector<std::unique_ptr<Animal>> newborns;
        std::bernoulli_distribution birth{0.2};
        for (auto& animal : animals) {
            animal->MoveTo(ChooseDestination(animal->GetPosition()));
            animal->Feed(world);
            animal->SpendGeneration();
            if (animal->CanReproduce() && birth(rng)) {
                newborns.push_back(animal->CreateOffspring(nextId++));
                animal->PayReproductionCost();
            }
        }

        std::erase_if(animals, [](const auto& animal) { return !animal->IsAlive(); });
        for (auto& newborn : newborns) animals.push_back(std::move(newborn));
        ++generation;
    }
}
