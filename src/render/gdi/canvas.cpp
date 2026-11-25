module;

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

module ecosim.render;

import std;

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
