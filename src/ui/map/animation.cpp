/**
 * @file
 * @brief Interpolation le long des trajets.
 */
module ecosim.animation;

import std;

namespace ecosim {
    Point PathPoint(const Path& path, const double progress) {
        const double scaled = std::clamp(progress, 0.0, 1.0) * TicksPerGeneration;
        const auto step = static_cast<std::size_t>(std::min(scaled, TicksPerGeneration - 1.0));
        const double part = scaled - static_cast<double>(step);
        return {std::lerp(path[step].x, path[step + 1].x, part),
                std::lerp(path[step].y, path[step + 1].y, part)};
    }

    double PathHeading(const Path& path, const double progress) {
        const double scaled = std::clamp(progress, 0.0, 1.0) * TicksPerGeneration;
        const int current = static_cast<int>(std::min(scaled, TicksPerGeneration - 1.0));
        // Le pas en cours, sinon le dernier pas effectué, sinon le premier à venir.
        const auto direction = [&path](const int step) -> std::optional<double> {
            const auto index = static_cast<std::size_t>(step);
            const double x = path[index + 1].x - path[index].x;
            const double y = path[index + 1].y - path[index].y;
            if (std::hypot(x, y) < 1e-9) return std::nullopt;
            return std::atan2(y, x);
        };
        for (int step = current; step >= 0; --step) {
            if (const auto angle = direction(step)) return *angle;
        }
        for (int step = current + 1; step < TicksPerGeneration; ++step) {
            if (const auto angle = direction(step)) return *angle;
        }
        return 0.0;
    }

    void AttachNewbornsToParents(std::vector<AnimalSnapshot>& animals) {
        std::unordered_map<std::uint64_t, Path> trips;
        for (const auto& animal : animals) trips.emplace(animal.id, animal.path);
        for (auto& animal : animals) {
            if (animal.age != 0 || !animal.parentId) continue;
            if (const auto parent = trips.find(*animal.parentId); parent != trips.end()) {
                animal.path = parent->second;
            }
        }
    }
}
