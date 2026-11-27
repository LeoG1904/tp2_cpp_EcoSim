/**
 * @file
 * @brief Ce qui est commun à tous les animaux, et ce qu'ils perçoivent.
 */
export module ecosim.animal;

import std;
export import ecosim.world;

export namespace ecosim {
    /// @brief Les espèces du projet.
    /// @ingroup moteur
    enum class AnimalKind {
        Deer, ///< Le cerf : le grand herbivore.
        Rabbit, ///< Le lapin : le petit herbivore, qui se contente d'une herbe rase.
        Wolf, ///< Le loup : chasse les cerfs et les lapins, parfois en meute.
        Fox ///< Le renard : ne chasse que les lapins, toujours seul.
    };
    /// @brief Nombre d'espèces.
    /// @ingroup moteur
    inline constexpr std::size_t SpeciesCount{4};

    /// @brief Le sexe d'un animal : il faut un de chaque pour avoir des petits.
    /// @ingroup moteur
    enum class Sex {
        Female, ///< Femelle : elle met les petits au monde.
        Male ///< Mâle.
    };

    /// @brief Une valeur pour chaque espèce, désignée par son nom plutôt que par un numéro.
    /// @tparam T Type de la valeur.
    /// @ingroup moteur
    template <typename T>
    struct PerSpecies {
        /// @brief Les valeurs, dans l'ordre d'AnimalKind.
        std::array<T, SpeciesCount> values{};

        /// @brief La valeur d'une espèce, modifiable.
        T& operator[](const AnimalKind kind) { return values.at(static_cast<std::size_t>(kind)); }
        /// @brief La valeur d'une espèce, en lecture.
        const T& operator[](const AnimalKind kind) const {
            return values.at(static_cast<std::size_t>(kind));
        }
        /// @brief Vrai si les deux valeurs sont identiques.
        bool operator==(const PerSpecies&) const = default;
    };

    /// @brief Nombre de petits pas dans une génération.
    /// @ingroup moteur
    inline constexpr int TicksPerGeneration{10};
    /// @brief Le trajet d'une génération : le point de départ, puis un point par petit pas.
    /// @ingroup moteur
    using Path = std::array<Point, TicksPerGeneration + 1>;

    /// @brief La décision de déplacement d'un animal pour un petit pas.
    /// @ingroup moteur
    struct Steering {
        /// @brief Direction voulue, en radians : 0 vers l'est, sens des y croissants.
        double heading{0.0};
        /// @brief Allure, de 0 (immobile) à 1 (pleine vitesse).
        double pace{0.0};
    };

    /// @brief Plus grand rayon de corps qu'une espèce puisse avoir, en cases.
    ///
    /// Il borne la recherche des animaux qui se touchent.
    /// @ingroup moteur
    inline constexpr double MaxBodyRadius{0.25};

    /// @brief Ce qui fait une espèce : des caractéristiques fixes, que l'utilisateur ne règle pas.
    /// @ingroup moteur
    struct SpeciesTraits {
        /// @brief Énergie perdue à chaque génération.
        int energyLoss;
        /// @brief Âge auquel l'animal meurt.
        int maxAge;
        /// @brief Herbe mangée au maximum (herbivore) ou énergie par proie (chasseur).
        int foodEnergy;
        /// @brief Énergie minimale pour se reproduire.
        int reproductionEnergy;
        /// @brief Distance par génération à pleine allure, en % de case.
        int speed;
        /// @brief Taille du repas offert au chasseur, en % de son énergie par proie.
        int mealPercent{100};
        /// @brief Rayon du corps rond de l'animal, en cases, au plus MaxBodyRadius.
        double bodyRadius{0.15};
        /// @brief Plus petit nombre de petits d'une portée.
        int litterMin{1};
        /// @brief Plus grand nombre de petits d'une portée.
        int litterMax{1};
        /// @brief Générations d'attente après une portée, pour les deux parents.
        int breedingRest{0};
        /// @brief Hydratation perdue à chaque génération, quand la soif est active.
        int waterLoss{3};

        /// @brief Vrai si les deux valeurs sont identiques.
        bool operator==(const SpeciesTraits&) const = default;
    };

