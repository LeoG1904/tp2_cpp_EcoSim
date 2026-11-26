/**
 * @file
 * @brief Construction du sol, des obstacles, de l'herbe et des animaux en 3D.
 */
module ecosim.scene3d;

import std;
import ecosim.animation;

namespace {
    using ecosim::Color;
    using ecosim::Face;
    using ecosim::FaceKind;
    using ecosim::Vec3;

    // Bloc posé à plat, orientable autour de la verticale.
    struct Box {
        double x;          // centre au sol
        double z;
        double halfAlong;  // demi-longueur dans le sens de heading
        double halfAcross;
        double bottom;
        double top;
        double heading;
        Color color;
        FaceKind kind;
        std::optional<ecosim::AnimalKind> species;
    };

    // Une touffe d'herbe : trois pointes, toujours tournées vers la caméra.
    struct Tuft {
        std::array<Vec3, ecosim::MaxFacePoints> corners;
        Color color;
    };

    // Un élément posé au sol : un obstacle, un animal fait de deux blocs, ou une touffe d'herbe.
    struct Object {
        double distance;
        std::vector<Box> boxes;
        std::optional<Tuft> tuft{};
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
    template <std::size_t Count>
    void AddFace(const Context& context, const std::array<Vec3, Count>& corners, const Color color,
                 const FaceKind kind, const std::optional<ecosim::AnimalKind> species = std::nullopt) {
        static_assert(Count <= ecosim::MaxFacePoints);
        Face face{{}, Count, color, kind, species};
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
        AddFace(context, std::array<Vec3, 4>{{{left, 0, back}, {right, 0, back}, {right, 0, front}, {left, 0, front}}},
                color, kind);
    }

