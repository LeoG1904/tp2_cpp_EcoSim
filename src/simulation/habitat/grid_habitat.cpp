/**
 * @file
 * @brief Rangement des animaux par case, recherche des voisins, captures et partage.
 */
module ecosim.grid_habitat;

import std;
import ecosim.wolf;

namespace ecosim {
    GridHabitat::GridHabitat(World& world, const std::vector<std::unique_ptr<Animal>>& animals,
                             SimulationStats& stats, const WaterRoutes* water)
        : world{world}, stats{stats}, water{water},
          buckets(static_cast<std::size_t>(world.GetWidth()) * world.GetHeight()) {
        for (const auto& animal : animals) {
            if (animal->IsAlive()) buckets[Index(animal->GetPosition())].push_back(animal.get());
        }
    }

    std::optional<Point> GridHabitat::TowardWater(const Point from) const {
        if (!water) return std::nullopt;
        return water->NextStep(CellAt(from));
    }

    void GridHabitat::Moved(Animal& animal, const Position from) {
        if (animal.GetPosition() == from) return;
        std::erase(buckets[Index(from)], &animal);
        buckets[Index(animal.GetPosition())].push_back(&animal);
    }

    Point GridHabitat::PushOn(const Animal& animal) const {
        const Point here = animal.GetLocation();
        const double radius = animal.GetTraits().bodyRadius;
        Point push{0.0, 0.0};
        ForEachNear(here, radius + MaxBodyRadius, [&](const Animal& other, const double distance) {
            if (&other == &animal) return;
            const double otherRadius = other.GetTraits().bodyRadius;
            const double overlap = radius + otherRadius - distance;
            if (overlap <= 0.0) return;
            // Chacun recule d'autant plus que l'autre est gros, selon les surfaces des deux corps.
            const double share = otherRadius * otherRadius
                                 / (radius * radius + otherRadius * otherRadius);
            double awayX = here.x - other.GetLocation().x;
            double awayY = here.y - other.GetLocation().y;
            if (distance < 1e-9) {
                // Exactement superposés, comme un nouveau-né : les identifiants choisissent un axe,
                // sans tirage au sort, et les deux partent en sens opposés.
                const std::uint64_t first = std::min(animal.GetId(), other.GetId());
                const double angle = static_cast<double>(first) * 2.399963;
                const double side = animal.GetId() == first ? 1.0 : -1.0;
                awayX = side * std::cos(angle);
                awayY = side * std::sin(angle);
            } else {
                awayX /= distance;
                awayY /= distance;
            }
            push.x += awayX * overlap * share;
            push.y += awayY * overlap * share;
        });
        const double length = std::hypot(push.x, push.y);
        if (length > MaxPushPerTick) {
            push.x *= MaxPushPerTick / length;
            push.y *= MaxPushPerTick / length;
        }
        return push;
    }

    void GridHabitat::Separate(const int tick) {
        std::vector<std::pair<Animal*, Point>> pushes;
        for (const std::vector<Animal*>& bucket : buckets) {
            for (Animal* animal : bucket) {
                if (!animal->IsAlive()) continue;
                const Point push = PushOn(*animal);
                if (push.x != 0.0 || push.y != 0.0) pushes.emplace_back(animal, push);
            }
        }
        for (const auto& [animal, push] : pushes) {
            const Position before = animal->GetPosition();
            animal->Push(world, push, tick);
            Moved(*animal, before);
        }
    }

    World& GridHabitat::GetWorld() { return world; }
    const World& GridHabitat::GetWorld() const { return world; }

    std::size_t GridHabitat::Index(const Position position) const {
        return static_cast<std::size_t>(position.y) * world.GetWidth()
             + static_cast<std::size_t>(position.x);
    }

