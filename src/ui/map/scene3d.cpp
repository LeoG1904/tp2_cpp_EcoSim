module ecosim.scene3d;

import std;

namespace {
    using ecosim::Color;
    using ecosim::Face;
    using ecosim::FaceKind;
    using ecosim::Vec3;

    struct Box {
        Vec3 low;
        Vec3 high;
        Color color;
        FaceKind kind;
        std::optional<ecosim::AnimalKind> species;
    };

    // Un volume posé au sol : un obstacle, ou un animal fait de deux blocs.
    struct Object {
        double distance;
        std::vector<Box> boxes;
    };

    struct Context {
        const ecosim::OrbitCamera& camera;
        ecosim::Viewport viewport;
        Vec3 eye;
        std::vector<Face>& faces;
    };

    Color Shade(const Color color, const double light) {
        return {static_cast<int>(color.red * light), static_cast<int>(color.green * light),
                static_cast<int>(color.blue * light)};
    }

    double SquaredDistance(const Vec3 a, const Vec3 b) {
        const double x = a.x - b.x;
        const double y = a.y - b.y;
        const double z = a.z - b.z;
        return x * x + y * y + z * z;
    }

    // Une face dont un sommet passe derrière la caméra est ignorée.
    void AddFace(const Context& context, const std::array<Vec3, 4>& corners, const Color color,
                 const FaceKind kind, const std::optional<ecosim::AnimalKind> species = std::nullopt) {
        Face face{{}, color, kind, species};
        for (std::size_t i = 0; i < corners.size(); ++i) {
            const auto point = context.camera.Project(corners[i], context.viewport);
            if (!point) return;
            face.points[i] = *point;
        }
        context.faces.push_back(face);
    }

    void AddTile(const Context& context, const double x, const double z, const double margin,
                 const Color color, const FaceKind kind) {
        const double left = x + margin;
        const double right = x + 1.0 - margin;
        const double back = z + margin;
        const double front = z + 1.0 - margin;
        AddFace(context, {{{left, 0, back}, {right, 0, back}, {right, 0, front}, {left, 0, front}}},
                color, kind);
    }

    // Seules les faces tournées vers la caméra sont ajoutées ; le dessous n'est jamais visible.
    void AddBox(const Context& context, const Box& box) {
        const Vec3& a = box.low;
        const Vec3& b = box.high;
        if (context.eye.y > b.y) {
            AddFace(context, {{{a.x, b.y, a.z}, {b.x, b.y, a.z}, {b.x, b.y, b.z}, {a.x, b.y, b.z}}},
                    box.color, box.kind, box.species);
        }
        if (context.eye.x < a.x) {
            AddFace(context, {{{a.x, a.y, a.z}, {a.x, a.y, b.z}, {a.x, b.y, b.z}, {a.x, b.y, a.z}}},
                    Shade(box.color, 0.72), box.kind, box.species);
        }
        if (context.eye.x > b.x) {
            AddFace(context, {{{b.x, a.y, a.z}, {b.x, a.y, b.z}, {b.x, b.y, b.z}, {b.x, b.y, a.z}}},
                    Shade(box.color, 0.72), box.kind, box.species);
        }
        if (context.eye.z < a.z) {
            AddFace(context, {{{a.x, a.y, a.z}, {b.x, a.y, a.z}, {b.x, b.y, a.z}, {a.x, b.y, a.z}}},
                    Shade(box.color, 0.56), box.kind, box.species);
        }
        if (context.eye.z > b.z) {
            AddFace(context, {{{a.x, a.y, b.z}, {b.x, a.y, b.z}, {b.x, b.y, b.z}, {a.x, b.y, b.z}}},
                    Shade(box.color, 0.56), box.kind, box.species);
        }
    }

    Color GroundColor(const ecosim::Cell& cell) {
        switch (cell.GetTerrain()) {
            case ecosim::Terrain::Eau: return {55, 112, 190};
            case ecosim::Terrain::Obstacle: return {78, 78, 74};
            case ecosim::Terrain::Prairie: break;
        }
        const double ratio = static_cast<double>(cell.GetVegetation()) / ecosim::Cell::MaxVegetation;
        return {static_cast<int>(150 - ratio * 90), static_cast<int>(110 + ratio * 90), 70};
    }

    // La case est partagée en une bande par espèce, avec quatre places chacune.
    std::pair<double, double> Slot(const ecosim::AnimalKind kind, const int rank) {
        constexpr double band = 1.0 / ecosim::SpeciesCount;
        const double x = band * (static_cast<double>(kind) + (rank % 2 == 0 ? 0.3 : 0.7));
        const double z = rank / 2 % 2 == 0 ? 0.30 : 0.70;
        return {x, z};
    }

