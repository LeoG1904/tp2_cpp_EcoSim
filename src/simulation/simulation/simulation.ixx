// État et règles du monde, indépendants de l'horloge et du rendu.
export module ecosim.simulation;

import std;
export import ecosim.clock;
export import ecosim.world;
export import ecosim.herbivore;
export import ecosim.predator;

export namespace ecosim {
    void GrowVegetation(World& world);

    struct AnimalSnapshot {
        std::uint64_t id;
        Position position;
        AnimalKind kind;
        int age;
        int energy;
    };

    struct PopulationSample {
        int generation;
        std::size_t herbivores;
        std::size_t predators;
    };

    class Simulation {
    public:
        Simulation(World world, std::mt19937 rng);

        const World& GetWorld() const;
        int GetGeneration() const;
        std::size_t HerbivoreCount() const;
        std::size_t PredatorCount() const;
        bool IsFinished() const;
        std::vector<AnimalSnapshot> Animals() const;
        const std::vector<PopulationSample>& PopulationHistory() const;

        void AddHerbivore(Position position, int energy = Herbivore::StartingEnergy);
        void SeedHerbivores(int count);
        void AddPredator(Position position, int energy = Predator::StartingEnergy);
        void SeedPredators(int count);
        void Step();

    private:
        World world;
        std::vector<std::unique_ptr<Animal>> animals;
        std::mt19937 rng;
        std::uint64_t nextId{1};
        int generation{0};
        std::vector<PopulationSample> populationHistory{{0, 0, 0}};

        Position ChooseDestination(Position origin);
        Position ChoosePredatorDestination(Position origin, const std::vector<int>& preyCounts);
        void SeedAnimals(int count, AnimalKind kind);
    };
}
