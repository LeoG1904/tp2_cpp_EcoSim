module;

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "layout.hpp"

module ecosim.render;

import std;

using namespace ecosim_render_detail;

namespace ecosim {
    void Renderer2D::DrawHome() const {
        const auto hwnd = static_cast<HWND>(window);
        const HDC screen = GetDC(hwnd);
        const HDC buffer = CreateCompatibleDC(screen);
        const HBITMAP bitmap = CreateCompatibleBitmap(screen, windowWidth, windowHeight);
        const HGDIOBJ previous = SelectObject(buffer, bitmap);
        const RECT bounds{0, 0, windowWidth, windowHeight};
        const HBRUSH background = CreateSolidBrush(RGB(19, 28, 34));
        FillRect(buffer, &bounds, background);
        DeleteObject(background);

        const HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(30, 42, 46));
        const HGDIOBJ previousPen = SelectObject(buffer, gridPen);
        for (int x = 0; x < windowWidth; x += cellSize) {
            MoveToEx(buffer, x, 0, nullptr);
            LineTo(buffer, x, windowHeight);
        }
        for (int y = 0; y < windowHeight; y += cellSize) {
            MoveToEx(buffer, 0, y, nullptr);
            LineTo(buffer, windowWidth, y);
        }
        SelectObject(buffer, previousPen);
        DeleteObject(gridPen);

        const int centerX = windowWidth / 2;
        const int centerY = windowHeight / 2;
        const int panelWidth = std::min(680, windowWidth - 40);
        const RECT panel{centerX - panelWidth / 2, centerY - 195,
                         centerX + panelWidth / 2, centerY + 195};
        const HBRUSH panelBrush = CreateSolidBrush(RGB(32, 45, 53));
        FillRect(buffer, &panel, panelBrush);
        DeleteObject(panelBrush);

        SetBkMode(buffer, TRANSPARENT);
        const HFONT titleFont = CreateFontW(72, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                            CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
        const HFONT bodyFont = CreateFontW(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                           CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
        const HFONT buttonFont = CreateFontW(28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                             CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
        const HGDIOBJ previousFont = SelectObject(buffer, titleFont);
        const auto writeCentered = [&](const std::wstring& value, const int y) {
            SIZE extent{};
            GetTextExtentPoint32W(buffer, value.c_str(), static_cast<int>(value.size()), &extent);
            TextOutW(buffer, centerX - extent.cx / 2, y, value.c_str(), static_cast<int>(value.size()));
        };
        SetTextColor(buffer, RGB(239, 246, 242));
        writeCentered(L"EcoSim", centerY - 155);
        SelectObject(buffer, bodyFont);
        SetTextColor(buffer, RGB(177, 203, 198));
        writeCentered(L"Un écosystème qui évolue génération après génération", centerY - 65);

        const HBRUSH herbivoreBrush = CreateSolidBrush(RGB(245, 210, 70));
        const HBRUSH predatorBrush = CreateSolidBrush(RGB(220, 85, 75));
        const HGDIOBJ previousBrush = SelectObject(buffer, herbivoreBrush);
        const HGDIOBJ oldPen = SelectObject(buffer, GetStockObject(NULL_PEN));
        Ellipse(buffer, centerX - 173, centerY + 6, centerX - 155, centerY + 24);
        SelectObject(buffer, predatorBrush);
        Ellipse(buffer, centerX + 25, centerY + 6, centerX + 43, centerY + 24);
        SelectObject(buffer, oldPen);
        SelectObject(buffer, previousBrush);
        DeleteObject(herbivoreBrush);
        DeleteObject(predatorBrush);
        SetTextColor(buffer, RGB(239, 246, 242));
        TextOutW(buffer, centerX - 145, centerY + 3, L"Herbivores", 10);
        TextOutW(buffer, centerX + 53, centerY + 3, L"Prédateurs", 9);

        const UiRect start = StartButton(windowWidth, windowHeight);
        const RECT startRect{start.x, start.y, start.x + start.width, start.y + start.height};
        const HBRUSH startBrush = CreateSolidBrush(RGB(62, 148, 105));
        FillRect(buffer, &startRect, startBrush);
        DeleteObject(startBrush);
        SelectObject(buffer, buttonFont);
        writeCentered(L"Démarrer", start.y + 11);
        SelectObject(buffer, bodyFont);
        SetTextColor(buffer, RGB(177, 203, 198));
        writeCentered(L"Vitesse : Tab ou touches 1 à 4   •   Courbes en jeu", centerY + 152);
        writeCentered(L"Échap : quitter", windowHeight - 37);

        SelectObject(buffer, previousFont);
        DeleteObject(titleFont);
        DeleteObject(bodyFont);
        DeleteObject(buttonFont);
        BitBlt(screen, 0, 0, windowWidth, windowHeight, buffer, 0, 0, SRCCOPY);
        SelectObject(buffer, previous);
        DeleteObject(bitmap);
        DeleteDC(buffer);
        ReleaseDC(hwnd, screen);
        Sleep(16);
    }
}
