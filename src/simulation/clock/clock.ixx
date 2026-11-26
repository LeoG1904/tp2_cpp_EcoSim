/**
 * @file
 * @brief L'horloge : elle convertit le temps réel en générations.
 */
export module ecosim.clock;

import std;

export namespace ecosim {
    /// @brief Règle la vitesse et la pause.
    ///
    /// Elle ne connaît ni le monde ni l'affichage.
    /// @ingroup moteur
    class SimulationClock {
    public:
        /// @brief Multiplicateurs de vitesse proposés.
        static constexpr std::array<int, 4> Speeds{1, 2, 5, 10};
        /// @brief Générations par seconde à la vitesse x1.
        static constexpr double BaseGenerationsPerSecond{1.0};

        /// @brief Choisit une vitesse.
        /// @throws std::invalid_argument si elle n'est pas dans Speeds.
        void SetSpeed(int multiplier);
        /// @brief Multiplicateur actuel.
        int GetSpeed() const;
        /// @brief Passe à la vitesse suivante, puis revient à la première.
        void CycleSpeed();
        /// @brief Met en pause ou reprend.
        void SetPaused(bool value);
        /// @brief Vrai pendant la pause.
        bool IsPaused() const;
        /// @brief Fait avancer le temps.
        /// @param realSeconds Temps réel écoulé.
        /// @return Le nombre de générations à simuler ; la fraction restante est conservée.
        int Advance(double realSeconds);
        /// @brief Part écoulée de la génération en cours, de 0 à 1.
        double Progress() const;
        /// @brief Replace l'horloge à un point de la génération en cours.
        void SetProgress(double fraction);

    private:
        int speed{1};
        bool paused{false};
        double accumulator{0.0};
    };
}
