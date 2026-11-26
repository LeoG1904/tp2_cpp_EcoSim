/**
 * @file
 * @brief Le formulaire des réglages de l'accueil.
 */
export module ecosim.settings_form;

import std;
export import ecosim.settings;
export import ecosim.species_style;

export namespace ecosim {
    /// @brief Un réglage affiché : son nom, ses limites et la valeur qu'il modifie.
    /// @ingroup interface
    struct SettingField {
        /// @brief Colonne : 0 pour le terrain, puis une par espèce.
        std::size_t column;
        /// @brief Nom affiché.
        std::string label;
        /// @brief Plus petite valeur autorisée.
        int minimum;
        /// @brief Plus grande valeur autorisée.
        int maximum;
        /// @brief Donne accès à la valeur dans les réglages.
        std::function<int&(SimulationSettings&)> value;
    };

    /// @brief Titre d'une colonne.
    /// @ingroup interface
    std::string ColumnTitle(std::size_t column);
    /// @brief Couleur d'une colonne.
    /// @ingroup interface
    Color ColumnColor(std::size_t column);
    /// @brief Tous les réglages de l'accueil.
    /// @ingroup interface
    const std::vector<SettingField>& SettingFields();
    /// @brief Nom d'un préréglage.
    /// @ingroup interface
    std::string PresetName(Preset preset);
    /// @brief Description d'un préréglage.
    /// @ingroup interface
    std::string PresetDescription(Preset preset);

    /// @brief La saisie d'une valeur au clavier, appliquée seulement à la validation.
    /// @ingroup interface
    class SettingEditor {
    public:
        /// @brief Commence la saisie ; le premier chiffre remplacera la valeur actuelle.
        void Begin(std::size_t field, SimulationSettings& settings);
        /// @brief Ajoute un chiffre, trois au maximum.
        void Type(int digit);
        /// @brief Efface le dernier chiffre, ou toute la valeur sélectionnée.
        void Erase();
        /// @brief Applique la valeur, ramenée dans les limites ; une saisie vide garde l'ancienne.
        void Commit(SimulationSettings& settings);
        /// @brief Abandonne la saisie.
        void Cancel();

        /// @brief Réglage en cours de saisie, le cas échéant.
        std::optional<std::size_t> Field() const;
        /// @brief Texte saisi.
        const std::string& Text() const;
        /// @brief Vrai tant que la valeur entière est sélectionnée.
        bool IsSelected() const;

    private:
        /// @brief Réglage en cours de saisie ; vide s'il n'y en a pas.
        std::optional<std::size_t> field;
        /// @brief Chiffres tapés.
        std::string text;
        /// @brief Vrai tant que le premier chiffre doit remplacer la valeur.
        bool selected{false};
    };
}
