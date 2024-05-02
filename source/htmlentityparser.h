#ifndef PLUTOBOOK_HTMLENTITYPARSER_H
#define PLUTOBOOK_HTMLENTITYPARSER_H

#include <string>

namespace plutobook {

class HTMLEntityParser {
public:
    HTMLEntityParser(const std::string_view& input, std::string& output, bool inAttributeValue)
        : m_input(input), m_output(output), m_inAttributeValue(inAttributeValue)
    {}

    bool parse();

private:
    size_t offset() const { return m_offset; }
    bool handleNamed(char cc);
    bool handleNumber(char cc);
    bool handleDecimal(char cc);
    bool handleMaybeHex(char cc);
    bool handleHex(char cc);
    void append(uint32_t cp);

    char currentInputCharacter() const;
    char nextInputCharacter();

    std::string_view m_input;
    std::string& m_output;
    bool m_inAttributeValue;
    size_t m_offset{0};
    friend class HTMLTokenizer;
};

inline char HTMLEntityParser::currentInputCharacter() const
{
    if(m_offset < m_input.length())
        return m_input[m_offset];
    return 0;
}

inline char HTMLEntityParser::nextInputCharacter()
{
    m_offset += 1;
    if(m_offset < m_input.length())
        return m_input[m_offset];
    return 0;
}

} // namespace plutobook

#endif // PLUTOBOOK_HTMLENTITYPARSER_H
