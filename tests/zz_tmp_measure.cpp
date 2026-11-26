#include <catch2/catch_all.hpp>
#include <cstdlib>
import std;
import ecosim.simulation;
using namespace ecosim;
namespace {
    void Apply(SimulationSettings& s, const std::string& spec) {
        std::istringstream in{spec};
        std::string item;
        while (in >> item) {
            const auto eq = item.find('=');
            const std::string key = item.substr(0, eq);
            const int v = std::stoi(item.substr(eq + 1));
            if (key == "growth") { s.vegetationGrowth = v; continue; }
            const std::string f = key.substr(2);
            SpeciesTraits& t = key[0] == 'd' ? Deer::Traits : key[0] == 'r' ? Rabbit::Traits
                             : key[0] == 'w' ? Wolf::Traits : Fox::Traits;
            if (f == "speed") { t.speed = v; continue; }
            if (f == "loss") { t.energyLoss = v; continue; }
            if (f == "age") { t.maxAge = v; continue; }
            if (f == "food") { t.foodEnergy = v; continue; }
            if (f == "repro") { t.reproductionEnergy = v; continue; }
            if (f == "meal") { t.mealPercent = v; continue; }
            if (f == "sight") { (key[0] == 'w' ? Wolf::Sight : Fox::Sight) = v / 10.0; continue; }
            if (f == "graze") { (key[0] == 'd' ? Deer::GrazingThreshold : Rabbit::GrazingThreshold) = v; continue; }
            const AnimalKind kind = key[0] == 'd' ? AnimalKind::Deer : key[0] == 'r' ? AnimalKind::Rabbit
                                  : key[0] == 'w' ? AnimalKind::Wolf : AnimalKind::Fox;
            if (f == "n") s.species[kind].initialCount = v;
            else if (f == "birth") s.species[kind].options.birthPercent = v;
            else if (f == "inst") s.species[kind].options.instinct = v;
            else throw std::runtime_error{key};
        }
    }
}
TEST_CASE("tmp measure", "[.m]") {
    const int preset = std::stoi(std::getenv("PRESET"));
    const int seeds = std::stoi(std::getenv("SEEDS"));
    const int first = std::getenv("FIRST") ? std::stoi(std::getenv("FIRST")) : 1;
    const int generations = std::stoi(std::getenv("GENS"));
    const char* spec = std::getenv("SPEC");
    long long duration = 0;
    int all = 0; std::array<int, SpeciesCount> alive{}; std::array<std::vector<int>, SpeciesCount> death; std::array<double, SpeciesCount> starved{}, eaten{}, deaths{}, mean{};
    for (int seed = first; seed < first + seeds; ++seed) {
        SimulationSettings s = preset < 0 ? DefaultSettings(42, 26) : PresetSettings(static_cast<Preset>(preset), 42, 26);
        if (spec) Apply(s, spec);
        Simulation sim = CreateRandomSimulation(42, 26, s, static_cast<std::uint32_t>(seed));
        std::array<int, SpeciesCount> gone{}; std::array<double, SpeciesCount> sum{};
        while (!sim.IsFinished() && sim.GetGeneration() < generations) {
            if (std::getenv("EARLY") && std::ranges::any_of(gone, [](int g) { return g > 0; })) break;
            sim.Step();
            for (std::size_t k = 0; k < SpeciesCount; ++k) {
                const auto n = sim.Count(static_cast<AnimalKind>(k));
                sum[k] += n;
                if (n == 0 && gone[k] == 0) gone[k] = sim.GetGeneration();
            }
        }
        bool every = true;
        int firstGone = generations;
        for (int g : gone) if (g > 0) firstGone = std::min(firstGone, g);
        duration += firstGone;
        for (std::size_t k = 0; k < SpeciesCount; ++k) {
            const auto kind = static_cast<AnimalKind>(k);
            if (gone[k] == 0 && sim.Count(kind) > 0) ++alive[k]; else { every = false; death[k].push_back(gone[k]); }
            const LifeEvents& e = sim.Stats().Total(kind);
            starved[k] += e.starved; eaten[k] += e.eaten; deaths[k] += e.Deaths();
            mean[k] += sum[k] / std::max(1, sim.GetGeneration());
        }
        all += every;
    }
    std::cout << "RES " << (spec ? spec : "") << " | all=" << all << "/" << seeds << " duree=" << duration / seeds;
    const char* names[]{"cerf", "lapin", "loup", "renard"};
    for (std::size_t k = 0; k < SpeciesCount; ++k) {
        std::sort(death[k].begin(), death[k].end());
        std::cout << " | " << names[k] << " vivant=" << alive[k] << " extinction_med=" << (death[k].empty() ? 0 : death[k][death[k].size() / 2])
                  << " faim=" << int(100 * starved[k] / std::max(1.0, deaths[k])) << "% mange=" << int(100 * eaten[k] / std::max(1.0, deaths[k]))
                  << "% moy=" << int(mean[k] / seeds);
    }
    std::cout << std::endl;
}
