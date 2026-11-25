// Lecture du clavier et de la souris sous Windows.
export module ecosim.win32_input;

import std;
export import ecosim.application;
export import ecosim.window;

export namespace ecosim {
    class Win32Input final : public InputSource {
    public:
        // window et controller doivent rester valides pendant la vie de cet objet.
        Win32Input(Win32Window& window, UiController& controller);

        FrameInput Read(bool isHomeScreen, bool paused, SimulationSettings& settings) override;

    private:
        RawInput ReadRawInput();
        static std::optional<int> SpeedKeyPressed();

        Win32Window& window;
        UiController& controller;
        bool leftMouseDown{false};
        bool spaceWasDown{false};
        bool escapeWasDown{false};
        bool tabWasDown{false};
        bool rightWasDown{false};
        bool upWasDown{false};
    };
}
