/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_STRINGUTILS_H
#define PLUTOBOOK_STRINGUTILS_H

#include <array>
#include <string>
#include <cstdint>

namespace plutobook {

enum CharFlags : uint8_t {
    Space    = 1 << 0,
    Digit    = 1 << 1,
    Upper    = 1 << 2,
    Lower    = 1 << 3,
    HexUpper = 1 << 4,
    HexLower = 1 << 5
};

constexpr std::array<uint8_t, 256> createCharTable()
{
    std::array<uint8_t, 256> table{};

    table[' ']  |= Space;
    table['\n'] |= Space;
    table['\t'] |= Space;
    table['\r'] |= Space;
    table['\f'] |= Space;

    for(uint8_t c = '0'; c <= '9'; ++c)
        table[c] |= Digit;
    for(uint8_t c = 'A'; c <= 'Z'; ++c)
        table[c] |= Upper;
    for(uint8_t c = 'a'; c <= 'z'; ++c)
        table[c] |= Lower;
    for(uint8_t c = 'A'; c <= 'F'; ++c)
        table[c] |= HexUpper;
    for(uint8_t c = 'a'; c <= 'f'; ++c)
        table[c] |= HexLower;
    return table;
}

constexpr auto kCharTable = createCharTable();

constexpr bool isSpace(uint8_t c)
{
    return kCharTable[c] & Space;
}

constexpr bool isDigit(uint8_t c)
{
    return kCharTable[c] & Digit;
}

constexpr bool isUpper(uint8_t c)
{
    return kCharTable[c] & Upper;
}

constexpr bool isLower(uint8_t c)
{
    return kCharTable[c] & Lower;
}

constexpr bool isAlpha(uint8_t c)
{
    return kCharTable[c] & (Upper | Lower);
}

constexpr bool isAlnum(uint8_t c)
{
    return kCharTable[c] & (Digit | Upper | Lower);
}

constexpr bool isHexUpper(uint8_t c)
{
    return kCharTable[c] & HexUpper;
}

constexpr bool isHexLower(uint8_t c)
{
    return kCharTable[c] & HexLower;
}

constexpr bool isHexAlpha(uint8_t c)
{
    return kCharTable[c] & (HexUpper | HexLower);
}

constexpr bool isHexDigit(uint8_t c)
{
    return kCharTable[c] & (Digit | HexUpper | HexLower);
}

constexpr uint8_t toHexDigit(uint8_t cc)
{
    if(isDigit(cc))
        return cc - '0';
    if(isHexUpper(cc))
        return 10 + cc - 'A';
    if(isHexLower(cc))
        return 10 + cc - 'a';
    return 0;
}

constexpr uint8_t toHexByte(uint8_t a, uint8_t b)
{
    return toHexDigit(a) << 4 | toHexDigit(b);
}

extern const uint8_t kAsciiCaseFoldTable[256];

inline char toLower(uint8_t cc)
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

std::string toString(int value);
std::string toString(float value);
std::string toUtf8(uint32_t codepoint);

} // namespace plutobook

#endif // PLUTOBOOK_STRINGUTILS_H
