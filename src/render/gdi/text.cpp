module;

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

module ecosim.render;

import std;

namespace {
    // Le projet manipule des textes UTF-8 ; Windows attend des caractères larges.
    // C'est le seul endroit où la conversion est faite.
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
