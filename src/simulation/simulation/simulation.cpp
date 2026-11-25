module ecosim.simulation;

import std;

namespace {
    // Vue offerte aux animaux pendant une phase ; les proies sont comptées à son début.
    class PhaseHabitat final : public ecosim::Habitat {
    public:
        PhaseHabitat(ecosim::World& world, std::vector<std::unique_ptr<ecosim::Animal>>& animals)
            : world{world}, animals{animals},
              preyCounts(static_cast<std::size_t>(world.GetWidth()) * world.GetHeight()) {
            for (const auto& animal : animals) {
                if (animal->IsPrey() && animal->IsAlive()) ++preyCounts[Index(animal->GetPosition())];
            }
        }

        ecosim::World& GetWorld() override { return world; }
        const ecosim::World& GetWorld() const override { return world; }

        int PreyCount(const ecosim::Position position) const override {
            return preyCounts[Index(position)];
        }

        bool CatchPrey(const ecosim::Position position) override {
            for (const auto& prey : animals) {
                if (prey->IsPrey() && prey->IsAlive() && prey->GetPosition() == position) {
                    prey->Die();
                    return true;
                }
            }
            return false;
        }

    private:
        std::size_t Index(const ecosim::Position position) const {
            return static_cast<std::size_t>(position.y) * world.GetWidth()
                 + static_cast<std::size_t>(position.x);
        }

        ecosim::World& world;
        std::vector<std::unique_ptr<ecosim::Animal>>& animals;
        std::vector<int> preyCounts;
    };
}

namespace ecosim {
    Simulation::Simulation(World world, std::mt19937 rng, const SimulationSettings& settings)
        : world{std::move(world)}, settings{settings}, rng{std::move(rng)} {}

    void Simulation::PlayPhase(const TurnPhase phase) {
        PhaseHabitat habitat{world, animals};
        for (auto& animal : animals) {
            if (animal->Phase() != phase || !animal->IsAlive()) continue;
            animal->MoveTo(animal->ChooseDestination(habitat, rng));
            animal->Feed(habitat);
        }
    }

    void Simulation::Step() {
        if (IsFinished()) return;
        GrowVegetation(world, settings.vegetationGrowth);
        // Les herbivores se déplacent et mangent avant la chasse.
        for (const TurnPhase phase : TurnPhases) PlayPhase(phase);

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
