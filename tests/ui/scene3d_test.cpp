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

TEST_CASE("Chaque animal a son modele, meme sur une case partagee", "[scene3d]") {
    const World world{4, 4};
    const auto count = [&world](const int herbivores, const int predators) {
        std::vector<AnimalSnapshot> animals;
        for (int i = 0; i < herbivores + predators; ++i) {
            const AnimalKind kind = i < herbivores ? AnimalKind::Deer : AnimalKind::Wolf;
            Path path;
            path.fill({1.2 + 0.1 * i, 1.5});
            animals.push_back({static_cast<std::uint64_t>(i + 1), {1, 1}, kind, 1, 50, path,
                               std::nullopt});
        }
        return BuildScene(world, animals, 1.0, OrbitCamera{4, 4}, Screen, std::nullopt);
    };
    const auto shared = count(1, 1);
    REQUIRE(std::ranges::count(shared, AnimalKind::Deer, &Face::species) > 0);
    REQUIRE(std::ranges::count(shared, AnimalKind::Wolf, &Face::species) > 0);
    REQUIRE(count(9, 0).size() > count(4, 0).size());
}

TEST_CASE("Males et femelles ont des modeles differents dans chaque espece", "[scene3d]") {
    const World world{4, 4};
    const auto faces = [&world](const AnimalKind kind, const Sex sex) {
        Path path;
        path.fill({2.0, 2.0});
        const std::vector<AnimalSnapshot> animals{
            {1, {2, 2}, kind, Animal::AdultAge, 50, path, std::nullopt, sex}};
        std::vector<Face> result = BuildScene(world, animals, 1.0, OrbitCamera{4, 4}, Screen, std::nullopt);
        std::erase_if(result, [](const Face& face) { return face.kind != FaceKind::Animal; });
        return result;
    };
    for (const SpeciesInfo& species : SpeciesCatalog()) {
        CAPTURE(species.name);
        const auto female = faces(species.kind, Sex::Female);
        const auto male = faces(species.kind, Sex::Male);
        // La forme ou au moins la couleur change.
        const bool sameColors = female.size() == male.size()
            && std::ranges::equal(female, male, [](const Face& a, const Face& b) {
                   return a.color.red == b.color.red && a.color.green == b.color.green
                          && a.color.blue == b.color.blue;
               });
        REQUIRE_FALSE(sameColors);
    }
    // Seul le cerf male porte des bois.
    REQUIRE(faces(AnimalKind::Deer, Sex::Male).size() > faces(AnimalKind::Deer, Sex::Female).size());
}

TEST_CASE("Chaque espece distingue la femelle du male par la couleur du corps", "[scene3d]") {
    const auto brightness = [](const Color color) { return color.red + color.green + color.blue; };
    for (const SpeciesInfo& species : SpeciesCatalog()) {
        CAPTURE(species.name);
        REQUIRE(brightness(LookOf(species.kind, Sex::Female, Animal::AdultAge).body)
                > brightness(LookOf(species.kind, Sex::Male, Animal::AdultAge).body));
    }
}

