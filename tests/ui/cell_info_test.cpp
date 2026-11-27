#include <catch2/catch_all.hpp>

import std;
import ecosim.cell_info;

using namespace ecosim;

TEST_CASE("Le panneau decrit le terrain et les animaux de la case", "[cell_info]") {
    World world{2, 1};
    world.GetCell({0, 0}).SetVegetation(42);
    world.GetCell({1, 0}) = Cell{Terrain::Eau};
    Simulation simulation{std::move(world), std::mt19937{1}};
    simulation.AddAnimal(AnimalKind::Deer, {0, 0}, 60);
    simulation.AddAnimal(AnimalKind::Wolf, {0, 0}, 70);

    const auto prairie = DescribeCell(simulation.Snapshot(), {0, 0});
    REQUIRE(prairie.size() == 6);
    REQUIRE(prairie[0] == "Case : colonne 1, ligne 1");
    REQUIRE(prairie[1] == "Terrain : Prairie");
    REQUIRE(prairie[2].ends_with("42 / 100"));
    REQUIRE(prairie[3].starts_with("Cerfs : 1"));
    // Un animal qui vient d'etre place est un jeune.
    REQUIRE(prairie[4].starts_with("Cerf jeune femelle "));
    REQUIRE(prairie[4].ends_with("60"));
    REQUIRE(prairie[5].ends_with("70"));

    const auto water = DescribeCell(simulation.Snapshot(), {1, 0});
    REQUIRE(water.size() == 4);
    REQUIRE(water[1] == "Terrain : Eau");
    REQUIRE(water[2] == "Inaccessible aux animaux");
}

TEST_CASE("Une case tres peuplee limite la liste", "[cell_info]") {
    Simulation simulation{World{1, 1}, std::mt19937{2}};
    for (std::size_t i = 0; i < MaxListedAnimals + 3; ++i) simulation.AddAnimal(AnimalKind::Deer, {0, 0});

    const auto lines = DescribeCell(simulation.Snapshot(), {0, 0});
    REQUIRE(lines.size() == 4 + MaxListedAnimals + 1);
    REQUIRE(lines.back() == "Autres animaux : 3");
}
