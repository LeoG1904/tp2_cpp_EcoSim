#include <catch2/catch_all.hpp>

// Le main des tests est fourni par Catch2WithMain (voir CMakeLists.txt)
import std;
import ecosim.world;
import ecosim.generation;

using namespace ecosim;

TEST_CASE("La carte générée a les dimensions demandées", "[generation]") {
    std::mt19937 rng{42};
    const World world = GenerateRandomWorld(30, 20, rng);
    REQUIRE(world.GetWidth() == 30);
    REQUIRE(world.GetHeight() == 20);
}

TEST_CASE("Même graine, même carte", "[generation]") {
    std::mt19937 rngA{7};
    std::mt19937 rngB{7};
    const World a = GenerateRandomWorld(10, 10, rngA);
    const World b = GenerateRandomWorld(10, 10, rngB);

    for (int y = 0; y < 10; ++y) {
        for (int x = 0; x < 10; ++x) {
            const Cell& ca = a.GetCell(Position{x, y});
            const Cell& cb = b.GetCell(Position{x, y});
            REQUIRE(ca.GetTerrain() == cb.GetTerrain());
            REQUIRE(ca.GetVegetation() == cb.GetVegetation());
        }
    }
}
