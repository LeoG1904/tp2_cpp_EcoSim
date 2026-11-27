/**
 * @file
 * @brief Dessin des compteurs et des boutons.
 */
module ecosim.render;

import std;
import ecosim.layout;
import ecosim.species;

namespace ecosim {
    void DrawHud(Canvas& canvas, const Simulation& simulation, const PlaybackState& playback,
                 const bool threeD) {
        const int width = canvas.Width();
        const std::string help = threeD
            ? "clic droit : tourner ; molette : zoom ; clic : détails d'une case"
            : "clic droit : déplacer ; molette : zoom ; clic : détails d'une case";
        std::string status;
        if (simulation.IsFinished()) status = "Fin de la simulation : plus aucun animal";
        else if (playback.paused) {
            status = std::format("En pause   Vitesse x{}   (flèche droite ou haut : avancer ; {})",
                                 playback.speed, help);
        } else {
            status = std::format("Vitesse x{}   (Tab ou touches 1 à 4 : x1 x2 x5 x10 ; {})",
                                 playback.speed, help);
        }
        std::string counts = std::format("Génération {}", simulation.GetGeneration());
        for (const SpeciesInfo& species : SpeciesCatalog()) {
            counts += std::format("   {} {}", species.pluralName, simulation.Count(species.kind));
        }
        canvas.Write(10, 10, counts, WhiteText);
        canvas.Write(10, 30, status, WhiteText);

        const auto button = [&](const UiRect area, const std::string& label) {
            canvas.SideButton(area, label, 9);
        };
        button(GraphButton(width), "Courbes");
        button(RestartButton(width), "Recommencer");
        if (!simulation.IsFinished()) {
            button(PauseButton(width, false), playback.paused ? "Reprendre" : "Pause");
            if (playback.paused) button(NextButton(width, false), "Génération +1");
        }
        button(ViewButton(width), threeD ? "Vue 2D" : "Vue 3D");
        button(RecenterButton(width), "Recentrer");
    }
}
