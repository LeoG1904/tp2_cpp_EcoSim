/**
 * @file
 * @brief Conversion des touches et des clics Windows en entrées.
 */
module;

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

module ecosim.win32_input;

import std;

namespace {
    /// @brief Convertit un code Windows en touche de saisie ; vide pour les autres touches. Les chiffres fonctionnent sans Maj en AZERTY.
    std::optional<ecosim::KeyStroke> ToKeyStroke(const int key) {
        using Kind = ecosim::KeyStroke::Kind;
        if (key >= '0' && key <= '9') return ecosim::KeyStroke{Kind::Digit, key - '0'};
        if (key >= VK_NUMPAD0 && key <= VK_NUMPAD9) {
            return ecosim::KeyStroke{Kind::Digit, key - VK_NUMPAD0};
        }
        if (key == VK_BACK || key == VK_DELETE) return ecosim::KeyStroke{Kind::Erase};
        if (key == VK_RETURN) return ecosim::KeyStroke{Kind::Confirm};
        if (key == VK_TAB) return ecosim::KeyStroke{Kind::Next};
        return std::nullopt;
    }
}

namespace ecosim {
    Win32Input::Win32Input(Win32Window& window, UiController& controller)
        : window{window}, controller{controller} {}

    FrameInput Win32Input::Read(const bool isHomeScreen, const bool paused,
                                SimulationSettings& settings) {
        std::optional<int> speed;
        const RawInput input = ReadRawInput(speed);
        const UiAction action = controller.Handle(input, isHomeScreen, paused, settings);
        return {action, speed, controller.IsExitDialogOpen()};
    }

    RawInput Win32Input::ReadRawInput(std::optional<int>& speed) {
        RawInput input;
        for (const Win32Window::KeyPress key : window.TakeKeys()) {
            // La saisie accepte les répétitions : on peut maintenir la touche d'effacement.
            if (const auto stroke = ToKeyStroke(key.code)) input.typed.push_back(*stroke);
            // Une commande n'agit qu'une fois par appui.
            if (key.repeated) continue;
            if (key.code == VK_ESCAPE) input.escape = true;
            if (key.code == VK_SPACE) input.space = true;
            if (key.code == VK_TAB) input.tab = true;
            if (key.code == VK_RIGHT) input.right = true;
            if (key.code == VK_UP) input.up = true;
            // Les touches 1 à 4 suivent l'ordre des vitesses de l'horloge.
            const int rank = key.code - '1';
            if (rank >= 0 && rank < static_cast<int>(SimulationClock::Speeds.size())) {
                speed = SimulationClock::Speeds[static_cast<std::size_t>(rank)];
            }
        }
        const int wheel = window.TakeWheel();

        // La souris est suivie même sans le focus, pour ne pas prendre un bouton maintenu pour un clic.
        const bool leftDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
        const bool click = leftDown && !leftMouseDown;
        leftMouseDown = leftDown;
        if (!window.HasFocus()) return input;

        input.wheel = wheel;
        input.rotating = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
        POINT mouse{};
        if (GetCursorPos(&mouse) && ScreenToClient(static_cast<HWND>(window.Handle()), &mouse)) {
            input.pointer = PointerPosition{mouse.x, mouse.y};
            if (click) input.click = input.pointer;
        }
        return input;
    }
}