    // Seules les faces tournées vers la caméra sont ajoutées ; le dessous n'est jamais visible.
    void AddBox(const Context& context, const Box& box) {
        const double alongX = std::cos(box.heading);
        const double alongZ = std::sin(box.heading);
        const auto corner = [&](const double along, const double across, const double y) {
            return Vec3{box.x + alongX * along * box.halfAlong - alongZ * across * box.halfAcross, y,
                        box.z + alongZ * along * box.halfAlong + alongX * across * box.halfAcross};
        };
        if (context.eye.y > box.top) {
            AddFace(context, std::array<Vec3, 4>{{corner(-1, -1, box.top), corner(1, -1, box.top),
                                                  corner(1, 1, box.top), corner(-1, 1, box.top)}},
                    box.color, box.kind, box.species);
        }
        // Les quatre côtés : deux extrémités, puis les deux flancs.
        constexpr std::array<std::array<std::pair<double, double>, 2>, 4> sides{{
            {{{1, -1}, {1, 1}}}, {{{-1, 1}, {-1, -1}}}, {{{1, 1}, {-1, 1}}}, {{{-1, -1}, {1, -1}}}}};
        for (const auto& side : sides) {
            const Vec3 a = corner(side[0].first, side[0].second, box.bottom);
            const Vec3 b = corner(side[1].first, side[1].second, box.bottom);
            const double centreX = (a.x + b.x) / 2.0;
            const double centreZ = (a.z + b.z) / 2.0;
            const double outX = centreX - box.x;
            const double outZ = centreZ - box.z;
            if ((context.eye.x - centreX) * outX + (context.eye.z - centreZ) * outZ <= 0.0) continue;
            // La lumière dépend de l'orientation du côté.
            const double light = 0.56 + 0.16 * std::abs(outX) / std::hypot(outX, outZ);
            AddFace(context, std::array<Vec3, 4>{{a, b, {b.x, box.top, b.z}, {a.x, box.top, a.z}}},
                    Shade(box.color, light), box.kind, box.species);
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

    // Nombre pseudo-aléatoire stable de 0 à 1 : l'herbe ne bouge pas d'une image à l'autre.
    double Noise(const int x, const int z, const int index) {
        std::uint32_t value = static_cast<std::uint32_t>(x) * 73856093u
                            ^ static_cast<std::uint32_t>(z) * 19349663u
                            ^ static_cast<std::uint32_t>(index) * 83492791u;
        value ^= value >> 13;
        value *= 1274126177u;
        value ^= value >> 16;
        return (value % 1000u) / 999.0;
    }

    void AddGrass(const Context& context, std::vector<Object>& objects, const int x, const int z,
                  const int vegetation) {
        // Places des touffes dans la case ; un petit décalage évite l'effet de quadrillage.
        constexpr std::array<std::pair<double, double>, ecosim::MaxGrassTufts> places{{
            {0.50, 0.50}, {0.24, 0.26}, {0.74, 0.72}, {0.73, 0.28}, {0.27, 0.74}}};
        // La largeur d'une touffe suit la droite de la caméra.
        const double rightX = std::cos(context.camera.Yaw());
        const double rightZ = -std::sin(context.camera.Yaw());
        const double height = ecosim::GrassHeight(vegetation);
        for (int i = 0; i < ecosim::GrassTuftCount(vegetation); ++i) {
            const double baseX = x + places[i].first + (Noise(x, z, i) - 0.5) * 0.12;
            const double baseZ = z + places[i].second + (Noise(z, x, i + 7) - 0.5) * 0.12;
            const double top = height * (0.75 + 0.25 * Noise(x, z, i + 13));
            const double lean = (Noise(x, z, i + 29) - 0.5) * 0.08;
            const auto corner = [&](const double side, const double y) {
                return Vec3{baseX + rightX * side, y, baseZ + rightZ * side};
            };
            // Vert foncé : les touffes se détachent du sol, même sur une prairie bien verte.
            const int shade = static_cast<int>(Noise(x, z, i + 41) * 30.0);
            const Tuft tuft{{corner(-0.09, 0.0), corner(-0.11 + lean, top * 0.75),
                             corner(-0.03, top * 0.30), corner(lean, top),
                             corner(0.03, top * 0.30), corner(0.11 + lean, top * 0.65),
                             corner(0.09, 0.0)},
                            {25 + shade / 2, 105 + shade, 35}};
            objects.push_back({SquaredDistance(context.eye, {baseX, top / 2, baseZ}), {}, tuft});
        }
    }

    // Corps et tête, orientés dans le sens de la marche.
    std::vector<Box> AnimalModel(const ecosim::AnimalKind kind, const double x, const double z,
                                 const double heading) {
        const ecosim::SpeciesStyle& style = ecosim::StyleOf(kind);
        const double headX = x + std::cos(heading) * style.length * 0.55;
        const double headZ = z + std::sin(heading) * style.length * 0.55;
        constexpr double head{0.08};
        return {
            Box{x, z, style.length / 2.0, style.thickness / 2.0, 0.03, style.height, heading,
                style.color, FaceKind::Animal, kind},
            Box{headX, headZ, head, head, style.height - 0.08, style.height + 0.10, heading,
                Shade(style.color, 0.88), FaceKind::Animal, kind},
        };
    }
}

namespace ecosim {
    // En dessous de 5, la case est considérée comme rase.
    int GrassTuftCount(const int vegetation) {
        if (vegetation < 5) return 0;
        return std::min(MaxGrassTufts, 1 + vegetation / 22);
    }

    double GrassHeight(const int vegetation) {
        if (vegetation < 5) return 0.0;
        const double ratio = std::min(1.0, static_cast<double>(vegetation) / Cell::MaxVegetation);
        return 0.04 + ratio * (MaxGrassHeight - 0.04);
    }

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
                if (cell.GetTerrain() == Terrain::Prairie) {
                    AddGrass(context, objects, x, z, cell.GetVegetation());
                }
                if (cell.GetTerrain() == Terrain::Obstacle) {
                    const Box rock{x + 0.5, z + 0.5, 0.4, 0.4, 0.0, 0.55, 0.0,
                                   {132, 132, 126}, FaceKind::Obstacle};
                    objects.push_back({SquaredDistance(context.eye, {x + 0.5, 0.27, z + 0.5}), {rock}});
                }
            }
        }

        for (const AnimalSnapshot& animal : animals) {
            const Point point = PathPoint(animal.path, progress);
            objects.push_back({SquaredDistance(context.eye, {point.x, 0.15, point.y}),
                               AnimalModel(animal.kind, point.x, point.y,
                                           PathHeading(animal.path, progress))});
        }

        // Algorithme du peintre : les volumes proches recouvrent les volumes lointains.
        std::ranges::stable_sort(objects, std::greater{}, &Object::distance);
        for (Object& object : objects) {
            std::ranges::stable_sort(object.boxes, std::greater{}, [&context](const Box& box) {
                return SquaredDistance(context.eye, {box.x, (box.bottom + box.top) / 2, box.z});
            });
            for (const Box& box : object.boxes) AddBox(context, box);
            if (object.tuft) {
                AddFace(context, object.tuft->corners, object.tuft->color, FaceKind::Grass);
            }
        }
        return faces;
    }
}
