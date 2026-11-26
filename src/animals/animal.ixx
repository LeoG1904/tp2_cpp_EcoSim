// Données et comportement communs aux animaux, indépendants du rendu.
export module ecosim.animal;

import std;
export import ecosim.world;

export namespace ecosim {
    enum class AnimalKind { Herbivore, Predator };
    inline constexpr std::size_t SpeciesCount{2};

    // Une valeur pour chaque espèce, désignée par son nom plutôt que par un indice.
    template <typename T>
    struct PerSpecies {
        std::array<T, SpeciesCount> values{};

        T& operator[](const AnimalKind kind) { return values.at(static_cast<std::size_t>(kind)); }
        const T& operator[](const AnimalKind kind) const {
            return values.at(static_cast<std::size_t>(kind));
        }
        bool operator==(const PerSpecies&) const = default;
    };

    // Une génération est découpée en petits pas : les animaux y ajustent leur direction.
    inline constexpr int TicksPerGeneration{10};
    // Trajet de la dernière génération : le point de départ, puis un point par petit pas.
    using Path = std::array<Point, TicksPerGeneration + 1>;

    // Direction voulue (radians, 0 vers l'est, sens des y croissants) et allure de 0 à 1.
    struct Steering {
        double heading{0.0};
        double pace{0.0};
    };

    // Statistiques réglables d'une espèce. Les valeurs initiales sont celles des herbivores.
    struct AnimalTraits {
        int startingEnergy{50};
        int energyLoss{2};            // énergie perdue à chaque génération
        int maxAge{80};
        int foodEnergy{10};           // végétation mangée au maximum, ou énergie par proie
        int reproductionEnergy{80};   // énergie minimale pour se reproduire
        int reproductionPercent{20};  // chance de naissance quand les conditions sont réunies
        // Instinct propre à l'espèce, 0 pour le désactiver. Proie : chance (%) de fuir un
        // chasseur repéré. Chasseur : esprit de meute, qui ajoute autant de cases à sa vue.
        int instinct{0};
        int speed{100};               // distance par génération à pleine allure, en % de case

        bool operator==(const AnimalTraits&) const = default;
    };

    class Animal;

    // Ce qu'un animal peut percevoir et modifier, sans connaître la simulation.
    // Les obstacles ne bloquent pas la vue.
    class Habitat {
    public:
        virtual ~Habitat() = default;
        virtual World& GetWorld() = 0;
        virtual const World& GetWorld() const = 0;
        // Proie vivante la plus proche dans le rayon.
        virtual std::optional<Point> NearestPrey(Point from, double radius) const = 0;
        // Positions des chasseurs vivants dans le rayon, sans compter except.
        virtual std::vector<Point> HuntersNear(Point from, double radius,
                                               const Animal* except) const = 0;
        // Capture la proie la plus proche à portée, s'il y en a une.
        virtual bool CatchPrey(Point from, double reach) = 0;
        // Partage un repas avec les congénères proches ; renvoie la part du chasseur.
        virtual int ShareMeal(const Animal& hunter, int energy) = 0;
    };

    class Animal {
    public:
        static constexpr int MaxEnergy{100};
        // Virage maximal à chaque petit pas : les trajectoires restent courbes.
        static constexpr double MaxTurnPerTick{0.65};

        virtual ~Animal() = default;
        Animal(const Animal&) = delete;
        Animal& operator=(const Animal&) = delete;

        std::uint64_t GetId() const;
        std::optional<std::uint64_t> GetParentId() const;
        Point GetLocation() const;
        // Case située sous l'animal.
        Position GetPosition() const;
        double GetHeading() const;
        const Path& GetPath() const;
        int GetAge() const;
        int GetEnergy() const;
        const AnimalTraits& GetTraits() const;
        bool IsAlive() const;

        // Installe l'animal ailleurs, sans trajet.
        void Place(Point point, double direction);
        // Efface le trajet de la génération précédente.
        void StartPath();
        // Un petit pas : tourne vers la direction voulue, avance et contourne les obstacles.
        void Advance(const World& world, Steering steering, int tick);
        void SpendGeneration();
        void Die();
        bool CanReproduce() const;
        void PayReproductionCost();
        std::unique_ptr<Animal> CreateOffspring(std::uint64_t id) const;
        // Gagne de l'énergie en mangeant, sans dépasser le maximum.
        void ReceiveMeal(int energy);

        // Comportement propre à chaque espèce.
        virtual AnimalKind GetKind() const = 0;
        // Les proies jouent avant les autres espèces, qui les chassent.
        virtual bool IsPrey() const;
        // Décisions prises une fois par génération.
        virtual void BeginGeneration(std::mt19937& rng);
        virtual Steering Steer(const Habitat& habitat, std::mt19937& rng) const = 0;
        // Après chaque petit pas : manger, chasser.
        virtual void Act(Habitat& habitat, int tick) = 0;

    protected:
        Animal(std::uint64_t id, Position cell, int energy, const AnimalTraits& traits);
        static double HeadingTo(Point from, Point to);
        // Écart aléatoire entre -spread et +spread radians.
        static double Jitter(std::mt19937& rng, double spread);
        virtual std::unique_ptr<Animal> MakeOffspring(std::uint64_t id) const = 0;

    private:
        std::uint64_t id;
        std::optional<std::uint64_t> parentId;
        Point location;
        double heading;
        Path path{};
        int age{0};
        int energy;
        AnimalTraits traits;
    };
}
