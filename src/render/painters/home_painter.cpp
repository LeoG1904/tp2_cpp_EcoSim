/**
 * @file
 * @brief Dessin de l'accueil.
 */
module;

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "../gdi/gdi.hpp"

module ecosim.render;

import std;
import ecosim.layout;

using namespace ecosim_gdi;

namespace ecosim {
    void DrawHomeScreen(Canvas& canvas, const SimulationSettings& current, const SettingEditor& editor,
                        const int cellSize) {
        // Copie : les accesseurs des champs demandent un objet modifiable.
        SimulationSettings settings = current;
        const HDC buffer = static_cast<HDC>(canvas.Context());
        const int windowWidth = canvas.Width();
        const int windowHeight = canvas.Height();
        const RECT bounds{0, 0, windowWidth, windowHeight};
        const HBRUSH background = CreateSolidBrush(RGB(19, 28, 34));
        FillRect(buffer, &bounds, background);
        DeleteObject(background);

        const HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(30, 42, 46));
        const HGDIOBJ previousPen = SelectObject(buffer, gridPen);
        for (int x = 0; x < windowWidth; x += cellSize) {
            MoveToEx(buffer, x, 0, nullptr);
            LineTo(buffer, x, windowHeight);
        }
        for (int y = 0; y < windowHeight; y += cellSize) {
            MoveToEx(buffer, 0, y, nullptr);
            LineTo(buffer, windowWidth, y);
        }
        SelectObject(buffer, previousPen);
        DeleteObject(gridPen);

        const int centerX = windowWidth / 2;
        const int top = SettingsTop(windowHeight);
        const int columnWidth = SettingColumnWidth(windowWidth);

        SetBkMode(buffer, TRANSPARENT);
        const HFONT titleFont = UiFont(56, FW_BOLD);
        const HFONT bodyFont = UiFont(18, FW_NORMAL);
        const HFONT buttonFont = UiFont(24, FW_BOLD);
        const HGDIOBJ previousFont = SelectObject(buffer, titleFont);
        const auto write = [&](const int x, const int y, const std::string& value) {
            WriteText(buffer, x, y, value);
        };
        const auto writeCentered = [&](const std::string& value, const int x, const int y) {
            const TextSize extent = MeasureText(buffer, value);
            write(x - extent.cx / 2, y, value);
        };
        const auto fill = [&](const RECT& rect, const COLORREF color) {
            FillArea(buffer, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, color);
        };
        const auto drawButton = [&](const UiRect button, const COLORREF color, const std::string& label) {
            fill({button.x, button.y, button.x + button.width, button.y + button.height}, color);
            const TextSize extent = MeasureText(buffer, label);
            write(button.x + (button.width - extent.cx) / 2,
                  button.y + (button.height - extent.cy) / 2, label);
        };

        SetTextColor(buffer, RGB(239, 246, 242));
        writeCentered("EcoSim", centerX, top - 150);
        SelectObject(buffer, bodyFont);
        SetTextColor(buffer, RGB(177, 203, 198));
        writeCentered("Réglez votre écosystème, puis démarrez la simulation", centerX, top - 82);

        for (int column = 0; column < SettingColumns; ++column) {
            const int x = SettingColumnX(windowWidth, column);
            fill({x + 5, top - 48, x + columnWidth - 5, top + SettingRows * SettingRowHeight},
                 RGB(32, 45, 53));
            SelectObject(buffer, buttonFont);
            const Color color = ColumnColor(static_cast<std::size_t>(column));
            SetTextColor(buffer, RGB(color.red, color.green, color.blue));
            writeCentered(ColumnTitle(static_cast<std::size_t>(column)), x + columnWidth / 2, top - 40);
        }

        SelectObject(buffer, bodyFont);
        SetTextColor(buffer, RGB(239, 246, 242));
        std::array<int, SettingColumns> rows{};
        const auto& fields = SettingFields();
        for (std::size_t i = 0; i < fields.size(); ++i) {
            const SettingField& field = fields[i];
            const int column = static_cast<int>(field.column);
            const int row = rows.at(field.column)++;
            const UiRect box = ValueBox(windowWidth, windowHeight, column, row);
            write(SettingColumnX(windowWidth, column) + 16, box.y + 6, field.label);

            const bool editing = editor.Field() == i;
            if (editing) {
                fill({box.x - 2, box.y - 2, box.x + box.width + 2, box.y + box.height + 2},
                     RGB(125, 183, 235));
            }
            // Fond bleu : la valeur entière est sélectionnée ; trait : saisie en cours.
            std::string value = editing ? editor.Text() : std::to_string(field.value(settings));
            if (editing && !editor.IsSelected()) value += "|";
            drawButton(box, editing && editor.IsSelected() ? RGB(52, 96, 140) : RGB(55, 64, 77),
                       value);
        }

        // Le préréglage reste surligné tant qu'aucune valeur n'a été modifiée.
        std::string description = "Préréglages : choisissez un scénario, puis ajustez-le si besoin.";
        for (std::size_t i = 0; i < Presets.size(); ++i) {
            const bool active = current == PresetSettings(Presets[i], windowWidth / cellSize,
                                                          windowHeight / cellSize);
            if (active) description = PresetDescription(Presets[i]);
            drawButton(PresetButton(windowWidth, windowHeight, static_cast<int>(i)),
                       active ? RGB(52, 96, 140) : RGB(55, 64, 77), PresetName(Presets[i]));
        }
        SetTextColor(buffer, RGB(177, 203, 198));
        const UiRect firstPreset = PresetButton(windowWidth, windowHeight, 0);
        writeCentered(description, centerX, firstPreset.y + firstPreset.height + 10);
        SetTextColor(buffer, RGB(239, 246, 242));

        SelectObject(buffer, buttonFont);
        drawButton(ResetButton(windowWidth, windowHeight), RGB(55, 64, 77), "Valeurs par défaut");
        drawButton(StartButton(windowWidth, windowHeight), RGB(62, 148, 105), "Démarrer");
        SelectObject(buffer, bodyFont);
        SetTextColor(buffer, RGB(177, 203, 198));
        std::string help = "Cliquez sur une valeur pour la modifier   •   Échap : quitter";
        if (const auto active = editor.Field()) {
            help = std::format("De {} à {}   •   Entrée : valider   •   Tab : case suivante   •   Échap : annuler",
                               fields[*active].minimum, fields[*active].maximum);
        }
        writeCentered(help, centerX, windowHeight - 37);

        SelectObject(buffer, previousFont);
        DeleteObject(titleFont);
        DeleteObject(bodyFont);
        DeleteObject(buttonFont);
    }
}
