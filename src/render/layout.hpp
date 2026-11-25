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
    // Accueil : trois colonnes de réglages (terrain, herbivores, prédateurs).
    constexpr int SettingRows{7};
    constexpr int SettingRowHeight{48};
    inline int SettingColumnWidth(const int width) {
        const int available = (width - 40) / 3;
        return available < 390 ? available : 390;
    }
    inline int SettingColumnX(const int width, const int column) {
        const int columnWidth = SettingColumnWidth(width);
        return width / 2 - columnWidth * 3 / 2 + column * columnWidth;
    }
    inline int SettingsTop(const int height) {
        const int centered = height / 2 - 200;
        return centered > 170 ? centered : 170;
    }
    inline UiRect ValueBox(const int width, const int height, const int column, const int row) {
        return {SettingColumnX(width, column) + SettingColumnWidth(width) - 108,
                SettingsTop(height) + row * SettingRowHeight, 92, 32};
    }
    inline UiRect StartButton(const int width, const int height) {
        return {width / 2 - 10, SettingsTop(height) + SettingRows * SettingRowHeight + 30, 300, 54};
    }
    inline UiRect ResetButton(const int width, const int height) {
        return {width / 2 - 290, SettingsTop(height) + SettingRows * SettingRowHeight + 30, 260, 54};
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
