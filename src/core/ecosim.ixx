// Entrée publique du moteur : aucun module d'interface ou d'affichage.
export module ecosim;

import std;
export import ecosim.world;
export import ecosim.animal;
export import ecosim.herbivore;
export import ecosim.predator;
export import ecosim.clock;
export import ecosim.settings;
export import ecosim.simulation;

export namespace ecosim {
    inline std::string Version() { return "0.1.0"; }
}
