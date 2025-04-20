#include <codecvt>
#include <locale>
#include "websocket_util.h"

bool is_utf8(const std::string& str) {
    try {
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
        std::u16string utf16 = converter.from_bytes(str);
        return true;
    } catch (...) {
        return false;
    }
}