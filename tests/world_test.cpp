#include <catch2/catch_all.hpp>

// Le main des tests est fourni par Catch2WithMain (voir CMakeLists.txt)
import std;
import ecosim.world;

using namespace ecosim;

TEST_CASE("Une cellule par défaut est une prairie vide", "[world][cell]") {
    const Cell cell{};
    REQUIRE(cell.GetTerrain() == Terrain::Prairie);
    REQUIRE(cell.GetVegetation() == 0);
    REQUIRE(cell.IsAccessible());
}

TEST_CASE("La végétation reste dans les bornes", "[world][cell]") {
    Cell cell{Terrain::Prairie, 50};
    REQUIRE(cell.GetVegetation() == 50);

    cell.SetVegetation(150);
    REQUIRE(cell.GetVegetation() == Cell::MaxVegetation);

    cell.SetVegetation(-10);
    REQUIRE(cell.GetVegetation() == 0);
}

TEST_CASE("Eau et obstacles ne sont pas accessibles", "[world][cell]") {
    REQUIRE_FALSE(Cell{Terrain::Eau}.IsAccessible());
    REQUIRE_FALSE(Cell{Terrain::Obstacle}.IsAccessible());
}

TEST_CASE("Dimensions invalides refusées", "[world]") {
    REQUIRE_THROWS_AS(World(0, 5), std::invalid_argument);
    REQUIRE_THROWS_AS(World(5, -1), std::invalid_argument);
}

TEST_CASE("Accès aux cellules et bornes de la grille", "[world]") {
    World world{4, 3};
    REQUIRE(world.GetWidth() == 4);
    REQUIRE(world.GetHeight() == 3);

    REQUIRE(world.IsInside(Position{3, 2}));
    REQUIRE_FALSE(world.IsInside(Position{4, 0}));
    REQUIRE_FALSE(world.IsInside(Position{0, -1}));

    world.GetCell(Position{1, 2}).SetVegetation(40);
    REQUIRE(std::as_const(world).GetCell(Position{1, 2}).GetVegetation() == 40);
    REQUIRE(world.GetCell(Position{0, 0}).GetVegetation() == 0);

    REQUIRE_THROWS_AS(world.GetCell(Position{4, 0}), std::out_of_range);
}

TEST_CASE("Nombre de voisins selon la position", "[world]") {
    const World world{3, 3};
    REQUIRE(world.Neighbors(Position{0, 0}).size() == 2);  // coin
    REQUIRE(world.Neighbors(Position{1, 0}).size() == 3);  // bord
    REQUIRE(world.Neighbors(Position{1, 1}).size() == 4);  // centre
}
