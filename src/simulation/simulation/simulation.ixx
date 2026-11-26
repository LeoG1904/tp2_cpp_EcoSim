/**
 * @file
 * @brief Le cœur du moteur : le cycle d'une génération.
 */
export module ecosim.simulation;

import std;
export import ecosim.clock;
export import ecosim.world;
export import ecosim.species;
export import ecosim.settings;
export import ecosim.stats;

export namespace ecosim {
    /// @brief Fait repousser l'herbe de toutes les prairies.
    /// @param world Carte à modifier.
    /// @param growth Quantité ajoutée à chaque prairie.
    /// @ingroup moteur
    void GrowVegetation(World& world, int growth = 1);

    /// @brief Une copie en lecture d'un animal, destinée à l'affichage et aux tests.
    /// @ingroup moteur
    struct AnimalSnapshot {
        /// @brief Identifiant unique.
        std::uint64_t id;
        /// @brief Case sous l'animal à la fin de la génération.
        Position position;
        /// @brief Espèce.
        AnimalKind kind;
        /// @brief Âge, en générations.
        int age;
        /// @brief Énergie restante.
        int energy;
        /// @brief Trajet de la dernière génération ; le dernier point est la position actuelle.
        Path path;
        /// @brief Identifiant du parent, s'il est né pendant la simulation.
        std::optional<std::uint64_t> parentId;
    };

    /// @brief Le monde, ses animaux et les règles qui les font évoluer.
    ///
    /// Elle fonctionne sans affichage.
    /// @ingroup moteur
    class Simulation {
    public:
        /// @brief Crée une simulation sans animal.
        /// @param world Carte utilisée.
        /// @param rng Générateur de hasard : même graine, même déroulement.
        /// @param settings Réglages de la simulation.
        Simulation(World world, std::mt19937 rng, const SimulationSettings& settings = {});

        /// @brief La carte, en lecture.
        const World& GetWorld() const;
        /// @brief Numéro de la génération actuelle.
        int GetGeneration() const;
        /// @brief Nombre d'animaux vivants d'une espèce.
        std::size_t Count(AnimalKind kind) const;
        /// @brief Vrai quand il ne reste plus aucun animal.
        bool IsFinished() const;
        /// @brief Une copie de l'état de tous les animaux.
        std::vector<AnimalSnapshot> Animals() const;
        /// @brief L'historique et les événements.
        const SimulationStats& Stats() const;

        /// @brief Ajoute un animal au centre d'une case.
        /// @param kind Espèce.
        /// @param position Case de départ.
        /// @param energy Énergie ; si absente, celle des réglages.
        /// @throws std::invalid_argument si la case n'est pas une prairie.
        void AddAnimal(AnimalKind kind, Position position, std::optional<int> energy = std::nullopt);
        /// @brief Place des animaux au hasard sur les prairies.
        /// @throws std::invalid_argument si le nombre est négatif.
        /// @throws std::runtime_error s'il n'y a aucune prairie.
        void Seed(AnimalKind kind, int count);
        /// @brief Calcule une génération complète. Sans effet si la simulation est terminée.
        ///
        /// Voir la page @ref fonctionnement.
        void Step();

    private:
        World world;
        SimulationSettings settings;
        std::vector<std::unique_ptr<Animal>> animals;
        std::mt19937 rng;
        std::uint64_t nextId{1};
        int generation{0};
        SimulationStats stats;

        // Les petits pas d'une génération : se déplacer, manger, chasser.
        void MoveAndInteract();
        // Naissances, métabolisme et morts de la génération.
        void LiveAndReproduce();
        PopulationSample Measure() const;
    };

    /// @brief Crée une carte au hasard et y place les populations de départ.
    /// @param width Nombre de colonnes.
    /// @param height Nombre de lignes.
    /// @param settings Réglages.
    /// @param seed Graine : la même redonne la même simulation.
    /// @ingroup moteur
    Simulation CreateRandomSimulation(int width, int height, const SimulationSettings& settings,
                                      std::uint32_t seed);
}
