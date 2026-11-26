/**
 * @file
 * @brief Image en mémoire et conversion du texte UTF-8 pour Windows.
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
}

namespace ecosim {
    void WriteText(void* context, const int x, const int y, const std::string& text) {
        const std::wstring wide = Widen(text);
        TextOutW(static_cast<HDC>(context), x, y, wide.c_str(), static_cast<int>(wide.size()));
    }

    TextSize MeasureText(void* context, const std::string& text) {
        const std::wstring wide = Widen(text);
        SIZE extent{};
        GetTextExtentPoint32W(static_cast<HDC>(context), wide.c_str(),
                              static_cast<int>(wide.size()), &extent);
        return {static_cast<int>(extent.cx), static_cast<int>(extent.cy)};
    }
}

namespace ecosim {
    Canvas::Canvas(void* window, const int width, const int height)
        : window{window}, width{width}, height{height},
          screen{GetDC(static_cast<HWND>(window))},
          buffer{CreateCompatibleDC(static_cast<HDC>(screen))},
          bitmap{CreateCompatibleBitmap(static_cast<HDC>(screen), width, height)},
          previous{SelectObject(static_cast<HDC>(buffer), static_cast<HBITMAP>(bitmap))} {}

    Canvas::~Canvas() {
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
}
