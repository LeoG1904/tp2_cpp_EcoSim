module;

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "../gdi/gdi.hpp"

module ecosim.render;

import std;
import ecosim.layout;
import ecosim.cell_info;

using namespace ecosim_gdi;

namespace ecosim {
    void CellPanelPainter::Draw(Canvas& canvas, const Simulation& simulation,
                                const std::optional<Position> selected, const bool onLeft) const {
        if (!selected || !simulation.GetWorld().IsInside(*selected)) return;
        const HDC buffer = static_cast<HDC>(canvas.Context());
        const auto fill = [buffer](const RECT& rect, const COLORREF color) {
            FillArea(buffer, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, color);
        };

        const UiRect panel = CellPanel(canvas.Width(), canvas.Height(), onLeft);
        fill({panel.x, panel.y, panel.x + panel.width, panel.y + panel.height}, RGB(32, 45, 53));
        const UiRect close = CellPanelClose(panel);
        fill({close.x, close.y, close.x + close.width, close.y + close.height}, RGB(55, 64, 77));
        SetBkMode(buffer, TRANSPARENT);
        SetTextColor(buffer, RGB(239, 246, 242));
        WriteText(buffer, close.x + 12, close.y + 6, "X");
        int y = panel.y + 14;
        for (const std::string& line : DescribeCell(simulation, *selected)) {
            WriteText(buffer, panel.x + 16, y, line);
            y += 24;
        }
    }
}
