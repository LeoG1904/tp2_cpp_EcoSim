// Fenêtre Windows en plein écran et réception de ses messages.
export module ecosim.window;

import std;

export namespace ecosim {
    class Win32Window {
    public:
        explicit Win32Window(const std::string& title);
        ~Win32Window();

        Win32Window(const Win32Window&) = delete;
        Win32Window& operator=(const Win32Window&) = delete;

        int Width() const;
        int Height() const;
        // Traite les messages en attente. Renvoie false quand la fenêtre est fermée.
        bool ProcessEvents();
        bool HasFocus() const;
        // Codes des touches enfoncées depuis le dernier appel.
        std::vector<int> TakeKeys();
        // Crans de molette depuis le dernier appel, positifs vers l'avant.
        int TakeWheel();
        void* Handle() const;

        // Rempli par la procédure de la fenêtre à la réception des messages.
        struct Events {
            std::vector<int> keys;
            int wheelDelta{0};
        };

    private:
        int width{0};
        int height{0};
        void* handle{};
        bool closed{false};
        Events events;
    };
}
