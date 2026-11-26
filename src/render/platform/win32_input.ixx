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
        /// @brief Lit l'état du clavier et de la souris ; les appuis sont ignorés sans le focus.
        RawInput ReadRawInput();
        /// @brief Vitesse demandée par les touches 1 à 4, le cas échéant.
        static std::optional<int> SpeedKeyPressed();

        /// @brief Fenêtre dont on lit les entrées.
        Win32Window& window;
        /// @brief Contrôleur qui interprète les entrées.
        UiController& controller;
        /// @brief Bouton gauche enfoncé à l'image précédente.
        bool leftMouseDown{false};
        /// @brief Espace enfoncée à l'image précédente.
        bool spaceWasDown{false};
        /// @brief Échap enfoncée à l'image précédente.
        bool escapeWasDown{false};
        /// @brief Tabulation enfoncée à l'image précédente.
        bool tabWasDown{false};
        /// @brief Flèche droite enfoncée à l'image précédente.
        bool rightWasDown{false};
        /// @brief Flèche haut enfoncée à l'image précédente.
        bool upWasDown{false};
    };
}
