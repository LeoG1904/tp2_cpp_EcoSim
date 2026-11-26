#include <catch2/catch_all.hpp>

import std;
import ecosim.animation;

using namespace ecosim;

namespace {
    // Trajet rectiligne régulier d'un point à un autre.
    Path Straight(const Point from, const Point to) {
        Path path;
        for (std::size_t i = 0; i < path.size(); ++i) {
            const double part = static_cast<double>(i) / TicksPerGeneration;
            path[i] = {std::lerp(from.x, to.x, part), std::lerp(from.y, to.y, part)};
        }
        return path;
    }
}

TEST_CASE("La position et la direction se lisent le long du trajet", "[animation]") {
    Path path = Straight({1.0, 1.0}, {2.0, 1.0});
    REQUIRE(PathPoint(path, 0.0) == Point{1.0, 1.0});
    REQUIRE(PathPoint(path, 0.5).x == Catch::Approx(1.5));
    REQUIRE(PathPoint(path, 1.0) == Point{2.0, 1.0});
    REQUIRE(PathHeading(path, 0.5) == Catch::Approx(0.0));

    // Arrêté à mi-chemin, l'animal garde sa dernière direction.
    for (std::size_t i = 5; i < path.size(); ++i) path[i] = path[5];
    REQUIRE(PathPoint(path, 0.9) == path[5]);
    REQUIRE(PathHeading(path, 0.9) == Catch::Approx(0.0));
    REQUIRE(PathHeading(Straight({1, 1}, {1, 3}), 0.2) == Catch::Approx(std::numbers::pi / 2));
}

TEST_CASE("A l'ecran, un nouveau-ne accompagne son parent", "[animation]") {
    const Path parentTrip = Straight({0.5, 0.5}, {1.5, 0.5});
    const Path still = Straight({1.5, 0.5}, {1.5, 0.5});
    std::vector<AnimalSnapshot> animals{
        {1, {1, 0}, AnimalKind::Herbivore, 9, 50, parentTrip, std::nullopt},
        {2, {1, 0}, AnimalKind::Herbivore, 0, 30, still, 1},
        {3, {1, 0}, AnimalKind::Herbivore, 4, 40, still, 1},
    };

    AttachNewbornsToParents(animals);

    REQUIRE(animals[1].path == parentTrip);
    REQUIRE(animals[2].path == still);
}
