module;
// Raylib est une bibliothèque C : on l'inclut dans le fragment global du module
#include <raylib.h>

// Affichage 2D de la carte avec Raylib. Ce module lit l'état du monde
// mais ne le modifie jamais : la logique reste indépendante de l'affichage.
export module ecosim.render;

import std;
import ecosim.world;

export namespace ecosim {

    // Ouvre une fenêtre qui couvre tout l'écran (sans bordure) et dessine la carte,
    // une case de cellSize pixels par cellule.
    // La fenêtre se ferme automatiquement quand l'objet est détruit (RAII).
    class Renderer2D {
    public:
        Renderer2D(const int cellSize, const char* title) : cellSize{cellSize} {
            // Fenêtre sans bordure, pour qu'elle recouvre exactement l'écran
            SetConfigFlags(FLAG_WINDOW_UNDECORATED);
            InitWindow(800, 600, title);

            const int monitor = GetCurrentMonitor();
            windowWidth = GetMonitorWidth(monitor);
            windowHeight = GetMonitorHeight(monitor);
            const Vector2 origin = GetMonitorPosition(monitor);
            SetWindowSize(windowWidth, windowHeight);
            SetWindowPosition(static_cast<int>(origin.x), static_cast<int>(origin.y));

            SetTargetFPS(60);
        }

        ~Renderer2D() { CloseWindow(); }

        Renderer2D(const Renderer2D&) = delete;
        Renderer2D& operator=(const Renderer2D&) = delete;

        // Taille de la fenêtre en pixels (égale à la taille de l'écran)
        int GetWidth() const { return windowWidth; }

        int GetHeight() const { return windowHeight; }

        bool ShouldClose() const { return WindowShouldClose(); }

        // Dessine une image complète de la carte
        void Draw(const World& world) const {
            BeginDrawing();
            ClearBackground(BackgroundColor());

            for (int y = 0; y < world.GetHeight(); ++y) {
                for (int x = 0; x < world.GetWidth(); ++x) {
                    const Cell& cell = world.GetCell(Position{x, y});
                    // Un écart d'un pixel entre les cases fait apparaître la grille
                    DrawRectangle(x * cellSize, y * cellSize, cellSize - 1, cellSize - 1, ColorOf(cell));
                }
            }

            EndDrawing();
        }

    private:
        static Color BackgroundColor() { return Color{30, 30, 30, 255}; }

        // Couleur d'une cellule : eau bleue, obstacle gris, prairie plus ou moins verte
        // selon la végétation (du vert clair à vide au vert foncé à pleine végétation)
        static Color ColorOf(const Cell& cell) {
            switch (cell.GetTerrain()) {
                case Terrain::Eau:
                    return Color{60, 120, 200, 255};
                case Terrain::Obstacle:
                    return Color{90, 90, 90, 255};
                case Terrain::Prairie:
                default:
                    break;
            }
            const double ratio = static_cast<double>(cell.GetVegetation()) / Cell::MaxVegetation;
            const auto green = static_cast<unsigned char>(110 + ratio * (200 - 110));
            const auto red = static_cast<unsigned char>(150 - ratio * 90);
            return Color{red, green, 70, 255};
        }

        int cellSize;
        int windowWidth{0};
        int windowHeight{0};
    };

}
