/**
 * @file
 * @brief Décisions de l'herbivore : fuir, flâner ou chercher de l'herbe.
 */
module ecosim.herbivore;

import std;

namespace ecosim {
    Herbivore::Herbivore(const std::uint64_t id, const Position cell, const int energy,
                         const SpeciesTraits& traits, const AnimalOptions& options,
                         const int grazingVegetation, const int ungrazableVegetation)
        : Animal{id, cell, energy, traits, options}, grazingVegetation{grazingVegetation},
          ungrazableVegetation{ungrazableVegetation} {}

    bool Herbivore::IsPrey() const { return true; }

    void Herbivore::BeginGeneration(std::mt19937& rng) {
        alert = false;
        if (const int reflex = GetOptions().instinct; reflex > 0) {
            std::bernoulli_distribution react{std::min(reflex, 100) / 100.0};
            alert = react(rng);
        }
    }

    Steering Herbivore::Steer(const Habitat& habitat, std::mt19937& rng) const {
        const Point here = GetLocation();
        if (alert) {
            // Chaque chasseur repousse, d'autant plus fort qu'il est proche.
            double awayX = 0.0;
            double awayY = 0.0;
            for (const Point hunter : habitat.ThreatsNear(*this, here, ThreatSight)) {
                const double weight = 1.0 / std::max(0.01, std::pow(Distance(here, hunter), 2.0));
                awayX += (here.x - hunter.x) * weight;
                awayY += (here.y - hunter.y) * weight;
            }
            if (awayX != 0.0 || awayY != 0.0) return {std::atan2(awayY, awayX), 1.0};
        }

        // La soif passe juste après la fuite.
        if (const std::optional<Steering> drinking = SeekWater(habitat)) return *drinking;

        // Bien nourri, il part à la recherche d'un partenaire.
        if (const std::optional<Steering> courtship = SeekMate(habitat)) return *courtship;

        const World& world = habitat.GetWorld();
        if (world.GetCell(GetPosition()).GetVegetation() >= grazingVegetation) {
            return {GetHeading() + Jitter(rng, 0.8), 0.2};
        }
        // Huit directions à deux distances : de l'herbe riche, proche et devant soi.
        std::uniform_real_distribution<double> hesitation{0.0, 12.0};
        double bestScore = -std::numeric_limits<double>::infinity();
        double bestHeading = GetHeading() + std::numbers::pi;
        for (int direction = 0; direction < 8; ++direction) {
            const double angle = GetHeading() + direction * std::numbers::pi / 4.0;
            const double turn = std::abs(std::remainder(angle - GetHeading(), 2.0 * std::numbers::pi));
            for (const double reach : {FoodSight / 2.0, FoodSight}) {
                const Point target{here.x + std::cos(angle) * reach, here.y + std::sin(angle) * reach};
                if (!world.IsWalkable(target)) continue;
                const double score = world.GetCell(CellAt(target)).GetVegetation()
                                   - 5.0 * reach - 6.0 * turn + hesitation(rng);
                if (score > bestScore) {
                    bestScore = score;
                    bestHeading = angle;
                }
            }
        }
        return {bestHeading, 0.8};
    }

    void Herbivore::Act(Habitat& habitat, const int tick) {
        if (tick != TicksPerGeneration - 1) return;
        Cell& cell = habitat.GetWorld().GetCell(GetPosition());
        if (!cell.IsAccessible()) return;
        const int reachable = std::max(0, cell.GetVegetation() - ungrazableVegetation);
        const int consumed = std::min({GetTraits().foodEnergy, reachable, MaxEnergy - GetEnergy()});
        cell.SetVegetation(cell.GetVegetation() - consumed);
        ReceiveMeal(consumed);
    }
}
