module;

#ifdef ECOSIM_USE_RAYLIB
#include <raylib.h>
#else
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

module ecosim.render;

import std;
import ecosim.simulation;

namespace {
#ifdef ECOSIM_USE_RAYLIB
    Color ColorOf(const ecosim::Cell& cell) {
        switch (cell.GetTerrain()) {
            case ecosim::Terrain::Eau: return Color{60, 120, 200, 255};
            case ecosim::Terrain::Obstacle: return Color{90, 90, 90, 255};
            case ecosim::Terrain::Prairie: break;
        }
        const double ratio = static_cast<double>(cell.GetVegetation()) / ecosim::Cell::MaxVegetation;
        return Color{static_cast<unsigned char>(150 - ratio * 90),
                     static_cast<unsigned char>(110 + ratio * 90), 70, 255};
    }
#else
    LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wp, LPARAM lp) {
        if (message == WM_CLOSE) {
            DestroyWindow(hwnd);
            return 0;
        }
        if (message == WM_DESTROY) {
            PostQuitMessage(0);
            return 0;
        }
        return DefWindowProcW(hwnd, message, wp, lp);
    }

    COLORREF ColorOf(const ecosim::Cell& cell) {
        switch (cell.GetTerrain()) {
            case ecosim::Terrain::Eau: return RGB(60, 120, 200);
            case ecosim::Terrain::Obstacle: return RGB(90, 90, 90);
            case ecosim::Terrain::Prairie: break;
        }
        const double ratio = static_cast<double>(cell.GetVegetation()) / ecosim::Cell::MaxVegetation;
        return RGB(static_cast<int>(150 - ratio * 90),
                   static_cast<int>(110 + ratio * 90), 70);
    }
#endif
}

namespace ecosim {
    Renderer2D::Renderer2D(const int size, const char* title) : cellSize{size} {
#ifdef ECOSIM_USE_RAYLIB
        SetConfigFlags(FLAG_WINDOW_UNDECORATED);
        InitWindow(800, 600, title);
        const int monitor = GetCurrentMonitor();
        windowWidth = GetMonitorWidth(monitor);
        windowHeight = GetMonitorHeight(monitor);
        const Vector2 origin = GetMonitorPosition(monitor);
        SetWindowSize(windowWidth, windowHeight);
        SetWindowPosition(static_cast<int>(origin.x), static_cast<int>(origin.y));
        SetTargetFPS(60);
#else
        windowWidth = GetSystemMetrics(SM_CXSCREEN);
        windowHeight = GetSystemMetrics(SM_CYSCREEN);
        const HINSTANCE instance = GetModuleHandleW(nullptr);
        WNDCLASSW windowClass{};
        windowClass.lpfnWndProc = WindowProcedure;
        windowClass.hInstance = instance;
        windowClass.lpszClassName = L"EcoSimWindow";
        windowClass.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
        RegisterClassW(&windowClass);
        window = CreateWindowExA(0, "EcoSimWindow", title, WS_POPUP | WS_VISIBLE,
                                 0, 0, windowWidth, windowHeight,
                                 nullptr, nullptr, instance, nullptr);
        if (!window) throw std::runtime_error{"Impossible de créer la fenêtre EcoSim"};
        lastFrame = std::chrono::steady_clock::now();
#endif
    }

    Renderer2D::~Renderer2D() {
#ifdef ECOSIM_USE_RAYLIB
        CloseWindow();
#else
        const auto hwnd = static_cast<HWND>(window);
        if (hwnd && IsWindow(hwnd)) DestroyWindow(hwnd);
#endif
    }

    int Renderer2D::GetWidth() const { return windowWidth; }
    int Renderer2D::GetHeight() const { return windowHeight; }

    bool Renderer2D::ShouldClose() const {
#ifdef ECOSIM_USE_RAYLIB
        return WindowShouldClose();
#else
        MSG message{};
        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT) closed = true;
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
        return closed || (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0;
#endif
    }

