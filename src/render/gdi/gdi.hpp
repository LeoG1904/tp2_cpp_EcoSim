#pragma once

// Petites aides GDI communes aux dessinateurs ; à inclure après <windows.h>.
namespace ecosim_gdi {
    inline void FillArea(const HDC dc, const int x, const int y, const int width, const int height,
                         const COLORREF color) {
        const RECT rect{x, y, x + width, y + height};
        const HBRUSH brush = CreateSolidBrush(color);
        FillRect(dc, &rect, brush);
        DeleteObject(brush);
    }

    inline HFONT UiFont(const int height, const int weight) {
        return CreateFontW(height, 0, 0, 0, weight, FALSE, FALSE, FALSE,
                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                           CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    }
}
