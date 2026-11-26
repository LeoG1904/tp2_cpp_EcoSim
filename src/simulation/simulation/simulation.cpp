/**
 * @file
 * @brief Le cycle d'une génération et la recherche des voisins.
 */
module ecosim.simulation;

import std;

namespace {
    // Vue offerte aux animaux pendant une génération. Ils sont rangés par case,
    // pour ne chercher leurs voisins que dans les cases proches.
    class GridHabitat final : public ecosim::Habitat {
    public:
        GridHabitat(ecosim::World& world, std::vector<std::unique_ptr<ecosim::Animal>>& animals,
                    ecosim::SimulationStats& stats)
            : world{world}, stats{stats},
              buckets(static_cast<std::size_t>(world.GetWidth()) * world.GetHeight()) {
            for (const auto& animal : animals) {
                if (animal->IsAlive()) buckets[Index(animal->GetPosition())].push_back(animal.get());
            }
        }

        // À appeler après le déplacement d'un animal.
        void Moved(ecosim::Animal& animal, const ecosim::Position from) {
            if (animal.GetPosition() == from) return;
            std::erase(buckets[Index(from)], &animal);
            buckets[Index(animal.GetPosition())].push_back(&animal);
        }

        ecosim::World& GetWorld() override { return world; }
        const ecosim::World& GetWorld() const override { return world; }

        std::optional<ecosim::Point> NearestPrey(const ecosim::Point from,
                                                 const double radius) const override {
            const ecosim::Animal* prey = ClosestPrey(from, radius);
            if (!prey) return std::nullopt;
            return prey->GetLocation();
        }

        std::vector<ecosim::Point> HuntersNear(const ecosim::Point from, const double radius,
                                               const ecosim::Animal* except) const override {
            std::vector<ecosim::Point> result;
            ForEachNear(from, radius, [&](ecosim::Animal& other, double) {
                if (&other != except && !other.IsPrey()) result.push_back(other.GetLocation());
            });
            return result;
        }

        bool CatchPrey(const ecosim::Point from, const double reach) override {
            ecosim::Animal* prey = ClosestPrey(from, reach);
            if (!prey) return false;
            prey->Die();
            stats.RecordDeath(prey->GetKind(), ecosim::DeathCause::Predation);
            return true;
        }

        int ShareMeal(const ecosim::Animal& hunter, const int energy) override {
            std::vector<ecosim::Animal*> mates;
            ForEachNear(hunter.GetLocation(), ecosim::Predator::SharingDistance,
                        [&](ecosim::Animal& other, double) {
                if (&other != &hunter && other.GetKind() == hunter.GetKind()) mates.push_back(&other);
            });
            const int share = energy / (static_cast<int>(mates.size()) + 1);
            for (ecosim::Animal* mate : mates) mate->ReceiveMeal(share);
            return energy - share * static_cast<int>(mates.size());
        }

    private:
        std::size_t Index(const ecosim::Position position) const {
            return static_cast<std::size_t>(position.y) * world.GetWidth()
                 + static_cast<std::size_t>(position.x);
        }

        // Appelle visit pour chaque animal vivant à moins de radius.
        template <typename Visit>
        void ForEachNear(const ecosim::Point from, const double radius, const Visit& visit) const {
            const ecosim::Position low = ecosim::CellAt({from.x - radius, from.y - radius});
            const ecosim::Position high = ecosim::CellAt({from.x + radius, from.y + radius});
            for (int y = std::max(0, low.y); y <= std::min(world.GetHeight() - 1, high.y); ++y) {
                for (int x = std::max(0, low.x); x <= std::min(world.GetWidth() - 1, high.x); ++x) {
                    for (ecosim::Animal* animal : buckets[Index({x, y})]) {
                        if (!animal->IsAlive()) continue;
                        const double distance = ecosim::Distance(from, animal->GetLocation());
                        if (distance <= radius) visit(*animal, distance);
                    }
                }
            }
        }

        ecosim::Animal* ClosestPrey(const ecosim::Point from, const double radius) const {
            ecosim::Animal* closest = nullptr;
            double best = std::numeric_limits<double>::infinity();
            ForEachNear(from, radius, [&](ecosim::Animal& other, const double distance) {
                if (other.IsPrey() && distance < best) {
                    closest = &other;
                    best = distance;
                }
            });
            return closest;
        }

        ecosim::World& world;
        ecosim::SimulationStats& stats;
        std::vector<std::vector<ecosim::Animal*>> buckets;
    };
}

namespace ecosim {
    void GrowVegetation(World& world, const int growth) {
        for (int y = 0; y < world.GetHeight(); ++y) {
            for (int x = 0; x < world.GetWidth(); ++x) {
                Cell& cell = world.GetCell(Position{x, y});
                if (cell.GetTerrain() == Terrain::Prairie) {
                    cell.SetVegetation(cell.GetVegetation() + growth);
                }
            }
        }
    }

    Simulation::Simulation(World world, std::mt19937 rng, const SimulationSettings& settings)
        : world{std::move(world)}, settings{settings}, rng{std::move(rng)} {
        stats.RecordSample(Measure());
    }

    void Simulation::MoveAndInteract() {
        GridHabitat habitat{world, animals, stats};
        for (auto& animal : animals) {
            animal->StartPath();
            animal->BeginGeneration(rng);
        }
        for (int tick = 0; tick < TicksPerGeneration; ++tick) {
            // Les proies jouent d'abord, puis les chasseurs.
            for (const bool preyTurn : {true, false}) {
                for (auto& animal : animals) {
                    if (animal->IsPrey() != preyTurn || !animal->IsAlive()) continue;
                    const Position before = animal->GetPosition();
                    animal->Advance(world, animal->Steer(habitat, rng), tick);
                    habitat.Moved(*animal, before);
                    animal->Act(habitat, tick);
                }
            }
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
        MoveAndInteract();
        LiveAndReproduce();
        ++generation;
        stats.RecordSample(Measure());
    }
}
