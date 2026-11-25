#include <catch2/catch_all.hpp>

import std;
import ecosim.simulation;

using namespace ecosim;

namespace {
    // Monde en ligne : le chasseur reste immobile pendant que l'on observe la proie.
    class LineHabitat final : public Habitat {
    public:
        World world{8, 1};
        std::vector<Position> hunters;
        std::vector<Position> prey;

        World& GetWorld() override { return world; }
        const World& GetWorld() const override { return world; }
        int PreyCount(const Position p) const override { return Count(prey, p); }
        int HunterCount(const Position p) const override { return Count(hunters, p); }
        bool CatchPrey(Position) override { return false; }
        int ShareMeal(const Animal&, const int energy) override { return energy; }

    private:
        static int Count(const std::vector<Position>& all, const Position p) {
            return static_cast<int>(std::ranges::count(all, p));
        }
    };

    AnimalTraits Instinct(AnimalTraits traits, const int value) {
        traits.instinct = value;
        return traits;
    }

    Position HerbivoreChoice(const int reflex, const Position hunter) {
        LineHabitat habitat;
        habitat.world.GetCell({2, 0}).SetVegetation(80); // la nourriture est du côté du chasseur
        habitat.hunters = {hunter};
        const Herbivore herbivore{1, {3, 0}, 50, Instinct(AnimalTraits{}, reflex)};
        std::mt19937 rng{1};
        return herbivore.ChooseDestination(habitat, rng);
    }
}

TEST_CASE("Une proie qui fuit s'eloigne du chasseur malgre la nourriture", "[instinct]") {
    REQUIRE(HerbivoreChoice(0, {1, 0}) == Position{2, 0});
    REQUIRE(HerbivoreChoice(100, {1, 0}) == Position{4, 0});
}

TEST_CASE("Un chasseur trop loin n'est pas repere", "[instinct]") {
    REQUIRE(HerbivoreChoice(100, {3 - Herbivore::ThreatSight - 1, 0}) == Position{2, 0});
}

TEST_CASE("En meute, un predateur suit la piste d'une proie lointaine", "[instinct]") {
    LineHabitat habitat;
    habitat.prey = {{6, 0}};
    habitat.hunters = {{3, 0}};
    std::mt19937 rng{2};
    const Predator predator{1, {3, 0}, 50, Instinct(Predator::DefaultTraits, 3)};
    REQUIRE(predator.ChooseDestination(habitat, rng) == Position{4, 0});
}

TEST_CASE("Sans proie, un predateur en meute rejoint un congenere", "[instinct]") {
    LineHabitat habitat;
    habitat.hunters = {{3, 0}, {1, 0}};
    std::mt19937 rng{3};
    const Predator predator{1, {3, 0}, 50, Instinct(Predator::DefaultTraits, 3)};
    REQUIRE(predator.ChooseDestination(habitat, rng) == Position{2, 0});
}

TEST_CASE("La meute partage le repas sans creer d'energie", "[instinct]") {
    SimulationSettings settings;
    settings.species[AnimalKind::Predator].traits.instinct = 2;
    Simulation simulation{World{1, 1}, std::mt19937{4}, settings};
    simulation.AddAnimal(AnimalKind::Herbivore, {0, 0});
    simulation.AddAnimal(AnimalKind::Predator, {0, 0});
    simulation.AddAnimal(AnimalKind::Predator, {0, 0});
    simulation.Step();

    // 35 points : 18 pour le chasseur et 17 pour son voisin, moins 2 pour la génération.
    const auto animals = simulation.Animals();
    REQUIRE(animals.size() == 2);
    REQUIRE(animals[0].energy == 66);
    REQUIRE(animals[1].energy == 65);
}

TEST_CASE("L'application active la fuite et les meutes, le moteur seul non", "[instinct]") {
    const SimulationSettings application = DefaultSettings(42, 26);
    REQUIRE(application.species[AnimalKind::Herbivore].traits.instinct == 25);
    REQUIRE(application.species[AnimalKind::Predator].traits.instinct == 2);
    for (const SpeciesSettings& species : SimulationSettings{}.species.values) {
        REQUIRE(species.traits.instinct == 0);
    }
}
