#include <catch2/catch_all.hpp>

import std;
import ecosim.deer;
import ecosim.rabbit;
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
        int mealPercent{100};

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
        std::optional<int> CatchPrey(const Point from, const double reach) override {
            if (!NearestPrey(from, reach)) return std::nullopt;
            ++captures;
            return mealPercent;
        }
        int ShareMeal(const Animal&, const int energy) override { return energy; }
    };
}

TEST_CASE("Chaque espece se nourrit a sa facon", "[species]") {
    FakeHabitat habitat;
    habitat.world.GetCell({6, 6}).SetVegetation(80);
    habitat.prey = {{6.7, 6.5}};
    Deer herbivore{1, {6, 6}};
    Predator predator{2, {6, 6}};

    herbivore.Act(habitat, 0);
    REQUIRE(herbivore.GetEnergy() == 50); // il ne mange qu'au dernier petit pas
    herbivore.Act(habitat, TicksPerGeneration - 1);
    REQUIRE(herbivore.GetEnergy() == 60);

    predator.Act(habitat, 3);
    predator.Act(habitat, 4); // une seule capture par generation
    REQUIRE(predator.GetEnergy() == 75);
    REQUIRE(habitat.captures == 1);
}

TEST_CASE("Un lapin offre un repas deux fois plus petit qu'un cerf", "[species]") {
    const Deer deer{1, {0, 0}};
    const Rabbit rabbit{2, {0, 0}};
    REQUIRE(deer.GetTraits().mealPercent == 100);
    REQUIRE(rabbit.GetTraits().mealPercent == 50);

    FakeHabitat habitat;
    habitat.prey = {{6.7, 6.5}};
    habitat.mealPercent = rabbit.GetTraits().mealPercent;
    Predator predator{3, {6, 6}};
    predator.Act(habitat, 0);
    REQUIRE(predator.GetEnergy()
            == 50 + Predator::Traits.foodEnergy * Rabbit::Traits.mealPercent / 100);
}

TEST_CASE("Sur une herbe rase, le lapin reste et le cerf repart", "[species]") {
    FakeHabitat habitat;
    const int shortGrass = (Rabbit::GrazingThreshold + Deer::GrazingThreshold) / 2;
    for (int y = 0; y < habitat.world.GetHeight(); ++y) {
        for (int x = 0; x < habitat.world.GetWidth(); ++x) {
            habitat.world.GetCell({x, y}).SetVegetation(shortGrass);
        }
    }
    std::mt19937 rng{1};
    const Deer deer{1, {6, 6}};
    const Rabbit rabbit{2, {6, 6}};
    REQUIRE(rabbit.Steer(habitat, rng).pace < deer.Steer(habitat, rng).pace);
}

TEST_CASE("Le petit d'un lapin est un lapin", "[species]") {
    const Rabbit parent{4, {0, 0}, 90};
    const auto child = parent.CreateOffspring(5);
    REQUIRE(child->GetKind() == AnimalKind::Rabbit);
    REQUIRE(child->GetTraits() == Rabbit::Traits);
}

TEST_CASE("Les cerfs et les lapins sont des proies, pas les predateurs", "[species]") {
    const Deer deer{1, {0, 0}};
    const Rabbit rabbit{3, {0, 0}};
    const Predator predator{2, {0, 0}};
    REQUIRE(deer.IsPrey());
    REQUIRE(rabbit.IsPrey());
    REQUIRE_FALSE(predator.IsPrey());
}

TEST_CASE("Un descendant connait son parent et nait aupres de lui", "[species]") {
    Deer parent{7, {0, 0}, 90};
    parent.Place({0.2, 0.8}, 1.0);
    REQUIRE_FALSE(parent.GetParentId().has_value());
    const auto child = parent.CreateOffspring(8);
    REQUIRE(child->GetParentId() == 7);
    REQUIRE(child->GetLocation() == parent.GetLocation());
}
