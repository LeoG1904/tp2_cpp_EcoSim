/**
 * @file
 * @brief Dessin du panneau d'une case.
 */
module ecosim.render;

import std;
import ecosim.layout;
import ecosim.cell_info;

namespace ecosim {
    void DrawCellPanel(Canvas& canvas, const Simulation& simulation,
                       const std::optional<Position> selected, const bool onLeft) {
        if (!selected || !simulation.GetWorld().IsInside(*selected)) return;
        const UiRect panel = CellPanel(canvas.Width(), canvas.Height(), onLeft);
        canvas.Fill(panel, PanelColor);
        const UiRect close = CellPanelClose(panel);
        canvas.Fill(close, ButtonColor);
        canvas.Write(close.x + 12, close.y + 6, "X", LightText);
        int y = panel.y + 14;
        for (const std::string& line : DescribeCell(simulation, *selected)) {
            canvas.Write(panel.x + 16, y, line, LightText);
            y += 24;
        }
    }
}
