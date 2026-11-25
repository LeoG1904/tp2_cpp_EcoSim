// Interface d'affichage : le monde ne dépend d'aucune bibliothèque graphique.
export module ecosim.render;

import std;
import ecosim.simulation;

export namespace ecosim {
    class Renderer2D {
    public:
        Renderer2D(int cellSize, const char* title);
        ~Renderer2D();

        Renderer2D(const Renderer2D&) = delete;
        Renderer2D& operator=(const Renderer2D&) = delete;

        int GetWidth() const;
        int GetHeight() const;
        bool ShouldClose() const;
        double FrameTime() const;
        std::optional<int> SpeedKeyPressed() const;
        void HandleInput();
        void Draw(const Simulation& simulation, int speed) const;

    private:
        void DrawWorld(const Simulation& simulation, int speed) const;
        void DrawGraph(const Simulation& simulation, int speed) const;

        int cellSize;
        int windowWidth{0};
        int windowHeight{0};
        void* window{};
        bool showingGraph{false};
        bool leftMouseDown{false};
        mutable bool closed{false};
        mutable std::chrono::steady_clock::time_point lastFrame{};
    };
}
