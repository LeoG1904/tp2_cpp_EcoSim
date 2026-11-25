module ecosim.clock;

import std;

namespace ecosim {
    void SimulationClock::SetSpeed(const int multiplier) {
        if (std::ranges::find(Speeds, multiplier) == Speeds.end()) {
            throw std::invalid_argument{"Vitesse non supportée"};
        }
        speed = multiplier;
    }

    int SimulationClock::GetSpeed() const { return speed; }

    void SimulationClock::CycleSpeed() {
        auto next = std::ranges::find(Speeds, speed) + 1;
        speed = next == Speeds.end() ? Speeds.front() : *next;
    }

    void SimulationClock::SetPaused(const bool value) { paused = value; }
    bool SimulationClock::IsPaused() const { return paused; }

    int SimulationClock::Advance(const double realSeconds) {
        if (paused) return 0;
        accumulator += realSeconds * speed * BaseGenerationsPerSecond;
        const int steps = static_cast<int>(accumulator);
        accumulator -= steps;
        return steps;
    }
}
