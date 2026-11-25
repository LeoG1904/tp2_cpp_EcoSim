// Carte pendant la simulation : vue active et case choisie.
export module ecosim.map_navigator;

import std;
export import ecosim.map_view;

export namespace ecosim {
    // Une valeur par vue, dans l'ordre du bouton qui passe de l'une à l'autre.
    enum class MapViewKind { Flat, Orbit };
    inline constexpr std::size_t MapViewCount{2};

    class MapNavigator {
    public:
        MapNavigator(int width, int height, int cellSize);

        // La vue choisie et la caméra sont conservées d'une simulation à l'autre.
        MapViewKind ActiveKind() const;
        const MapView& ActiveView() const;
        const MapView& NextView() const;
        const OrbitCamera& Camera() const;

        std::optional<Position> SelectedCell() const;
        void ClearSelection();
        // Le panneau évite de recouvrir la case choisie.
        bool IsCellPanelOnLeft() const;

        // Souris maintenue et molette ; inactive quand la carte n'est pas affichée.
        void Control(const RawInput& input, bool active);
        void Click(PointerPosition mouse);

    private:
        MapViewKind NextKind() const;
        MapView& View(MapViewKind kind);
        const MapView& View(MapViewKind kind) const;

        int width;
        int height;
        FlatMapView flat;
        OrbitMapView orbit;
        MapViewKind active{MapViewKind::Flat};
        std::optional<Position> selectedCell;
    };
}
