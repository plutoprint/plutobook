/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_ARGPARSER_H
#define PLUTOBOOK_ARGPARSER_H

namespace plutobook {

enum class ArgType {
    Flag,
    String,
    Int,
    Float,
    Length,
    Choice
};

using ArgFunc = bool(*)(void* closure, const char* value);

struct ArgDesc {
    const char* name;
    ArgType type;
    void* value;
    ArgFunc func = nullptr;
    const char* help = nullptr;
    bool required = false;
    bool positional = false;
};

void parseArgs(const char* program, const char* description, ArgDesc* args, int argc, char* argv[]);

struct ArgChoice {
    ArgChoice(const char* name, int value) : name(name), value(value) {}
    const char* name;
    int value;
};

template<typename Enum>
struct ArgEnum : public ArgChoice {
    ArgEnum(const char* name, Enum value)
        : ArgChoice(name, static_cast<int>(value))
    {}
};

bool parseArgChoices(void* closure, const char* name, const ArgChoice* choices, int nchoices);

} // namespace plutobook

#endif // PLUTOBOOK_ARGPARSER_H
