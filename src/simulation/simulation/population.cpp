module ecosim.simulation;

import std;

namespace ecosim {
    const World& Simulation::GetWorld() const { return world; }
    int Simulation::GetGeneration() const { return generation; }

    std::size_t Simulation::HerbivoreCount() const {
        return static_cast<std::size_t>(std::ranges::count_if(animals, [](const auto& animal) {
            return animal->GetKind() == AnimalKind::Herbivore;
        }));
    }

    std::size_t Simulation::PredatorCount() const {
        return static_cast<std::size_t>(std::ranges::count_if(animals, [](const auto& animal) {
            return animal->GetKind() == AnimalKind::Predator;
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

    const std::vector<PopulationSample>& Simulation::PopulationHistory() const {
        return populationHistory;
    }

    void Simulation::AddHerbivore(const Position position, const int energy) {
        if (!world.IsInside(position) || !world.GetCell(position).IsAccessible()) {
            throw std::invalid_argument{"Un herbivore doit naître sur une prairie"};
        }
        animals.push_back(std::make_unique<Herbivore>(nextId, position, energy));
        ++nextId;
        ++populationHistory.back().herbivores;
    }

    void Simulation::SeedHerbivores(const int count) {
        SeedAnimals(count, AnimalKind::Herbivore);
    }

    void Simulation::AddPredator(const Position position, const int energy) {
        if (!world.IsInside(position) || !world.GetCell(position).IsAccessible()) {
            throw std::invalid_argument{"Un prédateur doit naître sur une prairie"};
        }
        animals.push_back(std::make_unique<Predator>(nextId, position, energy));
        ++nextId;
        ++populationHistory.back().predators;
    }

    void Simulation::SeedPredators(const int count) {
        SeedAnimals(count, AnimalKind::Predator);
    }

    void Simulation::SeedAnimals(const int count, const AnimalKind kind) {
        if (count < 0) throw std::invalid_argument{"Population initiale négative"};
        if (count == 0) return;

        std::vector<Position> accessible;
        for (int y = 0; y < world.GetHeight(); ++y) {
            for (int x = 0; x < world.GetWidth(); ++x) {
                const Position position{x, y};
                if (world.GetCell(position).IsAccessible()) accessible.push_back(position);
            }
        }
        if (accessible.empty()) throw std::runtime_error{"Aucune prairie pour les animaux"};

        std::uniform_int_distribution<std::size_t> pick{0, accessible.size() - 1};
        for (int i = 0; i < count; ++i) {
            const Position position = accessible[pick(rng)];
            if (kind == AnimalKind::Herbivore) AddHerbivore(position);
            else AddPredator(position);
        }
    }
}
