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
    const char* help;
    ArgFunc func = nullptr;
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
