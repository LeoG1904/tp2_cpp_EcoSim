/**
 * @file
 * @brief La fenêtre Windows en plein écran.
 */
export module ecosim.window;

import std;

export namespace ecosim {
    /// @brief La fenêtre de l'application, détruite automatiquement avec l'objet.
    /// @ingroup affichage
    class Win32Window {
    public:
        /// @brief Ouvre une fenêtre de la taille de l'écran.
        /// @throws std::runtime_error si la création échoue.
        explicit Win32Window(const std::string& title);
        ~Win32Window();

        Win32Window(const Win32Window&) = delete;
        Win32Window& operator=(const Win32Window&) = delete;

        /// @brief Largeur, en pixels.
        int Width() const;
        /// @brief Hauteur, en pixels.
        int Height() const;
        /// @brief Traite les messages de Windows.
        /// @return Faux quand la fenêtre est fermée.
        bool ProcessEvents();
        /// @brief Vrai si la fenêtre est au premier plan.
        bool HasFocus() const;
        /// @brief Les touches enfoncées depuis le dernier appel.
        std::vector<int> TakeKeys();
        /// @brief Les crans de molette depuis le dernier appel.
        int TakeWheel();
        /// @brief Identifiant Windows de la fenêtre.
        void* Handle() const;

        /// @brief Ce que la fenêtre reçoit entre deux lectures.
        struct Events {
            /// @brief Codes des touches enfoncées.
            std::vector<int> keys;
            /// @brief Mouvement de molette accumulé.
            int wheelDelta{0};
        };

    private:
        /// @brief Largeur de la fenêtre, en pixels.
        int width{0};
        /// @brief Hauteur de la fenêtre, en pixels.
        int height{0};
        /// @brief Identifiant Windows de la fenêtre.
        void* handle{};
        /// @brief Vrai après la demande de fermeture.
        bool closed{false};
        /// @brief Messages reçus et pas encore lus.
        Events events;
    };
}
