module ecosim.simulation;

import std;

namespace {
    // Vue offerte aux animaux pendant une phase ; les proies sont comptées à son début.
    class PhaseHabitat final : public ecosim::Habitat {
    public:
        PhaseHabitat(ecosim::World& world, std::vector<std::unique_ptr<ecosim::Animal>>& animals,
                     ecosim::SimulationStats& stats)
            : world{world}, animals{animals}, stats{stats},
              preyCounts(static_cast<std::size_t>(world.GetWidth()) * world.GetHeight()),
              hunterCounts(preyCounts.size()) {
            for (const auto& animal : animals) {
                if (!animal->IsAlive()) continue;
                if (animal->IsPrey()) ++preyCounts[Index(animal->GetPosition())];
                if (animal->IsHunter()) ++hunterCounts[Index(animal->GetPosition())];
            }
        }

        int HunterCount(const ecosim::Position position) const override {
            return hunterCounts[Index(position)];
        }

        // Le repas est divisé avec les membres de la meute situés à un pas au plus.
        int ShareMeal(const ecosim::Animal& hunter, const int energy) override {
            std::vector<ecosim::Animal*> mates;
            for (const auto& other : animals) {
                if (other.get() == &hunter || !other->IsAlive() || other->GetKind() != hunter.GetKind()) {
                    continue;
                }
                const ecosim::Position a = other->GetPosition();
                const ecosim::Position b = hunter.GetPosition();
                if (std::abs(a.x - b.x) + std::abs(a.y - b.y) <= 1) mates.push_back(other.get());
            }
            const int share = energy / (static_cast<int>(mates.size()) + 1);
            for (ecosim::Animal* mate : mates) mate->ReceiveMeal(share);
            return energy - share * static_cast<int>(mates.size());
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
                    stats.RecordDeath(prey->GetKind(), ecosim::DeathCause::Predation);
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
        ecosim::SimulationStats& stats;
        std::vector<int> preyCounts;
        std::vector<int> hunterCounts;
    };
}

namespace ecosim {
    Simulation::Simulation(World world, std::mt19937 rng, const SimulationSettings& settings)
        : world{std::move(world)}, settings{settings}, rng{std::move(rng)} {
        stats.RecordSample(Measure());
    }

    void Simulation::PlayPhase(const TurnPhase phase) {
        PhaseHabitat habitat{world, animals, stats};
        for (auto& animal : animals) {
            if (animal->Phase() != phase || !animal->IsAlive()) continue;
            animal->MoveTo(animal->ChooseDestination(habitat, rng));
            animal->Feed(habitat);
        }
    }

    void Simulation::LiveAndReproduce() {
        std::vector<std::unique_ptr<Animal>> newborns;
        for (auto& animal : animals) {
            // Les proies capturées sont déjà comptées parmi les morts.
            if (!animal->IsAlive()) continue;
            animal->SpendGeneration();
            std::bernoulli_distribution birth{animal->GetTraits().reproductionPercent / 100.0};
            if (animal->CanReproduce() && birth(rng)) {
                newborns.push_back(animal->CreateOffspring(nextId++));
                animal->PayReproductionCost();
                stats.RecordBirth(animal->GetKind());
            }
            if (!animal->IsAlive()) {
                // Un animal à la fois épuisé et âgé est compté comme mort de faim.
                stats.RecordDeath(animal->GetKind(), animal->GetEnergy() <= 0
                    ? DeathCause::Starvation : DeathCause::OldAge);
            }
        }

        std::erase_if(animals, [](const auto& animal) { return !animal->IsAlive(); });
        for (auto& newborn : newborns) animals.push_back(std::move(newborn));
    }

    void Simulation::Step() {
        if (IsFinished()) return;
        stats.BeginGeneration();
        GrowVegetation(world, settings.vegetationGrowth);
        // Les herbivores se déplacent et mangent avant la chasse.
        for (const TurnPhase phase : TurnPhases) PlayPhase(phase);
        LiveAndReproduce();
        ++generation;
        stats.RecordSample(Measure());
    }
}
