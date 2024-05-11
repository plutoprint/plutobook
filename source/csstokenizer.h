#ifndef PLUTOBOOK_CSSTOKENIZER_H
#define PLUTOBOOK_CSSTOKENIZER_H

#include <vector>
#include <list>
#include <string>
#include <cassert>
#include <cstdint>

namespace plutobook {

class CSSToken {
public:
    enum class Type : uint8_t {
        Unknown,
        Ident,
        Function,
        AtKeyword,
        Hash,
        String,
        BadString,
        Url,
        BadUrl,
        Delim,
        Number,
        Percentage,
        Dimension,
        Whitespace,
        Comment,
        CDO,
        CDC,
        Colon,
        Semicolon,
        Comma,
        LeftParenthesis,
        RightParenthesis,
        LeftSquareBracket,
        RightSquareBracket,
        LeftCurlyBracket,
        RightCurlyBracket,
        EndOfFile
    };

    enum class HashType : uint8_t {
        Identifier,
        Unrestricted
    };

    enum class NumberType : uint8_t {
        Integer,
        Number
    };

    enum class NumberSign : uint8_t {
        None,
        Plus,
        Minus
    };

    CSSToken() = default;
    explicit CSSToken(Type type) : m_type(type) {}
    CSSToken(Type type, uint32_t delim) : m_type(type), m_delim(delim) {}
    CSSToken(Type type, const std::string_view& data) : m_type(type), m_data(data) {}
    CSSToken(Type type, HashType hashType, const std::string_view& data) : m_type(type), m_hashType(hashType), m_data(data) {}

    CSSToken(Type type, NumberType numberType, NumberSign numberSign, double number)
        : m_type(type), m_numberType(numberType), m_numberSign(numberSign), m_number(number)
    {}

    CSSToken(Type type, NumberType numberType, NumberSign numberSign, double number, const std::string_view& unit)
        : m_type(type), m_numberType(numberType), m_numberSign(numberSign), m_number(number), m_data(unit)
    {}

    Type type() const { return m_type; }
    HashType hashType() const { return m_hashType; }
    NumberType numberType() const { return m_numberType; }
    NumberSign numberSign() const { return m_numberSign; }
    uint32_t delim() const { return m_delim; }
    double number() const { return m_number; }
    int integer() const { return static_cast<int>(m_number); }
    const std::string_view& data() const { return m_data; }

    static Type closeType(Type type) {
        switch(type) {
        case Type::Function:
        case Type::LeftParenthesis:
            return Type::RightParenthesis;
        case Type::LeftSquareBracket:
            return Type::RightSquareBracket;
        case Type::LeftCurlyBracket:
            return Type::RightCurlyBracket;
        default:
            assert(false);
        }

        return Type::Unknown;
    }

private:
    Type m_type{Type::Unknown};
    HashType m_hashType{HashType::Identifier};
    NumberType m_numberType{NumberType::Integer};
    NumberSign m_numberSign{NumberSign::None};
    uint32_t m_delim{0};
    double m_number{0};
    std::string_view m_data;
};

using CSSTokenList = std::vector<CSSToken>;

class CSSTokenStream {
public:
    CSSTokenStream(const CSSToken* data, size_t size)
        : CSSTokenStream(data, data + size)
    {}

    CSSTokenStream(const CSSToken* begin, const CSSToken* end)
        : m_begin(begin), m_end(end)
    {}

    const CSSToken& get() const {
        static const CSSToken eofToken(CSSToken::Type::EndOfFile);
        if(m_begin == m_end)
            return eofToken;
        return *m_begin;
    }

    void consume() {
        assert(m_begin < m_end);
        m_begin += 1;
    }

    void consumeWhitespace() {
        while(m_begin < m_end && m_begin->type() == CSSToken::Type::Whitespace) {
            m_begin += 1;
        }
    }

    void consumeIncludingWhitespace() {
        assert(m_begin < m_end);
        do {
            m_begin += 1;
        } while(m_begin < m_end && m_begin->type() == CSSToken::Type::Whitespace);
    }

    bool consumeCommaIncludingWhitespace() {
        if(m_begin < m_end && m_begin->type() == CSSToken::Type::Comma) {
            consumeIncludingWhitespace();
            return true;
        }

        return false;
    }