    // Corps et tête ; la tête est tournée dans le sens du dernier déplacement.
    std::vector<Box> AnimalModel(const ecosim::AnimalKind kind, const double x, const double z,
                                 const int directionX, const int directionZ) {
        const ecosim::SpeciesStyle& style = ecosim::StyleOf(kind);
        const bool alongX = directionX != 0;
        const double halfX = (alongX ? style.length : style.thickness) / 2.0;
        const double halfZ = (alongX ? style.thickness : style.length) / 2.0;
        const int forwardZ = alongX ? 0 : directionZ < 0 ? -1 : 1;
        const double headX = x + directionX * style.length * 0.55;
        const double headZ = z + forwardZ * style.length * 0.55;
        constexpr double head{0.08};
        return {
            Box{{x - halfX, 0.03, z - halfZ}, {x + halfX, style.height, z + halfZ}, style.color,
                FaceKind::Animal, kind},
            Box{{headX - head, style.height - 0.08, headZ - head},
                {headX + head, style.height + 0.10, headZ + head}, Shade(style.color, 0.88),
                FaceKind::Animal, kind},
        };
    }
}

namespace ecosim {
    std::vector<Face> BuildScene(const World& world, const std::vector<AnimalSnapshot>& animals,
                                 const double progress, const OrbitCamera& camera,
                                 const Viewport viewport, const std::optional<Position> selected) {
        std::vector<Face> faces;
        const Context context{camera, viewport, camera.Position3d(), faces};
        std::vector<Object> objects;

        // Le sol est plat : il ne cache jamais un volume et peut être dessiné en premier.
        for (int z = 0; z < world.GetHeight(); ++z) {
            for (int x = 0; x < world.GetWidth(); ++x) {
                const Cell& cell = world.GetCell(Position{x, z});
                if (selected == Position{x, z}) {
                    AddTile(context, x, z, 0.0, {255, 255, 255}, FaceKind::Selection);
                    AddTile(context, x, z, 0.12, GroundColor(cell), FaceKind::Ground);
                } else {
                    AddTile(context, x, z, 0.025, GroundColor(cell), FaceKind::Ground);
                }
                if (cell.GetTerrain() == Terrain::Obstacle) {
                    const Box rock{{x + 0.1, 0.0, z + 0.1}, {x + 0.9, 0.55, z + 0.9},
                                   {132, 132, 126}, FaceKind::Obstacle};
                    objects.push_back({SquaredDistance(context.eye, {x + 0.5, 0.27, z + 0.5}), {rock}});
                }
            }
        }

        // Rang de chaque animal parmi ceux de son espèce, au départ et à l'arrivée.
        std::map<std::tuple<int, int, AnimalKind>, int> departures;
        std::map<std::tuple<int, int, AnimalKind>, int> arrivals;
        for (const AnimalSnapshot& animal : animals) {
            const int from = departures[{animal.previousPosition.x, animal.previousPosition.y,
                                         animal.kind}]++;
            const int to = arrivals[{animal.position.x, animal.position.y, animal.kind}]++;
            if (to >= MaxModelsPerCell) continue;
            const auto [fromX, fromZ] = Slot(animal.kind, from % MaxModelsPerCell);
            const auto [toX, toZ] = Slot(animal.kind, to);
            const double x = std::lerp(animal.previousPosition.x + fromX,
                                       animal.position.x + toX, progress);
            const double z = std::lerp(animal.previousPosition.y + fromZ,
                                       animal.position.y + toZ, progress);
            const int directionX = animal.position.x - animal.previousPosition.x;
            const int directionZ = animal.position.y - animal.previousPosition.y;
            objects.push_back({SquaredDistance(context.eye, {x, 0.15, z}),
                               AnimalModel(animal.kind, x, z, directionX, directionZ)});
        }

        // Algorithme du peintre : les volumes proches recouvrent les volumes lointains.
        std::ranges::stable_sort(objects, std::greater{}, &Object::distance);
        for (Object& object : objects) {
            std::ranges::stable_sort(object.boxes, std::greater{}, [&context](const Box& box) {
                return SquaredDistance(context.eye, {(box.low.x + box.high.x) / 2,
                                                     (box.low.y + box.high.y) / 2,
                                                     (box.low.z + box.high.z) / 2});
            });
            for (const Box& box : object.boxes) AddBox(context, box);
        }
        return faces;
    }
}