TEST_CASE("Un jeune est plus petit qu'un adulte, et les deux sexes s'y ressemblent", "[scene3d]") {
    const World world{4, 4};
    // Largeur et hauteur a l'ecran de tout le modele, et nombre de faces.
    const auto shape = [&world](const AnimalKind kind, const Sex sex, const int age) {
        Path path;
        path.fill({2.0, 2.0});
        const std::vector<AnimalSnapshot> animals{{1, {2, 2}, kind, age, 50, path, std::nullopt, sex}};
        int left = Screen.width, right = 0, top = Screen.height, bottom = 0, count = 0;
        for (const Face& face : BuildScene(world, animals, 1.0, OrbitCamera{4, 4}, Screen, std::nullopt)) {
            if (face.kind != FaceKind::Animal) continue;
            ++count;
            for (std::size_t i = 0; i < face.pointCount; ++i) {
                left = std::min(left, face.points[i].x);
                right = std::max(right, face.points[i].x);
                top = std::min(top, face.points[i].y);
                bottom = std::max(bottom, face.points[i].y);
            }
        }
        return std::array<int, 3>{right - left, bottom - top, count};
    };
    for (const SpeciesInfo& species : SpeciesCatalog()) {
        CAPTURE(species.name);
        const auto child = shape(species.kind, Sex::Female, 0);
        REQUIRE(child == shape(species.kind, Sex::Male, Animal::AdultAge - 1));
        for (const Sex sex : {Sex::Female, Sex::Male}) {
            const auto adult = shape(species.kind, sex, Animal::AdultAge);
            REQUIRE(child[0] < adult[0]);
            REQUIRE(child[1] < adult[1]);
        }
        const Color young = LookOf(species.kind, Sex::Female, 0).body;
        const Color grown = LookOf(species.kind, Sex::Female, Animal::AdultAge).body;
        REQUIRE((young.red != grown.red || young.green != grown.green || young.blue != grown.blue));
    }
}

TEST_CASE("Un animal glisse entre deux cases en 3D", "[scene3d]") {
    const World world{8, 3};
    Path path;
    for (std::size_t i = 0; i < path.size(); ++i) path[i] = {2.5 + 3.0 * i / TicksPerGeneration, 1.5};
    const std::vector<AnimalSnapshot> animals{
        {1, {5, 1}, AnimalKind::Deer, 1, 50, path, std::nullopt}};
    const auto leftEdge = [&](const double progress) {
        int edge = Screen.width;
        for (const Face& face : BuildScene(world, animals, progress, OrbitCamera{8, 3}, Screen,
                                           std::nullopt)) {
            if (face.kind != FaceKind::Animal) continue;
            for (std::size_t i = 0; i < face.pointCount; ++i) edge = std::min(edge, face.points[i].x);
        }
        return edge;
    };
    REQUIRE(leftEdge(0.0) < leftEdge(0.5));
    REQUIRE(leftEdge(0.5) < leftEdge(1.0));
}

TEST_CASE("L'herbe grandit et s'epaissit avec la vegetation", "[scene3d]") {
    REQUIRE(GrassTuftCount(0) == 0);
    REQUIRE(GrassHeight(0) == 0.0);
    for (int vegetation = 1; vegetation <= Cell::MaxVegetation; ++vegetation) {
        REQUIRE(GrassHeight(vegetation) >= GrassHeight(vegetation - 1));
        REQUIRE(GrassTuftCount(vegetation) >= GrassTuftCount(vegetation - 1));
    }
    REQUIRE(GrassTuftCount(Cell::MaxVegetation) == MaxGrassTufts);
    REQUIRE(GrassHeight(Cell::MaxVegetation) == Catch::Approx(MaxGrassHeight));
}

TEST_CASE("L'herbe ne pousse en 3D que sur les prairies, et suit la repousse", "[scene3d]") {
    const auto tufts = [](const World& world) {
        return CountFaces(BuildScene(world, {}, 1.0, OrbitCamera{3, 3}, Screen, std::nullopt),
                          FaceKind::Grass);
    };
    World world{3, 3};
    world.GetCell({0, 0}) = Cell{Terrain::Eau, 100};
    world.GetCell({1, 0}) = Cell{Terrain::Obstacle, 100};
    REQUIRE(tufts(world) == 0);

    world.GetCell({1, 1}).SetVegetation(30);
    const auto young = tufts(world);
    REQUIRE(young > 0);
    world.GetCell({1, 1}).SetVegetation(Cell::MaxVegetation);
    REQUIRE(tufts(world) == MaxGrassTufts);
    REQUIRE(tufts(world) > young);

    // Un herbivore qui broute la case la rase de nouveau.
    world.GetCell({1, 1}).SetVegetation(0);
    REQUIRE(tufts(world) == 0);
}