    /// @brief Ce que l'utilisateur peut régler sur les animaux d'une espèce.
    /// @ingroup moteur
    struct AnimalOptions {
        /// @brief Chance de naissance, en %, quand les conditions sont réunies.
        int birthPercent{20};
        /// @brief Instinct de l'espèce, 0 pour le désactiver.
        ///
        /// Proie : chance, en %, d'être sur ses gardes. Chasseur : ajoute autant de cases à sa vue ; pour le loup, c'est aussi l'esprit de meute.
        int instinct{0};

        /// @brief Vrai si les deux valeurs sont identiques.
        bool operator==(const AnimalOptions&) const = default;
    };

    class Animal;

    /// @brief Ce qu'un animal peut percevoir et modifier autour de lui.
    ///
    /// Il évite aux animaux de connaître toute la simulation. Les obstacles ne bloquent pas la vue.
    /// @ingroup moteur
    class Habitat {
    public:
        virtual ~Habitat() = default;
        /// @brief La carte, modifiable (pour brouter).
        virtual World& GetWorld() = 0;
        /// @brief La carte, en lecture.
        virtual const World& GetWorld() const = 0;
        /// @brief La proie la plus proche que ce chasseur mange (Animal::Eats), s'il y en a une dans le rayon.
        /// @param hunter Chasseur qui cherche.
        /// @param from Centre de la recherche.
        /// @param radius Rayon, en cases.
        virtual std::optional<Point> NearestPrey(const Animal& hunter, Point from,
                                                 double radius) const = 0;
        /// @brief Les positions des chasseurs qui mangent cette proie, dans le rayon.
        /// @param prey Animal menacé.
        /// @param from Centre de la recherche.
        /// @param radius Rayon, en cases.
        virtual std::vector<Point> ThreatsNear(const Animal& prey, Point from,
                                               double radius) const = 0;
        /// @brief Les positions des autres animaux vivants de la même espèce, dans le rayon.
        /// @param animal Animal qui cherche ; il n'est pas compté.
        /// @param from Centre de la recherche.
        /// @param radius Rayon, en cases.
        virtual std::vector<Point> MatesNear(const Animal& animal, Point from,
                                             double radius) const = 0;
        /// @brief Le partenaire possible le plus proche (Animal::CanBreedWith) et encore sans couple, dans le rayon.
        /// @param seeker Animal qui cherche.
        /// @param from Centre de la recherche.
        /// @param radius Rayon, en cases.
        virtual std::optional<Point> NearestMate(const Animal& seeker, Point from,
                                                 double radius) const = 0;
        /// @brief Le prochain point à viser pour rejoindre l'eau ; vide si la soif est inactive, sur une berge ou sans chemin.
        /// @param from Position de l'animal.
        virtual std::optional<Point> TowardWater(Point from) const = 0;
        /// @brief Capture la proie à portée la plus proche parmi celles que ce chasseur mange.
        /// @return La taille du repas qu'offre la proie (SpeciesTraits::mealPercent), ou rien sans capture.
        virtual std::optional<int> CatchPrey(const Animal& hunter, Point from, double reach) = 0;
        /// @brief Partage un repas avec les congénères proches.
        /// @return La part qui reste au chasseur.
        virtual int ShareMeal(const Animal& hunter, int energy) = 0;
    };

