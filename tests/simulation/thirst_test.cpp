#include <catch2/catch_all.hpp>

import std;
import ecosim.simulation;
import ecosim.grid_habitat;
import ecosim.deer;
import ecosim.rabbit;
import ecosim.wolf;
import ecosim.fox;

using namespace ecosim;

namespace {
    // Une bande de prairie, avec de l'eau sur la case la plus a l'ouest.
    World Strip(const int length) {
        World world{length, 1};
        world.GetCell({0, 0}) = Cell{Terrain::Eau};
        for (int x = 1; x < length; ++x) world.GetCell({x, 0}).SetVegetation(Cell::MaxVegetation);
        return world;
    }

    void Dry(Animal& animal, const int generations) {
        for (int i = 0; i < generations; ++i) animal.LoseWater();
    }
}

TEST_CASE("Le chemin vers l'eau contourne les obstacles case par case", "[thirst]") {
    // Un mur separe la gauche et la droite, sauf sur la derniere ligne.
    World world{5, 4};
    world.GetCell({0, 0}) = Cell{Terrain::Eau};
    for (int y = 0; y < 3; ++y) world.GetCell({2, y}) = Cell{Terrain::Obstacle};
    const WaterRoutes routes{world};

    REQUIRE(routes.CanDrink({1, 0}));
    REQUIRE(routes.CanDrink({0, 1}));
    REQUIRE_FALSE(routes.CanDrink({1, 1}));
    REQUIRE_FALSE(routes.NextStep({1, 0}).has_value());
    REQUIRE_FALSE(routes.StepsToDrink({2, 0}).has_value());
    REQUIRE_FALSE(routes.StepsToDrink({-1, 0}).has_value());

    // De l'autre cote du mur, il faut passer par la derniere ligne : pas de raccourci.
    Position here{3, 0};
    int steps = *routes.StepsToDrink(here);
    REQUIRE(steps == 8);
    while (const auto next = routes.NextStep(here)) {
        const Position cell = CellAt(*next);
        REQUIRE(std::abs(cell.x - here.x) + std::abs(cell.y - here.y) == 1);
        REQUIRE(world.GetCell(cell).IsAccessible());
        REQUIRE(*routes.StepsToDrink(cell) == --steps);
        here = cell;
    }
    REQUIRE(routes.CanDrink(here));
}

TEST_CASE("Sans eau accessible, la soif reste active et finit par tuer", "[thirst]") {
    // De l'eau entouree d'obstacles ne touche aucune prairie.
    World closed{3, 3};
    for (const Position cell : {Position{1, 0}, Position{0, 1}}) closed.GetCell(cell) = Cell{Terrain::Obstacle};
    closed.GetCell({0, 0}) = Cell{Terrain::Eau};
    const WaterRoutes routes{closed};
    REQUIRE_FALSE(routes.StepsToDrink({2, 2}).has_value());
    REQUIRE_FALSE(routes.NextStep({2, 2}).has_value());

    World dry{4, 4};
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) dry.GetCell({x, y}).SetVegetation(Cell::MaxVegetation);
    }
    Simulation simulation{std::move(dry), std::mt19937{1}};
    REQUIRE(simulation.HasThirst());
    simulation.AddAnimal(AnimalKind::Rabbit, {1, 1}, Animal::MaxEnergy);
    // Il perd son eau generation apres generation, et meurt quand elle est epuisee.
    const int lifetime = (Animal::MaxHydration + Rabbit::Traits.waterLoss - 1) / Rabbit::Traits.waterLoss;
    for (int i = 0; i < lifetime - 1; ++i) simulation.Step();
    REQUIRE(*simulation.Animals().front().hydration == Animal::MaxHydration % Rabbit::Traits.waterLoss
            + (Animal::MaxHydration % Rabbit::Traits.waterLoss == 0 ? Rabbit::Traits.waterLoss : 0));
    simulation.Step();
    REQUIRE(simulation.IsFinished());
    REQUIRE(simulation.Stats().Total(AnimalKind::Rabbit).parched == 1);
}

TEST_CASE("Un animal assoiffe a soif sous le seuil, boit, et meurt a zero", "[thirst]") {
    Deer deer{1, {3, 0}};
    REQUIRE(deer.GetHydration() == Animal::MaxHydration);
    REQUIRE_FALSE(deer.IsThirsty());
    Dry(deer, (Animal::MaxHydration - Animal::ThirstThreshold) / Deer::Traits.waterLoss);
    REQUIRE(deer.GetHydration() > Animal::ThirstThreshold);
    Dry(deer, 1);
    REQUIRE(deer.IsThirsty());
    deer.Drink();
    REQUIRE(deer.GetHydration() == Animal::MaxHydration);
    Dry(deer, Animal::MaxHydration);
    REQUIRE(deer.GetHydration() == 0);
    REQUIRE_FALSE(deer.IsAlive());
}

