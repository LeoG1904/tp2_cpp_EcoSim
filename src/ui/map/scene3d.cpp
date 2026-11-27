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

    /// @brief Un bloc posé à plat, orientable autour de la verticale.
    struct Box {
        /// @brief Centre, sur l'axe des colonnes.
        double x;
        /// @brief Centre, sur l'axe des lignes.
        double z;
        /// @brief Demi-longueur dans la direction heading.
        double halfAlong;
        /// @brief Demi-largeur perpendiculaire à heading.
        double halfAcross;
        /// @brief Hauteur du dessous.
        double bottom;
        /// @brief Hauteur du dessus.
        double top;
        /// @brief Orientation, en radians.
        double heading;
        /// @brief Couleur du dessus ; les côtés sont assombris.
        Color color;
        /// @brief Ce que représente le bloc.
        FaceKind kind;
        /// @brief Espèce, si le bloc fait partie d'un animal.
        std::optional<ecosim::AnimalKind> species;
    };

    /// @brief Une touffe d'herbe à trois pointes, tournée vers la caméra.
    struct Tuft {
        /// @brief Sommets de la touffe dans l'espace.
        std::array<Vec3, ecosim::MaxFacePoints> corners;
        /// @brief Couleur de la touffe.
        Color color;
    };

    /// @brief Un élément posé au sol, trié par distance avant le dessin.
    struct Object {
        /// @brief Carré de la distance à la caméra.
        double distance;
        /// @brief Blocs d'un obstacle ou d'un animal.
        std::vector<Box> boxes;
        /// @brief Touffe d'herbe, le cas échéant.
        std::optional<Tuft> tuft{};
    };

    /// @brief Ce que partagent les fonctions qui ajoutent des polygones.
    struct Context {
        /// @brief Caméra utilisée pour la projection.
        const ecosim::OrbitCamera& camera;
        /// @brief Taille de l'image.
        ecosim::Viewport viewport;
        /// @brief Position de la caméra.
        Vec3 eye;
        /// @brief Liste de polygones à remplir, dans l'ordre de dessin.
        std::vector<Face>& faces;
    };

    /// @brief Assombrit une couleur.
    /// @param light Part de lumière conservée, de 0 à 1.
    /// @param color Couleur de départ.
    Color Shade(const Color color, const double light) {
        return {static_cast<int>(color.red * light), static_cast<int>(color.green * light),
                static_cast<int>(color.blue * light)};
    }

    /// @brief Carré de la distance entre deux points ; suffit pour les comparer.
    double SquaredDistance(const Vec3 a, const Vec3 b) {
        const double x = a.x - b.x;
        const double y = a.y - b.y;
        const double z = a.z - b.z;
        return x * x + y * y + z * z;
    }

    /// @brief Projette un polygone et l'ajoute ; il est ignoré si un sommet passe derrière la caméra.
    /// @tparam Count Nombre de sommets.
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

    /// @brief Ajoute une dalle du sol.
    /// @param margin Retrait sur les bords, qui dessine le quadrillage.
    /// @param context Caméra, image et liste à remplir.
    /// @param x Bord de la case sur l'axe des colonnes.
    /// @param z Bord de la case sur l'axe des lignes.
    /// @param y Hauteur de la dalle.
    /// @param color Couleur de la dalle.
    /// @param kind Ce que représente la dalle.
    void AddTile(const Context& context, const double x, const double z, const double margin,
                 const double y, const Color color, const FaceKind kind) {
        const double left = x + margin;
        const double right = x + 1.0 - margin;
        const double back = z + margin;
        const double front = z + 1.0 - margin;
        AddFace(context, std::array<Vec3, 4>{{{left, y, back}, {right, y, back}, {right, y, front}, {left, y, front}}},
                color, kind);
    }

    /// @brief La hauteur du dessus d'une case.
    double TopOf(const ecosim::World& world, const ecosim::Position cell) {
        return world.GetCell(cell).GetElevation() * ecosim::LevelHeight;
    }

    /// @brief Ajoute les flancs visibles d'une case : ceux qui dominent une voisine, ou le bord de la carte, du côté de la caméra.
    void AddCliffs(const Context& context, const ecosim::World& world, const ecosim::Position cell) {
        const double top = TopOf(world, cell);
        if (top <= 0.0) return;
        const double x = cell.x;
        const double z = cell.y;
        /// @brief Un côté : la voisine d'en face, les deux bouts de son bord et sa part de lumière.
        struct Side {
            /// @brief Case voisine, qui peut être hors de la carte.
            ecosim::Position neighbor;
            /// @brief Premier bout du bord.
            std::pair<double, double> from;
            /// @brief Second bout du bord.
            std::pair<double, double> to;
            /// @brief Vrai si la caméra est de ce côté.
            bool faces;
            /// @brief Part de lumière.
            double light;
        };
        const std::array<Side, 4> sides{{
            {{cell.x - 1, cell.y}, {x, z}, {x, z + 1}, context.eye.x < x, 0.72},
            {{cell.x + 1, cell.y}, {x + 1, z}, {x + 1, z + 1}, context.eye.x > x + 1, 0.72},
            {{cell.x, cell.y - 1}, {x, z}, {x + 1, z}, context.eye.z < z, 0.92},
            {{cell.x, cell.y + 1}, {x, z + 1}, {x + 1, z + 1}, context.eye.z > z + 1, 0.92}}};
        for (const Side& side : sides) {
            if (!side.faces) continue;
            const double bottom = world.IsInside(side.neighbor) ? TopOf(world, side.neighbor) : 0.0;
            if (bottom >= top) continue;
            const Color soil = Shade(ecosim::LevelTint({112, 86, 56}, world.GetCell(cell).GetElevation()),
                                     side.light);
            AddFace(context, std::array<Vec3, 4>{{{side.from.first, bottom, side.from.second},
                                                  {side.to.first, bottom, side.to.second},
                                                  {side.to.first, top, side.to.second},
                                                  {side.from.first, top, side.from.second}}},
                    soil, FaceKind::Cliff);
        }
    }

    /// @brief Ajoute le dessus et les côtés d'un bloc tournés vers la caméra.
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

    /// @brief Couleur du sol selon le terrain et l'herbe.
    Color GroundColor(const ecosim::Cell& cell) {
        switch (cell.GetTerrain()) {
            case ecosim::Terrain::Eau: return {55, 112, 190};
            case ecosim::Terrain::Obstacle: return {78, 78, 74};
            case ecosim::Terrain::Prairie: break;
        }
        const double ratio = static_cast<double>(cell.GetVegetation()) / ecosim::Cell::MaxVegetation;
        return {static_cast<int>(150 - ratio * 90), static_cast<int>(110 + ratio * 90), 70};
    }

    /// @brief Nombre pseudo-aléatoire stable, de 0 à 1 : l'herbe ne bouge pas d'une image à l'autre.
    double Noise(const int x, const int z, const int index) {
        std::uint32_t value = static_cast<std::uint32_t>(x) * 73856093u
                            ^ static_cast<std::uint32_t>(z) * 19349663u
                            ^ static_cast<std::uint32_t>(index) * 83492791u;
        value ^= value >> 13;
        value *= 1274126177u;
        value ^= value >> 16;
        return (value % 1000u) / 999.0;
    }

    /// @brief Ajoute les touffes d'herbe d'une prairie, posées à la hauteur donnée.
    void AddGrass(const Context& context, std::vector<Object>& objects, const int x, const int z,
                  const int vegetation, const double ground) {
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
                return Vec3{baseX + rightX * side, ground + y, baseZ + rightZ * side};
            };
            // Vert foncé : les touffes se détachent du sol, même sur une prairie bien verte.
            const int shade = static_cast<int>(Noise(x, z, i + 41) * 30.0);
            const Tuft tuft{{corner(-0.09, 0.0), corner(-0.11 + lean, top * 0.75),
                             corner(-0.03, top * 0.30), corner(lean, top),
                             corner(0.03, top * 0.30), corner(0.11 + lean, top * 0.65),
                             corner(0.09, 0.0)},
                            {25 + shade / 2, 105 + shade, 35}};
            objects.push_back({SquaredDistance(context.eye, {baseX, ground + top / 2, baseZ}), {}, tuft});
        }
    }

    /// @brief Distance autour de son centre, en cases, où un animal est supposé recouvrir le sol.
    constexpr double BodyReach{0.3};

    /// @brief La hauteur du sol sous un animal ; elle varie en douceur quand son trajet change de palier.
    double GroundUnder(const ecosim::World& world, const ecosim::Path& path, const double progress) {
        const double along = std::clamp(progress, 0.0, 1.0) * ecosim::TicksPerGeneration;
        const std::size_t step = std::min(static_cast<std::size_t>(along),
                                          static_cast<std::size_t>(ecosim::TicksPerGeneration - 1));
        const auto height = [&world](const ecosim::Point point) {
            const ecosim::Position cell = ecosim::CellAt(point);
            return world.IsInside(cell) ? TopOf(world, cell) : 0.0;
        };
        return std::lerp(height(path[step]), height(path[step + 1]), along - static_cast<double>(step));
    }

    /// @brief Construit le modèle d'un animal, orienté dans le sens de sa marche, avec les détails de son sexe ou de son jeune âge.
    std::vector<Box> AnimalModel(const ecosim::AnimalKind kind, const ecosim::Sex sex, const int age,
                                 const double x, const double z, const double heading) {
        const ecosim::SpeciesStyle& style = ecosim::StyleOf(kind);
        const ecosim::SexLook& look = ecosim::LookOf(kind, sex, age);
        const bool young = ecosim::IsYoung(age);
        const bool male = sex == ecosim::Sex::Male;
        const double scale = young ? ecosim::YoungScale : 1.0;
        // Un jeune a la tête grosse pour son corps.
        const double head = std::min(0.08, style.thickness / 2.0) * (young ? 1.25 : 1.0);
        const double headAt = style.length * 0.55;
        const double headTop = style.height + head * 1.25;
        std::vector<Box> boxes;
        // Un bloc placé par rapport à l'animal : en avant de son centre, et sur le côté. Tout est réduit pour un jeune.
        const auto part = [&](const double forward, const double side, const double halfAlong,
                              const double halfAcross, const double bottom, const double top,
                              const Color color) {
            boxes.push_back({x + (std::cos(heading) * forward - std::sin(heading) * side) * scale,
                             z + (std::sin(heading) * forward + std::cos(heading) * side) * scale,
                             halfAlong * scale, halfAcross * scale, bottom * scale, top * scale, heading,
                             color, FaceKind::Animal, kind});
        };
        part(0.0, 0.0, style.length / 2.0, style.thickness / 2.0, 0.03, style.height, look.body);
        part(headAt, 0.0, head, head, style.height - head, headTop, Shade(look.body, 0.88));

        if (young) {
            switch (kind) {
                case ecosim::AnimalKind::Deer:
                    // Le faon a le dos taché de blanc.
                    for (const auto& [forward, side] : {std::pair{-0.07, -0.04}, std::pair{-0.02, 0.045},
                                                        std::pair{0.05, -0.03}}) {
                        part(forward, side, 0.022, 0.022, style.height, style.height + 0.012, look.accent);
                    }
                    break;
                case ecosim::AnimalKind::Rabbit:
                    // Le lapereau n'a que de courtes oreilles.
                    for (const double side : {-1.0, 1.0}) {
                        part(headAt - 0.03, side * 0.036, 0.016, 0.022, headTop, headTop + 0.05, look.accent);
                    }
                    break;
                case ecosim::AnimalKind::Wolf:
                    // Le louveteau a deux petites oreilles rondes.
                    for (const double side : {-1.0, 1.0}) {
                        part(headAt - 0.02, side * 0.07, 0.025, 0.028, headTop, headTop + 0.06, look.accent);
                    }
                    break;
                case ecosim::AnimalKind::Fox:
                    // Le renardeau a une queue courte.
                    part(-style.length / 2.0 - 0.035, 0.0, 0.04, 0.032, 0.08, 0.15, look.body);
                    break;
            }
            return boxes;
        }
        switch (kind) {
            case ecosim::AnimalKind::Deer:
                // Le cerf porte des bois : une perche et une branche de chaque côté.
                if (!male) break;
                for (const double side : {-1.0, 1.0}) {
                    part(headAt, side * 0.05, 0.014, 0.014, headTop, headTop + 0.17, look.accent);
                    part(headAt, side * 0.095, 0.014, 0.045, headTop + 0.09, headTop + 0.12, look.accent);
                }
                break;
            case ecosim::AnimalKind::Rabbit:
                // Les lapins ont de longues oreilles : bleues pour le mâle, roses pour la femelle.
                for (const double side : {-1.0, 1.0}) {
                    part(headAt - 0.03, side * 0.032, 0.014, 0.018, headTop, headTop + 0.13, look.accent);
                }
                break;
            case ecosim::AnimalKind::Wolf:
                // Le loup a une crinière sombre, la louve un museau clair.
                if (male) {
                    part(headAt * 0.55, 0.0, 0.055, style.thickness * 0.68, style.height * 0.55,
                         style.height + 0.05, look.accent);
                } else {
                    part(headAt + head, 0.0, 0.035, head * 0.6, style.height - head * 0.6,
                         style.height + head * 0.2, look.accent);
                }
                break;
            case ecosim::AnimalKind::Fox:
                // Les renards ont une longue queue ; celle du mâle a le bout blanc.
                part(-style.length / 2.0 - 0.07, 0.0, 0.075, 0.032, 0.08, 0.15, look.body);
                if (male) part(-style.length / 2.0 - 0.17, 0.0, 0.035, 0.034, 0.08, 0.155, look.accent);
                break;
        }
        return boxes;
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
        const auto indexOf = [&world](const Position cell) {
            return static_cast<std::size_t>(cell.y) * world.GetWidth() + cell.x;
        };

        std::vector<Position> order;
        bool flat = true;
        for (int z = 0; z < world.GetHeight(); ++z) {
            for (int x = 0; x < world.GetWidth(); ++x) {
                order.push_back({x, z});
                flat = flat && world.GetCell(order.back()).GetElevation() == 0;
            }
        }
        // Avec du relief, les cases sont rangées de la plus lointaine à la plus proche. Cet écart en
        // colonnes plus lignes garantit qu'une case n'en cache jamais une qui est dessinée après elle.
        if (!flat) {
            std::ranges::stable_sort(order, std::greater{}, [&context](const Position cell) {
                return std::abs(cell.x + 0.5 - context.eye.x) + std::abs(cell.y + 0.5 - context.eye.z);
            });
        }
        // Un groupe réunit des sols dessinés ensemble, puis ce qu'ils portent. Un sol plat ne cache
        // jamais un volume : il forme un seul groupe. Avec du relief, chaque case a le sien.
        std::vector<std::size_t> groupOf(order.size(), 0);
        if (!flat) {
            for (std::size_t rank = 0; rank < order.size(); ++rank) groupOf[indexOf(order[rank])] = rank;
        }
        std::vector<std::vector<Object>> content(flat ? 1 : order.size());

        for (const Position position : order) {
            const Cell& cell = world.GetCell(position);
            std::vector<Object>& objects = content[groupOf[indexOf(position)]];
            const double ground = TopOf(world, position);
            if (cell.GetTerrain() == Terrain::Prairie) {
                AddGrass(context, objects, position.x, position.y, cell.GetVegetation(), ground);
            }
            if (cell.GetTerrain() == Terrain::Obstacle) {
                const Box rock{position.x + 0.5, position.y + 0.5, 0.4, 0.4, ground, ground + 0.55, 0.0,
                               {132, 132, 126}, FaceKind::Obstacle};
                objects.push_back({SquaredDistance(context.eye, {rock.x, ground + 0.27, rock.z}), {rock}});
            }
        }

        for (const AnimalSnapshot& animal : animals) {
            const Point point = PathPoint(animal.path, progress);
            const double ground = GroundUnder(world, animal.path, progress);
            std::vector<Box> model = AnimalModel(animal.kind, animal.sex, animal.age, point.x, point.y,
                                                 PathHeading(animal.path, progress));
            for (Box& box : model) {
                box.bottom += ground;
                box.top += ground;
            }
            // L'animal est dessiné avec la plus proche des cases qu'il recouvre sans qu'elle le domine :
            // le sol voisin ne le recouvre pas, mais une hauteur placée devant lui le cache.
            std::size_t group = 0;
            if (!flat && world.IsInside(CellAt(point))) {
                const int level = world.GetCell(CellAt(point)).GetElevation();
                for (const double dx : {-BodyReach, 0.0, BodyReach}) {
                    for (const double dz : {-BodyReach, 0.0, BodyReach}) {
                        const Position under = CellAt({point.x + dx, point.y + dz});
                        if (!world.IsInside(under) || world.GetCell(under).GetElevation() > level) continue;
                        group = std::max(group, groupOf[indexOf(under)]);
                    }
                }
            }
            content[group].push_back(
                {SquaredDistance(context.eye, {point.x, ground + 0.15, point.y}), std::move(model)});
        }

        for (std::size_t group = 0; group < content.size(); ++group) {
            const std::size_t first = flat ? 0 : group;
            const std::size_t last = flat ? order.size() : group + 1;
            for (std::size_t rank = first; rank < last; ++rank) {
                const Position position = order[rank];
                const Cell& cell = world.GetCell(position);
                const double ground = TopOf(world, position);
                const Color color = LevelTint(GroundColor(cell), cell.GetElevation());
                AddCliffs(context, world, position);
                if (selected == position) {
                    AddTile(context, position.x, position.y, 0.0, ground, {255, 255, 255},
                            FaceKind::Selection);
                    AddTile(context, position.x, position.y, 0.12, ground, color, FaceKind::Ground);
                } else {
                    AddTile(context, position.x, position.y, 0.025, ground, color, FaceKind::Ground);
                }
            }
            // Algorithme du peintre : les volumes proches recouvrent les volumes lointains.
            std::ranges::stable_sort(content[group], std::greater{}, &Object::distance);
            for (Object& object : content[group]) {
                std::ranges::stable_sort(object.boxes, std::greater{}, [&context](const Box& box) {
                    return SquaredDistance(context.eye, {box.x, (box.bottom + box.top) / 2, box.z});
                });
                for (const Box& box : object.boxes) AddBox(context, box);
                if (object.tuft) {
                    AddFace(context, object.tuft->corners, object.tuft->color, FaceKind::Grass);
                }
            }
        }
        return faces;
    }
}
