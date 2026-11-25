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
    inline UiRect PresetButton(const int width, const int height, const int index) {
        return {width / 2 - 316 + index * 216, SettingsTop(height) + SettingRows * SettingRowHeight + 16,
                200, 38};
    }
    inline UiRect StartButton(const int width, const int height) {
        return {width / 2 - 10, SettingsTop(height) + SettingRows * SettingRowHeight + 96, 300, 54};
    }
    inline UiRect ResetButton(const int width, const int height) {
        return {width / 2 - 290, SettingsTop(height) + SettingRows * SettingRowHeight + 96, 260, 54};
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
    // Panneau de la case choisie, en bas de l'écran, du côté opposé à cette case.
    inline UiRect CellPanel(const int width, const int height, const bool onLeft) {
        return {onLeft ? 20 : width - 400, height - 330, 380, 310};
    }
    inline UiRect CellPanelClose(const UiRect panel) {
        return {panel.x + panel.width - 40, panel.y + 8, 32, 28};
    }
    // Écran des statistiques : deux rangées de cases à cocher, puis les graphiques.
    inline UiRect MetricChip(const int width, const int index) {
        const int available = (width - 40) / 7;
        const int step = available < 190 ? available : 190;
        return {20 + index * step, 112, step - 8, 34};
    }
    inline UiRect SpeciesChip(const int index) { return {20 + index * 180, 154, 172, 34}; }
    inline UiRect CumulativeButton() { return {404, 154, 280, 34}; }
    inline UiRect ChartArea(const int width, const int height) {
        return {20, 204, width - 40, height - 220};
    }
    // Grille de petits graphiques : plus il y en a, plus il y a de colonnes.
    inline UiRect ChartPanel(const int width, const int height, const int index, const int count) {
        const UiRect area = ChartArea(width, height);
        const int columns = count <= 2 ? 1 : count <= 4 ? 2 : 3;
        const int rows = (count + columns - 1) / columns;
        constexpr int gap{12};
        const int panelWidth = (area.width - gap * (columns - 1)) / columns;
        const int panelHeight = (area.height - gap * (rows - 1)) / rows;
        return {area.x + index % columns * (panelWidth + gap),
                area.y + index / columns * (panelHeight + gap), panelWidth, panelHeight};
    }
    // Zone de tracé, sous le titre et la légende, à droite des graduations.
    inline UiRect ChartPlot(const UiRect panel) {
        return {panel.x + 70, panel.y + 64, panel.width - 88, panel.height - 92};
    }
}
