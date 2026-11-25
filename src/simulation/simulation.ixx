// Règles et horloge : l'état du monde évolue indépendamment du rendu.
export module ecosim.simulation;

import std;
export import ecosim.world;

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
}
