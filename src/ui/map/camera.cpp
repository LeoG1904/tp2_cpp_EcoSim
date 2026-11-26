/**
 * @file
 * @brief Calculs de projection et de sélection en 3D.
 */
module ecosim.camera;

import std;

namespace {
    constexpr double FieldOfView{0.78}; // environ 45 degrés, en vertical
    constexpr double NearPlane{0.2};

    ecosim::Vec3 Minus(const ecosim::Vec3 a, const ecosim::Vec3 b) {
        return {a.x - b.x, a.y - b.y, a.z - b.z};
    }

    double Dot(const ecosim::Vec3 a, const ecosim::Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

    ecosim::Vec3 Cross(const ecosim::Vec3 a, const ecosim::Vec3 b) {
        return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
    }

    ecosim::Vec3 Normalize(const ecosim::Vec3 v) {
        const double length = std::sqrt(Dot(v, v));
        return {v.x / length, v.y / length, v.z / length};
    }
}

namespace ecosim {
    OrbitCamera::OrbitCamera(const int mapWidth, const int mapHeight)
        : mapWidth{mapWidth}, mapHeight{mapHeight} {
        Reset();
    }

    void OrbitCamera::Reset() {
        yaw = 0.0;
        pitch = 0.95;
        distance = std::max(mapWidth, mapHeight) * 0.96;
    }

    void OrbitCamera::Rotate(const double yawDelta, const double pitchDelta) {
        yaw = std::remainder(yaw + yawDelta, 2.0 * std::numbers::pi);
        pitch = std::clamp(pitch + pitchDelta, MinPitch, MaxPitch);
    }

    void OrbitCamera::Zoom(const int notches) {
        const double size = std::max(mapWidth, mapHeight);
        distance = std::clamp(distance * std::pow(0.88, notches), std::max(4.0, size * 0.2), size * 2.5);
    }

    double OrbitCamera::Yaw() const { return yaw; }
    double OrbitCamera::Pitch() const { return pitch; }
    double OrbitCamera::Distance() const { return distance; }

    Vec3 OrbitCamera::Position3d() const {
        const double flat = distance * std::cos(pitch);
        return {mapWidth / 2.0 + flat * std::sin(yaw), distance * std::sin(pitch),
                mapHeight / 2.0 + flat * std::cos(yaw)};
    }

    OrbitCamera::Basis OrbitCamera::Axes() const {
        const Vec3 target{mapWidth / 2.0, 0.0, mapHeight / 2.0};
        const Vec3 forward = Normalize(Minus(target, Position3d()));
        const Vec3 right = Normalize(Cross(forward, {0.0, 1.0, 0.0}));
        return {forward, right, Cross(right, forward)};
    }

    double OrbitCamera::FocalLength(const Viewport viewport) {
        return viewport.height / 2.0 / std::tan(FieldOfView / 2.0);
    }

    std::optional<ScreenPoint> OrbitCamera::Project(const Vec3 point, const Viewport viewport) const {
        const Basis axes = Axes();
        const Vec3 relative = Minus(point, Position3d());
        const double depth = Dot(relative, axes.forward);
        if (depth < NearPlane) return std::nullopt;
        const double scale = FocalLength(viewport) / depth;
        return ScreenPoint{
            static_cast<int>(std::lround(viewport.width / 2.0 + Dot(relative, axes.right) * scale)),
            static_cast<int>(std::lround(viewport.height / 2.0 - Dot(relative, axes.up) * scale))};
    }

    std::optional<Position> OrbitCamera::PickCell(const ScreenPoint screen,
                                                  const Viewport viewport) const {
        // Rayon partant de la caméra, prolongé jusqu'au sol.
        const Basis axes = Axes();
        const double focal = FocalLength(viewport);
        const double across = (screen.x - viewport.width / 2.0) / focal;
        const double above = (viewport.height / 2.0 - screen.y) / focal;
        const Vec3 ray{axes.forward.x + axes.right.x * across + axes.up.x * above,
                       axes.forward.y + axes.right.y * across + axes.up.y * above,
                       axes.forward.z + axes.right.z * across + axes.up.z * above};
        if (ray.y >= 0.0) return std::nullopt;
        const Vec3 eye = Position3d();
        const double travel = -eye.y / ray.y;
        const double x = eye.x + ray.x * travel;
        const double z = eye.z + ray.z * travel;
        if (x < 0.0 || z < 0.0 || x >= mapWidth || z >= mapHeight) return std::nullopt;
        return Position{static_cast<int>(x), static_cast<int>(z)};
    }
}
