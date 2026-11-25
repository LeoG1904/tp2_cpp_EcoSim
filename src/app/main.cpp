// Assemblage : moteur, fenêtre Windows, entrées et rendu 2D.
import std;
import ecosim.application;
import ecosim.window;
import ecosim.win32_input;
import ecosim.render;

int main() {
    // Taille d'une case en pixels ; la carte s'adapte à la taille de l'écran
    constexpr int CellSize{40};

    // La fenêtre prend la taille de l'écran : on l'ouvre avant de dimensionner la carte
    ecosim::Win32Window window{"EcoSim"};
    ecosim::UiController controller{window.Width(), window.Height()};
    ecosim::Win32Input input{window, controller};
    ecosim::Renderer2D renderer{window, controller, CellSize};
    ecosim::Application application{input, renderer, window.Width() / CellSize,
                                    window.Height() / CellSize, std::random_device{}()};

    auto previousFrame = std::chrono::steady_clock::now();
    while (window.ProcessEvents()) {
        const auto now = std::chrono::steady_clock::now();
        const double elapsed = std::chrono::duration<double>(now - previousFrame).count();
        previousFrame = now;
        if (!application.RunFrame(elapsed)) break;
    }
    return 0;
}