    /// @brief Classe de base de tous les animaux.
    ///
    /// Elle gère l'identité, l'âge, l'énergie et le déplacement. Chaque espèce décide où aller (Steer) et quoi faire (Act).
    /// @ingroup moteur
    class Animal {
    public:
        /// @brief Énergie maximale.
        static constexpr int MaxEnergy{100};
        /// @brief Énergie d'un animal placé au départ.
        static constexpr int StartingEnergy{50};
        /// @brief Hydratation d'un animal qui vient de boire ou d'être placé au départ.
        static constexpr int MaxHydration{100};
        /// @brief Hydratation d'un nouveau-né : sous ThirstThreshold, il naît donc assoiffé.
        static constexpr int NewbornHydration{50};
        /// @brief Hydratation à partir de laquelle un animal a soif et va boire.
        static constexpr int ThirstThreshold{60};
        /// @brief Âge à partir duquel un animal peut se reproduire.
        static constexpr int AdultAge{5};
        /// @brief Distance, en cases, à laquelle un animal prêt repère un partenaire.
        static constexpr double MateSight{6.0};
        /// @brief Distance à laquelle deux partenaires forment un couple.
        static constexpr double MatingDistance{0.6};
        /// @brief Énergie que chaque parent dépense pour une portée.
        static constexpr int ReproductionCost{20};
        /// @brief Énergie d'un nouveau-né.
        static constexpr int NewbornEnergy{30};
        /// @brief Virage maximal par petit pas, en radians : les trajectoires restent courbes.
        static constexpr double MaxTurnPerTick{0.65};
        /// @brief Part de sa vitesse qu'un animal garde pendant qu'il monte.
        static constexpr double ClimbPace{0.5};
        /// @brief Distance devant l'animal, en cases, à partir de laquelle un palier plus haut le ralentit.
        static constexpr double ClimbReach{0.5};

        virtual ~Animal() = default;
        Animal(const Animal&) = delete;
        Animal& operator=(const Animal&) = delete;

        /// @brief Identifiant unique.
        std::uint64_t GetId() const;
        /// @brief Identifiant de la mère ; vide pour un animal placé au départ.
        std::optional<std::uint64_t> GetParentId() const;
        /// @brief Le sexe.
        Sex GetSex() const;
        /// @brief Choisit le sexe, à la création de l'animal.
        void SetSex(Sex value);
        /// @brief Le partenaire trouvé pendant cette génération, ou nullptr. Il n'est pas possédé et ne vaut que jusqu'à la fin de la génération.
        Animal* GetPartner() const;
        /// @brief Position libre actuelle.
        Point GetLocation() const;
        /// @brief Case située sous l'animal.
        Position GetPosition() const;
        /// @brief Direction actuelle, en radians.
        double GetHeading() const;
        /// @brief Trajet de la dernière génération.
        const Path& GetPath() const;
        /// @brief Âge, en générations.
        int GetAge() const;
        /// @brief Énergie restante.
        int GetEnergy() const;
        /// @brief Hydratation restante, de 0 à MaxHydration ; elle ne baisse que si la soif est active.
        int GetHydration() const;
        /// @brief Vrai quand l'hydratation est tombée à ThirstThreshold ou moins.
        bool IsThirsty() const;
        /// @brief Les caractéristiques de son espèce.
        const SpeciesTraits& GetTraits() const;
        /// @brief Les réglages de cet individu, transmis à ses petits.
        const AnimalOptions& GetOptions() const;
        /// @brief Vrai s'il lui reste de l'énergie et de l'eau, et qu'il n'a pas atteint l'âge maximum.
        bool IsAlive() const;

        /// @brief Installe l'animal à un endroit, sans trajet.
        void Place(Point point, double direction);
        /// @brief Efface le trajet de la génération précédente.
        void StartPath();
        /// @brief Effectue un petit pas : tourne vers la direction voulue, avance et contourne les obstacles.
        /// @param world Carte à respecter.
        /// @param steering Direction et allure voulues.
        /// @param tick Numéro du petit pas, de 0 à TicksPerGeneration - 1.
        void Advance(const World& world, Steering steering, int tick);
        /// @brief Déplace l'animal parce qu'un autre le pousse.
        ///
        /// Devant un obstacle, il glisse le long d'un axe, ou reste en place : il n'est jamais poussé hors des prairies. Sa direction ne change pas.
        /// @param world Carte à respecter.
        /// @param offset Déplacement voulu, en cases.
        /// @param tick Numéro du petit pas qui vient d'être joué.
        void Push(const World& world, Point offset, int tick);
        /// @brief Vieillit d'une génération et perd de l'énergie.
        void SpendGeneration();
        /// @brief Boit : l'hydratation revient à MaxHydration.
        void Drink();
        /// @brief Perd l'eau d'une génération (SpeciesTraits::waterLoss).
        void LoseWater();
        /// @brief Tue l'animal.
        void Die();
        /// @brief Vrai s'il est vivant, adulte, assez nourri et reposé depuis sa dernière portée.
        bool CanReproduce() const;
        /// @brief Vrai si les deux sont de la même espèce, de sexes différents et prêts à se reproduire.
        bool CanBreedWith(const Animal& other) const;
        /// @brief Forme un couple pour cette génération : chacun devient le partenaire de l'autre.
        void PairWith(Animal& other);
        /// @brief Dépense ReproductionCost et commence le temps de repos.
        void PayReproductionCost();
        /// @brief Crée un petit auprès de cet animal, sa mère.
        /// @param id Identifiant du nouveau-né.
        /// @param sex Sexe du nouveau-né.
        std::unique_ptr<Animal> CreateOffspring(std::uint64_t id, Sex sex) const;
        /// @brief Gagne de l'énergie en mangeant, sans dépasser MaxEnergy.
        void ReceiveMeal(int energy);

