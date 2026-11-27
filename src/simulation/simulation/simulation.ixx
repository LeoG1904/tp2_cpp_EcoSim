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
        /// @brief Identifiant de la mère, s'il est né pendant la simulation.
        std::optional<std::uint64_t> parentId;
        /// @brief Sexe.
        Sex sex{Sex::Female};
        /// @brief Hydratation restante ; vide quand la soif est inactive.
        std::optional<int> hydration{};
    };

    /// @brief Une copie complète de ce qu'il faut afficher, à la fin d'une génération.
    ///
    /// Elle ne dépend plus de la simulation : on peut la lire pendant qu'un autre thread calcule la suite.
    /// @ingroup moteur
    struct SimulationSnapshot {
        /// @brief La carte et son herbe.
        World world;
        /// @brief Les animaux vivants.
        std::vector<AnimalSnapshot> animals;
        /// @brief Numéro de la génération.
        int generation;
        /// @brief Une mesure par génération, la génération 0 comprise.
        std::vector<PopulationSample> history;

        /// @brief Nombre d'animaux vivants d'une espèce.
        std::size_t Count(AnimalKind kind) const;
        /// @brief Vrai quand il ne reste plus aucun animal.
        bool IsFinished() const;
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
        /// @brief Vrai si le réglage de la soif est activé, même quand aucune eau n'est accessible.
        bool HasThirst() const;
        /// @brief Une copie de l'état de tous les animaux.
        std::vector<AnimalSnapshot> Animals() const;
        /// @brief L'historique et les événements.
        const SimulationStats& Stats() const;
        /// @brief Copie l'état à afficher.
        SimulationSnapshot Snapshot() const;

        /// @brief Ajoute un animal au centre d'une case.
        /// @param kind Espèce.
        /// @param position Case de départ.
        /// @param energy Énergie ; si absente, Animal::StartingEnergy.
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
        /// @brief La carte et son herbe.
        World world;
        /// @brief Réglages fixés à la création.
        SimulationSettings settings;
        /// @brief Chemins vers l'eau, calculés à la création.
        WaterRoutes water;
        /// @brief Les animaux vivants, possédés par la simulation.
        std::vector<std::unique_ptr<Animal>> animals;
        /// @brief Générateur de hasard : il rend le déroulement reproductible.
        std::mt19937 rng;
        /// @brief Animaux placés par espèce : ils alternent femelle et mâle pour que chaque espèce ait les deux.
        PerSpecies<std::size_t> placed{};
        /// @brief Identifiant du prochain animal créé.
        std::uint64_t nextId{1};
        /// @brief Numéro de la dernière génération calculée.
        int generation{0};
        /// @brief Historique et événements.
        SimulationStats stats;

        /// @brief Crée un animal au centre d'une case, sans mesurer les populations.
        /// @throws std::invalid_argument si la case n'est pas une prairie.
        void Spawn(AnimalKind kind, Position position, std::optional<int> energy);
        /// @brief Joue les petits pas d'une génération : se déplacer, manger et chasser.
        void MoveAndInteract();
        /// @brief Ajoute aux nouveau-nés la portée d'une mère, de sexes tirés au sort, et compte les naissances. Quand la soif est coupée, les petits naissent désaltérés.
        void GiveBirth(const Animal& mother, std::vector<std::unique_ptr<Animal>>& newborns);
        /// @brief Fait vieillir les animaux, gère les naissances, puis retire les morts.
        void LiveAndReproduce();
        /// @brief Mesure les effectifs, l'énergie moyenne et l'herbe à cet instant.
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
