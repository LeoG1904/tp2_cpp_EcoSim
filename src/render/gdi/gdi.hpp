/**
 * @file
 * @brief Petites aides de dessin communes.
 */
#pragma once

// Petites aides GDI communes aux dessinateurs ; à inclure après <windows.h>.
namespace ecosim_gdi {
    /// @brief Remplit un rectangle d'une couleur.
    inline void FillArea(const HDC dc, const int x, const int y, const int width, const int height,
                         const COLORREF color) {
        const RECT rect{x, y, x + width, y + height};
        const HBRUSH brush = CreateSolidBrush(color);
        FillRect(dc, &rect, brush);
        DeleteObject(brush);
    }

    /// @brief Crée la police de l'interface. L'appelant doit la détruire.
    /// @param height Hauteur des caractères, en pixels.
    /// @param weight Épaisseur (normale ou grasse).
    inline HFONT UiFont(const int height, const int weight) {
        return CreateFontW(height, 0, 0, 0, weight, FALSE, FALSE, FALSE,
                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                           CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    }
}
