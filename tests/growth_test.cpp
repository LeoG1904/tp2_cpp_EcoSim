#include <catch2/catch_all.hpp>

// Le main des tests est fourni par Catch2WithMain (voir CMakeLists.txt)
import ecosim.world;
import ecosim.growth;

using namespace ecosim;

TEST_CASE("Une prairie gagne 1 de végétation par génération", "[growth]") {
    World world{2, 2};
    GrowVegetation(world);
    REQUIRE(world.GetCell(Position{0, 0}).GetVegetation() == 1);

    GrowVegetation(world);
    GrowVegetation(world);
    REQUIRE(world.GetCell(Position{1, 1}).GetVegetation() == 3);
}

TEST_CASE("La végétation ne dépasse pas 100", "[growth]") {
    World world{1, 1};
    world.GetCell(Position{0, 0}).SetVegetation(Cell::MaxVegetation - 1);

    GrowVegetation(world);
    REQUIRE(world.GetCell(Position{0, 0}).GetVegetation() == Cell::MaxVegetation);

    GrowVegetation(world);
    REQUIRE(world.GetCell(Position{0, 0}).GetVegetation() == Cell::MaxVegetation);
}

TEST_CASE("L'eau et les obstacles ne repoussent pas", "[growth]") {
    World world{2, 1};
    world.GetCell(Position{0, 0}) = Cell{Terrain::Eau, 0};
    world.GetCell(Position{1, 0}) = Cell{Terrain::Obstacle, 0};

    GrowVegetation(world);
    REQUIRE(world.GetCell(Position{0, 0}).GetVegetation() == 0);
    REQUIRE(world.GetCell(Position{1, 0}).GetVegetation() == 0);
}
