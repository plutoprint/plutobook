/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "stringutils.h"

#include <unicode/utf8.h>
#include <cstdio>

namespace plutobook {

std::string toString(int value)
{
    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "%d", value);
    return buffer;
}

std::string toString(float value)
{
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%g", value);
    return buffer;
}

std::string toUtf8(uint32_t codepoint)
{
    size_t length = 0;
    char buffer[U8_MAX_LENGTH];
    U8_APPEND_UNSAFE(buffer, length, codepoint);
    return {buffer, length};
}

std::string ellipsize(std::string_view input, size_t maxLength)
{
    constexpr std::string_view ellipsis = "...";
    if(input.length() <= maxLength)
        return std::string(input);
    if(maxLength <= ellipsis.length()) {
        return std::string(input.substr(0, maxLength));
    }

    const size_t remainingLength = maxLength - ellipsis.length();
    const size_t leftLength = remainingLength - remainingLength / 2;
    const size_t rightLength = remainingLength / 2;

    std::string output;
    output.reserve(maxLength);
    output.append(input.substr(0, leftLength));
    output.append(ellipsis);
    output.append(input.substr(input.length() - rightLength));
    return output;
}

} // namespace plutobook
