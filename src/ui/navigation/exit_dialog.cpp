module ecosim.exit_dialog;

import std;
import ecosim.layout;

namespace ecosim {
    ExitDialog::ExitDialog(const int width, const int height) : width{width}, height{height} {}

    bool ExitDialog::IsOpen() const { return open; }

    UiAction ExitDialog::Toggle() {
        open = !open;
        return open ? UiAction::None : UiAction::CancelExit;
    }

    UiAction ExitDialog::Click(const PointerPosition mouse) {
        if (ExitMenuButton(width, height).Contains(mouse.x, mouse.y)) {
            open = false;
            return UiAction::ReturnHome;
        }
        if (ExitQuitButton(width, height).Contains(mouse.x, mouse.y)) return UiAction::Quit;
        if (ExitCancelButton(width, height).Contains(mouse.x, mouse.y)) {
            open = false;
            return UiAction::CancelExit;
        }
        return UiAction::None;
    }
}
