// Entrée publique : réexporte les domaines du projet.
export module ecosim;

import std;
export import ecosim.world;
export import ecosim.animal;
export import ecosim.herbivore;
export import ecosim.predator;
export import ecosim.simulation;
export import ecosim.render;

export namespace ecosim {
    constexpr std::string_view Version() { return "0.1.0"; }
}
