module ecosim.stats_screen;

import std;
import ecosim.layout;
import ecosim.species;

namespace ecosim {
    StatsScreen::StatsScreen(const int width) : width{width} {}

    bool StatsScreen::IsOpen() const { return open; }
    void StatsScreen::Open() { open = true; }
    void StatsScreen::Close() { open = false; }
    const ChartSelection& StatsScreen::Selection() const { return selection; }

    void StatsScreen::Click(const PointerPosition mouse) {
        if (BackButton().Contains(mouse.x, mouse.y)) {
            Close();
            return;
        }
        for (std::size_t i = 0; i < MetricCount; ++i) {
            if (MetricChip(width, static_cast<int>(i)).Contains(mouse.x, mouse.y)) {
                selection.metrics[i] = !selection.metrics[i];
            }
        }
        for (const SpeciesInfo& species : SpeciesCatalog()) {
            if (SpeciesChip(static_cast<int>(species.kind)).Contains(mouse.x, mouse.y)) {
                selection.hidden[species.kind] = !selection.hidden[species.kind];
            }
        }
        if (CumulativeButton().Contains(mouse.x, mouse.y)) selection.cumulative = !selection.cumulative;
    }
}
