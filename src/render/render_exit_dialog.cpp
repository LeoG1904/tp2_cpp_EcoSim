module;

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "../ui/layout.hpp"

module ecosim.render;

import std;

using namespace ecosim_render_detail;

namespace ecosim {
    void Renderer2D::DrawExitDialog(void* context) const {
        const HDC buffer = static_cast<HDC>(context);
        const int centerX = windowWidth / 2;
        const int centerY = windowHeight / 2;
        const RECT border{centerX - 253, centerY - 118, centerX + 253, centerY + 118};
        const RECT panel{centerX - 250, centerY - 115, centerX + 250, centerY + 115};
        const HBRUSH borderBrush = CreateSolidBrush(RGB(74, 93, 100));
        const HBRUSH panelBrush = CreateSolidBrush(RGB(32, 45, 53));
        FillRect(buffer, &border, borderBrush);
        FillRect(buffer, &panel, panelBrush);
        DeleteObject(borderBrush);
        DeleteObject(panelBrush);

        SetBkMode(buffer, TRANSPARENT);
        const HFONT titleFont = CreateFontW(30, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                            CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
        const HFONT textFont = CreateFontW(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                           CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
        const HGDIOBJ previousFont = SelectObject(buffer, titleFont);
        const auto writeCentered = [&](const std::string& value, const int y) {
            const TextSize size = MeasureText(buffer, value);
            WriteText(buffer, centerX - size.cx / 2, y, value);
        };
        SetTextColor(buffer, RGB(239, 246, 242));
        writeCentered("Quitter la simulation ?", centerY - 86);
        SelectObject(buffer, textFont);
        writeCentered("Revenir au menu ou fermer EcoSim ?", centerY - 26);

        const auto drawButton = [&](const UiRect button, const COLORREF color,
                                    const std::string& label) {
            const RECT rect{button.x, button.y, button.x + button.width, button.y + button.height};
            const HBRUSH brush = CreateSolidBrush(color);
            FillRect(buffer, &rect, brush);
            DeleteObject(brush);
            const TextSize size = MeasureText(buffer, label);
            WriteText(buffer, button.x + (button.width - size.cx) / 2, button.y + (button.height - size.cy) / 2, label);
        };
        drawButton(ExitMenuButton(windowWidth, windowHeight), RGB(62, 148, 105), "Menu");
        drawButton(ExitQuitButton(windowWidth, windowHeight), RGB(166, 77, 72), "Quitter");
        drawButton(ExitCancelButton(windowWidth, windowHeight), RGB(55, 64, 77), "Annuler");
        SelectObject(buffer, previousFont);
        DeleteObject(titleFont);
        DeleteObject(textFont);
    }
}
