// Déroulement de l'application. Elle ne connaît que des interfaces d'entrée et d'affichage.
export module ecosim.application;

import std;
export import ecosim.simulation;
export import ecosim.controller;
export import ecosim.animation;

export namespace ecosim {
    struct FrameInput {
        UiAction action{UiAction::None};
        std::optional<int> speed;
        bool exitDialogOpen{false};
    };

    // Source des commandes : clavier et souris, ou un scénario dans les tests.
    class InputSource {
    public:
        virtual ~InputSource() = default;
        virtual FrameInput Read(bool isHomeScreen, bool paused, SimulationSettings& settings) = 0;
    };

    struct PlaybackState {
        int speed;
        bool paused;
        double progress; // part écoulée de la génération en cours, de 0 à 1
    };

    // Affichage : le rendu 2D actuel, ou un autre rendu (3D par exemple).
    class View {
    public:
        virtual ~View() = default;
        virtual void DrawHome(const SimulationSettings& settings) = 0;
        virtual void DrawSimulation(const Simulation& simulation, const PlaybackState& playback) = 0;
    };

    class Application {
    public:
        // input et view doivent rester valides pendant toute la vie de l'application.
        Application(InputSource& input, View& view, int mapWidth, int mapHeight, std::uint32_t seed);

        // Traite une image. Renvoie false quand l'utilisateur quitte.
        bool RunFrame(double elapsedSeconds);

        const std::optional<Simulation>& CurrentSimulation() const;
        const SimulationClock& Clock() const;
        const SimulationSettings& Settings() const;

    private:
        void Apply(UiAction action);
        void ResetPlayback();
        void Advance(const FrameInput& frame, double elapsedSeconds);
        Simulation CreateSimulation();

        InputSource& input;
        View& view;
        int mapWidth;
        int mapHeight;
        std::mt19937 seeds;
        // Choisis sur l'accueil, conservés au retour au menu et par Recommencer.
        SimulationSettings settings;
        // La simulation n'existe qu'après le clic sur Démarrer.
        std::optional<Simulation> simulation;
        SimulationClock clock;
        ManualStepAnimation animation;
    };
}
