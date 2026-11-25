module ecosim.simulation;

import std;

namespace {
    // Seul endroit du moteur qui associe une espèce à sa classe.
    std::unique_ptr<ecosim::Animal> MakeAnimal(const ecosim::AnimalKind kind, const std::uint64_t id,
                                                const ecosim::Position position, const int energy,
                                                const ecosim::AnimalTraits& traits) {
        switch (kind) {
            case ecosim::AnimalKind::Herbivore:
                return std::make_unique<ecosim::Herbivore>(id, position, energy, traits);
            case ecosim::AnimalKind::Predator:
                return std::make_unique<ecosim::Predator>(id, position, energy, traits);
        }
        throw std::invalid_argument{"Espèce inconnue"};
    }
}

namespace ecosim {
    const World& Simulation::GetWorld() const { return world; }
    int Simulation::GetGeneration() const { return generation; }

    std::size_t Simulation::Count(const AnimalKind kind) const {
        return static_cast<std::size_t>(std::ranges::count_if(animals, [kind](const auto& animal) {
            return animal->GetKind() == kind;
        }));
    }

    std::size_t Simulation::HerbivoreCount() const { return Count(AnimalKind::Herbivore); }
    std::size_t Simulation::PredatorCount() const { return Count(AnimalKind::Predator); }

    bool Simulation::IsFinished() const { return animals.empty(); }

    std::vector<AnimalSnapshot> Simulation::Animals() const {
        std::vector<AnimalSnapshot> result;
        result.reserve(animals.size());
        for (const auto& animal : animals) {
            result.push_back({animal->GetId(), animal->GetPosition(), animal->GetKind(),
                              animal->GetAge(), animal->GetEnergy(),
                              animal->GetPreviousPosition(), animal->GetParentId()});
        }
        return result;
    }

    const std::vector<PopulationSample>& Simulation::PopulationHistory() const {
        return populationHistory;
    }

    void Simulation::AddAnimal(const AnimalKind kind, const Position position,
                               const std::optional<int> energy) {
        if (!world.IsInside(position) || !world.GetCell(position).IsAccessible()) {
            throw std::invalid_argument{"Un animal doit naître sur une prairie"};
        }
        const AnimalTraits& traits = TraitsOf(settings, kind);
        animals.push_back(MakeAnimal(kind, nextId, position,
                                     energy.value_or(traits.startingEnergy), traits));
        ++nextId;
        populationHistory.back().herbivores = HerbivoreCount();
        populationHistory.back().predators = PredatorCount();
    }

    void Simulation::AddHerbivore(const Position position, const std::optional<int> energy) {
        AddAnimal(AnimalKind::Herbivore, position, energy);
    }

    void Simulation::SeedHerbivores(const int count) {
        SeedAnimals(count, AnimalKind::Herbivore);
    }

    void Simulation::AddPredator(const Position position, const std::optional<int> energy) {
        AddAnimal(AnimalKind::Predator, position, energy);
    }

    void Simulation::SeedPredators(const int count) {
        SeedAnimals(count, AnimalKind::Predator);
    }

    void Simulation::SeedAnimals(const int count, const AnimalKind kind) {
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
        for (int i = 0; i < count; ++i) {
            AddAnimal(kind, accessible[pick(rng)]);
        }
    }
}
