module ecosim.home_controller;

import std;
import ecosim.layout;

namespace ecosim {
    HomeController::HomeController(const int width, const int height, const int cellSize)
        : width{width}, height{height}, cellSize{cellSize} {}

    const SettingEditor& HomeController::Editor() const { return editor; }

    void HomeController::Type(const std::vector<KeyStroke>& typed, SimulationSettings& settings) {
        for (const KeyStroke& key : typed) {
            const auto field = editor.Field();
            if (!field) return;
            switch (key.kind) {
                case KeyStroke::Kind::Digit: editor.Type(key.digit); break;
                case KeyStroke::Kind::Erase: editor.Erase(); break;
                case KeyStroke::Kind::Confirm: editor.Commit(settings); break;
                case KeyStroke::Kind::Next:
                    editor.Commit(settings);
                    editor.Begin((*field + 1) % SettingFields().size(), settings);
                    break;
            }
        }
    }

    bool HomeController::CancelEditing() {
        if (!editor.Field()) return false;
        editor.Cancel();
        return true;
    }

    bool HomeController::ClickValue(const PointerPosition mouse, SimulationSettings& settings) {
        const auto& fields = SettingFields();
        std::array<int, SettingColumns> rows{};
        for (std::size_t i = 0; i < fields.size(); ++i) {
            const int column = static_cast<int>(fields[i].column);
            const int row = rows.at(fields[i].column)++;
            if (!ValueBox(width, height, column, row).Contains(mouse.x, mouse.y)) continue;
            if (editor.Field() != i) {
                editor.Commit(settings);
                editor.Begin(i, settings);
            }
            return true;
        }
        return false;
    }

    UiAction HomeController::Click(const PointerPosition mouse, SimulationSettings& settings) {
        if (ClickValue(mouse, settings)) return UiAction::None;
        // Un clic en dehors des cases valide la saisie en cours.
        editor.Commit(settings);
        for (std::size_t i = 0; i < Presets.size(); ++i) {
            if (PresetButton(width, height, static_cast<int>(i)).Contains(mouse.x, mouse.y)) {
                settings = PresetSettings(Presets[i], width / cellSize, height / cellSize);
                return UiAction::None;
            }
        }
        if (StartButton(width, height).Contains(mouse.x, mouse.y)) return UiAction::StartOrRestart;
        return ResetButton(width, height).Contains(mouse.x, mouse.y)
            ? UiAction::ResetSettings : UiAction::None;
    }
}
