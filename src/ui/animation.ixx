// Aides à l'animation des déplacements, sans API graphique.
export module ecosim.animation;

import std;
export import ecosim.simulation;

export namespace ecosim {
    // Fait glisser les animaux jusqu'à l'arrivée après un pas avancé pendant la pause.
    class ManualStepAnimation {
    public:
        // Un pas manuel est animé en un quart de seconde.
        static constexpr double StepsPerSecond{4.0};

        void Start(SimulationClock& clock);
        void Cancel();
        void Update(SimulationClock& clock, double realSeconds);

    private:
        bool active{false};
    };

    // À l'écran, un nouveau-né accompagne le dernier déplacement de son parent.
    void AttachNewbornsToParents(std::vector<AnimalSnapshot>& animals);
}
