/**
 * @file
 * @brief Le thread qui calcule la simulation, à part de l'affichage.
 */
export module ecosim.runner;

import std;
export import ecosim.simulation;

export namespace ecosim {
    /// @brief Ce que le thread de calcul montre à un instant donné.
    /// @ingroup interface
    struct RunnerState {
        /// @brief La dernière photo publiée ; jamais vide.
        std::shared_ptr<const SimulationSnapshot> snapshot;
        /// @brief Vrai tant que des générations demandées restent à calculer.
        bool busy{false};
        /// @brief Vrai si le calcul a échoué, par exemple faute de mémoire ; le thread ne calcule plus.
        bool failed{false};
    };

    /// @brief Possède une simulation et la fait avancer dans son propre thread.
    ///
    /// Après la construction, seul ce thread touche à la simulation. Les autres lui demandent des générations et lisent des photos qui ne changent plus : un verrou ne protège que cet échange.
    /// @ingroup interface
    class SimulationRunner {
    public:
        /// @brief Durée au-delà de laquelle une série de générations est écourtée, en secondes.
        static constexpr double MaxSecondsPerRequest{0.25};
        /// @brief Pendant une longue série, délai entre deux photos intermédiaires, en secondes.
        static constexpr double SecondsBetweenSnapshots{0.05};

        /// @brief Prend la simulation, publie sa première photo et démarre le thread.
        explicit SimulationRunner(Simulation simulation);
        /// @brief Arrête le thread, après la génération en cours.
        ~SimulationRunner() = default;
        SimulationRunner(const SimulationRunner&) = delete;
        SimulationRunner& operator=(const SimulationRunner&) = delete;

        /// @brief Demande des générations, sans attendre leur calcul.
        ///
        /// Une série trop longue est écourtée : le retard n'est pas rattrapé. Au moins une génération est calculée.
        /// @param steps Nombre de générations ; une valeur nulle ou négative est ignorée.
        void Request(int steps);
        /// @brief La dernière photo et l'activité du thread, lues ensemble.
        RunnerState State() const;
        /// @brief Attend que tout ce qui a été demandé soit calculé. Sert surtout aux tests.
        void Wait() const;

    private:
        /// @brief La boucle du thread : attendre une demande, la calculer, publier.
        void Work(std::stop_token stop);
        /// @brief Calcule une série de générations, hors du verrou.
        void Compute(int steps, const std::stop_token& stop);
        /// @brief Remplace la photo publiée par l'état actuel.
        void Publish();

        /// @brief La simulation ; après la construction, réservée au thread de calcul.
        Simulation simulation;
        /// @brief Protège les quatre membres suivants.
        mutable std::mutex mutex;
        /// @brief Réveille le thread de calcul quand une demande arrive.
        std::condition_variable_any wake;
        /// @brief Signale à Wait que tout est calculé.
        mutable std::condition_variable idle;
        /// @brief La dernière photo publiée.
        std::shared_ptr<const SimulationSnapshot> latest;
        /// @brief Générations demandées et pas encore commencées.
        int requested{0};
        /// @brief Vrai pendant qu'une demande est en cours.
        bool busy{false};
        /// @brief Vrai après un échec.
        bool failed{false};
        /// @brief Le thread. Déclaré en dernier : il est arrêté avant la destruction de ce qu'il utilise.
        std::jthread worker;
    };
}
