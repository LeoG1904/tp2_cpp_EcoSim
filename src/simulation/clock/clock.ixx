// Horloge indépendante du monde et du rendu : temps réel -> générations.
export module ecosim.clock;

import std;

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
}
