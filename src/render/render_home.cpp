module;

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "../ui/layout.hpp"

module ecosim.render;

import std;

using namespace ecosim_render_detail;

namespace ecosim {
    void Renderer2D::DrawHome(const SimulationSettings& current) {
        // Copie : les accesseurs des champs demandent un objet modifiable.
        SimulationSettings settings = current;
        const SettingEditor& editor = controller.Editor();
        const auto hwnd = static_cast<HWND>(window);    
        const HDC screen = GetDC(hwnd);
        const HDC buffer = CreateCompatibleDC(screen);
        const HBITMAP bitmap = CreateCompatibleBitmap(screen, windowWidth, windowHeight);
        const HGDIOBJ previous = SelectObject(buffer, bitmap);
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
        const auto createFont = [](const int height, const int weight) {
            return CreateFontW(height, 0, 0, 0, weight, FALSE, FALSE, FALSE,
                               DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                               CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
        };
        const HFONT titleFont = createFont(56, FW_BOLD);
        const HFONT bodyFont = createFont(18, FW_NORMAL);
        const HFONT buttonFont = createFont(24, FW_BOLD);
        const HGDIOBJ previousFont = SelectObject(buffer, titleFont);
        const auto write = [&](const int x, const int y, const std::string& value) {
            WriteText(buffer, x, y, value);
        };
        const auto writeCentered = [&](const std::string& value, const int x, const int y) {
            const TextSize extent = MeasureText(buffer, value);
            write(x - extent.cx / 2, y, value);
        };
        const auto fill = [&](const RECT& rect, const COLORREF color) {
            const HBRUSH brush = CreateSolidBrush(color);
            FillRect(buffer, &rect, brush);
            DeleteObject(brush);
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

        const std::array<std::string, 3> titles{"Terrain", "Herbivores", "Prédateurs"};
        constexpr std::array<COLORREF, 3> titleColors{RGB(120, 200, 130), RGB(245, 210, 70),
                                                      RGB(220, 85, 75)};
        for (int column = 0; column < 3; ++column) {
            const int x = SettingColumnX(windowWidth, column);
            fill({x + 5, top - 48, x + columnWidth - 5, top + SettingRows * SettingRowHeight},
                 RGB(32, 45, 53));
            SelectObject(buffer, buttonFont);
            SetTextColor(buffer, titleColors[column]);
            writeCentered(titles[column], x + columnWidth / 2, top - 40);
        }

        SelectObject(buffer, bodyFont);
        SetTextColor(buffer, RGB(239, 246, 242));
        std::array<int, 3> rows{};
        const auto fields = SettingFields();
        for (std::size_t i = 0; i < fields.size(); ++i) {
            const SettingField& field = fields[i];
            const int column = static_cast<int>(field.group);
            const int row = rows[column]++;
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
        BitBlt(screen, 0, 0, windowWidth, windowHeight, buffer, 0, 0, SRCCOPY);
        SelectObject(buffer, previous);
        DeleteObject(bitmap);
        DeleteDC(buffer);
        ReleaseDC(hwnd, screen);
        Sleep(16);
    }
}
