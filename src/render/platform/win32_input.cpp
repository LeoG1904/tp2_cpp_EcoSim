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
    /// @brief Vrai seulement à l'image où la touche vient d'être enfoncée.
    /// @param key Code Windows de la touche.
    /// @param wasDown État précédent, mis à jour.
    bool JustPressed(const int key, bool& wasDown) {
        const bool down = (GetAsyncKeyState(key) & 0x8000) != 0;
        const bool result = down && !wasDown;
        wasDown = down;
        return result;
    }

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
        const UiAction action = controller.Handle(ReadRawInput(), isHomeScreen, paused, settings);
        // Lue à chaque image pour oublier les appuis faits sur l'accueil ou la confirmation.
        const std::optional<int> speed = SpeedKeyPressed();
        return {action, speed, controller.IsExitDialogOpen()};
    }

    RawInput Win32Input::ReadRawInput() {
        RawInput input;
        for (const int key : window.TakeKeys()) {
            if (const auto stroke = ToKeyStroke(key)) input.typed.push_back(*stroke);
        }
        const int wheel = window.TakeWheel();
        // L'état des touches est suivi même sans le focus ; les appuis n'agissent qu'avec lui.
        const bool escape = JustPressed(VK_ESCAPE, escapeWasDown);
        const bool space = JustPressed(VK_SPACE, spaceWasDown);
        const bool tab = JustPressed(VK_TAB, tabWasDown);
        const bool right = JustPressed(VK_RIGHT, rightWasDown);
        const bool up = JustPressed(VK_UP, upWasDown);
        const bool click = JustPressed(VK_LBUTTON, leftMouseDown);
        if (!window.HasFocus()) return input;
        input.escape = escape;
        input.space = space;
        // Alt+Tab sert à changer de fenêtre, pas de vitesse.
        input.tab = tab && (GetAsyncKeyState(VK_MENU) & 0x8000) == 0;
        input.right = right;
        input.up = up;
        input.wheel = wheel;
        input.rotating = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
        POINT mouse{};
        if (GetCursorPos(&mouse) && ScreenToClient(static_cast<HWND>(window.Handle()), &mouse)) {
            input.pointer = PointerPosition{mouse.x, mouse.y};
            if (click) input.click = input.pointer;
        }
        return input;
    }

    // Les touches 1 à 4 suivent l'ordre des vitesses de l'horloge.
    std::optional<int> Win32Input::SpeedKeyPressed() {
        for (std::size_t i = 0; i < SimulationClock::Speeds.size(); ++i) {
            if (GetAsyncKeyState('1' + static_cast<int>(i)) & 1) return SimulationClock::Speeds[i];
        }
        return std::nullopt;
    }
}