TEST_CASE("Un nouveau-ne commence avec moins d'eau et nait assoiffe", "[thirst]") {
    const Deer mother{1, {3, 0}};
    const auto child = mother.CreateOffspring(2, Sex::Male);
    REQUIRE(Animal::NewbornHydration == 50);
    REQUIRE(child->GetHydration() == Animal::NewbornHydration);
    REQUIRE(child->IsThirsty());
    REQUIRE(mother.GetHydration() == Animal::MaxHydration);
    child->Drink();
    REQUIRE(child->GetHydration() == Animal::MaxHydration);
}

TEST_CASE("Un animal assoiffe ne se reproduit pas", "[thirst]") {
    World world = Strip(12);
    SimulationStats stats;
    std::vector<std::unique_ptr<Animal>> animals;
    animals.push_back(std::make_unique<Deer>(1, Position{5, 0}, Animal::MaxEnergy));
    animals.push_back(std::make_unique<Deer>(2, Position{5, 0}, Animal::MaxEnergy));
    Animal& female = *animals[0];
    Animal& male = *animals[1];
    female.SetSex(Sex::Female);
    male.SetSex(Sex::Male);
    for (int i = 0; i < Animal::AdultAge; ++i) {
        female.SpendGeneration();
        male.SpendGeneration();
    }
    REQUIRE(female.CanBreedWith(male));

    // Un seul des deux a soif : aucun couple ne se forme.
    Dry(male, 14);
    REQUIRE(male.IsThirsty());
    REQUIRE_FALSE(male.CanReproduce());
    REQUIRE_FALSE(female.CanBreedWith(male));
    GridHabitat habitat{world, animals, stats};
    REQUIRE_FALSE(habitat.NearestMate(female, female.GetLocation(), Animal::MateSight).has_value());
    habitat.FormCouples();
    REQUIRE(female.GetPartner() == nullptr);

    male.Drink();
    habitat.FormCouples();
    REQUIRE(female.GetPartner() == &male);
}

TEST_CASE("Les petits naissent desalteres quand la soif est coupee", "[thirst]") {
    for (const int thirst : {0, 1}) {
        SimulationSettings settings;
        settings.thirstEnabled = thirst;
        settings.vegetationGrowth = 20;
        settings.species[AnimalKind::Rabbit].options.birthPercent = 100;
        Simulation simulation{World{1, 1}, std::mt19937{3}, settings};
        simulation.AddAnimal(AnimalKind::Rabbit, {0, 0}, 100);
        simulation.AddAnimal(AnimalKind::Rabbit, {0, 0}, 100);
        for (int i = 0; i < 20 && simulation.Stats().Total(AnimalKind::Rabbit).births == 0; ++i) {
            simulation.Step();
        }
        REQUIRE(simulation.Stats().Total(AnimalKind::Rabbit).births > 0);
        for (const AnimalSnapshot& animal : simulation.Animals()) {
            if (!animal.parentId) continue;
            if (thirst == 0) REQUIRE_FALSE(animal.hydration.has_value());
            else REQUIRE(*animal.hydration == Animal::NewbornHydration);
        }
        // Sans soif, les petits devenus adultes se reproduisent a leur tour.
        if (thirst == 0) {
            const std::size_t first = simulation.Stats().Total(AnimalKind::Rabbit).births;
            bool grandchildren = false;
            for (int i = 0; i < 40 && !grandchildren && !simulation.IsFinished(); ++i) {
                simulation.Step();
                const auto all = simulation.Animals();
                std::set<std::uint64_t> born;
                for (const AnimalSnapshot& animal : all) {
                    if (animal.parentId) born.insert(animal.id);
                }
                grandchildren = std::ranges::any_of(all, [&born](const AnimalSnapshot& animal) {
                    return animal.parentId && born.contains(*animal.parentId);
                });
            }
            REQUIRE(first > 0);
            REQUIRE(grandchildren);
        }
    }
}

TEST_CASE("La soif passe avant la nourriture, la chasse et la reproduction", "[thirst]") {
    World world = Strip(12);
    const WaterRoutes routes{world};
    SimulationStats stats;
    std::vector<std::unique_ptr<Animal>> animals;
    animals.push_back(std::make_unique<Deer>(1, Position{8, 0}));
    animals.push_back(std::make_unique<Wolf>(2, Position{9, 0}));
    GridHabitat wet{world, animals, stats, &routes};
    const GridHabitat dry{world, animals, stats};
    std::mt19937 rng{1};

    for (auto& animal : animals) {
        Dry(*animal, 20);
        REQUIRE(animal->IsThirsty());
        const Steering toWater = animal->Steer(wet, rng);
        // L'eau est a l'ouest ; le cerf est a l'ouest du loup, qui ne le poursuivrait que vers l'ouest aussi.
        REQUIRE(std::abs(toWater.heading) == Catch::Approx(std::numbers::pi));
        REQUIRE(toWater.pace == Catch::Approx(0.8));
    }
    REQUIRE_FALSE(dry.TowardWater({8.5, 0.5}).has_value());
}

