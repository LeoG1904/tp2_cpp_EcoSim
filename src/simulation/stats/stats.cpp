/**
 * @file
 * @brief Enregistrement des statistiques.
 */
module ecosim.stats;

import std;

namespace {
    std::size_t& Counter(ecosim::LifeEvents& events, const ecosim::DeathCause cause) {
        switch (cause) {
            case ecosim::DeathCause::Starvation: return events.starved;
            case ecosim::DeathCause::OldAge: return events.diedOfAge;
            case ecosim::DeathCause::Predation: return events.eaten;
        }
        throw std::invalid_argument{"Cause de mort inconnue"};
    }
}

namespace ecosim {
    void SimulationStats::BeginGeneration() { lastGeneration = {}; }

    void SimulationStats::RecordBirth(const AnimalKind kind) {
        ++lastGeneration[kind].births;
        ++total[kind].births;
    }

    void SimulationStats::RecordDeath(const AnimalKind kind, const DeathCause cause) {
        ++Counter(lastGeneration[kind], cause);
        ++Counter(total[kind], cause);
    }

    void SimulationStats::RecordSample(const PopulationSample& measure) {
        PopulationSample sample = measure;
        for (std::size_t i = 0; i < SpeciesCount; ++i) {
            sample.species.values[i].events = lastGeneration.values[i];
        }
        if (!history.empty() && history.back().generation == sample.generation) history.back() = sample;
        else history.push_back(sample);
    }

    const std::vector<PopulationSample>& SimulationStats::History() const { return history; }

    const LifeEvents& SimulationStats::LastGeneration(const AnimalKind kind) const {
        return lastGeneration[kind];
    }

    const LifeEvents& SimulationStats::Total(const AnimalKind kind) const {
        return total[kind];
    }
}
