#include <catch2/catch_all.hpp>

import std;
import ecosim.grid_habitat;
import ecosim.simulation;

using namespace ecosim;

namespace {
    // Des animaux poses a la main, puis ranges dans un habitat.
    struct Crowd {
        World world{12, 12};
        SimulationStats stats;
        std::vector<std::unique_ptr<Animal>> animals;

        template <typename Species>
        Animal& Add(const Point point) {
            animals.push_back(std::make_unique<Species>(animals.size() + 1, CellAt(point)));
            animals.back()->Place(point, 0.0);
            return *animals.back();
        }
        // Un petit pas d'ecartement, range comme dans la simulation.
        void Separate(const int times = 1) {
            for (int i = 0; i < times; ++i) {
                GridHabitat habitat{world, animals, stats};
                habitat.Separate(0);
            }
        }
    };

    double Touching(const Animal& a, const Animal& b) {
        return a.GetTraits().bodyRadius + b.GetTraits().bodyRadius;
    }
}

TEST_CASE("Deux animaux superposes sont ecartes jusqu'a se toucher", "[collision]") {
    Crowd crowd;
    Animal& a = crowd.Add<Deer>({6.0, 6.0});
    Animal& b = crowd.Add<Deer>({6.1, 6.0});
    crowd.Separate(5);
    REQUIRE(Distance(a.GetLocation(), b.GetLocation()) == Catch::Approx(Touching(a, b)));
    // Deux corps egaux reculent autant : leur milieu ne bouge pas.
    REQUIRE((a.GetLocation().x + b.GetLocation().x) / 2.0 == Catch::Approx(6.05));
    REQUIRE(a.GetLocation().y == Catch::Approx(6.0));
}

TEST_CASE("Le petit corps recule davantage que le gros", "[collision]") {
    Crowd crowd;
    Animal& wolf = crowd.Add<Wolf>({6.0, 6.0});
    Animal& rabbit = crowd.Add<Rabbit>({6.2, 6.0});
    crowd.Separate();
    const double wolfMove = 6.0 - wolf.GetLocation().x;
    const double rabbitMove = rabbit.GetLocation().x - 6.2;
    REQUIRE(wolfMove > 0.0);
    REQUIRE(rabbitMove > wolfMove);
}

TEST_CASE("Deux animaux au meme point partent en sens opposes, toujours de la meme facon", "[collision]") {
    const auto separate = [] {
        Crowd crowd;
        crowd.Add<Deer>({6.5, 6.5});
        crowd.Add<Deer>({6.5, 6.5});
        crowd.Separate();
        return std::pair{crowd.animals[0]->GetLocation(), crowd.animals[1]->GetLocation()};
    };
    const auto [first, second] = separate();
    REQUIRE(Distance(first, second) > 0.0);
    REQUIRE((first.x + second.x) / 2.0 == Catch::Approx(6.5));
    REQUIRE(separate() == std::pair{first, second});
}

TEST_CASE("Des animaux qui ne se touchent pas ne bougent pas", "[collision]") {
    Crowd crowd;
    Animal& a = crowd.Add<Deer>({6.0, 6.0});
    crowd.Add<Deer>({7.0, 6.0});
    crowd.Separate();
    REQUIRE(a.GetLocation() == Point{6.0, 6.0});
}

TEST_CASE("Une poussee reste limitee, meme dans une foule", "[collision]") {
    Crowd crowd;
    for (int i = 0; i < 12; ++i) crowd.Add<Deer>({6.0 + 0.001 * i, 6.0});
    std::vector<Point> before;
    for (const auto& animal : crowd.animals) before.push_back(animal->GetLocation());
    crowd.Separate();
    for (std::size_t i = 0; i < before.size(); ++i) {
        REQUIRE(Distance(before[i], crowd.animals[i]->GetLocation()) <= MaxPushPerTick + 1e-9);
    }
}

TEST_CASE("Personne n'est pousse dans l'eau ni hors de la carte", "[collision]") {
    Crowd crowd;
    for (int y = 0; y < 12; ++y) crowd.world.GetCell({4, y}) = Cell{Terrain::Eau};
    for (int i = 0; i < 6; ++i) crowd.Add<Deer>({5.13 + 0.01 * i, 0.13 + 0.01 * i});
    for (int round = 0; round < 30; ++round) {
        crowd.Separate();
        for (const auto& animal : crowd.animals) {
            REQUIRE(crowd.world.IsWalkable(animal->GetLocation()));
        }
    }
}

TEST_CASE("La poussee est inscrite dans le trajet", "[collision]") {
    Crowd crowd;
    Animal& a = crowd.Add<Deer>({6.0, 6.0});
    crowd.Add<Deer>({6.1, 6.0});
    crowd.Separate();
    REQUIRE(a.GetPath().back() == a.GetLocation());
    REQUIRE(a.GetPath().front() == Point{6.0, 6.0});
}

TEST_CASE("Un chasseur qui touche sa proie peut toujours l'attraper", "[collision]") {
    for (const SpeciesInfo& hunter : SpeciesCatalog()) {
        for (const SpeciesInfo& prey : SpeciesCatalog()) {
            if (hunter.canBeEaten || !prey.canBeEaten) continue;
            REQUIRE(hunter.bodyRadius + prey.bodyRadius < Predator::Reach);
        }
    }
    for (const SpeciesInfo& species : SpeciesCatalog()) REQUIRE(species.bodyRadius <= MaxBodyRadius);
}
