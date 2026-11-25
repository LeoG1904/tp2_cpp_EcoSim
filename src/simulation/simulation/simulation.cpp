module ecosim.simulation;

import std;

namespace ecosim {
    Simulation::Simulation(World world, std::mt19937 rng, const SimulationSettings& settings)
        : world{std::move(world)}, settings{settings}, rng{std::move(rng)} {}

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

    Position Simulation::ChoosePredatorDestination(const Position origin,
                                                   const std::vector<int>& preyCounts) {
        Position best = origin;
        int bestCount = -1;
        int ties = 0;
        const auto consider = [&](const Position position) {
            if (!world.GetCell(position).IsAccessible()) return;
            const auto index = static_cast<std::size_t>(position.y) * world.GetWidth()
                             + static_cast<std::size_t>(position.x);
            const int count = preyCounts[index];
            if (count > bestCount) {
                bestCount = count;
                best = position;
                ties = 1;
            } else if (count == bestCount) {
                std::uniform_int_distribution<int> pick{1, ++ties};
                if (pick(rng) == 1) best = position;
            }
        };
        consider(origin);
        for (const Position neighbor : world.Neighbors(origin)) consider(neighbor);
        return best;
    }

    void Simulation::Step() {
        if (IsFinished()) return;
        GrowVegetation(world, settings.vegetationGrowth);
        // Les herbivores se déplacent et mangent avant la chasse.
        for (auto& animal : animals) {
            auto* herbivore = dynamic_cast<Herbivore*>(animal.get());
            if (!herbivore) continue;
            herbivore->MoveTo(ChooseDestination(herbivore->GetPosition()));
            herbivore->Feed(world);
        }

        std::vector<int> preyCounts(static_cast<std::size_t>(world.GetWidth()) * world.GetHeight());
        for (const auto& animal : animals) {
            if (animal->GetKind() != AnimalKind::Herbivore) continue;
            const Position position = animal->GetPosition();
            ++preyCounts[static_cast<std::size_t>(position.y) * world.GetWidth() + position.x];
        }

        // Chaque prédateur vise une case avec des proies et ne chasse qu'une fois.
        for (auto& animal : animals) {
            auto* predator = dynamic_cast<Predator*>(animal.get());
            if (!predator) continue;
            predator->MoveTo(ChoosePredatorDestination(predator->GetPosition(), preyCounts));
            predator->Hunt(animals);
        }

        std::vector<std::unique_ptr<Animal>> newborns;
        for (auto& animal : animals) {
            if (!animal->IsAlive()) continue;
            animal->SpendGeneration();
            std::bernoulli_distribution birth{animal->GetTraits().reproductionPercent / 100.0};
            if (animal->CanReproduce() && birth(rng)) {
                newborns.push_back(animal->CreateOffspring(nextId++));
                animal->PayReproductionCost();
            }
        }

        std::erase_if(animals, [](const auto& animal) { return !animal->IsAlive(); });
        for (auto& newborn : newborns) animals.push_back(std::move(newborn));
        ++generation;
        populationHistory.push_back({generation, HerbivoreCount(), PredatorCount()});
    }
}
