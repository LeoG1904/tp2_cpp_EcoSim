#include <catch2/catch_all.hpp>

import std;
import ecosim.scene3d;

using namespace ecosim;

namespace {
    constexpr Viewport Screen{1280, 720};

    std::ptrdiff_t CountFaces(const std::vector<Face>& faces, const FaceKind kind) {
        return std::ranges::count(faces, kind, &Face::kind);
    }
}

TEST_CASE("Le sol est dessine avant les volumes", "[scene3d]") {
    World world{6, 4};
    world.GetCell({2, 1}) = Cell{Terrain::Obstacle};
    const auto faces = BuildScene(world, {}, 1.0, OrbitCamera{6, 4}, Screen, std::nullopt);

    REQUIRE(CountFaces(faces, FaceKind::Ground) == 24);
    REQUIRE(CountFaces(faces, FaceKind::Obstacle) >= 2);
    for (std::size_t i = 0; i < 24; ++i) REQUIRE(faces[i].kind == FaceKind::Ground);
}

TEST_CASE("La case choisie est encadree", "[scene3d]") {
    const World world{4, 4};
    const auto faces = BuildScene(world, {}, 1.0, OrbitCamera{4, 4}, Screen, Position{1, 1});
    REQUIRE(CountFaces(faces, FaceKind::Selection) == 1);
}

TEST_CASE("Les deux especes partagent une case, avec un nombre de modeles limite", "[scene3d]") {
    const World world{4, 4};
    const auto count = [&world](const int herbivores, const int predators) {
        std::vector<AnimalSnapshot> animals;
        for (int i = 0; i < herbivores + predators; ++i) {
            const AnimalKind kind = i < herbivores ? AnimalKind::Herbivore : AnimalKind::Predator;
            animals.push_back({static_cast<std::uint64_t>(i + 1), {1, 1}, kind, 1, 50, {1, 1},
                               std::nullopt});
        }
        return BuildScene(world, animals, 1.0, OrbitCamera{4, 4}, Screen, std::nullopt);
    };
    const auto shared = count(1, 1);
    REQUIRE(std::ranges::count(shared, AnimalKind::Herbivore, &Face::species) > 0);
    REQUIRE(std::ranges::count(shared, AnimalKind::Predator, &Face::species) > 0);
    REQUIRE(count(MaxModelsPerCell + 5, 0).size() == count(MaxModelsPerCell, 0).size());
}

TEST_CASE("Un animal glisse entre deux cases en 3D", "[scene3d]") {
    const World world{8, 3};
    const std::vector<AnimalSnapshot> animals{
        {1, {5, 1}, AnimalKind::Herbivore, 1, 50, {2, 1}, std::nullopt}};
    const auto leftEdge = [&](const double progress) {
        int edge = Screen.width;
        for (const Face& face : BuildScene(world, animals, progress, OrbitCamera{8, 3}, Screen,
                                           std::nullopt)) {
            if (face.kind != FaceKind::Animal) continue;
            for (const ScreenPoint& point : face.points) edge = std::min(edge, point.x);
        }
        return edge;
    };
    REQUIRE(leftEdge(0.0) < leftEdge(0.5));
    REQUIRE(leftEdge(0.5) < leftEdge(1.0));
}
