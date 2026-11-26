/**
 * @file
 * @brief Dessin de l'accueil.
 */
module ecosim.render;

import std;
import ecosim.layout;

namespace ecosim {
    void DrawHomeScreen(Canvas& canvas, const SimulationSettings& current, const SettingEditor& editor,
                        const int cellSize) {
        // Copie : les accesseurs des champs demandent un objet modifiable.
        SimulationSettings settings = current;
        const int windowWidth = canvas.Width();
        const int windowHeight = canvas.Height();
        // Le fond quadrillé rappelle la carte.
        canvas.Fill({0, 0, windowWidth, windowHeight}, {19, 28, 34});
        constexpr Color gridColor{30, 42, 46};
        for (int x = 0; x < windowWidth; x += cellSize) canvas.Fill({x, 0, 1, windowHeight}, gridColor);
        for (int y = 0; y < windowHeight; y += cellSize) canvas.Fill({0, y, windowWidth, 1}, gridColor);

        const int centerX = windowWidth / 2;
        const int top = SettingsTop(windowHeight);
        const int columnWidth = SettingColumnWidth(windowWidth);

        canvas.SetFont(56, true);
        canvas.WriteCentered(centerX, top - 150, "EcoSim", LightText);
        canvas.SetFont(18);
        canvas.WriteCentered(centerX, top - 82, "Réglez votre écosystème, puis démarrez la simulation",
                             SoftText);

        canvas.SetFont(24, true);
        for (int column = 0; column < SettingColumns; ++column) {
            const int x = SettingColumnX(windowWidth, column);
            canvas.Fill({x + 5, top - 48, columnWidth - 10, SettingRows * SettingRowHeight + 48},
                        PanelColor);
            canvas.WriteCentered(x + columnWidth / 2, top - 40,
                                 ColumnTitle(static_cast<std::size_t>(column)),
                                 ColumnColor(static_cast<std::size_t>(column)));
        }

        canvas.SetFont(18);
        std::array<int, SettingColumns> rows{};
        const auto& fields = SettingFields();
        for (std::size_t i = 0; i < fields.size(); ++i) {
            const SettingField& field = fields[i];
            const int column = static_cast<int>(field.column);
            const int row = rows.at(field.column)++;
            const UiRect box = ValueBox(windowWidth, windowHeight, column, row);
            canvas.Write(SettingColumnX(windowWidth, column) + 16, box.y + 6, field.label, LightText);

            const bool editing = editor.Field() == i;
            if (editing) {
                canvas.Fill({box.x - 2, box.y - 2, box.width + 4, box.height + 4}, HighlightColor);
            }
            // Fond bleu : la valeur entière est sélectionnée ; trait : saisie en cours.
            std::string value = editing ? editor.Text() : std::to_string(field.value(settings));
            if (editing && !editor.IsSelected()) value += "|";
            canvas.Button(box, value, editing && editor.IsSelected() ? ActiveColor : ButtonColor);
        }

        // Le préréglage reste surligné tant qu'aucune valeur n'a été modifiée.
        std::string description = "Préréglages : choisissez un scénario, puis ajustez-le si besoin.";
        for (std::size_t i = 0; i < Presets.size(); ++i) {
            const bool active = current == PresetSettings(Presets[i], windowWidth / cellSize,
                                                          windowHeight / cellSize);
            if (active) description = PresetDescription(Presets[i]);
            canvas.Button(PresetButton(windowWidth, windowHeight, static_cast<int>(i)),
                          PresetName(Presets[i]), active ? ActiveColor : ButtonColor);
        }
        const UiRect firstPreset = PresetButton(windowWidth, windowHeight, 0);
        canvas.WriteCentered(centerX, firstPreset.y + firstPreset.height + 10, description, SoftText);

        canvas.SetFont(24, true);
        canvas.Button(ResetButton(windowWidth, windowHeight), "Valeurs par défaut", ButtonColor);
        canvas.Button(StartButton(windowWidth, windowHeight), "Démarrer", ConfirmColor);
        canvas.SetFont(18);
        std::string help = "Cliquez sur une valeur pour la modifier   •   Échap : quitter";
        if (const auto active = editor.Field()) {
            help = std::format("De {} à {}   •   Entrée : valider   •   Tab : case suivante   •   Échap : annuler",
                               fields[*active].minimum, fields[*active].maximum);
        }
        canvas.WriteCentered(centerX, windowHeight - 37, help, SoftText);
    }
}
