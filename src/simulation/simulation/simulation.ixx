// État et règles du monde, indépendants de l'horloge et du rendu.
export module ecosim.simulation;

import std;
export import ecosim.clock;
export import ecosim.world;
export import ecosim.herbivore;
export import ecosim.predator;
export import ecosim.settings;

export namespace ecosim {
    void GrowVegetation(World& world, int growth = 1);

    struct AnimalSnapshot {
        std::uint64_t id;
        Position position;
        AnimalKind kind;
        int age;
        int energy;
        Position previousPosition;
        std::optional<std::uint64_t> parentId;
    };

    struct PopulationSample {
        int generation;
        std::size_t herbivores;
        std::size_t predators;
    };

    class Simulation {
    public:
        Simulation(World world, std::mt19937 rng, const SimulationSettings& settings = {});

        const World& GetWorld() const;
        int GetGeneration() const;
        std::size_t Count(AnimalKind kind) const;
        std::size_t HerbivoreCount() const;
        std::size_t PredatorCount() const;
        bool IsFinished() const;
        std::vector<AnimalSnapshot> Animals() const;
        const std::vector<PopulationSample>& PopulationHistory() const;

        // Sans énergie précisée, l'animal reçoit l'énergie de départ des réglages.
        void AddAnimal(AnimalKind kind, Position position, std::optional<int> energy = std::nullopt);
        void AddHerbivore(Position position, std::optional<int> energy = std::nullopt);
        void SeedHerbivores(int count);
        void AddPredator(Position position, std::optional<int> energy = std::nullopt);
        void SeedPredators(int count);
        void Step();

    private:
        World world;
        SimulationSettings settings;
        std::vector<std::unique_ptr<Animal>> animals;
        std::mt19937 rng;
        std::uint64_t nextId{1};
        int generation{0};
        std::vector<PopulationSample> populationHistory{{0, 0, 0}};

        void PlayPhase(TurnPhase phase);
        void SeedAnimals(int count, AnimalKind kind);
    };
}
