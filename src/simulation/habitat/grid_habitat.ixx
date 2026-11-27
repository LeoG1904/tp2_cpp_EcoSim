/**
 * @file
 * @brief Ce que les animaux perçoivent pendant une génération.
 */
export module ecosim.grid_habitat;

import std;
export import ecosim.animal;
export import ecosim.stats;

export namespace ecosim {
    /// @brief Part de la plus longue poussée permise en un petit pas, en cases : dans une foule, personne n'est projeté.
    /// @ingroup moteur
    inline constexpr double MaxPushPerTick{0.1};

    /// @brief L'habitat réel d'une génération.
    ///
    /// Les animaux sont rangés par case : chercher ses voisins ne demande de parcourir que les cases proches. Elle ne possède pas les animaux.
    /// @ingroup moteur
    class GridHabitat final : public Habitat {
    public:
        /// @brief Range les animaux vivants dans leur case.
        /// @param world Carte, qui doit vivre aussi longtemps que l'habitat.
        /// @param animals Animaux à ranger, qui doivent vivre aussi longtemps que l'habitat.
        /// @param stats Statistiques où compter les captures.
        GridHabitat(World& world, const std::vector<std::unique_ptr<Animal>>& animals,
                    SimulationStats& stats);

        /// @brief Change un animal de rangement après son déplacement.
        /// @param animal Animal déplacé.
        /// @param from Case qu'il occupait.
        void Moved(Animal& animal, Position from);

        /// @brief Écarte les animaux dont les corps se superposent : les collisions souples.
        ///
        /// Toutes les poussées sont calculées avant d'être appliquées : le résultat ne favorise pas les premiers de la liste. Le plus petit corps recule davantage. Un petit chevauchement peut subsister dans une foule ou contre un obstacle.
        /// @param tick Numéro du petit pas qui vient d'être joué.
        void Separate(int tick);

        /// @brief La carte, modifiable.
        World& GetWorld() override;
        /// @brief La carte, en lecture.
        const World& GetWorld() const override;
        /// @copydoc Habitat::NearestPrey
        std::optional<Point> NearestPrey(const Animal& hunter, Point from,
                                         double radius) const override;
        /// @copydoc Habitat::ThreatsNear
        std::vector<Point> ThreatsNear(const Animal& prey, Point from, double radius) const override;
        /// @copydoc Habitat::MatesNear
        std::vector<Point> MatesNear(const Animal& animal, Point from, double radius) const override;
        /// @brief Capture la proie à portée la plus proche et compte sa mort.
        std::optional<int> CatchPrey(const Animal& hunter, Point from, double reach) override;
        /// @brief Partage le repas avec les congénères à Wolf::SharingDistance au plus.
        int ShareMeal(const Animal& hunter, int energy) override;

    private:
        /// @brief Rang d'une case dans le tableau des rangements.
        std::size_t Index(Position position) const;
        /// @brief Appelle visit pour chaque animal vivant à une distance inférieure ou égale à radius.
        /// @tparam Visit Fonction recevant l'animal et sa distance.
        template <typename Visit>
        void ForEachNear(Point from, double radius, const Visit& visit) const;
        /// @brief La poussée que les voisins exercent sur un animal, limitée à MaxPushPerTick.
        Point PushOn(const Animal& animal) const;
        /// @brief La proie la plus proche dans le rayon parmi celles que le chasseur mange, ou nullptr.
        Animal* ClosestPrey(const Animal& hunter, Point from, double radius) const;

        /// @brief La carte, partagée avec la simulation.
        World& world;
        /// @brief Statistiques où sont comptées les captures.
        SimulationStats& stats;
        /// @brief Pour chaque case, les animaux qui s'y trouvent (références non possédées).
        std::vector<std::vector<Animal*>> buckets;
    };
}