    double Renderer2D::FrameTime() const {
#ifdef ECOSIM_USE_RAYLIB
        return GetFrameTime();
#else
        const auto now = std::chrono::steady_clock::now();
        const double seconds = std::chrono::duration<double>(now - lastFrame).count();
        lastFrame = now;
        return seconds;
#endif
    }

    std::optional<int> Renderer2D::SpeedKeyPressed() const {
#ifdef ECOSIM_USE_RAYLIB
        if (IsKeyPressed(KEY_ONE)) return 1;
        if (IsKeyPressed(KEY_TWO)) return 2;
        if (IsKeyPressed(KEY_THREE)) return 5;
        if (IsKeyPressed(KEY_FOUR)) return 10;
#else
        if (GetAsyncKeyState('1') & 1) return 1;
        if (GetAsyncKeyState('2') & 1) return 2;
        if (GetAsyncKeyState('3') & 1) return 5;
        if (GetAsyncKeyState('4') & 1) return 10;
#endif
        return std::nullopt;
    }

    void Renderer2D::Draw(const Simulation& simulation, const int speed) const {
        const World& world = simulation.GetWorld();
        std::vector<int> herbivores(static_cast<std::size_t>(world.GetWidth()) * world.GetHeight());
        std::vector<int> predators(herbivores.size());
        for (const auto& animal : simulation.Animals()) {
            const auto index = static_cast<std::size_t>(animal.position.y) * world.GetWidth()
                             + static_cast<std::size_t>(animal.position.x);
            if (animal.kind == AnimalKind::Herbivore) ++herbivores[index];
            else if (animal.kind == AnimalKind::Predator) ++predators[index];
        }
#ifdef ECOSIM_USE_RAYLIB
        BeginDrawing();
        ClearBackground(Color{30, 30, 30, 255});
        for (int y = 0; y < world.GetHeight(); ++y) {
            for (int x = 0; x < world.GetWidth(); ++x) {
                DrawRectangle(x * cellSize, y * cellSize, cellSize - 1, cellSize - 1,
                              ColorOf(world.GetCell(Position{x, y})));
                const auto index = static_cast<std::size_t>(y) * world.GetWidth() + x;
                const int herbivoreCount = herbivores[index];
                const int predatorCount = predators[index];
                const bool shared = herbivoreCount > 0 && predatorCount > 0;
                const int centerY = y * cellSize + cellSize / 2;
                const int radius = shared ? cellSize / 5 : cellSize * 36 / 100;
                const auto drawAnimal = [&](const int count, const int centerX, const Color color) {
                    if (count == 0) return;
                    DrawCircle(centerX, centerY, static_cast<float>(radius), color);
                    if (count > 1) {
                        const std::string label = std::to_string(count);
                        DrawText(label.c_str(), centerX - MeasureText(label.c_str(), 15) / 2,
                                 centerY - 7, 15, BLACK);
                    }
                };
                drawAnimal(herbivoreCount, x * cellSize + (shared ? cellSize / 4 : cellSize / 2),
                           Color{245, 210, 70, 255});
                drawAnimal(predatorCount, x * cellSize + (shared ? 3 * cellSize / 4 : cellSize / 2),
                           Color{220, 85, 75, 255});
            }
        }
        const std::string hud = std::format(
            "Génération {}   Herbivores {}   Prédateurs {}   Vitesse x{}   (touches 1 à 4 : x1 x2 x5 x10)",
            simulation.GetGeneration(), simulation.HerbivoreCount(), simulation.PredatorCount(), speed);
        DrawText(hud.c_str(), 10, 10, 20, RAYWHITE);
        EndDrawing();
#else
        const auto hwnd = static_cast<HWND>(window);
        const HDC screen = GetDC(hwnd);
        const HDC buffer = CreateCompatibleDC(screen);
        const HBITMAP bitmap = CreateCompatibleBitmap(screen, windowWidth, windowHeight);
        const HGDIOBJ previous = SelectObject(buffer, bitmap);
        const RECT bounds{0, 0, windowWidth, windowHeight};
        const HBRUSH background = CreateSolidBrush(RGB(30, 30, 30));
        FillRect(buffer, &bounds, background);
        DeleteObject(background);

        for (int y = 0; y < world.GetHeight(); ++y) {
            for (int x = 0; x < world.GetWidth(); ++x) {
                const HBRUSH brush = CreateSolidBrush(ColorOf(world.GetCell(Position{x, y})));
                const RECT cell{x * cellSize, y * cellSize,
                                (x + 1) * cellSize - 1, (y + 1) * cellSize - 1};
                FillRect(buffer, &cell, brush);
                DeleteObject(brush);
            }
        }

        const HBRUSH animalBrush = CreateSolidBrush(RGB(245, 210, 70));
        const HBRUSH predatorBrush = CreateSolidBrush(RGB(220, 85, 75));
        const HGDIOBJ previousBrush = SelectObject(buffer, animalBrush);
        const HGDIOBJ previousPen = SelectObject(buffer, GetStockObject(NULL_PEN));
        SetBkMode(buffer, TRANSPARENT);
        for (int y = 0; y < world.GetHeight(); ++y) {
            for (int x = 0; x < world.GetWidth(); ++x) {
                const auto index = static_cast<std::size_t>(y) * world.GetWidth() + x;
                const int herbivoreCount = herbivores[index];
                const int predatorCount = predators[index];
                if (herbivoreCount == 0 && predatorCount == 0) continue;
                const bool shared = herbivoreCount > 0 && predatorCount > 0;
                const int centerY = y * cellSize + cellSize / 2;
                const int radius = shared ? cellSize / 5 : cellSize * 36 / 100;
                const auto drawAnimal = [&](const int count, const int centerX,
                                            const HBRUSH brush) {
                    if (count == 0) return;
                    SelectObject(buffer, brush);
                    Ellipse(buffer, centerX - radius, centerY - radius,
                            centerX + radius, centerY + radius);
                    if (count > 1) {
                        const std::wstring label = std::to_wstring(count);
                        SIZE extent{};
                        GetTextExtentPoint32W(buffer, label.c_str(), static_cast<int>(label.size()), &extent);
                        SetTextColor(buffer, RGB(30, 30, 30));
                        TextOutW(buffer, centerX - extent.cx / 2, centerY - extent.cy / 2,
                                 label.c_str(), static_cast<int>(label.size()));
                    }
                };
                drawAnimal(herbivoreCount, x * cellSize + (shared ? cellSize / 4 : cellSize / 2),
                           animalBrush);
                drawAnimal(predatorCount, x * cellSize + (shared ? 3 * cellSize / 4 : cellSize / 2),
                           predatorBrush);
            }
        }
        SelectObject(buffer, previousPen);
        SelectObject(buffer, previousBrush);
        DeleteObject(animalBrush);
        DeleteObject(predatorBrush);

        const auto hud = std::format(L"Génération {}   Herbivores {}   Prédateurs {}   Vitesse x{}   (touches 1 à 4 : x1 x2 x5 x10 ; Échap : quitter)",
                                     simulation.GetGeneration(), simulation.HerbivoreCount(), simulation.PredatorCount(), speed);
        SetTextColor(buffer, RGB(255, 255, 255));
        TextOutW(buffer, 10, 10, hud.c_str(), static_cast<int>(hud.size()));
        BitBlt(screen, 0, 0, windowWidth, windowHeight, buffer, 0, 0, SRCCOPY);
        SelectObject(buffer, previous);
        DeleteObject(bitmap);
        DeleteDC(buffer);
        ReleaseDC(hwnd, screen);
        Sleep(16);
#endif
    }
}