    // Seules les cases touchées par le rayon sont parcourues.
    template <typename Visit>
    void GridHabitat::ForEachNear(const Point from, const double radius, const Visit& visit) const {
        const Position low = CellAt({from.x - radius, from.y - radius});
        const Position high = CellAt({from.x + radius, from.y + radius});
        for (int y = std::max(0, low.y); y <= std::min(world.GetHeight() - 1, high.y); ++y) {
            for (int x = std::max(0, low.x); x <= std::min(world.GetWidth() - 1, high.x); ++x) {
                for (Animal* animal : buckets[Index({x, y})]) {
                    if (!animal->IsAlive()) continue;
                    const double distance = Distance(from, animal->GetLocation());
                    if (distance <= radius) visit(*animal, distance);
                }
            }
        }
    }

    Animal* GridHabitat::ClosestPrey(const Animal& hunter, const Point from,
                                     const double radius) const {
        Animal* closest = nullptr;
        double best = std::numeric_limits<double>::infinity();
        ForEachNear(from, radius, [&](Animal& other, const double distance) {
            if (hunter.Eats(other.GetKind()) && distance < best) {
                closest = &other;
                best = distance;
            }
        });
        return closest;
    }

    std::optional<Point> GridHabitat::NearestPrey(const Animal& hunter, const Point from,
                                                  const double radius) const {
        const Animal* prey = ClosestPrey(hunter, from, radius);
        if (!prey) return std::nullopt;
        return prey->GetLocation();
    }

    std::vector<Point> GridHabitat::ThreatsNear(const Animal& prey, const Point from,
                                                const double radius) const {
        std::vector<Point> result;
        ForEachNear(from, radius, [&](Animal& other, double) {
            if (other.Eats(prey.GetKind())) result.push_back(other.GetLocation());
        });
        return result;
    }

    std::vector<Point> GridHabitat::MatesNear(const Animal& animal, const Point from,
                                              const double radius) const {
        std::vector<Point> result;
        ForEachNear(from, radius, [&](Animal& other, double) {
            if (&other != &animal && other.GetKind() == animal.GetKind()) {
                result.push_back(other.GetLocation());
            }
        });
        return result;
    }

    Animal* GridHabitat::ClosestMate(const Animal& seeker, const Point from,
                                     const double radius) const {
        Animal* closest = nullptr;
        double best = std::numeric_limits<double>::infinity();
        ForEachNear(from, radius, [&](Animal& other, const double distance) {
            if (distance < best && !other.GetPartner() && seeker.CanBreedWith(other)) {
                closest = &other;
                best = distance;
            }
        });
        return closest;
    }

    std::optional<Point> GridHabitat::NearestMate(const Animal& seeker, const Point from,
                                                  const double radius) const {
        const Animal* mate = ClosestMate(seeker, from, radius);
        if (!mate) return std::nullopt;
        return mate->GetLocation();
    }

    void GridHabitat::FormCouples() {
        for (const std::vector<Animal*>& bucket : buckets) {
            for (Animal* female : bucket) {
                if (female->GetSex() != Sex::Female || female->GetPartner() || !female->CanReproduce()) {
                    continue;
                }
                if (Animal* male = ClosestMate(*female, female->GetLocation(), Animal::MatingDistance)) {
                    female->PairWith(*male);
                }
            }
        }
    }

    std::optional<int> GridHabitat::CatchPrey(const Animal& hunter, const Point from,
                                              const double reach) {
        Animal* prey = ClosestPrey(hunter, from, reach);
        if (!prey) return std::nullopt;
        prey->Die();
        stats.RecordDeath(prey->GetKind(), DeathCause::Predation);
        return prey->GetTraits().mealPercent;
    }

    int GridHabitat::ShareMeal(const Animal& hunter, const int energy) {
        std::vector<Animal*> mates;
        ForEachNear(hunter.GetLocation(), Wolf::SharingDistance, [&](Animal& other, double) {
            if (&other != &hunter && other.GetKind() == hunter.GetKind()) mates.push_back(&other);
        });
        const int share = energy / (static_cast<int>(mates.size()) + 1);
        for (Animal* mate : mates) mate->ReceiveMeal(share);
        return energy - share * static_cast<int>(mates.size());
    }
}
