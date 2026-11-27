#include <catch2/catch_all.hpp>

import std;
import ecosim.grid_habitat;
import ecosim.simulation;

using namespace ecosim;

namespace {
    // Des animaux adultes et bien nourris, poses a la main.
    struct Meadow {
        World world{20, 20};
        SimulationStats stats;
        std::vector<std::unique_ptr<Animal>> animals;

        template <typename Species>
        Animal& Add(const Point point, const Sex sex, const int energy = 100, const int age = Animal::AdultAge) {
            animals.push_back(std::make_unique<Species>(animals.size() + 1, CellAt(point), energy));
            Animal& animal = *animals.back();
            animal.Place(point, 0.0);
            animal.SetSex(sex);
            // Vieillir coute de l'energie : on la rend avant le test.
            for (int i = 0; i < age; ++i) animal.SpendGeneration();
            animal.ReceiveMeal(std::max(0, energy - animal.GetEnergy()));
            return animal;
        }
    };

    // Compte les petits d'une simulation d'une seule case apres quelques generations.
    std::size_t Born(const AnimalKind kind, const std::uint32_t seed) {
        SimulationSettings settings;
        settings.vegetationGrowth = 20;
        settings.species[kind].options.birthPercent = 100;
        Simulation simulation{World{1, 1}, std::mt19937{seed}, settings};
        simulation.AddAnimal(kind, {0, 0}, 100);
        simulation.AddAnimal(kind, {0, 0}, 100);
        for (int i = 0; i < 20; ++i) {
            simulation.Step();
            if (const std::size_t born = simulation.Stats().Total(kind).births) return born;
        }
        return 0;
    }
}

TEST_CASE("Il faut une femelle et un male de la meme espece, adultes et nourris", "[breeding]") {
    Meadow meadow;
    Animal& female = meadow.Add<Deer>({5.0, 5.0}, Sex::Female);
    REQUIRE(female.CanBreedWith(meadow.Add<Deer>({5.2, 5.0}, Sex::Male)));
    REQUIRE_FALSE(female.CanBreedWith(meadow.Add<Deer>({5.2, 5.0}, Sex::Female)));
    REQUIRE_FALSE(female.CanBreedWith(meadow.Add<Rabbit>({5.2, 5.0}, Sex::Male)));
    REQUIRE_FALSE(female.CanBreedWith(meadow.Add<Deer>({5.2, 5.0}, Sex::Male, 20)));
    REQUIRE_FALSE(female.CanBreedWith(meadow.Add<Deer>({5.2, 5.0}, Sex::Male, 100, Animal::AdultAge - 1)));
}

TEST_CASE("Un couple se forme au contact, avec le male libre le plus proche", "[breeding]") {
    Meadow meadow;
    Animal& female = meadow.Add<Deer>({5.0, 5.0}, Sex::Female);
    Animal& far = meadow.Add<Deer>({5.5, 5.0}, Sex::Male);
    Animal& near = meadow.Add<Deer>({5.3, 5.0}, Sex::Male);
    Animal& alone = meadow.Add<Deer>({9.0, 9.0}, Sex::Female);
    GridHabitat habitat{meadow.world, meadow.animals, meadow.stats};
    habitat.FormCouples();
    REQUIRE(female.GetPartner() == &near);
    REQUIRE(near.GetPartner() == &female);
    REQUIRE(far.GetPartner() == nullptr);
    REQUIRE(alone.GetPartner() == nullptr);

    female.StartPath(); // nouvelle generation
    REQUIRE(female.GetPartner() == nullptr);
}

TEST_CASE("Un animal pret se dirige vers un partenaire eloigne", "[breeding]") {
    Meadow meadow;
    Animal& female = meadow.Add<Deer>({5.0, 5.0}, Sex::Female);
    meadow.Add<Deer>({9.0, 5.0}, Sex::Male);
    const GridHabitat habitat{meadow.world, meadow.animals, meadow.stats};
    std::mt19937 rng{1};
    const Steering steering = female.Steer(habitat, rng);
    REQUIRE(std::cos(steering.heading) > 0.99);
    REQUIRE(steering.pace == 0.8);
}

TEST_CASE("Au-dela de sa portee de recherche, il ne le repere pas", "[breeding]") {
    Meadow meadow;
    Animal& female = meadow.Add<Deer>({2.0, 2.0}, Sex::Female);
    meadow.Add<Deer>({2.0 + Animal::MateSight + 1.0, 2.0}, Sex::Male);
    const GridHabitat habitat{meadow.world, meadow.animals, meadow.stats};
    REQUIRE_FALSE(habitat.NearestMate(female, female.GetLocation(), Animal::MateSight).has_value());
}

TEST_CASE("Un animal seul ne se reproduit jamais", "[breeding]") {
    SimulationSettings settings;
    settings.vegetationGrowth = 20;
    settings.species[AnimalKind::Rabbit].options.birthPercent = 100;
    Simulation simulation{World{1, 1}, std::mt19937{3}, settings};
    simulation.AddAnimal(AnimalKind::Rabbit, {0, 0}, 100);
    for (int i = 0; i < 40; ++i) simulation.Step();
    REQUIRE(simulation.Stats().Total(AnimalKind::Rabbit).births == 0);
}

TEST_CASE("La taille d'une portee depend de l'espece", "[breeding]") {
    for (const AnimalKind kind : {AnimalKind::Deer, AnimalKind::Rabbit}) {
        const SpeciesTraits traits = SpeciesOf(kind).create(1, {0, 0}, 50, {})->GetTraits();
        std::set<std::size_t> sizes;
        for (std::uint32_t seed = 1; seed <= 40; ++seed) sizes.insert(Born(kind, seed));
        CAPTURE(SpeciesOf(kind).name);
        REQUIRE(*sizes.begin() >= static_cast<std::size_t>(traits.litterMin));
        REQUIRE(*sizes.rbegin() <= static_cast<std::size_t>(traits.litterMax));
    }
}

TEST_CASE("Les animaux places au depart alternent femelle et male", "[breeding]") {
    Simulation simulation{World{10, 10}, std::mt19937{4}};
    simulation.Seed(AnimalKind::Wolf, 6);
    const auto wolves = simulation.Animals();
    REQUIRE(std::ranges::count(wolves, Sex::Female, &AnimalSnapshot::sex) == 3);
}
