// Règles et horloge : l'état du monde évolue indépendamment du rendu.
export module ecosim.simulation;

import std;
export import ecosim.world;
export import ecosim.herbivore;

export namespace ecosim {
    class SimulationClock {
    public:
        static constexpr std::array<int, 4> Speeds{1, 2, 5, 10};
        static constexpr double BaseGenerationsPerSecond{1.0};

        void SetSpeed(int multiplier);
        int GetSpeed() const;
        int Advance(double realSeconds);

    private:
        int speed{1};
        double accumulator{0.0};
    };

    void GrowVegetation(World& world);

    struct AnimalSnapshot {
        std::uint64_t id;
        Position position;
        AnimalKind kind;
        int age;
        int energy;
    };

    class Simulation {
    public:
        Simulation(World world, std::mt19937 rng);

        const World& GetWorld() const;
        int GetGeneration() const;
        std::size_t HerbivoreCount() const;
        std::vector<AnimalSnapshot> Animals() const;

        void AddHerbivore(Position position, int energy = Herbivore::StartingEnergy);
        void SeedHerbivores(int count);
        void Step();

    private:
        World world;
        std::vector<std::unique_ptr<Animal>> animals;
        std::mt19937 rng;
        std::uint64_t nextId{1};
        int generation{0};

        Position ChooseDestination(Position origin);
    };
}
