#include <catch2/catch_all.hpp>

import std;
import ecosim.herbivore;
import ecosim.predator;

using namespace ecosim;

namespace {
    // Habitat de test : les espèces s'utilisent sans la simulation.
    class FakeHabitat final : public Habitat {
    public:
        World world{12, 12};
        std::vector<Point> prey;
        std::vector<Point> hunters;
        int captures{0};

        World& GetWorld() override { return world; }
        const World& GetWorld() const override { return world; }
        std::optional<Point> NearestPrey(const Point from, const double radius) const override {
            std::optional<Point> best;
            for (const Point point : prey) {
                if (Distance(from, point) <= radius
                    && (!best || Distance(from, point) < Distance(from, *best))) {
                    best = point;
                }
            }
            return best;
        }
        std::vector<Point> HuntersNear(const Point from, const double radius,
                                       const Animal*) const override {
            std::vector<Point> result;
            for (const Point point : hunters) {
                if (Distance(from, point) <= radius) result.push_back(point);
            }
            return result;
        }
        bool CatchPrey(const Point from, const double reach) override {
            if (!NearestPrey(from, reach)) return false;
            ++captures;
            return true;
        }
        int ShareMeal(const Animal&, const int energy) override { return energy; }
    };
}

TEST_CASE("Chaque espece se nourrit a sa facon", "[species]") {
    FakeHabitat habitat;
    habitat.world.GetCell({6, 6}).SetVegetation(80);
    habitat.prey = {{6.7, 6.5}};
    Herbivore herbivore{1, {6, 6}};
    Predator predator{2, {6, 6}};

    herbivore.Act(habitat, 0);
    REQUIRE(herbivore.GetEnergy() == 50); // il ne mange qu'au dernier petit pas
    herbivore.Act(habitat, TicksPerGeneration - 1);
    REQUIRE(herbivore.GetEnergy() == 60);

    predator.Act(habitat, 3);
    predator.Act(habitat, 4); // une seule capture par generation
    REQUIRE(predator.GetEnergy() == 85);
    REQUIRE(habitat.captures == 1);
}

TEST_CASE("Seuls les herbivores sont des proies", "[species]") {
    const Herbivore herbivore{1, {0, 0}};
    const Predator predator{2, {0, 0}};
    REQUIRE(herbivore.IsPrey());
    REQUIRE_FALSE(predator.IsPrey());
}

TEST_CASE("Un descendant connait son parent et nait aupres de lui", "[species]") {
    Herbivore parent{7, {0, 0}, 90};
    parent.Place({0.2, 0.8}, 1.0);
    REQUIRE_FALSE(parent.GetParentId().has_value());
    const auto child = parent.CreateOffspring(8);
    REQUIRE(child->GetParentId() == 7);
    REQUIRE(child->GetLocation() == parent.GetLocation());
}