        /// @brief L'espèce de l'animal.
        virtual AnimalKind GetKind() const = 0;
        /// @brief Vrai pour une proie. Les proies jouent avant les chasseurs.
        virtual bool IsPrey() const;
        /// @brief Vrai si les animaux de cette espèce font partie de son régime ; faux par défaut.
        virtual bool Eats(AnimalKind kind) const;
        /// @brief Décisions prises une seule fois au début d'une génération.
        virtual void BeginGeneration(std::mt19937& rng);
        /// @brief Choisit la direction et l'allure du prochain petit pas.
        virtual Steering Steer(const Habitat& habitat, std::mt19937& rng) const = 0;
        /// @brief Agit après un petit pas : manger ou chasser.
        virtual void Act(Habitat& habitat, int tick) = 0;

    protected:
        /// @brief Crée un animal au centre d'une case.
        /// @throws std::invalid_argument si l'énergie n'est pas entre 1 et MaxEnergy.
        Animal(std::uint64_t id, Position cell, int energy, const SpeciesTraits& traits,
               const AnimalOptions& options);
        /// @brief La direction qui mène d'un point à un autre, en radians.
        static double HeadingTo(Point from, Point to);
        /// @brief La marche vers l'eau, si l'animal a soif et connaît un chemin.
        std::optional<Steering> SeekWater(const Habitat& habitat) const;
        /// @brief La direction vers un partenaire, si l'animal est prêt, sans couple, et qu'il en repère un.
        std::optional<Steering> SeekMate(const Habitat& habitat) const;
        /// @brief Un écart aléatoire entre -spread et +spread radians.
        static double Jitter(std::mt19937& rng, double spread);
        /// @brief Crée un petit de la même espèce.
        virtual std::unique_ptr<Animal> MakeOffspring(std::uint64_t id) const = 0;

    private:
        /// @brief Écrit la position actuelle dans le trajet, du petit pas donné jusqu'à la fin.
        void FollowPath(int tick);

        /// @brief Identifiant unique, attribué par la simulation.
        std::uint64_t id;
        /// @brief Identifiant de la mère ; vide pour un animal placé au départ.
        std::optional<std::uint64_t> parentId;
        /// @brief Sexe ; par défaut d'après l'identifiant, la simulation le choisit ensuite.
        Sex sex;
        /// @brief Partenaire de la génération en cours (référence non possédée).
        Animal* partner{nullptr};
        /// @brief Générations d'attente avant de pouvoir se reproduire à nouveau.
        int restLeft{0};
        /// @brief Position libre actuelle, en cases.
        Point location;
        /// @brief Direction actuelle, en radians.
        double heading;
        /// @brief Trajet de la génération en cours ou de la dernière calculée.
        Path path{};
        /// @brief Âge, en générations.
        int age{0};
        /// @brief Énergie restante ; l'animal meurt à 0.
        int energy;
        /// @brief Hydratation restante ; l'animal meurt de soif à 0.
        int hydration{MaxHydration};
        /// @brief Caractéristiques fixes de l'espèce.
        SpeciesTraits traits;
        /// @brief Réglages propres à l'individu, transmis à ses petits.
        AnimalOptions options;
    };
}
