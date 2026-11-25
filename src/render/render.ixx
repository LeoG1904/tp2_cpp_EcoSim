// Interface d'affichage : le monde ne dépend d'aucune bibliothèque graphique.
export module ecosim.render;

import std;
import ecosim.simulation;

export namespace ecosim {
    enum class UiAction { None, StartOrRestart, TogglePause, NextGeneration,
                          CycleSpeed, ResetSettings,
                          CancelExit, ReturnHome, Quit };

    class Renderer2D {
    public:
        Renderer2D(int cellSize, const char* title);
        ~Renderer2D();

        Renderer2D(const Renderer2D&) = delete;
        Renderer2D& operator=(const Renderer2D&) = delete;

        int GetWidth() const;
        int GetHeight() const;
        bool ShouldClose() const;
        bool IsExitDialogOpen() const;
        double FrameTime() const;
        std::optional<int> SpeedKeyPressed() const;
        // Sur l'accueil, la saisie au clavier modifie directement les réglages.
        UiAction HandleInput(bool isHomeScreen, bool paused, SimulationSettings& settings);
        void DrawHome(SimulationSettings settings) const;
        void Draw(const Simulation& simulation, int speed, bool paused) const;

    private:
        void HandleSettingsKeys(std::span<const int> keys, SimulationSettings& settings);
        void HandleSettingsClick(int mouseX, int mouseY, SimulationSettings& settings);
        void DrawWorld(const Simulation& simulation, int speed, bool paused) const;
        void DrawGraph(const Simulation& simulation, int speed, bool paused) const;
        void DrawExitDialog(void* context) const;

        int cellSize;
        int windowWidth{0};
        int windowHeight{0};
        void* window{};
        bool showingGraph{false};
        bool showingExitDialog{false};
        bool leftMouseDown{false};
        bool spaceWasDown{false};
        bool escapeWasDown{false};
        bool tabWasDown{false};
        bool rightWasDown{false};
        bool upWasDown{false};
        SettingEditor settingEditor;
        mutable bool closed{false};
        mutable std::chrono::steady_clock::time_point lastFrame{};
    };
}
