// Horloge de la simulation : transforme le temps réel écoulé en générations à jouer,
// selon la vitesse choisie (x1, x2, x5 ou x10). Ne dépend pas de l'affichage.
export module ecosim.clock;

import std;

export namespace ecosim {

    class SimulationClock {
    public:
        // Vitesses disponibles, en multiples de la vitesse de base
        static constexpr std::array<int, 4> Speeds{1, 2, 5, 10};

        // Vitesse de base : nombre de générations par seconde à vitesse x1
        static constexpr double BaseGenerationsPerSecond{1.0};

        // Lève std::invalid_argument si la vitesse n'est pas l'une de Speeds
        void SetSpeed(const int multiplier) {
            if (std::ranges::find(Speeds, multiplier) == Speeds.end()) {
                throw std::invalid_argument{"Vitesse non supportée"};
            }
            speed = multiplier;
        }

        int GetSpeed() const { return speed; }

        // Ajoute le temps réel écoulé et renvoie le nombre de générations à jouer.
        // Les fractions de génération sont conservées pour la prochaine fois.
        int Advance(const double realSeconds) {
            accumulator += realSeconds * speed * BaseGenerationsPerSecond;
            const int steps = static_cast<int>(accumulator);
            accumulator -= steps;
            return steps;
        }

    private:
        int speed{1};
        double accumulator{0.0};
    };

}
