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
    inline UiRect StartButton(const int width, const int height) {
        return {width / 2 - 150, height / 2 + 75, 300, 54};
    }
    inline UiRect PauseButton(const int width, const bool graph) {
        return graph ? UiRect{width - 350, 54, 150, 36} : UiRect{width - 230, 100, 210, 36};
    }
    inline UiRect NextButton(const int width, const bool graph) {
        return graph ? UiRect{width - 190, 54, 170, 36} : UiRect{width - 230, 146, 210, 36};
    }
    inline UiRect ExitMenuButton(const int width, const int height) {
        return {width / 2 - 220, height / 2 + 45, 140, 40};
    }
    inline UiRect ExitQuitButton(const int width, const int height) {
        return {width / 2 - 65, height / 2 + 45, 130, 40};
    }
    inline UiRect ExitCancelButton(const int width, const int height) {
        return {width / 2 + 80, height / 2 + 45, 140, 40};
    }
    inline UiRect PopulationPlot(const int width, const int height) {
        return {90, 160, width > 151 ? width - 150 : 1, height > 251 ? height - 250 : 1};
    }
}
