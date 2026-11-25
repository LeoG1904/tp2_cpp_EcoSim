// Façons de regarder la carte. Une nouvelle vue dérive de MapView sans modifier les autres.
export module ecosim.map_view;

import std;
export import ecosim.input;
export import ecosim.camera;

export namespace ecosim {
    class MapView {
    public:
        virtual ~MapView() = default;

        // Texte du bouton qui mène à cette vue, et aide affichée quand elle est active.
        virtual std::string ButtonLabel() const = 0;
        virtual std::string Help() const = 0;
        virtual std::optional<Position> PickCell(PointerPosition mouse) const = 0;
        virtual bool IsOnRightHalf(Position cell) const = 0;

        // Réservé aux vues dont le point de vue se déplace.
        virtual void Control(const RawInput& input);
        virtual void StopControl();
        virtual bool CanRecenter() const;
        virtual void Recenter();
    };

    class FlatMapView final : public MapView {
    public:
        FlatMapView(int width, int height, int cellSize);

        std::string ButtonLabel() const override;
        std::string Help() const override;
        std::optional<Position> PickCell(PointerPosition mouse) const override;
        bool IsOnRightHalf(Position cell) const override;

    private:
        int width;
        int height;
        int cellSize;
    };

    class OrbitMapView final : public MapView {
    public:
        OrbitMapView(int width, int height, int cellSize);

        std::string ButtonLabel() const override;
        std::string Help() const override;
        std::optional<Position> PickCell(PointerPosition mouse) const override;
        bool IsOnRightHalf(Position cell) const override;
        void Control(const RawInput& input) override;
        void StopControl() override;
        bool CanRecenter() const override;
        void Recenter() override;

        const OrbitCamera& Camera() const;

    private:
        Viewport screen;
        OrbitCamera camera;
        std::optional<PointerPosition> dragOrigin;
    };
}
