/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_CSSTOKENIZER_H
#define PLUTOBOOK_CSSTOKENIZER_H

#include <list>
#include <vector>
#include <string>
#include <cassert>
#include <cstdint>

namespace plutobook {

class CSSToken {
public:
    enum class Type : uint8_t {
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
        UnicodeRange,
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

    explicit CSSToken(Type type) : m_type(type) {}
    CSSToken(Type type, uint32_t delim) : m_type(type), m_delim(delim) {}
    CSSToken(Type type, uint32_t from, uint32_t to) : m_type(type), m_from(from), m_to(to) {}
    CSSToken(Type type, const std::string_view& data) : m_type(type), m_data(data) {}
    CSSToken(Type type, HashType hashType, const std::string_view& data) : m_type(type), m_hashType(hashType), m_data(data) {}

    CSSToken(Type type, NumberType numberType, NumberSign numberSign, float number)
        : m_type(type), m_numberType(numberType), m_numberSign(numberSign), m_number(number)
    {}

    CSSToken(Type type, NumberType numberType, NumberSign numberSign, float number, const std::string_view& unit)
        : m_type(type), m_numberType(numberType), m_numberSign(numberSign), m_number(number), m_data(unit)
    {}

    Type type() const { return m_type; }
    HashType hashType() const { return m_hashType; }
    NumberType numberType() const { return m_numberType; }
    NumberSign numberSign() const { return m_numberSign; }
    uint32_t delim() const { return m_delim; }
    float number() const { return m_number; }
    int integer() const { return static_cast<int>(m_number); }
    uint32_t from() const { return m_from; }
    uint32_t to() const { return m_to; }
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

        return type;
    }

private:
    Type m_type;
    HashType m_hashType;
    NumberType m_numberType;
    NumberSign m_numberSign;
    union {
        uint32_t m_delim;
        float m_number;
    };

    uint32_t m_from;
    uint32_t m_to;
    std::string_view m_data;
    friend class CSSVariableData;
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

    const CSSToken& get() const { return m_begin < m_end ? *m_begin : eofToken; }

    const CSSToken& operator*() const { return get(); }
    const CSSToken* operator->() const { return &get(); }

    const CSSToken* begin() const { return m_begin; }
    const CSSToken* end() const { return m_end; }

    bool empty() const { return m_begin == m_end; }

private:
    static const CSSToken eofToken;
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
        : m_data(input.data()), m_length(input.length())
    {}

    char peek(size_t count = 0) const {
        auto index = m_offset + count;
        if(index < m_length)
            return m_data[index];
        assert(index == m_length);
        return char(0);
    }

    void advance(size_t count = 1) {
        assert(m_offset + count <= m_length);
        m_offset += count;
    }

    char consume() {
        auto index = ++m_offset;
        if(index < m_length)
            return m_data[index];
        assert(index == m_length);
        return char(0);
    }

    std::string_view substring(size_t offset, size_t count) const {
        assert(offset + count <= m_length);
        return std::string_view(m_data + offset, count);
    }

    const char& operator*() const {
        assert(m_offset < m_length);
        return m_data[m_offset];
    }

    const char* data() const { return m_data; }
    size_t length() const { return m_length; }
    size_t offset() const { return m_offset; }

    bool empty() const { return m_offset == m_length; }

private:
    const char* m_data;
    size_t m_length;
    size_t m_offset{0};
};

class CSSTokenizer {
public:
    explicit CSSTokenizer(const std::string_view& input);

    CSSTokenStream tokenize();

private:
    static bool isEscapeSequence(char first, char second);
    static bool isIdentSequence(char first, char second, char third);
    static bool isNumberSequence(char first, char second, char third);

    bool isEscapeSequence() const;
    bool isIdentSequence() const;
    bool isNumberSequence() const;
    bool isExponentSequence() const;
    bool isUnicodeRangeSequence() const;

    std::string_view addstring(std::string&& value);

    std::string_view consumeName();
    uint32_t consumeEscape();

    CSSToken consumeStringToken();
    CSSToken consumeNumericToken();
    CSSToken consumeUnicodeRangeToken();
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
