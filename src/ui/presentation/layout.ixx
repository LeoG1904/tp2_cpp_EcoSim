/**
 * @file
 * @brief La position des éléments, partagée par le dessin et la détection des clics.
 */
export module ecosim.layout;

import std;
export import ecosim.chart;
import ecosim.species;

export namespace ecosim {
    /// @brief Un rectangle de l'écran, en pixels.
    /// @ingroup interface
    struct UiRect {
        /// @brief Bord gauche.
        int x;
        /// @brief Bord haut.
        int y;
        /// @brief Largeur.
        int width;
        /// @brief Hauteur.
        int height;

        /// @brief Vrai si le point est dans le rectangle.
        bool Contains(const int px, const int py) const {
            return px >= x && px < x + width && py >= y && py < y + height;
        }
    };

    /// @brief Bouton « Courbes ».
    UiRect GraphButton(const int windowWidth) { return {windowWidth - 230, 54, 210, 36}; }
    /// @brief Bouton « Recommencer ».
    UiRect RestartButton(const int windowWidth) { return {windowWidth - 230, 8, 210, 36}; }
    /// @brief Bouton « Retour » des graphiques.
    UiRect BackButton() { return {20, 20, 110, 36}; }
    /// @brief Bouton de changement de vue 2D ou 3D.
    UiRect ViewButton(const int width) { return {width - 230, 192, 210, 36}; }
    /// @brief Bouton « Recentrer » en 3D.
    UiRect RecenterButton(const int width) { return {width - 230, 238, 210, 36}; }
    /// @brief Colonnes de l'accueil : le terrain, puis une par espèce.
    inline constexpr int SettingColumns{1 + static_cast<int>(SpeciesCount)};
    /// @brief Nombre de lignes de réglages.
    inline constexpr int SettingRows{9};
    /// @brief Hauteur d'une ligne de réglage.
    inline constexpr int SettingRowHeight{40};
    /// @brief Largeur d'une colonne de réglages.
    int SettingColumnWidth(const int width) {
        const int available = (width - 40) / SettingColumns;
        return available < 390 ? available : 390;
    }
    /// @brief Position horizontale d'une colonne.
    int SettingColumnX(const int width, const int column) {
        const int columnWidth = SettingColumnWidth(width);
        return width / 2 - columnWidth * SettingColumns / 2 + column * columnWidth;
    }
    /// @brief Position verticale de la première ligne.
    int SettingsTop(const int height) {
        const int centered = height / 2 - 200;
        return centered > 170 ? centered : 170;
    }
    /// @brief Case de saisie d'un réglage.
    UiRect ValueBox(const int width, const int height, const int column, const int row) {
        return {SettingColumnX(width, column) + SettingColumnWidth(width) - 108,
                SettingsTop(height) + row * SettingRowHeight, 92, 32};
    }
    /// @brief Bouton d'un préréglage.
    UiRect PresetButton(const int width, const int height, const int index) {
        return {width / 2 - 316 + index * 216, SettingsTop(height) + SettingRows * SettingRowHeight + 16,
                200, 38};
    }
    /// @brief Bouton « Démarrer ».
    UiRect StartButton(const int width, const int height) {
        return {width / 2 - 10, SettingsTop(height) + SettingRows * SettingRowHeight + 96, 300, 54};
    }
    /// @brief Bouton « Valeurs par défaut ».
    UiRect ResetButton(const int width, const int height) {
        return {width / 2 - 290, SettingsTop(height) + SettingRows * SettingRowHeight + 96, 260, 54};
    }
    /// @brief Bouton « Pause » de la carte ou des graphiques.
    UiRect PauseButton(const int width, const bool graph) {
        return graph ? UiRect{width - 350, 54, 150, 36} : UiRect{width - 230, 100, 210, 36};
    }
    /// @brief Bouton « Génération +1 ».
    UiRect NextButton(const int width, const bool graph) {
        return graph ? UiRect{width - 190, 54, 170, 36} : UiRect{width - 230, 146, 210, 36};
    }
    /// @brief Bouton « Menu » de la confirmation.
    UiRect ExitMenuButton(const int width, const int height) {
        return {width / 2 - 220, height / 2 + 45, 140, 40};
    }
    /// @brief Bouton « Quitter » de la confirmation.
    UiRect ExitQuitButton(const int width, const int height) {
        return {width / 2 - 65, height / 2 + 45, 130, 40};
    }
    /// @brief Bouton « Annuler » de la confirmation.
    UiRect ExitCancelButton(const int width, const int height) {
        return {width / 2 + 80, height / 2 + 45, 140, 40};
    }
    /// @brief Panneau d'information d'une case.
    UiRect CellPanel(const int width, const int height, const bool onLeft) {
        return {onLeft ? 20 : width - 400, height - 330, 380, 310};
    }
    /// @brief Bouton de fermeture du panneau.
    UiRect CellPanelClose(const UiRect panel) {
        return {panel.x + panel.width - 40, panel.y + 8, 32, 28};
    }
    /// @brief Case à cocher d'une statistique.
    UiRect MetricChip(const int width, const int index) {
        const int available = (width - 40) / static_cast<int>(MetricCount);
        const int step = available < 190 ? available : 190;
        return {20 + index * step, 112, step - 8, 34};
    }
    /// @brief Case à cocher d'une espèce.
    UiRect SpeciesChip(const int index) { return {20 + index * 180, 154, 172, 34}; }
    /// @brief Bouton « total ou par génération ».
    UiRect CumulativeButton() {
        return {SpeciesChip(static_cast<int>(SpeciesCount)).x + 24, 154, 280, 34};
    }
    /// @brief Zone réservée aux graphiques.
    UiRect ChartArea(const int width, const int height) {
        return {20, 204, width - 40, height - 220};
    }
    /// @brief Emplacement d'un graphique dans la grille.
    UiRect ChartPanel(const int width, const int height, const int index, const int count) {
        const UiRect area = ChartArea(width, height);
        const int columns = count <= 2 ? 1 : count <= 4 ? 2 : 3;
        const int rows = (count + columns - 1) / columns;
        constexpr int gap{12};
        const int panelWidth = (area.width - gap * (columns - 1)) / columns;
        const int panelHeight = (area.height - gap * (rows - 1)) / rows;
        return {area.x + index % columns * (panelWidth + gap),
                area.y + index / columns * (panelHeight + gap), panelWidth, panelHeight};
    }
    /// @brief Zone de tracé d'un graphique.
    UiRect ChartPlot(const UiRect panel) {
        return {panel.x + 70, panel.y + 64, panel.width - 88, panel.height - 92};
    }
}
