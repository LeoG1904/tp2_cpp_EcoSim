#include <catch2/catch_all.hpp>

// Le main des tests est fourni par Catch2WithMain (voir CMakeLists.txt)
import std;
import ecosim.world;

using namespace ecosim;

namespace {
    int CountTerrain(const World& world, const Terrain terrain) {
        int count = 0;
        for (int y = 0; y < world.GetHeight(); ++y) {
            for (int x = 0; x < world.GetWidth(); ++x) {
                count += world.GetCell({x, y}).GetTerrain() == terrain;
            }
        }
        return count;
    }

    // Prairies atteintes en partant de la première, d'un côté à l'autre.
    int ReachablePrairies(const World& world) {
        std::vector<Position> reached;
        for (int y = 0; y < world.GetHeight() && reached.empty(); ++y) {
            for (int x = 0; x < world.GetWidth() && reached.empty(); ++x) {
                if (world.GetCell({x, y}).IsAccessible()) reached.push_back({x, y});
            }
        }
        for (std::size_t next = 0; next < reached.size(); ++next) {
            for (const Position neighbor : world.Neighbors(reached[next])) {
                if (world.GetCell(neighbor).IsAccessible()
                    && std::ranges::find(reached, neighbor) == reached.end()) {
                    reached.push_back(neighbor);
                }
            }
        }
        return static_cast<int>(reached.size());
    }
}

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

TEST_CASE("Toutes les prairies sont reliees entre elles", "[generation]") {
    for (const auto& [water, obstacle] : {std::pair{10, 10}, std::pair{25, 20}, std::pair{50, 40}}) {
        for (std::uint32_t seed = 1; seed <= 40; ++seed) {
            std::mt19937 rng{seed};
            const World world = GenerateRandomWorld(42, 26, rng, water, obstacle);
            CAPTURE(water, obstacle, seed);
            REQUIRE(ReachablePrairies(world) == CountTerrain(world, Terrain::Prairie));
        }
    }
}

TEST_CASE("Les proportions demandees sont respectees, a la correction pres", "[generation]") {
    const int cells = 42 * 26;
    for (std::uint32_t seed = 1; seed <= 40; ++seed) {
        std::mt19937 rng{seed};
        const World world = GenerateRandomWorld(42, 26, rng, 10, 10);
        const int water = CountTerrain(world, Terrain::Eau);
        const int obstacle = CountTerrain(world, Terrain::Obstacle);
        CAPTURE(seed, water, obstacle);
        // 109 cases de chaque, plus les rares prairies isolees comblees.
        REQUIRE(water >= 109);
        REQUIRE(obstacle >= 109);
        REQUIRE(water + obstacle <= 218 + cells / 20);
    }
}

TEST_CASE("Un terrain regle a 0 % n'apparait jamais", "[generation]") {
    for (std::uint32_t seed = 1; seed <= 40; ++seed) {
        std::mt19937 rng{seed};
        REQUIRE(CountTerrain(GenerateRandomWorld(42, 26, rng, 30, 0), Terrain::Obstacle) == 0);
        REQUIRE(CountTerrain(GenerateRandomWorld(42, 26, rng, 0, 30), Terrain::Eau) == 0);
    }
}

TEST_CASE("L'eau forme des etendues au lieu de cases dispersees", "[generation]") {
    int pairs = 0;
    int wetPairs = 0;
    for (std::uint32_t seed = 1; seed <= 20; ++seed) {
        std::mt19937 rng{seed};
        const World world = GenerateRandomWorld(42, 26, rng, 10, 10);
        for (int y = 0; y < 26; ++y) {
            for (int x = 0; x < 42; ++x) {
                if (world.GetCell({x, y}).GetTerrain() != Terrain::Eau) continue;
                for (const Position neighbor : world.Neighbors({x, y})) {
                    ++pairs;
                    wetPairs += world.GetCell(neighbor).GetTerrain() == Terrain::Eau;
                }
            }
        }
    }
    // Au hasard, un voisin sur dix serait de l'eau ; ici plus d'un sur deux.
    REQUIRE(wetPairs * 2 > pairs);
}

TEST_CASE("L'herbe de depart varie doucement d'une case a l'autre", "[generation]") {
    std::mt19937 rng{3};
    const World world = GenerateRandomWorld(42, 26, rng, 0, 0);
    long long difference = 0;
    long long total = 0;
    int pairs = 0;
    for (int y = 0; y < 26; ++y) {
        for (int x = 0; x < 42; ++x) {
            total += world.GetCell({x, y}).GetVegetation();
            if (x == 0) continue;
            difference += std::abs(world.GetCell({x, y}).GetVegetation()
                                   - world.GetCell({x - 1, y}).GetVegetation());
            ++pairs;
        }
    }
    // Au hasard, l'ecart moyen entre voisines serait d'environ 33.
    REQUIRE(difference / pairs < 15);
    // La quantite totale d'herbe reste celle d'une moyenne de 50 par case.
    REQUIRE(total / (42 * 26) >= 48);
    REQUIRE(total / (42 * 26) <= 51);
}

TEST_CASE("Le relief ne change pas les terrains et decoupe la terre en paliers", "[generation]") {
    std::mt19937 first{7};
    std::mt19937 second{7};
    const World flat = GenerateRandomWorld(42, 26, first, 10, 10);
    const World hilly = GenerateRandomWorld(42, 26, second, 10, 10, 3);
    // Le relief ne consomme pas de hasard : la suite de la partie reste la même.
    REQUIRE(first == second);

    std::array<int, Cell::MaxElevation + 1> land{};
    for (int y = 0; y < flat.GetHeight(); ++y) {
        for (int x = 0; x < flat.GetWidth(); ++x) {
            const Cell& cell = hilly.GetCell({x, y});
            REQUIRE(flat.GetCell({x, y}).GetElevation() == 0);
            REQUIRE(cell.GetTerrain() == flat.GetCell({x, y}).GetTerrain());
            REQUIRE(cell.GetVegetation() == flat.GetCell({x, y}).GetVegetation());
            if (cell.GetTerrain() == Terrain::Eau) REQUIRE(cell.GetElevation() == 0);
            else ++land[cell.GetElevation()];
        }
    }
    // Quatre paliers de surfaces égales, à une case près, et rien au-dessus.
    for (int level = 1; level <= 3; ++level) REQUIRE(std::abs(land[level] - land[0]) <= 1);
    REQUIRE(land[4] == 0);
}

TEST_CASE("La hauteur reste dans ses limites", "[generation]") {
    Cell cell;
    cell.SetElevation(-2);
    REQUIRE(cell.GetElevation() == 0);
    cell.SetElevation(99);
    REQUIRE(cell.GetElevation() == Cell::MaxElevation);

    std::mt19937 rng{1};
    REQUIRE_THROWS_AS(GenerateRandomWorld(20, 20, rng, 10, 10, -1), std::invalid_argument);
    REQUIRE_THROWS_AS(GenerateRandomWorld(20, 20, rng, 10, 10, Cell::MaxElevation + 1),
                      std::invalid_argument);
}
