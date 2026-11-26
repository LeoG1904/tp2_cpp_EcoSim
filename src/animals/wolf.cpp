/**
 * @file
 * @brief Ce qui est propre au loup : la meute.
 */
module;
#include <cstdlib>
module ecosim.wolf;

import std;

namespace ecosim {
    Wolf::Wolf(const std::uint64_t id, const Position cell, const int energy,
               const AnimalOptions& options)
        : Predator{id, cell, energy, Traits, options, Sight} {}

    AnimalKind Wolf::GetKind() const { return AnimalKind::Wolf; }

    bool Wolf::Eats(const AnimalKind kind) const {
        static const char* mode = std::getenv("WOLFDIET");
        if (mode && mode[0] == '1') return kind == AnimalKind::Deer;
        return kind == AnimalKind::Deer || kind == AnimalKind::Rabbit;
    }

    std::vector<Point> Wolf::Pack(const Habitat& habitat, const double range) const {
        if (GetOptions().instinct <= 0) return {};
        return habitat.MatesNear(*this, GetLocation(), range);
    }

    Steering Wolf::Chase(const Habitat& habitat, const Point prey, const double range) const {
        const Point here = GetLocation();
        Point target = prey;
        const double distance = Distance(here, target);
        if (distance > 1.0 && !Pack(habitat, range).empty()) {
            // En meute, chacun vise un côté de la proie pour lui couper la route.
            const double side = GetId() % 2 == 0 ? 1.0 : -1.0;
            const double offset = side * std::min(1.2, distance * 0.35) / distance;
            const double towardX = prey.x - here.x;
            const double towardY = prey.y - here.y;
            target.x -= towardY * offset;
            target.y += towardX * offset;
        }
        return {HeadingTo(here, target), 1.0};
    }

    Steering Wolf::Search(const Habitat& habitat, const double range, std::mt19937& rng) const {
        // Sans proie en vue, la meute se regroupe sans s'entasser.
        const Point here = GetLocation();
        const std::vector<Point> mates = Pack(habitat, range);
        if (!mates.empty()) {
            const Point nearest = *std::ranges::min_element(mates, {}, [here](const Point mate) {
                return Distance(here, mate);
            });
            if (Distance(here, nearest) > 1.2) return {HeadingTo(here, nearest), 0.7};
        }
        return Predator::Search(habitat, range, rng);
    }

    int Wolf::KeptMeal(Habitat& habitat, const int meal) const {
        return GetOptions().instinct > 0 ? habitat.ShareMeal(*this, meal) : meal;
    }

    std::unique_ptr<Animal> Wolf::MakeOffspring(const std::uint64_t id) const {
        return std::make_unique<Wolf>(id, GetPosition(), 30, GetOptions());
    }
}
