/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_STRINGUTILS_H
#define PLUTOBOOK_STRINGUTILS_H

#include <string>
#include <cstdio>
#include <cstdint>

namespace plutobook {

constexpr bool isSpace(uint8_t cc) { return (cc == ' ' || cc == '\n' || cc == '\t' || cc == '\r' || cc == '\f'); }
constexpr bool isDigit(uint8_t cc) { return (cc >= '0' && cc <= '9'); }
constexpr bool isUpper(uint8_t cc) { return (cc >= 'A' && cc <= 'Z'); }
constexpr bool isLower(uint8_t cc) { return (cc >= 'a' && cc <= 'z'); }
constexpr bool isAlpha(uint8_t cc) { return isUpper(cc) || isLower(cc); }
constexpr bool isAlnum(uint8_t cc) { return isDigit(cc) || isAlpha(cc); }

constexpr bool isHexUpper(uint8_t cc) { return (cc >= 'A' && cc <= 'F'); }
constexpr bool isHexLower(uint8_t cc) { return (cc >= 'a' && cc <= 'f'); }
constexpr bool isHexAlpha(uint8_t cc) { return isHexUpper(cc) || isHexLower(cc); }
constexpr bool isHexDigit(uint8_t cc) { return isDigit(cc) || isHexAlpha(cc); }

constexpr int toHexDigit(uint8_t cc)
{
    if(isDigit(cc))
        return cc - '0';
    if(isHexUpper(cc))
        return 10 + cc - 'A';
    if(isHexLower(cc))
        return 10 + cc - 'a';
    return 0;
}

constexpr int toHexByte(uint8_t a, uint8_t b)
{
    return toHexDigit(a) << 4 | toHexDigit(b);
}

constexpr uint8_t kAsciiCaseFoldTable[256] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
    0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};

constexpr char toLower(uint8_t cc)
{
    return kAsciiCaseFoldTable[cc];
}

constexpr bool equals(uint8_t a, uint8_t b, bool caseSensitive)
{
    return caseSensitive ? (a == b) : toLower(a) == toLower(b);
}

constexpr bool equals(const char* aData, size_t aLength, const char* bData, size_t bLength, bool caseSensitive)
{
    if(aLength != bLength)
        return false;
    for(size_t i = 0; i < aLength; ++i) {
        if(!equals(aData[i], bData[i], caseSensitive)) {
            return false;
        }
    }

    return true;
}

constexpr bool equals(std::string_view a, std::string_view b, bool caseSensitive)
{
    return equals(a.data(), a.length(), b.data(), b.length(), caseSensitive);
}

constexpr bool equalsIgnoringCase(std::string_view a, std::string_view b)
{
    return equals(a, b, false);
}

constexpr bool contains(std::string_view haystack, std::string_view needle, bool caseSensitive)
{
    if(needle.empty() || needle.length() > haystack.length())
        return false;
    const auto limit = haystack.length() - needle.length();
    for(size_t i = 0; i <= limit; ++i) {
        if(equals(haystack.substr(i, needle.length()), needle, caseSensitive)) {
            return true;
        }
    }

    return false;
}

constexpr bool includes(std::string_view haystack, std::string_view needle, bool caseSensitive)
{
    if(needle.empty() || needle.length() > haystack.length())
        return false;
    size_t begin = 0;
    while(true) {
        while(begin < haystack.length() && isSpace(haystack[begin]))
            ++begin;
        if(begin >= haystack.length())
            break;
        size_t end = begin + 1;
        while(end < haystack.length() && !isSpace(haystack[end]))
            ++end;
        if(equals(haystack.substr(begin, end - begin), needle, caseSensitive))
            return true;
        begin = end + 1;
    }

    return false;
}

constexpr bool startswith(std::string_view input, std::string_view prefix, bool caseSensitive)
{
    if(prefix.empty() || prefix.length() > input.length())
        return false;
    return equals(input.substr(0, prefix.length()), prefix, caseSensitive);
}

constexpr bool endswith(std::string_view input, std::string_view suffix, bool caseSensitive)
{
    if(suffix.empty() || suffix.length() > input.length())
        return false;
    return equals(input.substr(input.length() - suffix.length(), suffix.length()), suffix, caseSensitive);
}

constexpr bool dashequals(std::string_view input, std::string_view prefix, bool caseSensitive)
{
    if(startswith(input, prefix, caseSensitive))
        return (input.length() == prefix.length() || input.at(prefix.length()) == '-');
    return false;
}

constexpr void stripLeadingSpaces(std::string_view& input)
{
    while(!input.empty() && isSpace(input.front())) {
        input.remove_prefix(1);
    }
}

constexpr void stripTrailingSpaces(std::string_view& input)
{
    while(!input.empty() && isSpace(input.back())) {
        input.remove_suffix(1);
    }
}

constexpr void stripLeadingAndTrailingSpaces(std::string_view& input)
{
    stripLeadingSpaces(input);
    stripTrailingSpaces(input);
}

static std::string toString(int value)
{
    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "%d", value);
    return buffer;
}

static std::string toString(float value)
{
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%g", value);
    return buffer;
}

static void appendCodepoint(std::string& output, uint32_t cp)
{
    char c[5] = {0, 0, 0, 0, 0};
    if(cp < 0x80) {
        c[1] = 0;
        c[0] = cp;
    } else if(cp < 0x800) {
        c[2] = 0;
        c[1] = (cp & 0x3F) | 0x80;
        cp >>= 6;
        c[0] = cp | 0xC0;
    } else if(cp < 0x10000) {
        c[3] = 0;
        c[2] = (cp & 0x3F) | 0x80;
        cp >>= 6;
        c[1] = (cp & 0x3F) | 0x80;
        cp >>= 6;
        c[0] = cp | 0xE0;
    } else if(cp < 0x110000) {
        c[4] = 0;
        c[3] = (cp & 0x3F) | 0x80;
        cp >>= 6;
        c[2] = (cp & 0x3F) | 0x80;
        cp >>= 6;
        c[1] = (cp & 0x3F) | 0x80;
        cp >>= 6;
        c[0] = cp | 0xF0;
    }

    output.append(c);
}

} // namespace plutobook

#endif // PLUTOBOOK_STRINGUTILS_H
