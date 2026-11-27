/**
 * @file
 * @brief Boucle du thread de calcul et publication des photos.
 */
module ecosim.runner;

import std;

namespace ecosim {
    SimulationRunner::SimulationRunner(Simulation source)
        : simulation{std::move(source)},
          latest{std::make_shared<const SimulationSnapshot>(simulation.Snapshot())},
          worker{[this](std::stop_token stop) { Work(std::move(stop)); }} {}

    void SimulationRunner::Request(const int steps) {
        if (steps <= 0) return;
        {
            const std::lock_guard lock{mutex};
            if (failed) return;
            requested += steps;
            busy = true;
        }
        wake.notify_one();
    }

    void SimulationRunner::Add(const AnimalKind kind, const Position cell) {
        {
            const std::lock_guard lock{mutex};
            if (failed) return;
            additions.push_back({kind, cell});
            inserting = true;
        }
        wake.notify_one();
    }

    RunnerState SimulationRunner::State() const {
        const std::lock_guard lock{mutex};
        return {latest, busy, failed};
    }

    void SimulationRunner::Wait() const {
        std::unique_lock lock{mutex};
        idle.wait(lock, [this] { return !busy && !inserting; });
    }

    void SimulationRunner::Work(const std::stop_token stop) {
        std::unique_lock lock{mutex};
        while (wake.wait(lock, stop, [this] { return requested > 0 || !additions.empty(); })) {
            const int steps = std::exchange(requested, 0);
            const std::vector<Addition> added = std::exchange(additions, {});
            lock.unlock();
            bool succeeded = true;
            try {
                if (!added.empty()) Insert(added);
                Compute(steps, stop);
            } catch (const std::exception&) {
                // Une exception qui sortirait du thread fermerait l'application.
                succeeded = false;
            }
            lock.lock();
            failed = failed || !succeeded;
            if (failed) {
                requested = 0;
                additions.clear();
            }
            busy = requested > 0;
            inserting = !additions.empty();
            if (!busy && !inserting) idle.notify_all();
        }
        // À l'arrêt, personne ne doit rester bloqué dans Wait.
        busy = false;
        inserting = false;
        idle.notify_all();
    }

    void SimulationRunner::Insert(const std::vector<Addition>& animals) {
        for (const Addition& animal : animals) {
            const World& world = simulation.GetWorld();
            if (!world.IsInside(animal.cell) || !world.GetCell(animal.cell).IsAccessible()) continue;
            simulation.AddAnimal(animal.kind, animal.cell);
        }
        Publish();
    }

    void SimulationRunner::Compute(const int steps, const std::stop_token& stop) {
        using Clock = std::chrono::steady_clock;
        const auto start = Clock::now();
        auto published = start;
        bool unpublished = false;
        for (int i = 0; i < steps && !simulation.IsFinished() && !stop.stop_requested(); ++i) {
            simulation.Step();
            unpublished = true;
            const auto now = Clock::now();
            if (std::chrono::duration<double>(now - start).count() > MaxSecondsPerRequest) break;
            if (std::chrono::duration<double>(now - published).count() > SecondsBetweenSnapshots) {
                Publish();
                published = now;
                unpublished = false;
            }
        }
        // Inutile de copier quand la fenêtre se ferme.
        if (unpublished && !stop.stop_requested()) Publish();
    }

    void SimulationRunner::Publish() {
        // La copie se fait hors du verrou : seul l'échange des pointeurs le prend.
        auto snapshot = std::make_shared<const SimulationSnapshot>(simulation.Snapshot());
        const std::lock_guard lock{mutex};
        latest.swap(snapshot);
    }
}
