/**
 * @file
 * @brief Dessin de la carte 3D.
 */
module;

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

module ecosim.render;

import std;
import ecosim.animation;
import ecosim.scene3d;
import ecosim.layout;

namespace ecosim {
    void DrawOrbitMap(Canvas& canvas, const Simulation& simulation, const double progress,
                      const std::optional<Position> selected, const OrbitCamera& camera) {
        std::vector<AnimalSnapshot> animals = simulation.Animals();
        AttachNewbornsToParents(animals);
        const std::vector<Face> faces = BuildScene(simulation.GetWorld(), animals, progress, camera,
                                                   {canvas.Width(), canvas.Height()}, selected);
        const HDC buffer = static_cast<HDC>(canvas.Context());
        canvas.Fill({0, 0, canvas.Width(), canvas.Height()}, {22, 30, 42});

        // Le pinceau système change de couleur sans créer un objet par face.
        const HGDIOBJ previousBrush = SelectObject(buffer, GetStockObject(DC_BRUSH));
        const HGDIOBJ previousPen = SelectObject(buffer, GetStockObject(NULL_PEN));
        for (const Face& face : faces) {
            SetDCBrushColor(buffer, RGB(face.color.red, face.color.green, face.color.blue));
            std::array<POINT, MaxFacePoints> points{};
            for (std::size_t i = 0; i < face.pointCount; ++i) {
                points[i] = {face.points[i].x, face.points[i].y};
            }
            Polygon(buffer, points.data(), static_cast<int>(face.pointCount));
        }
        SelectObject(buffer, previousPen);
        SelectObject(buffer, previousBrush);
    }
}
