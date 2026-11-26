/**
 * @file
 * @brief Décisions du prédateur : poursuivre, se placer en meute ou errer.
 */
module ecosim.predator;

import std;

namespace ecosim {
    Predator::Predator(const std::uint64_t id, const Position cell, const int energy,
                       const AnimalOptions& options)
        : Animal{id, cell, energy, Traits, options} {}

    AnimalKind Predator::GetKind() const { return AnimalKind::Predator; }
    void Predator::BeginGeneration(std::mt19937&) { hasEaten = false; }

    Steering Predator::Steer(const Habitat& habitat, std::mt19937& rng) const {
        // Repu, il flâne jusqu'à la génération suivante.
        if (hasEaten) return {GetHeading() + Jitter(rng, 0.6), 0.3};

        const Point here = GetLocation();
        const int pack = GetOptions().instinct;
        const double range = Sight + pack;
        const std::vector<Point> mates =
            pack > 0 ? habitat.HuntersNear(here, range, this) : std::vector<Point>{};
        if (const auto prey = habitat.NearestPrey(here, range)) {
            Point target = *prey;
            const double distance = Distance(here, target);
            if (!mates.empty() && distance > 1.0) {
                // En meute, chacun vise un côté de la proie pour lui couper la route.
                const double side = GetId() % 2 == 0 ? 1.0 : -1.0;
                const double offset = side * std::min(1.2, distance * 0.35) / distance;
                const double towardX = prey->x - here.x;
                const double towardY = prey->y - here.y;
                target.x -= towardY * offset;
                target.y += towardX * offset;
            }
            return {HeadingTo(here, target), 1.0};
        }
        // Sans proie en vue, la meute se regroupe sans s'entasser.
        if (!mates.empty()) {
            const Point nearest = *std::ranges::min_element(mates, {}, [here](const Point mate) {
                return Distance(here, mate);
            });
            if (Distance(here, nearest) > 1.2) return {HeadingTo(here, nearest), 0.7};
        }
        return {GetHeading() + Jitter(rng, 0.7), 0.55};
    }

    void Predator::Act(Habitat& habitat, int) {
        if (hasEaten) return;
        const std::optional<int> size = habitat.CatchPrey(GetLocation(), Reach);
        if (!size) return;
        hasEaten = true;
        // Une petite proie nourrit moins.
        const int meal = GetTraits().foodEnergy * *size / 100;
        ReceiveMeal(GetOptions().instinct > 0 ? habitat.ShareMeal(*this, meal) : meal);
    }

    std::unique_ptr<Animal> Predator::MakeOffspring(const std::uint64_t id) const {
        return std::make_unique<Predator>(id, GetPosition(), 30, GetOptions());
    }
}