TEST_CASE("La fuite passe avant la soif", "[thirst]") {
    World world = Strip(12);
    const WaterRoutes routes{world};
    SimulationStats stats;
    std::vector<std::unique_ptr<Animal>> animals;
    animals.push_back(std::make_unique<Deer>(1, Position{5, 0}, Animal::StartingEnergy,
                                             AnimalOptions{30, 100}));
    animals.push_back(std::make_unique<Wolf>(2, Position{4, 0}));
    const GridHabitat habitat{world, animals, stats, &routes};
    std::mt19937 rng{1};
    Animal& deer = *animals.front();
    Dry(deer, 20);
    deer.BeginGeneration(rng);
    // Le loup est entre le cerf et l'eau : il s'en eloigne, vers l'est.
    REQUIRE(deer.Steer(habitat, rng).heading == Catch::Approx(0.0));
}

TEST_CASE("En simulation, un animal rejoint la berge pour boire ; couper la soif l'arrete", "[thirst]") {
    SimulationSettings settings;
    Simulation thirsty{Strip(16), std::mt19937{4}, settings};
    REQUIRE(thirsty.HasThirst());
    thirsty.AddAnimal(AnimalKind::Deer, {10, 0}, Animal::MaxEnergy);
    int lowest = Animal::MaxHydration;
    bool drank = false;
    int previous = Animal::MaxHydration;
    for (int i = 0; i < 60 && !thirsty.IsFinished(); ++i) {
        thirsty.Step();
        if (thirsty.IsFinished()) break;
        const int now = *thirsty.Animals().front().hydration;
        drank = drank || now > previous;
        previous = now;
        lowest = std::min(lowest, now);
    }
    REQUIRE(drank);
    REQUIRE(lowest > 0);
    REQUIRE(thirsty.Stats().Total(AnimalKind::Deer).parched == 0);

    settings.thirstEnabled = 0;
    Simulation quiet{Strip(16), std::mt19937{4}, settings};
    REQUIRE_FALSE(quiet.HasThirst());
}

TEST_CASE("Une mort de soif est comptee comme telle", "[thirst]") {
    // L'eau est trop loin ; le lapin broute en chemin et n'a donc que soif.
    Simulation simulation{Strip(400), std::mt19937{2}};
    simulation.AddAnimal(AnimalKind::Rabbit, {399, 0}, Animal::MaxEnergy);
    std::size_t parched = 0;
    for (int i = 0; i < 140 && !simulation.IsFinished(); ++i) {
        simulation.Step();
        parched += simulation.Stats().LastGeneration(AnimalKind::Rabbit).parched;
    }
    REQUIRE(simulation.IsFinished());
    REQUIRE(parched == 1);
    REQUIRE(simulation.Stats().Total(AnimalKind::Rabbit).Deaths() == 1);
    REQUIRE(simulation.Stats().Total(AnimalKind::Rabbit).starved == 0);
}

TEST_CASE("Sur des cartes generees, chaque prairie permet de marcher jusqu'a l'eau", "[thirst]") {
    for (std::uint32_t seed = 1; seed <= 12; ++seed) {
        std::mt19937 rng{seed};
        const World world = GenerateRandomWorld(42, 26, rng, 10, 20, 3);
        const WaterRoutes routes{world};
        for (int y = 0; y < world.GetHeight(); ++y) {
            for (int x = 0; x < world.GetWidth(); ++x) {
                if (!world.GetCell({x, y}).IsAccessible()) continue;
                const auto steps = routes.StepsToDrink({x, y});
                REQUIRE(steps.has_value());
                // Un cerf suit le chemin avec ses vrais deplacements, virages et obstacles compris.
                Deer walker{1, {x, y}};
                int ticks = 0;
                const int limit = 40 + *steps * 40;
                while (!routes.CanDrink(walker.GetPosition()) && ticks < limit) {
                    const Point target = *routes.NextStep(walker.GetPosition());
                    const double heading = std::atan2(target.y - walker.GetLocation().y,
                                                      target.x - walker.GetLocation().x);
                    walker.Advance(world, {heading, 0.8}, ticks % TicksPerGeneration);
                    ++ticks;
                }
                INFO("graine " << seed << ", case " << x << ' ' << y << ", pas " << *steps);
                REQUIRE(routes.CanDrink(walker.GetPosition()));
            }
        }
    }
}
