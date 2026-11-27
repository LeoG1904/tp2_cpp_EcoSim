/**
 * @file
 * @brief Le cycle d'une génération.
 */
module ecosim.simulation;

import std;
import ecosim.grid_habitat;

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
            // Les captures sont déjà faites : une collision ne sauve pas une proie atteinte.
            habitat.Separate(tick);
        }
    }

    void Simulation::LiveAndReproduce() {
        std::vector<std::unique_ptr<Animal>> newborns;
        for (auto& animal : animals) {
            // Les proies capturées sont déjà comptées parmi les morts.
            if (!animal->IsAlive()) continue;
            animal->SpendGeneration();
            std::bernoulli_distribution birth{animal->GetOptions().birthPercent / 100.0};
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
