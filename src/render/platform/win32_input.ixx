/**
 * @file
 * @brief La lecture du clavier et de la souris.
 */
export module ecosim.win32_input;

import std;
export import ecosim.application;
export import ecosim.window;

export namespace ecosim {
    /// @brief La source des commandes réelles de l'utilisateur.
    /// @ingroup affichage
    class Win32Input final : public InputSource {
    public:
        /// @brief Crée le lecteur. La fenêtre et le contrôleur doivent vivre aussi longtemps que lui.
        Win32Input(Win32Window& window, UiController& controller);

        /// @copydoc InputSource::Read
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
