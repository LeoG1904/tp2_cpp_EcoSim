module;

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

module ecosim.window;

import std;

namespace {
    LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wp, LPARAM lp) {
        // La fenêtre range les messages dans l'objet qui la possède.
        auto* events = reinterpret_cast<ecosim::Win32Window::Events*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (message == WM_KEYDOWN) {
            if (events) events->keys.push_back(static_cast<int>(wp));
            return 0;
        }
        if (message == WM_MOUSEWHEEL) {
            if (events) events->wheelDelta += GET_WHEEL_DELTA_WPARAM(wp);
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
    Win32Window::Win32Window(const std::string& title)
        : width{GetSystemMetrics(SM_CXSCREEN)}, height{GetSystemMetrics(SM_CYSCREEN)} {
        const HINSTANCE instance = GetModuleHandleW(nullptr);
        WNDCLASSW windowClass{};
        windowClass.lpfnWndProc = WindowProcedure;
        windowClass.hInstance = instance;
        windowClass.lpszClassName = L"EcoSimWindow";
        windowClass.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
        RegisterClassW(&windowClass);
        handle = CreateWindowExA(0, "EcoSimWindow", title.c_str(), WS_POPUP | WS_VISIBLE,
                                 0, 0, width, height, nullptr, nullptr, instance, nullptr);
        if (!handle) throw std::runtime_error{"Impossible de créer la fenêtre EcoSim"};
        SetWindowLongPtrW(static_cast<HWND>(handle), GWLP_USERDATA,
                          reinterpret_cast<LONG_PTR>(&events));
    }

    Win32Window::~Win32Window() {
        const auto hwnd = static_cast<HWND>(handle);
        if (hwnd && IsWindow(hwnd)) DestroyWindow(hwnd);
    }

    int Win32Window::Width() const { return width; }
    int Win32Window::Height() const { return height; }
    void* Win32Window::Handle() const { return handle; }

    bool Win32Window::ProcessEvents() {
        MSG message{};
        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT) closed = true;
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
        return !closed;
    }

    bool Win32Window::HasFocus() const {
        return GetForegroundWindow() == static_cast<HWND>(handle);
    }

    std::vector<int> Win32Window::TakeKeys() { return std::exchange(events.keys, {}); }

    int Win32Window::TakeWheel() {
        // Les pavés tactiles envoient des fractions de cran : le reste est conservé.
        const int notches = events.wheelDelta / WHEEL_DELTA;
        events.wheelDelta -= notches * WHEEL_DELTA;
        return notches;
    }
}
