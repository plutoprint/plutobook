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

} // namespace plutobook