    void consumeComponent() {
        assert(m_begin < m_end);
        switch(m_begin->type()) {
        case CSSToken::Type::Function:
        case CSSToken::Type::LeftParenthesis:
        case CSSToken::Type::LeftSquareBracket:
        case CSSToken::Type::LeftCurlyBracket: {
            auto closeType = CSSToken::closeType(m_begin->type());
            m_begin += 1;
            while(m_begin < m_end && m_begin->type() != closeType)
                consumeComponent();
            if(m_begin < m_end)
                m_begin += 1;
            break;
        }

        default:
            m_begin += 1;
            break;
        }
    }

    CSSTokenStream consumeBlock() {
        assert(m_begin < m_end);
        auto closeType = CSSToken::closeType(m_begin->type());
        m_begin += 1;
        auto blockBegin = m_begin;
        while(m_begin < m_end && m_begin->type() != closeType)
            consumeComponent();
        auto blockEnd = m_begin;
        if(m_begin < m_end)
            m_begin += 1;
        return CSSTokenStream(blockBegin, blockEnd);
    }

    const CSSToken& operator*() const { return get(); }
    const CSSToken* operator->() const { return &get(); }

    const CSSToken* begin() const { return m_begin; }
    const CSSToken* end() const { return m_end; }

    bool empty() const { return m_begin == m_end; }

private:
    const CSSToken* m_begin;
    const CSSToken* m_end;
};

class CSSTokenStreamGuard {
public:
    CSSTokenStreamGuard(CSSTokenStream& input)
        : m_input(input), m_state(input)
    {}

    void release() { m_state = m_input; }

    ~CSSTokenStreamGuard() { m_input = m_state; }

private:
    CSSTokenStream& m_input;
    CSSTokenStream m_state;
};

class CSSTokenizerInputStream {
public:
    explicit CSSTokenizerInputStream(const std::string_view& input)
        : m_data(input)
    {}

    char peek(size_t count = 0) const {
        auto index = count + m_offset;
        assert(index <= m_data.size());
        if(index == m_data.size())
            return 0;
        return m_data[index];
    }

    char advance(size_t count = 1) {
        m_offset += count;
        assert(m_offset <= m_data.size());
        if(m_offset == m_data.size())
            return 0;
        return m_data[m_offset];
    }

    bool empty() const { return m_offset >= m_data.length(); }
    const char& operator*() const { return m_data.at(m_offset); }
    const std::string_view& data() const { return m_data; }
    size_t offset() const { return m_offset; }

private:
    std::string_view m_data;
    size_t m_offset{0};
};

class CSSTokenizer {
public:
    explicit CSSTokenizer(const std::string_view& input)
        : m_input(input)
    {}

    CSSTokenStream tokenize();

private:
    static bool isEscapeSequence(char first, char second);
    static bool isIdentSequence(char first, char second, char third);
    static bool isNumberSequence(char first, char second, char third);

    bool isEscapeSequence() const;
    bool isIdentSequence() const;
    bool isNumberSequence() const;
    bool isExponentSequence() const;

    std::string_view substring(size_t offset, size_t count);
    std::string_view addstring(std::string&& value);

    std::string_view consumeName();
    uint32_t consumeEscape();

    CSSToken consumeStringToken();
    CSSToken consumeNumericToken();
    CSSToken consumeIdentLikeToken();
    CSSToken consumeUrlToken();
    CSSToken consumeBadUrlRemnants();
    CSSToken consumeWhitespaceToken();
    CSSToken consumeCommentToken();
    CSSToken consumeSolidusToken();
    CSSToken consumeHashToken();
    CSSToken consumePlusSignToken();
    CSSToken consumeHyphenMinusToken();
    CSSToken consumeFullStopToken();
    CSSToken consumeLessThanSignToken();
    CSSToken consumeCommercialAtToken();
    CSSToken consumeReverseSolidusToken();

    CSSToken nextToken();

    using StringList = std::list<std::string>;
    CSSTokenizerInputStream m_input;
    CSSTokenList m_tokenList;
    StringList m_stringList;
};

} // namespace plutobook

#endif // PLUTOBOOK_CSSTOKENIZER_H
