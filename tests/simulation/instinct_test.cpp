#include <catch2/catch_all.hpp>

import std;
import ecosim.simulation;

using namespace ecosim;

namespace {
    // Habitat de test : les espèces s'utilisent sans la simulation.
    class FakeHabitat final : public Habitat {
    public:
        World world{12, 12};
        std::vector<Point> prey;
        std::vector<Point> hunters;
        int captures{0};
        int mealPercent{100};

        World& GetWorld() override { return world; }
        const World& GetWorld() const override { return world; }
        std::optional<Point> NearestPrey(const Animal&, const Point from,
                                         const double radius) const override {
            return Closest(from, radius);
        }
        std::optional<Point> Closest(const Point from, const double radius) const {
            std::optional<Point> best;
            for (const Point point : prey) {
                if (Distance(from, point) <= radius
                    && (!best || Distance(from, point) < Distance(from, *best))) {
                    best = point;
                }
            }
            return best;
        }
        std::vector<Point> ThreatsNear(const Animal&, const Point from,
                                       const double radius) const override {
            return HuntersWithin(from, radius);
        }
        std::vector<Point> MatesNear(const Animal&, const Point from,
                                     const double radius) const override {
            return HuntersWithin(from, radius);
        }
        std::vector<Point> HuntersWithin(const Point from, const double radius) const {
            std::vector<Point> result;
            for (const Point point : hunters) {
                if (Distance(from, point) <= radius) result.push_back(point);
            }
            return result;
        }
        std::optional<int> CatchPrey(const Animal&, const Point from, const double reach) override {
            if (!Closest(from, reach)) return std::nullopt;
            ++captures;
            return mealPercent;
        }
        int ShareMeal(const Animal&, const int energy) override { return energy; }
    };

    AnimalOptions Instinct(const int value) {
        return {.instinct = value};
    }

    Steering HerbivoreSteering(const int reflex, const Point hunter) {
        FakeHabitat habitat;
        habitat.world.GetCell({5, 6}).SetVegetation(100); // l'herbe est du cote du chasseur
        habitat.hunters = {hunter};
        Deer herbivore{1, {6, 6}, 50, Instinct(reflex)};
        herbivore.Place({6.5, 6.5}, std::numbers::pi);
        std::mt19937 rng{1};
        herbivore.BeginGeneration(rng);
        return herbivore.Steer(habitat, rng);
    }

    Steering PredatorSteering(const int pack, const std::uint64_t id, const std::vector<Point>& prey,
                              const std::vector<Point>& mates) {
        FakeHabitat habitat;
        habitat.prey = prey;
        habitat.hunters = mates;
        Wolf predator{id, {2, 6}, 50, Instinct(pack)};
        predator.Place({2.5, 6.5}, 0.0);
        std::mt19937 rng{2};
        return predator.Steer(habitat, rng);
    }
}

TEST_CASE("Une proie sur ses gardes fuit a l'oppose du chasseur", "[instinct]") {
    const Steering fleeing = HerbivoreSteering(100, {5.0, 6.5});
    REQUIRE(std::cos(fleeing.heading) > 0.99);
    REQUIRE(fleeing.pace == 1.0);

    const Steering grazing = HerbivoreSteering(0, {5.0, 6.5});
    REQUIRE(std::cos(grazing.heading) < 0.0);
}

TEST_CASE("Un chasseur trop loin n'est pas repere", "[instinct]") {
    REQUIRE(HerbivoreSteering(100, {6.5 - Herbivore::ThreatSight - 0.5, 6.5}).pace < 1.0);
}

TEST_CASE("L'esprit de meute fait voir les proies de plus loin", "[instinct]") {
    const std::vector<Point> prey{{2.5 + Wolf::Sight + 2.0, 6.5}};
    REQUIRE(PredatorSteering(0, 1, prey, {}).pace < 1.0);
    const Steering pack = PredatorSteering(3, 1, prey, {});
    REQUIRE(pack.pace == 1.0);
    REQUIRE(std::cos(pack.heading) > 0.99);
}

TEST_CASE("En meute, les chasseurs visent des cotes opposes de la proie", "[instinct]") {
    const std::vector<Point> prey{{5.0, 6.5}};
    const std::vector<Point> mate{{2.5, 7.5}};
    const double even = PredatorSteering(2, 2, prey, mate).heading;
    const double odd = PredatorSteering(2, 3, prey, mate).heading;
    REQUIRE(even > 0.1);
    REQUIRE(odd == Catch::Approx(-even));
    // Seul, chacun fonce droit sur la proie.
    REQUIRE(PredatorSteering(2, 2, prey, {}).heading == Catch::Approx(0.0));
}

TEST_CASE("Sans proie, un predateur en meute rejoint un congenere", "[instinct]") {
    const Steering steering = PredatorSteering(2, 1, {}, {{2.5, 9.5}});
    REQUIRE(std::sin(steering.heading) > 0.99);
    REQUIRE(steering.pace == 0.7);
}

TEST_CASE("La meute partage le repas sans creer d'energie", "[instinct]") {
    SimulationSettings settings;
    settings.species[AnimalKind::Wolf].options.instinct = 2;
    Simulation simulation{World{1, 1}, std::mt19937{4}, settings};
    simulation.AddAnimal(AnimalKind::Deer, {0, 0});
    simulation.AddAnimal(AnimalKind::Wolf, {0, 0});
    simulation.AddAnimal(AnimalKind::Wolf, {0, 0});
    simulation.Step();

    // 25 points : 13 pour le chasseur et 12 pour son voisin, moins 2 pour la génération.
    const auto animals = simulation.Animals();
    REQUIRE(animals.size() == 2);
    REQUIRE(animals[0].energy + animals[1].energy == 61 + 60);
}

TEST_CASE("L'application active la fuite et les meutes, le moteur seul non", "[instinct]") {
    const SimulationSettings application = DefaultSettings(42, 26);
    REQUIRE(application.species[AnimalKind::Deer].options.instinct == 25);
    REQUIRE(application.species[AnimalKind::Wolf].options.instinct == 2);
    for (const SpeciesSettings& species : SimulationSettings{}.species.values) {
        REQUIRE(species.options.instinct == 0);
    }
}
