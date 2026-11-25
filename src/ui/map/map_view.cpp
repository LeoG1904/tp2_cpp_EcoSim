module ecosim.map_view;

import std;

namespace ecosim {
    void MapView::Control(const RawInput&) {}
    void MapView::StopControl() {}
    bool MapView::CanRecenter() const { return false; }
    void MapView::Recenter() {}

    FlatMapView::FlatMapView(const int width, const int height, const int cellSize)
        : width{width}, height{height}, cellSize{cellSize} {}

    std::string FlatMapView::ButtonLabel() const { return "Vue 2D"; }
    std::string FlatMapView::Help() const { return "Échap : options ; clic : détails d'une case"; }

    std::optional<Position> FlatMapView::PickCell(const PointerPosition mouse) const {
        // La bande restante au bord de l'écran ne contient pas de case.
        if (mouse.x < 0 || mouse.y < 0 || mouse.x / cellSize >= width / cellSize
            || mouse.y / cellSize >= height / cellSize) {
            return std::nullopt;
        }
        return Position{mouse.x / cellSize, mouse.y / cellSize};
    }

    bool FlatMapView::IsOnRightHalf(const Position cell) const { return cell.x * cellSize >= width / 2; }

    OrbitMapView::OrbitMapView(const int width, const int height, const int cellSize)
        : screen{width, height},
          camera{std::max(1, width / cellSize), std::max(1, height / cellSize)} {}

    std::string OrbitMapView::ButtonLabel() const { return "Vue 3D"; }
    std::string OrbitMapView::Help() const {
        return "clic droit : tourner ; molette : zoom ; clic : détails d'une case";
    }

    std::optional<Position> OrbitMapView::PickCell(const PointerPosition mouse) const {
        return camera.PickCell({mouse.x, mouse.y}, screen);
    }

    bool OrbitMapView::IsOnRightHalf(const Position cell) const {
        const auto centre = camera.Project({cell.x + 0.5, 0.0, cell.y + 0.5}, screen);
        return centre && centre->x >= screen.width / 2;
    }

    void OrbitMapView::Control(const RawInput& input) {
        if (input.wheel != 0) camera.Zoom(input.wheel);
        if (!input.rotating || !input.pointer) {
            StopControl();
            return;
        }
        // La carte suit la souris : glisser vers le bas fait passer la caméra au-dessus.
        if (dragOrigin) {
            constexpr double radiansPerPixel{0.006};
            camera.Rotate((dragOrigin->x - input.pointer->x) * radiansPerPixel,
                          (input.pointer->y - dragOrigin->y) * radiansPerPixel);
        }
        dragOrigin = input.pointer;
    }

    void OrbitMapView::StopControl() { dragOrigin.reset(); }
    bool OrbitMapView::CanRecenter() const { return true; }
    void OrbitMapView::Recenter() { camera.Reset(); }
    const OrbitCamera& OrbitMapView::Camera() const { return camera; }
}
