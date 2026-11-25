module;

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "../gdi/gdi.hpp"

module ecosim.render;

import std;
import ecosim.layout;
import ecosim.species;

using namespace ecosim_gdi;

namespace ecosim {
    void HudPainter::Draw(Canvas& canvas, const Simulation& simulation, const PlaybackState& playback,
                          const MapView& view, const MapView& nextView) const {
        const HDC buffer = static_cast<HDC>(canvas.Context());
        const int width = canvas.Width();
        std::string status;
        if (simulation.IsFinished()) status = "Fin de la simulation : plus aucun animal";
        else if (playback.paused) {
            status = std::format("En pause   Vitesse x{}   (flèche droite ou haut : avancer ; {})",
                                 playback.speed, view.Help());
        } else {
            status = std::format("Vitesse x{}   (Tab ou touches 1 à 4 : x1 x2 x5 x10 ; {})",
                                 playback.speed, view.Help());
        }
        std::string counts = std::format("Génération {}", simulation.GetGeneration());
        for (const SpeciesInfo& species : SpeciesCatalog()) {
            counts += std::format("   {} {}", species.pluralName, simulation.Count(species.kind));
        }
        SetBkMode(buffer, TRANSPARENT);
        SetTextColor(buffer, RGB(255, 255, 255));
        WriteText(buffer, 10, 10, counts);
        WriteText(buffer, 10, 30, status);

        const auto button = [&](const UiRect area, const std::string& label) {
            FillArea(buffer, area.x, area.y, area.width, area.height, RGB(55, 64, 77));
            WriteText(buffer, area.x + 20, area.y + 9, label);
        };
        button(GraphButton(width), "Courbes");
        button(RestartButton(width), "Recommencer");
        if (!simulation.IsFinished()) {
            button(PauseButton(width, false), playback.paused ? "Reprendre" : "Pause");
            if (playback.paused) button(NextButton(width, false), "Génération +1");
        }
        button(ViewButton(width), nextView.ButtonLabel());
        if (view.CanRecenter()) button(RecenterButton(width), "Recentrer");
    }
}
