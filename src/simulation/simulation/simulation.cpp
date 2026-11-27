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
        // La carte est déjà rangée dans le membre : les chemins sont calculés sur elle.
        : world{std::move(world)}, settings{settings}, water{this->world}, rng{std::move(rng)} {
        stats.RecordSample(Measure());
    }

    bool Simulation::HasThirst() const { return settings.thirstEnabled != 0; }

    void Simulation::MoveAndInteract() {
        const bool thirst = HasThirst();
        GridHabitat habitat{world, animals, stats, thirst ? &water : nullptr};
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
                    // Passer sur une berge suffit pour boire.
                    if (thirst && water.CanDrink(animal->GetPosition())) animal->Drink();
                    animal->Act(habitat, tick);
                }
            }
            // Les captures sont déjà faites : une collision ne sauve pas une proie atteinte.
            habitat.Separate(tick);
            habitat.FormCouples();
        }
    }

    void Simulation::GiveBirth(const Animal& mother, std::vector<std::unique_ptr<Animal>>& newborns) {
        const SpeciesTraits& traits = mother.GetTraits();
        std::uniform_int_distribution<int> litter{traits.litterMin, traits.litterMax};
        std::bernoulli_distribution female{0.5};
        const int size = litter(rng);
        for (int i = 0; i < size; ++i) {
            const Sex sex = female(rng) ? Sex::Female : Sex::Male;
            newborns.push_back(mother.CreateOffspring(nextId++, sex));
            stats.RecordBirth(mother.GetKind());
        }
    }

    void Simulation::LiveAndReproduce() {
        std::vector<std::unique_ptr<Animal>> newborns;
        for (auto& animal : animals) {
            // Les proies capturées sont déjà comptées parmi les morts.
            if (!animal->IsAlive()) continue;
            animal->SpendGeneration();
            if (HasThirst()) animal->LoseWater();
            // La femelle d'un couple met la portée au monde, si son partenaire a survécu.
            Animal* const partner = animal->GetPartner();
            if (animal->GetSex() == Sex::Female && partner && partner->IsAlive() && animal->IsAlive()) {
                std::bernoulli_distribution birth{animal->GetOptions().birthPercent / 100.0};
                if (birth(rng)) {
                    GiveBirth(*animal, newborns);
                    animal->PayReproductionCost();
                    partner->PayReproductionCost();
                }
            }
            if (!animal->IsAlive()) {
                // Une seule cause est comptée : la faim, puis la soif, puis la vieillesse.
                stats.RecordDeath(animal->GetKind(), animal->GetEnergy() <= 0 ? DeathCause::Starvation
                    : animal->GetHydration() <= 0 ? DeathCause::Thirst : DeathCause::OldAge);
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
