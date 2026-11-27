/**
 * @file
 * @brief Ce que tous les chasseurs font de la même manière : poursuivre, errer et manger.
 */
module ecosim.predator;

import std;

namespace ecosim {
    Predator::Predator(const std::uint64_t id, const Position cell, const int energy,
                       const SpeciesTraits& traits, const AnimalOptions& options, const double sight)
        : Animal{id, cell, energy, traits, options}, sight{sight} {}

    void Predator::BeginGeneration(std::mt19937&) { hasEaten = false; }

    Steering Predator::Steer(const Habitat& habitat, std::mt19937& rng) const {
        // Assez nourri pour se reproduire, il cherche d'abord un partenaire.
        if (const std::optional<Steering> courtship = SeekMate(habitat)) return *courtship;
        // Repu, il flâne jusqu'à la génération suivante.
        if (hasEaten) return {GetHeading() + Jitter(rng, 0.6), 0.3};

        const double range = sight + GetOptions().instinct;
        if (const auto prey = habitat.NearestPrey(*this, GetLocation(), range)) {
            return Chase(habitat, *prey, range);
        }
        return Search(habitat, range, rng);
    }

    Steering Predator::Chase(const Habitat&, const Point prey, double) const {
        return {HeadingTo(GetLocation(), prey), 1.0};
    }

    Steering Predator::Search(const Habitat&, double, std::mt19937& rng) const {
        return {GetHeading() + Jitter(rng, 0.7), 0.55};
    }

    int Predator::KeptMeal(Habitat&, const int meal) const { return meal; }

    void Predator::Act(Habitat& habitat, int) {
        if (hasEaten) return;
        const std::optional<int> size = habitat.CatchPrey(*this, GetLocation(), Reach);
        if (!size) return;
        hasEaten = true;
        // Une petite proie nourrit moins.
        ReceiveMeal(KeptMeal(habitat, GetTraits().foodEnergy * *size / 100));
    }
}
