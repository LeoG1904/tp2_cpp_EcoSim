/**
 * @file
 * @brief Calculs de zoom, de déplacement et de sélection en 2D.
 */
module ecosim.flat_view;

import std;

namespace {
    /// @brief Place une dimension de la carte : centrée si elle tient, sinon sans vide aux bords.
    double Placed(const double origin, const double mapPixels, const double screenPixels) {
        if (mapPixels <= screenPixels) return std::floor((screenPixels - mapPixels) / 2.0);
        return std::clamp(origin, screenPixels - mapPixels, 0.0);
    }
}

namespace ecosim {
    FlatView::FlatView(const int mapWidth, const int mapHeight, const Viewport screen)
        : mapWidth{mapWidth}, mapHeight{mapHeight}, screen{screen} {
        Reset();
    }

    double FlatView::FitSize() const {
        return std::min(static_cast<double>(screen.width) / mapWidth,
                        static_cast<double>(screen.height) / mapHeight);
    }

    void FlatView::Clamp() {
        originX = Placed(originX, mapWidth * cellSize, screen.width);
        originY = Placed(originY, mapHeight * cellSize, screen.height);
    }

    void FlatView::Reset() {
        cellSize = FitSize();
        originX = 0.0;
        originY = 0.0;
        Clamp();
    }

    void FlatView::Zoom(const int notches, const ScreenPoint anchor) {
        const double fit = FitSize();
        const double wanted = std::clamp(cellSize * std::pow(ZoomStep, notches), fit,
                                         std::max(fit, MaxCellSize));
        const double ratio = wanted / cellSize;
        originX = anchor.x - (anchor.x - originX) * ratio;
        originY = anchor.y - (anchor.y - originY) * ratio;
        cellSize = wanted;
        Clamp();
    }

    void FlatView::Pan(const int dx, const int dy) {
        originX += dx;
        originY += dy;
        Clamp();
    }

    double FlatView::CellSize() const { return cellSize; }
    double FlatView::ScreenX(const double mapX) const { return originX + mapX * cellSize; }
    double FlatView::ScreenY(const double mapY) const { return originY + mapY * cellSize; }

    std::optional<Position> FlatView::PickCell(const ScreenPoint point) const {
        const double x = std::floor((point.x - originX) / cellSize);
        const double y = std::floor((point.y - originY) / cellSize);
        if (x < 0.0 || y < 0.0 || x >= mapWidth || y >= mapHeight) return std::nullopt;
        return Position{static_cast<int>(x), static_cast<int>(y)};
    }

    CellRange FlatView::VisibleCells() const {
        const auto first = [this](const double origin) {
            return std::max(0, static_cast<int>(std::floor(-origin / cellSize)));
        };
        const auto end = [this](const double origin, const int pixels, const int cells) {
            return static_cast<int>(std::min<double>(cells, std::ceil((pixels - origin) / cellSize)));
        };
        return {first(originX), first(originY), end(originX, screen.width, mapWidth),
                end(originY, screen.height, mapHeight)};
    }
}
