// Interface d'affichage : le monde ne dépend d'aucune bibliothèque graphique.
export module ecosim.render;

import std;
import ecosim.simulation;

export namespace ecosim {
    enum class UiAction { None, StartOrRestart, TogglePause, NextGeneration,
                          CycleSpeed,
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
        UiAction HandleInput(bool isHomeScreen, bool paused);
        void DrawHome() const;
        void Draw(const Simulation& simulation, int speed, bool paused) const;

    private:
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
        mutable bool closed{false};
        mutable std::chrono::steady_clock::time_point lastFrame{};
    };
}
