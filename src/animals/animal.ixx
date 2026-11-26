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
        Herbivore, ///< Mange l'herbe et sert de proie.
        Predator ///< Chasse les herbivores.
    };
    /// @brief Nombre d'espèces.
    /// @ingroup moteur
    inline constexpr std::size_t SpeciesCount{2};

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

    /// @brief Les statistiques réglables d'une espèce.
    ///
    /// Les valeurs initiales sont celles des herbivores.
    /// @ingroup moteur
    struct AnimalTraits {
        /// @brief Énergie d'un animal placé au départ.
        int startingEnergy{50};
        /// @brief Énergie perdue à chaque génération.
        int energyLoss{2};
        /// @brief Âge auquel l'animal meurt.
        int maxAge{80};
        /// @brief Herbe mangée au maximum (herbivore) ou énergie par proie (prédateur).
        int foodEnergy{10};
        /// @brief Énergie minimale pour se reproduire.
        int reproductionEnergy{80};
        /// @brief Chance de naissance, en %, quand les conditions sont réunies.
        int reproductionPercent{20};
        /// @brief Instinct de l'espèce, 0 pour le désactiver.
        ///
        /// Proie : chance, en %, d'être sur ses gardes. Prédateur : esprit de meute, qui ajoute autant de cases à sa vue.
        int instinct{0};
        /// @brief Distance par génération à pleine allure, en % de case.
        int speed{100};

        /// @brief Vrai si les deux valeurs sont identiques.
        bool operator==(const AnimalTraits&) const = default;
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
        /// @brief La proie vivante la plus proche dans le rayon, s'il y en a une.
        virtual std::optional<Point> NearestPrey(Point from, double radius) const = 0;
        /// @brief Les positions des prédateurs vivants dans le rayon.
        /// @param from Centre de la recherche.
        /// @param radius Rayon, en cases.
        /// @param except Animal à ne pas compter.
        virtual std::vector<Point> HuntersNear(Point from, double radius,
                                               const Animal* except) const = 0;
        /// @brief Capture la proie la plus proche à portée.
        /// @return Vrai si une proie a été capturée.
        virtual bool CatchPrey(Point from, double reach) = 0;
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
        /// @brief Virage maximal par petit pas, en radians : les trajectoires restent courbes.
        static constexpr double MaxTurnPerTick{0.65};

        virtual ~Animal() = default;
        Animal(const Animal&) = delete;
        Animal& operator=(const Animal&) = delete;

        /// @brief Identifiant unique.
        std::uint64_t GetId() const;
        /// @brief Identifiant du parent ; vide pour un animal placé au départ.
        std::optional<std::uint64_t> GetParentId() const;
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
        /// @brief Statistiques de cet individu.
        const AnimalTraits& GetTraits() const;
        /// @brief Vrai s'il lui reste de l'énergie et qu'il n'a pas atteint l'âge maximum.
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
        /// @brief Vieillit d'une génération et perd de l'énergie.
        void SpendGeneration();
        /// @brief Tue l'animal.
        void Die();
        /// @brief Vrai s'il est vivant, âgé d'au moins 5 générations et assez nourri.
        bool CanReproduce() const;
        /// @brief Retire les 30 points d'énergie que coûte une naissance.
        void PayReproductionCost();
        /// @brief Crée un petit auprès de cet animal.
        /// @param id Identifiant du nouveau-né.
        std::unique_ptr<Animal> CreateOffspring(std::uint64_t id) const;
        /// @brief Gagne de l'énergie en mangeant, sans dépasser MaxEnergy.
        void ReceiveMeal(int energy);

        /// @brief L'espèce de l'animal.
        virtual AnimalKind GetKind() const = 0;
        /// @brief Vrai pour une proie. Les proies jouent avant les chasseurs.
        virtual bool IsPrey() const;
        /// @brief Décisions prises une seule fois au début d'une génération.
        virtual void BeginGeneration(std::mt19937& rng);
        /// @brief Choisit la direction et l'allure du prochain petit pas.
        virtual Steering Steer(const Habitat& habitat, std::mt19937& rng) const = 0;
        /// @brief Agit après un petit pas : manger ou chasser.
        virtual void Act(Habitat& habitat, int tick) = 0;

    protected:
        /// @brief Crée un animal au centre d'une case.
        /// @throws std::invalid_argument si l'énergie n'est pas entre 1 et MaxEnergy.
        Animal(std::uint64_t id, Position cell, int energy, const AnimalTraits& traits);
        /// @brief La direction qui mène d'un point à un autre, en radians.
        static double HeadingTo(Point from, Point to);
        /// @brief Un écart aléatoire entre -spread et +spread radians.
        static double Jitter(std::mt19937& rng, double spread);
        /// @brief Crée un petit de la même espèce.
        virtual std::unique_ptr<Animal> MakeOffspring(std::uint64_t id) const = 0;

    private:
        /// @brief Identifiant unique, attribué par la simulation.
        std::uint64_t id;
        /// @brief Identifiant du parent ; vide pour un animal placé au départ.
        std::optional<std::uint64_t> parentId;
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
        /// @brief Statistiques propres à l'individu, transmises à ses petits.
        AnimalTraits traits;
    };
}
