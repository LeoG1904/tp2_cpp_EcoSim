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
        FrameInput Read(bool isHomeScreen, bool paused, SimulationSettings& settings,
                        const World* world) override;

    private:
        /// @brief Lit les touches reçues, puis la souris si la fenêtre a le focus.
        /// @param speed Reçoit la vitesse demandée par les touches 1 à 4, le cas échéant.
        RawInput ReadRawInput(std::optional<int>& speed);

        /// @brief Fenêtre dont on lit les entrées.
        Win32Window& window;
        /// @brief Contrôleur qui interprète les entrées.
        UiController& controller;
        /// @brief Bouton gauche enfoncé à l'image précédente.
        bool leftMouseDown{false};
    };
}
