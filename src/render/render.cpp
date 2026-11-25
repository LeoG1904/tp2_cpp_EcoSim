module;

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "layout.hpp"

module ecosim.render;

import std;
import ecosim.simulation;

using namespace ecosim_render_detail;

namespace {
    // Touches reçues par la fenêtre depuis la dernière lecture des entrées.
    std::vector<int> pendingKeys;

    LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wp, LPARAM lp) {
        if (message == WM_KEYDOWN) {
            pendingKeys.push_back(static_cast<int>(wp));
            return 0;
        }
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

}

namespace ecosim {
    Renderer2D::Renderer2D(const int size, const char* title) : cellSize{size} {
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
    }

    Renderer2D::~Renderer2D() {
        const auto hwnd = static_cast<HWND>(window);
        if (hwnd && IsWindow(hwnd)) DestroyWindow(hwnd);
    }

    int Renderer2D::GetWidth() const { return windowWidth; }
    int Renderer2D::GetHeight() const { return windowHeight; }

    bool Renderer2D::ShouldClose() const {
        MSG message{};
        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT) closed = true;
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
        return closed;
    }

    bool Renderer2D::IsExitDialogOpen() const { return showingExitDialog; }

    double Renderer2D::FrameTime() const {
        const auto now = std::chrono::steady_clock::now();
        const double seconds = std::chrono::duration<double>(now - lastFrame).count();
        lastFrame = now;
        return seconds;
    }

    std::optional<int> Renderer2D::SpeedKeyPressed() const {
        if (GetAsyncKeyState('1') & 1) return 1;
        if (GetAsyncKeyState('2') & 1) return 2;
        if (GetAsyncKeyState('3') & 1) return 5;
        if (GetAsyncKeyState('4') & 1) return 10;
        return std::nullopt;
    }

    void Renderer2D::HandleSettingsKeys(const std::span<const int> keys, SimulationSettings& settings) {
        for (const int key : keys) {
            const auto field = settingEditor.Field();
            if (!field) return;
            // Les codes de touche évitent d'avoir à maintenir Maj sur un clavier AZERTY.
            if (key >= '0' && key <= '9') settingEditor.Type(key - '0');
            else if (key >= VK_NUMPAD0 && key <= VK_NUMPAD9) settingEditor.Type(key - VK_NUMPAD0);
            else if (key == VK_BACK || key == VK_DELETE) settingEditor.Erase();
            else if (key == VK_RETURN) settingEditor.Commit(settings);
            else if (key == VK_TAB) {
                settingEditor.Commit(settings);
                settingEditor.Begin((*field + 1) % SettingFields().size(), settings);
            }
        }
    }

    void Renderer2D::HandleSettingsClick(const int mouseX, const int mouseY,
                                         SimulationSettings& settings) {
        const auto fields = SettingFields();
        std::array<int, 3> rows{};
        for (std::size_t i = 0; i < fields.size(); ++i) {
            const int column = static_cast<int>(fields[i].group);
            const int row = rows[column]++;
            if (!ValueBox(windowWidth, windowHeight, column, row).Contains(mouseX, mouseY)) continue;
            if (settingEditor.Field() != i) {
                settingEditor.Commit(settings);
                settingEditor.Begin(i, settings);
            }
            return;
        }
        // Un clic en dehors des cases valide la saisie en cours.
        settingEditor.Commit(settings);
    }

    UiAction Renderer2D::HandleInput(const bool isHomeScreen, const bool paused,
                                     SimulationSettings& settings) {
        const std::vector<int> keys = std::exchange(pendingKeys, {});
        if (isHomeScreen) HandleSettingsKeys(keys, settings);
        const bool escapeDown = (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0;
        const bool escapePressed = escapeDown && !escapeWasDown;
        escapeWasDown = escapeDown;
        const bool spaceDown = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;
        const bool spacePressed = spaceDown && !spaceWasDown;
        spaceWasDown = spaceDown;
        // Un appui ne compte qu'une fois, même si la touche reste enfoncée.
        const auto justPressed = [](const int key, bool& wasDown) {
            const bool down = (GetAsyncKeyState(key) & 0x8000) != 0;
            const bool result = down && !wasDown;
            wasDown = down;
            return result;
        };
        const bool tabPressed = justPressed(VK_TAB, tabWasDown);
        const bool rightPressed = justPressed(VK_RIGHT, rightWasDown);
        const bool upPressed = justPressed(VK_UP, upWasDown);
        const bool pressed = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
        const bool wasPressed = leftMouseDown;
        leftMouseDown = pressed;
        const bool focused = GetForegroundWindow() == static_cast<HWND>(window);
        if (escapePressed && focused) {
            if (isHomeScreen) {
                // Pendant une saisie, Échap annule au lieu de quitter.
                if (!settingEditor.Field()) return UiAction::Quit;
                settingEditor.Cancel();
                return UiAction::None;
            }
            showingExitDialog = !showingExitDialog;
            return showingExitDialog ? UiAction::None : UiAction::CancelExit;
        }
        if (spacePressed && !isHomeScreen && !showingExitDialog && focused) {
            return UiAction::TogglePause;
        }
        if (!isHomeScreen && !showingExitDialog && focused) {
            // Alt+Tab sert à changer de fenêtre, pas de vitesse.
            if (tabPressed && (GetAsyncKeyState(VK_MENU) & 0x8000) == 0) return UiAction::CycleSpeed;
            if (paused && (rightPressed || upPressed)) return UiAction::NextGeneration;
        }
        if (!pressed || wasPressed || !focused) return UiAction::None;
        POINT mouse{};
        if (!GetCursorPos(&mouse) || !ScreenToClient(static_cast<HWND>(window), &mouse)) return UiAction::None;
        const int mouseX = mouse.x;
        const int mouseY = mouse.y;
        if (showingExitDialog) {
            if (ExitMenuButton(windowWidth, windowHeight).Contains(mouseX, mouseY)) {
                showingExitDialog = false;
                showingGraph = false;
                return UiAction::ReturnHome;
            }
            if (ExitQuitButton(windowWidth, windowHeight).Contains(mouseX, mouseY)) return UiAction::Quit;
            if (ExitCancelButton(windowWidth, windowHeight).Contains(mouseX, mouseY)) {
                showingExitDialog = false;
                return UiAction::CancelExit;
            }
            return UiAction::None;
        }
        if (isHomeScreen) {
            HandleSettingsClick(mouseX, mouseY, settings);
            if (StartButton(windowWidth, windowHeight).Contains(mouseX, mouseY)) {
                return UiAction::StartOrRestart;
            }
            return ResetButton(windowWidth, windowHeight).Contains(mouseX, mouseY)
                ? UiAction::ResetSettings : UiAction::None;
        }
        if (RestartButton(windowWidth).Contains(mouseX, mouseY)) {
            showingGraph = false;
            return UiAction::StartOrRestart;
        }
        if (PauseButton(windowWidth, showingGraph).Contains(mouseX, mouseY)) return UiAction::TogglePause;
        if (paused && NextButton(windowWidth, showingGraph).Contains(mouseX, mouseY)) {
            return UiAction::NextGeneration;
        }
        const UiRect button = showingGraph ? BackButton() : GraphButton(windowWidth);
        if (button.Contains(mouseX, mouseY)) showingGraph = !showingGraph;
        return UiAction::None;
    }

    void Renderer2D::Draw(const Simulation& simulation, const int speed, const bool paused) const {
        if (showingGraph) DrawGraph(simulation, speed, paused);
        else DrawWorld(simulation, speed, paused);
    }
}
