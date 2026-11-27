/**
 * @file
 * @brief Le déroulement de l'application et ses interfaces d'entrée et d'affichage.
 */
export module ecosim.application;

import std;
export import ecosim.simulation;
export import ecosim.runner;
export import ecosim.controller;

export namespace ecosim {
    /// @brief Les commandes de l'utilisateur pour une image.
    /// @ingroup interface
    struct FrameInput {
        /// @brief Action demandée.
        UiAction action{UiAction::None};
        /// @brief Vitesse choisie au clavier, le cas échéant.
        std::optional<int> speed;
        /// @brief Vrai tant que la confirmation de sortie est affichée.
        bool exitDialogOpen{false};
    };

    /// @brief D'où viennent les commandes : le clavier et la souris, ou un scénario de test.
    /// @ingroup interface
    class InputSource {
    public:
        virtual ~InputSource() = default;
        /// @brief Lit les commandes ; sur l'accueil, peut modifier les réglages.
        virtual FrameInput Read(bool isHomeScreen, bool paused, SimulationSettings& settings) = 0;
    };

    /// @brief L'état de la lecture, utile à l'affichage.
    /// @ingroup interface
    struct PlaybackState {
        /// @brief Multiplicateur de vitesse.
        int speed;
        /// @brief Vrai pendant la pause.
        bool paused;
        /// @brief Part écoulée de la génération en cours, de 0 à 1.
        double progress;
    };

    /// @brief Ce qu'un affichage doit savoir faire.
    /// @ingroup interface
    class View {
    public:
        virtual ~View() = default;
        /// @brief Dessine l'accueil.
        /// @param settings Réglages à afficher.
        /// @param notice Message à signaler, par exemple une carte impossible à créer ; vide sinon.
        virtual void DrawHome(const SimulationSettings& settings, const std::string& notice) = 0;
        /// @brief Dessine la simulation.
        virtual void DrawSimulation(const SimulationSnapshot& simulation,
                                    const PlaybackState& playback) = 0;
    };

    /// @brief Relie les commandes, la simulation, l'horloge et l'affichage.
    ///
    /// Elle ne connaît que des interfaces : elle se teste sans fenêtre.
    /// @ingroup interface
    class Application {
    public:
        /// @brief Vitesse d'animation d'un pas avancé à la main (un quart de seconde).
        static constexpr double ManualStepsPerSecond{4.0};

        /// @brief Crée l'application sur l'accueil.
        /// @param input Source des commandes, qui doit vivre aussi longtemps que l'application.
        /// @param view Affichage, qui doit vivre aussi longtemps que l'application.
        /// @param mapWidth Largeur de la carte proposée au départ, en cases.
        /// @param mapHeight Hauteur de la carte proposée au départ, en cases.
        /// @param seed Graine des cartes à venir.
        Application(InputSource& input, View& view, int mapWidth, int mapHeight, std::uint32_t seed);

        /// @brief Traite une image.
        /// @param elapsedSeconds Temps écoulé depuis la précédente.
        /// @return Faux quand l'utilisateur quitte.
        bool RunFrame(double elapsedSeconds);

        /// @brief La dernière photo de la simulation en cours ; vide sur l'accueil.
        std::shared_ptr<const SimulationSnapshot> CurrentSimulation() const;
        /// @brief Attend que le thread ait calculé les générations demandées. Sert aux tests.
        void WaitForSimulation() const;
        /// @brief L'horloge.
        const SimulationClock& Clock() const;
        /// @brief Les réglages choisis.
        const SimulationSettings& Settings() const;
        /// @brief Le message affiché sur l'accueil ; vide s'il n'y a rien à signaler.
        const std::string& Notice() const;

    private:
        /// @brief Exécute l'action demandée : démarrer, revenir au menu, pause, vitesse ou pas à pas.
        void Apply(UiAction action);
        /// @brief Crée et lance une simulation ; si la mémoire manque, reste sur l'accueil avec un message.
        void Start();
        /// @brief Remet l'horloge et l'animation à zéro.
        void ResetPlayback();
        /// @brief Fait passer le temps et compte les générations dues.
        /// @param frame Commandes de l'image.
        /// @param elapsedSeconds Temps écoulé.
        /// @param state État du thread au début de l'image.
        void Advance(const FrameInput& frame, double elapsedSeconds, const RunnerState& state);
        /// @brief Envoie au thread les générations dues, s'il est libre.
        void Dispatch();
        /// @brief Abandonne la simulation dont le calcul a échoué et le signale sur l'accueil.
        void Abandon();
        /// @brief Crée une simulation avec les réglages choisis et une nouvelle graine.
        Simulation CreateSimulation();

        /// @brief Source des commandes.
        InputSource& input;
        /// @brief Affichage utilisé.
        View& view;
        /// @brief Largeur de la carte proposée au départ, en cases.
        int mapWidth;
        /// @brief Hauteur de la carte proposée au départ, en cases.
        int mapHeight;
        /// @brief Fournit la graine de chaque nouvelle simulation.
        std::mt19937 seeds;
        /// @brief Réglages de l'accueil, conservés au retour au menu.
        SimulationSettings settings;
        /// @brief Le thread de la simulation en cours ; vide sur l'accueil.
        std::unique_ptr<SimulationRunner> runner;
        /// @brief Générations dues et pas encore demandées au thread.
        int due{0};
        /// @brief Vitesse, pause et temps écoulé.
        SimulationClock clock;
        /// @brief Vrai pendant l'animation d'un pas avancé à la main.
        bool animatingStep{false};
        /// @brief Message à afficher sur l'accueil.
        std::string notice;
    };
}
