/**
 * @file
 * @brief Ajout, comptage et mesure des animaux.
 */
module ecosim.simulation;

import std;

namespace ecosim {
    const World& Simulation::GetWorld() const { return world; }
    int Simulation::GetGeneration() const { return generation; }

    std::size_t Simulation::Count(const AnimalKind kind) const {
        return static_cast<std::size_t>(std::ranges::count_if(animals, [kind](const auto& animal) {
            return animal->GetKind() == kind;
        }));
    }

    bool Simulation::IsFinished() const { return animals.empty(); }

    std::vector<AnimalSnapshot> Simulation::Animals() const {
        std::vector<AnimalSnapshot> result;
        result.reserve(animals.size());
        for (const auto& animal : animals) {
            result.push_back({animal->GetId(), animal->GetPosition(), animal->GetKind(),
                              animal->GetAge(), animal->GetEnergy(),
                              animal->GetPath(), animal->GetParentId()});
        }
        return result;
    }

    const SimulationStats& Simulation::Stats() const { return stats; }

    PopulationSample Simulation::Measure() const {
        PopulationSample sample{generation};
        for (const auto& animal : animals) {
            SpeciesSample& species = sample.species[animal->GetKind()];
            ++species.population;
            species.energy += animal->GetEnergy();
        }
        for (SpeciesSample& species : sample.species.values) {
            if (species.population > 0) species.energy /= static_cast<double>(species.population);
        }
        for (int y = 0; y < world.GetHeight(); ++y) {
            for (int x = 0; x < world.GetWidth(); ++x) {
                const Cell& cell = world.GetCell(Position{x, y});
                if (cell.IsAccessible()) sample.vegetation += cell.GetVegetation();
            }
        }
        return sample;
    }

    void Simulation::AddAnimal(const AnimalKind kind, const Position position,
                               const std::optional<int> energy) {
        if (!world.IsInside(position) || !world.GetCell(position).IsAccessible()) {
            throw std::invalid_argument{"Un animal doit naître sur une prairie"};
        }
        animals.push_back(SpeciesOf(kind).create(nextId, position,
                                                 energy.value_or(Animal::StartingEnergy),
                                                 settings.species[kind].options));
        ++nextId;
        stats.RecordSample(Measure());
    }

    void Simulation::Seed(const AnimalKind kind, const int count) {
        if (count < 0) throw std::invalid_argument{"Population initiale négative"};
        if (count == 0) return;

        std::vector<Position> accessible;
        for (int y = 0; y < world.GetHeight(); ++y) {
            for (int x = 0; x < world.GetWidth(); ++x) {
                const Position position{x, y};
                if (world.GetCell(position).IsAccessible()) accessible.push_back(position);
            }
        }
        if (accessible.empty()) throw std::runtime_error{"Aucune prairie pour les animaux"};

        std::uniform_int_distribution<std::size_t> pick{0, accessible.size() - 1};
        // Les animaux ne partent pas du centre exact des cases.
        std::uniform_real_distribution<double> within{0.2, 0.8};
        std::uniform_real_distribution<double> direction{-std::numbers::pi, std::numbers::pi};
        for (int i = 0; i < count; ++i) {
            const Position cell = accessible[pick(rng)];
            AddAnimal(kind, cell);
            const double x = cell.x + within(rng);
            const double y = cell.y + within(rng);
            animals.back()->Place({x, y}, direction(rng));
        }
    }

    Simulation CreateRandomSimulation(const int width, const int height,
                                      const SimulationSettings& settings, const std::uint32_t seed) {
        std::mt19937 rng{seed};
        World world = GenerateRandomWorld(width, height, rng,
                                          settings.waterPercent, settings.obstaclePercent);
        Simulation simulation{std::move(world), std::mt19937{static_cast<std::uint32_t>(rng())},
                              settings};
        for (const SpeciesInfo& info : SpeciesCatalog()) {
            simulation.Seed(info.kind, settings.species[info.kind].initialCount);
        }
        return simulation;
    }
}
