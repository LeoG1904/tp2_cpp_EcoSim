module;

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "../ui/layout.hpp"

module ecosim.render;

import std;
import ecosim.simulation;
import ecosim.cell_info;

using namespace ecosim_render_detail;

namespace ecosim {
    // Cadre autour de la case choisie et panneau de ses informations.
    void Renderer2D::DrawSelection(void* context, const Simulation& simulation) const {
        const auto selected = controller.SelectedCell();
        if (!selected || !simulation.GetWorld().IsInside(*selected)) return;
        const HDC buffer = static_cast<HDC>(context);
        const auto fill = [buffer](const RECT& rect, const COLORREF color) {
            const HBRUSH brush = CreateSolidBrush(color);
            FillRect(buffer, &rect, brush);
            DeleteObject(brush);
        };

        RECT frame{selected->x * cellSize, selected->y * cellSize,
                   (selected->x + 1) * cellSize - 1, (selected->y + 1) * cellSize - 1};
        const HBRUSH frameBrush = CreateSolidBrush(RGB(255, 255, 255));
        for (int thickness = 0; thickness < 3; ++thickness) {
            FrameRect(buffer, &frame, frameBrush);
            InflateRect(&frame, -1, -1);
        }
        DeleteObject(frameBrush);

        const UiRect panel = CellPanel(windowWidth, windowHeight, controller.IsCellPanelOnLeft());
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
