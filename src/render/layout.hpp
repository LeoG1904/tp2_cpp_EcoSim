#pragma once

// Geometrie commune aux deux ecrans et a la detection des clics.
namespace ecosim_render_detail {
    struct UiRect {
        int x;
        int y;
        int width;
        int height;

        bool Contains(const int px, const int py) const {
            return px >= x && px < x + width && py >= y && py < y + height;
        }
    };

    inline UiRect GraphButton(const int windowWidth) { return {windowWidth - 230, 54, 210, 36}; }
    inline UiRect RestartButton(const int windowWidth) { return {windowWidth - 230, 8, 210, 36}; }
    inline UiRect BackButton() { return {20, 20, 110, 36}; }
    inline UiRect PopulationPlot(const int width, const int height) {
        return {90, 160, width > 151 ? width - 150 : 1, height > 251 ? height - 250 : 1};
    }
}
