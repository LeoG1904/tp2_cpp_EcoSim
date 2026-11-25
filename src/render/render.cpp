module;

#ifdef ECOSIM_USE_RAYLIB
#include <raylib.h>
#else
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

#include "layout.hpp"

module ecosim.render;

import std;
import ecosim.simulation;

using namespace ecosim_render_detail;

namespace {
#ifndef ECOSIM_USE_RAYLIB
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

    void Renderer2D::HandleInput() {
#ifdef ECOSIM_USE_RAYLIB
        if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;
        const Vector2 mouse = GetMousePosition();
        const int mouseX = static_cast<int>(mouse.x);
        const int mouseY = static_cast<int>(mouse.y);
#else
        const bool pressed = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
        const bool wasPressed = leftMouseDown;
        leftMouseDown = pressed;
        if (!pressed || wasPressed || GetForegroundWindow() != static_cast<HWND>(window)) return;
        POINT mouse{};
        if (!GetCursorPos(&mouse) || !ScreenToClient(static_cast<HWND>(window), &mouse)) return;
        const int mouseX = mouse.x;
        const int mouseY = mouse.y;
#endif
        const UiRect button = showingGraph ? BackButton() : GraphButton(windowWidth);
        if (button.Contains(mouseX, mouseY)) showingGraph = !showingGraph;
    }

    void Renderer2D::Draw(const Simulation& simulation, const int speed) const {
        if (showingGraph) DrawGraph(simulation, speed);
        else DrawWorld(simulation, speed);
    }
}
