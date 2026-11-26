/**
 * @file
 * @brief Image en mémoire, dessins courants et conversion du texte UTF-8 pour Windows.
 */
module;

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

module ecosim.render;

import std;

namespace {
    /// @brief Convertit un texte UTF-8 en caractères larges pour Windows ; seul endroit où cette conversion est faite.
    std::wstring Widen(const std::string& text) {
        if (text.empty()) return {};
        const int bytes = static_cast<int>(text.size());
        const int length = MultiByteToWideChar(CP_UTF8, 0, text.data(), bytes, nullptr, 0);
        std::wstring result(static_cast<std::size_t>(length), L'\0');
        MultiByteToWideChar(CP_UTF8, 0, text.data(), bytes, result.data(), length);
        return result;
    }

    /// @brief Convertit une couleur du projet en couleur Windows.
    COLORREF ToColorRef(const ecosim::Color color) { return RGB(color.red, color.green, color.blue); }
}

namespace ecosim {
    Canvas::Canvas(void* window, const int width, const int height)
        : window{window}, width{width}, height{height},
          screen{GetDC(static_cast<HWND>(window))},
          buffer{CreateCompatibleDC(static_cast<HDC>(screen))},
          bitmap{CreateCompatibleBitmap(static_cast<HDC>(screen), width, height)},
          previous{SelectObject(static_cast<HDC>(buffer), static_cast<HBITMAP>(bitmap))} {
        SetBkMode(static_cast<HDC>(this->buffer), TRANSPARENT);
    }

    Canvas::~Canvas() {
        // La police d'origine doit être remise avant de détruire les nôtres.
        if (initialFont) SelectObject(static_cast<HDC>(buffer), static_cast<HGDIOBJ>(initialFont));
        for (void* font : fonts) DeleteObject(static_cast<HFONT>(font));
        SelectObject(static_cast<HDC>(buffer), static_cast<HGDIOBJ>(previous));
        DeleteObject(static_cast<HBITMAP>(bitmap));
        DeleteDC(static_cast<HDC>(buffer));
        ReleaseDC(static_cast<HWND>(window), static_cast<HDC>(screen));
    }

    void* Canvas::Context() const { return buffer; }
    int Canvas::Width() const { return width; }
    int Canvas::Height() const { return height; }

    void Canvas::Present() const {
        BitBlt(static_cast<HDC>(screen), 0, 0, width, height, static_cast<HDC>(buffer), 0, 0, SRCCOPY);
    }

    void Canvas::Fill(const UiRect area, const Color color) {
        const RECT rect{area.x, area.y, area.x + area.width, area.y + area.height};
        const HBRUSH brush = CreateSolidBrush(ToColorRef(color));
        FillRect(static_cast<HDC>(buffer), &rect, brush);
        DeleteObject(brush);
    }

    void Canvas::SetFont(const int size, const bool bold) {
        const HFONT font = CreateFontW(size, 0, 0, 0, bold ? FW_BOLD : FW_NORMAL, FALSE, FALSE, FALSE,
                                       DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                       CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
        fonts.push_back(font);
        const HGDIOBJ replaced = SelectObject(static_cast<HDC>(buffer), font);
        if (!initialFont) initialFont = replaced;
    }

    void Canvas::Write(const int x, const int y, const std::string& text, const Color color) {
        const std::wstring wide = Widen(text);
        SetTextColor(static_cast<HDC>(buffer), ToColorRef(color));
        TextOutW(static_cast<HDC>(buffer), x, y, wide.c_str(), static_cast<int>(wide.size()));
    }

    void Canvas::WriteCentered(const int centerX, const int y, const std::string& text,
                               const Color color) {
        Write(centerX - Measure(text).cx / 2, y, text, color);
    }

    TextSize Canvas::Measure(const std::string& text) const {
        const std::wstring wide = Widen(text);
        SIZE extent{};
        GetTextExtentPoint32W(static_cast<HDC>(buffer), wide.c_str(),
                              static_cast<int>(wide.size()), &extent);
        return {static_cast<int>(extent.cx), static_cast<int>(extent.cy)};
    }

    void Canvas::Button(const UiRect area, const std::string& label, const Color background,
                        const Color ink) {
        Fill(area, background);
        const TextSize size = Measure(label);
        Write(area.x + (area.width - size.cx) / 2, area.y + (area.height - size.cy) / 2, label, ink);
    }

    void Canvas::SideButton(const UiRect area, const std::string& label, const int textTop) {
        Fill(area, ButtonColor);
        Write(area.x + 20, area.y + textTop, label, WhiteText);
    }
}
