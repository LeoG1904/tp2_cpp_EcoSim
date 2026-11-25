module ecosim.map_navigator;

import std;
import ecosim.layout;

namespace ecosim {
    MapNavigator::MapNavigator(const int width, const int height, const int cellSize)
        : width{width}, height{height}, flat{width, height, cellSize},
          orbit{width, height, cellSize} {}

    MapViewKind MapNavigator::ActiveKind() const { return active; }
    const MapView& MapNavigator::ActiveView() const { return View(active); }
    const MapView& MapNavigator::NextView() const { return View(NextKind()); }
    const OrbitCamera& MapNavigator::Camera() const { return orbit.Camera(); }
    std::optional<Position> MapNavigator::SelectedCell() const { return selectedCell; }
    void MapNavigator::ClearSelection() { selectedCell.reset(); }

    MapViewKind MapNavigator::NextKind() const {
        return static_cast<MapViewKind>((static_cast<std::size_t>(active) + 1) % MapViewCount);
    }

    const MapView& MapNavigator::View(const MapViewKind kind) const {
        const std::array<const MapView*, MapViewCount> views{&flat, &orbit};
        return *views.at(static_cast<std::size_t>(kind));
    }

    MapView& MapNavigator::View(const MapViewKind kind) {
        const std::array<MapView*, MapViewCount> views{&flat, &orbit};
        return *views.at(static_cast<std::size_t>(kind));
    }

    bool MapNavigator::IsCellPanelOnLeft() const {
        return selectedCell && ActiveView().IsOnRightHalf(*selectedCell);
    }

    void MapNavigator::Control(const RawInput& input, const bool isActive) {
        if (isActive) View(active).Control(input);
        else View(active).StopControl();
    }

    void MapNavigator::Click(const PointerPosition mouse) {
        if (ViewButton(width).Contains(mouse.x, mouse.y)) {
            View(active).StopControl();
            active = NextKind();
            return;
        }
        if (ActiveView().CanRecenter() && RecenterButton(width).Contains(mouse.x, mouse.y)) {
            View(active).Recenter();
            return;
        }
        if (selectedCell) {
            const UiRect panel = CellPanel(width, height, IsCellPanelOnLeft());
            if (panel.Contains(mouse.x, mouse.y)) {
                if (CellPanelClose(panel).Contains(mouse.x, mouse.y)) ClearSelection();
                return;
            }
        }
        const std::optional<Position> cell = ActiveView().PickCell(mouse);
        if (!cell) return;
        // Un second clic sur la case choisie ferme le panneau.
        if (selectedCell == cell) ClearSelection();
        else selectedCell = cell;
    }
}
