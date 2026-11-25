// Confirmation affichée par Échap pendant la simulation.
export module ecosim.exit_dialog;

import std;
export import ecosim.input;

export namespace ecosim {
    class ExitDialog {
    public:
        ExitDialog(int width, int height);

        bool IsOpen() const;
        // Appui sur Échap : ouvre la confirmation, ou l'annule.
        UiAction Toggle();
        UiAction Click(PointerPosition mouse);

    private:
        int width;
        int height;
        bool open{false};
    };
}
