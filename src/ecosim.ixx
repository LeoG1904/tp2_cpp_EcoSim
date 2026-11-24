// Module racine d'EcoSim. La logique de simulation (étapes suivantes)
// sera ajoutée dans ce module ou dans des modules dédiés, sans dépendre de l'affichage.
export module ecosim;

// Import rapide de la bibliothèque standard
import std;

// Espace de nom ayant le même nom que le module, exporté en entier.
export namespace ecosim {

    // Version du moteur. Sert de premier test que le module se compile et se lie.
    constexpr std::string_view Version() { return "0.1.0"; }

}
