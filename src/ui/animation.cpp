module ecosim.animation;

import std;

namespace ecosim {
    void ManualStepAnimation::Start(SimulationClock& clock) {
        clock.SetProgress(0.0);
        active = true;
    }

    void ManualStepAnimation::Cancel() { active = false; }

    void ManualStepAnimation::Update(SimulationClock& clock, const double realSeconds) {
        if (!active) return;
        if (!clock.IsPaused()) {
            active = false;
            return;
        }
        // L'horloge garde la position atteinte : la reprise ne fait pas reculer les animaux.
        clock.SetProgress(clock.Progress() + realSeconds * StepsPerSecond);
        active = clock.Progress() < 1.0;
    }

    void AttachNewbornsToParents(std::vector<AnimalSnapshot>& animals) {
        std::unordered_map<std::uint64_t, Position> origins;
        for (const auto& animal : animals) origins.emplace(animal.id, animal.previousPosition);
        for (auto& animal : animals) {
            if (animal.age != 0 || !animal.parentId) continue;
            if (const auto parent = origins.find(*animal.parentId); parent != origins.end()) {
                animal.previousPosition = parent->second;
            }
        }
    }
}
