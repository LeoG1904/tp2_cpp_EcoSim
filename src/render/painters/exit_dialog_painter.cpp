/**
 * @file
 * @brief Dessin de la confirmation de sortie.
 */
module ecosim.render;

import std;
import ecosim.layout;

namespace ecosim {
    void DrawExitDialog(Canvas& canvas) {
        const int windowWidth = canvas.Width();
        const int windowHeight = canvas.Height();
        const int centerX = windowWidth / 2;
        const int centerY = windowHeight / 2;
        // Un rectangle un peu plus grand sert de bordure.
        canvas.Fill({centerX - 253, centerY - 118, 506, 236}, {74, 93, 100});
        canvas.Fill({centerX - 250, centerY - 115, 500, 230}, PanelColor);

        canvas.SetFont(30, true);
        canvas.WriteCentered(centerX, centerY - 86, "Quitter la simulation ?", LightText);
        canvas.SetFont(20);
        canvas.WriteCentered(centerX, centerY - 26, "Revenir au menu ou fermer EcoSim ?", LightText);

        canvas.Button(ExitMenuButton(windowWidth, windowHeight), "Menu", ConfirmColor);
        canvas.Button(ExitQuitButton(windowWidth, windowHeight), "Quitter", {166, 77, 72});
        canvas.Button(ExitCancelButton(windowWidth, windowHeight), "Annuler", ButtonColor);
    }
}
