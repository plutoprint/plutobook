/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_HTMLTOKENIZER_H
#define PLUTOBOOK_HTMLTOKENIZER_H

#include "document.h"

#include <span>

namespace plutobook {

class HTMLToken {
public:
    enum class Type : uint8_t {
        Unknown,
        DOCTYPE,
        StartTag,
        EndTag,
        Comment,
        Character,
        SpaceCharacter,
        EndOfFile
    };

    explicit HTMLToken(Heap* heap)
        : m_heap(heap)
    {}

    Type type() const { return m_type; }
    bool selfClosing() const { return m_selfClosing; }
    bool forceQuirks() const { return m_forceQuirks; }
    bool hasPublicIdentifier() const { return m_hasPublicIdentifier; }
    bool hasSystemIdentifier() const { return m_hasSystemIdentifier; }
    const std::string& publicIdentifier() const { return m_publicIdentifier; }
    const std::string& systemIdentifier() const { return m_systemIdentifier; }
    const std::string& data() const { return m_data; }

    const std::vector<Attribute>& attributes() const { return m_attributes; }
    std::vector<Attribute>& attributes() { return m_attributes; }

    void beginStartTag() {
        assert(m_type == Type::Unknown);
        m_type = Type::StartTag;
        m_selfClosing = false;
        m_attributes.clear();
        m_data.clear();
    }

    void beginEndTag() {
        assert(m_type == Type::Unknown);
        m_type = Type::EndTag;
        m_selfClosing = false;
        m_attributes.clear();
        m_data.clear();
    }

    void setSelfClosing() {
        assert(m_type == Type::StartTag || m_type == Type::EndTag);
        m_selfClosing = true;
    }

    void addToTagName(char cc) {
        assert(m_type == Type::StartTag || m_type == Type::EndTag);
        m_data += cc;
    }

    void beginAttribute() {
        assert(m_type == Type::StartTag || m_type == Type::EndTag);
        m_attributeName.clear();
        m_attributeValue.clear();
    }

    void addToAttributeName(char cc) {
        assert(m_type == Type::StartTag || m_type == Type::EndTag);
        m_attributeName += cc;
    }

    void addToAttributeValue(char cc) {
        assert(m_type == Type::StartTag || m_type == Type::EndTag);
        m_attributeValue += cc;
    }

    void addToAttributeValue(const std::string& data) {
        assert(m_type == Type::StartTag || m_type == Type::EndTag);
        m_attributeValue += data;
    }

    void endAttribute() {
        assert(m_type == Type::StartTag || m_type == Type::EndTag);
        auto name = GlobalString(m_attributeName);
        auto value = m_heap->createString(m_attributeValue);
        m_attributes.emplace_back(name, value);
    }

    void beginComment() {
        assert(m_type == Type::Unknown);
        m_type = Type::Comment;
        m_data.clear();
    }

    void addToComment(char cc) {
        assert(m_type == Type::Comment);
        m_data += cc;
    }

    void beginCharacter() {
        assert(m_type == Type::Unknown);
        m_type = Type::Character;
        m_data.clear();
    }

    void addToCharacter(char cc) {
        assert(m_type == Type::Character);
        m_data += cc;
    }

    void addToCharacter(const std::string& data) {
        assert(m_type == Type::Character);
        m_data += data;
    }

    void beginSpaceCharacter() {
        assert(m_type == Type::Unknown);
        m_type = Type::SpaceCharacter;
        m_data.clear();
    }

    void addToSpaceCharacter(char cc) {
        assert(m_type == Type::SpaceCharacter);
        m_data += cc;
    }

    void beginDOCTYPE() {
        assert(m_type == Type::Unknown);
        m_type = Type::DOCTYPE;
        m_forceQuirks = false;
        m_hasPublicIdentifier = false;
        m_hasSystemIdentifier = false;
        m_publicIdentifier.clear();
        m_systemIdentifier.clear();
        m_data.clear();
    }

    void setForceQuirks() {
        assert(m_type == Type::DOCTYPE);
        m_forceQuirks = true;
    }

    void addToDOCTYPEName(char cc) {
        assert(m_type == Type::DOCTYPE);
        m_data += cc;
    }

    void setPublicIdentifier() {
        assert(m_type == Type::DOCTYPE);
        m_hasPublicIdentifier = true;
        m_publicIdentifier.clear();
    }

    void setSystemIdentifier() {
        assert(m_type == Type::DOCTYPE);
        m_hasSystemIdentifier = true;
        m_systemIdentifier.clear();
    }

    void addToPublicIdentifier(char cc) {
        assert(m_type == Type::DOCTYPE);
        m_publicIdentifier += cc;
    }

    void addToSystemIdentifier(char cc) {
        assert(m_type == Type::DOCTYPE);
        m_systemIdentifier += cc;
    }

    void setEndOfFile() {
        m_type = Type::EndOfFile;
        m_data.clear();
    }

    void reset() {
        m_type = Type::Unknown;
        m_data.clear();
    }

private:
    Heap* m_heap;
    Type m_type{Type::Unknown};
    bool m_selfClosing{false};
    bool m_forceQuirks{false};
    bool m_hasPublicIdentifier{false};
    bool m_hasSystemIdentifier{false};
    std::string m_publicIdentifier;
    std::string m_systemIdentifier;
    std::string m_attributeName;
    std::string m_attributeValue;
    std::vector<Attribute> m_attributes;
    std::string m_data;
};

class HTMLTokenView {
public:
    HTMLTokenView(HTMLToken& token)
        : m_type(token.type())
    {
        switch(m_type) {
        case HTMLToken::Type::DOCTYPE:
            m_forceQuirks = token.forceQuirks();
            m_hasPublicIdentifier = token.hasPublicIdentifier();
            m_hasSystemIdentifier = token.hasSystemIdentifier();
            m_publicIdentifier = token.publicIdentifier();
            m_systemIdentifier = token.systemIdentifier();
            m_data = token.data();
        case HTMLToken::Type::StartTag:
        case HTMLToken::Type::EndTag:
            m_selfClosing = token.selfClosing();
            m_tagName = GlobalString(token.data());
            m_attributes = token.attributes();
            break;
        case HTMLToken::Type::Comment:
        case HTMLToken::Type::Character:
        case HTMLToken::Type::SpaceCharacter:
            m_data = token.data();
            break;
        default:
            break;
        }
    }

    HTMLTokenView(HTMLToken::Type type, const GlobalString& tagName)
        : m_type(type), m_tagName(tagName)
    {}

    HTMLToken::Type type() const { return m_type; }
    bool selfClosing() const { return m_selfClosing; }
    bool forceQuirks() const { return m_forceQuirks; }
    bool hasPublicIdentifier() const { return m_hasPublicIdentifier; }
    bool hasSystemIdentifier() const { return m_hasSystemIdentifier; }
    const std::string_view& publicIdentifier() const { return m_publicIdentifier; }
    const std::string_view& systemIdentifier() const { return m_systemIdentifier; }
    const GlobalString& tagName() const { return m_tagName; }
    const std::span<Attribute>& attributes() const { return m_attributes; }
    const std::string_view& data() const { return m_data; }

    bool hasCamelCase() const { return m_hasCamelCase; }
    void setHasCamelCase(bool value) { m_hasCamelCase = value; }

    const Attribute* findAttribute(const GlobalString& name) const {
        assert(m_type == HTMLToken::Type::StartTag || m_type == HTMLToken::Type::EndTag);
        for(const auto& attribute : m_attributes) {
            if(name == attribute.name()) {
                return &attribute;
            }
        }

        return nullptr;
    }

    bool hasAttribute(const GlobalString& name) const {
        assert(m_type == HTMLToken::Type::StartTag || m_type == HTMLToken::Type::EndTag);
        for(const auto& attribute : m_attributes) {
            if(name == attribute.name()) {
                return true;
            }
        }

        return false;
    }

    void adjustTagName(const GlobalString& newName) {
        assert(m_type == HTMLToken::Type::StartTag || m_type == HTMLToken::Type::EndTag);
        m_tagName = newName;
    }

    void skipLeadingNewLine() {
        assert(m_type == HTMLToken::Type::SpaceCharacter);
        if(m_data.front() == '\n') {
            m_data.remove_prefix(1);
        }
    }

private:
    HTMLToken::Type m_type;
    bool m_selfClosing{false};
    bool m_forceQuirks{false};
    bool m_hasPublicIdentifier{false};
    bool m_hasSystemIdentifier{false};
    bool m_hasCamelCase{false};
    std::string_view m_publicIdentifier;
    std::string_view m_systemIdentifier;
    GlobalString m_tagName;
    std::span<Attribute> m_attributes;
    std::string_view m_data;
};

class HTMLTokenizer {
public:
    enum class State {
        Data,
        CharacterReferenceInData,
        RCDATA,
        CharacterReferenceInRCDATA,
        RAWTEXT,
        ScriptData,
        PLAINTEXT,
        TagOpen,
        EndTagOpen,
        TagName,
        RCDATALessThanSign,
        RCDATAEndTagOpen,
        RCDATAEndTagName,
        RAWTEXTLessThanSign,
        RAWTEXTEndTagOpen,
        RAWTEXTEndTagName,
        ScriptDataLessThanSign,
        ScriptDataEndTagOpen,
        ScriptDataEndTagName,
        ScriptDataEscapeStart,
        ScriptDataEscapeStartDash,
        ScriptDataEscaped,
        ScriptDataEscapedDash,
        ScriptDataEscapedDashDash,
        ScriptDataEscapedLessThanSign,
        ScriptDataEscapedEndTagOpen,
        ScriptDataEscapedEndTagName,
        ScriptDataDoubleEscapeStart,
        ScriptDataDoubleEscaped,
        ScriptDataDoubleEscapedDash,
        ScriptDataDoubleEscapedDashDash,
        ScriptDataDoubleEscapedLessThanSign,
        ScriptDataDoubleEscapeEnd,
        BeforeAttributeName,
        AttributeName,
        AfterAttributeName,
        BeforeAttributeValue,
        AttributeValueDoubleQuoted,
        AttributeValueSingleQuoted,
        AttributeValueUnquoted,
        CharacterReferenceInAttributeValue,
        AfterAttributeValueQuoted,
        SelfClosingStartTag,
        BogusComment,
        MarkupDeclarationOpen,
        CommentStart,
        CommentStartDash,
        Comment,
        CommentEndDash,
        CommentEnd,
        CommentEndBang,
        DOCTYPE,
        BeforeDOCTYPEName,
        DOCTYPEName,
        AfterDOCTYPEName,
        AfterDOCTYPEPublicKeyword,
        BeforeDOCTYPEPublicIdentifier,
        DOCTYPEPublicIdentifierDoubleQuoted,
        DOCTYPEPublicIdentifierSingleQuoted,
        AfterDOCTYPEPublicIdentifier,
        BetweenDOCTYPEPublicAndSystemIdentifiers,
        AfterDOCTYPESystemKeyword,
        BeforeDOCTYPESystemIdentifier,
        DOCTYPESystemIdentifierDoubleQuoted,
        DOCTYPESystemIdentifierSingleQuoted,
        AfterDOCTYPESystemIdentifier,
        BogusDOCTYPE,
        CDATASection,
        CDATASectionRightSquareBracket, //
        CDATASectionDoubleRightSquareBracket //
    };

    HTMLTokenizer(const std::string_view& content, Heap* heap)
        : m_input(content), m_currentToken(heap)
    {}

    HTMLTokenView nextToken();

    State state() const { return m_state; }
    void setState(State state) { m_state = state; }
    bool atEOF() const { return m_currentToken.type() == HTMLToken::Type::EndOfFile; }

private:
    bool handleState(char cc);
    bool handleDataState(char cc);
    bool handleCharacterReferenceInDataState(char cc);
    bool handleRCDATAState(char cc);
    bool handleCharacterReferenceInRCDATAState(char cc);
    bool handleRAWTEXTState(char cc);
    bool handleScriptDataState(char cc);
    bool handlePLAINTEXTState(char cc);
    bool handleTagOpenState(char cc);
    bool handleEndTagOpenState(char cc);
    bool handleTagNameState(char cc);
    bool handleRCDATALessThanSignState(char cc);
    bool handleRCDATAEndTagOpenState(char cc);
    bool handleRCDATAEndTagNameState(char cc);
    bool handleRAWTEXTLessThanSignState(char cc);
    bool handleRAWTEXTEndTagOpenState(char cc);
    bool handleRAWTEXTEndTagNameState(char cc);
    bool handleScriptDataLessThanSignState(char cc);
    bool handleScriptDataEndTagOpenState(char cc);
    bool handleScriptDataEndTagNameState(char cc);
    bool handleScriptDataEscapeStartState(char cc);
    bool handleScriptDataEscapeStartDashState(char cc);
    bool handleScriptDataEscapedState(char cc);
    bool handleScriptDataEscapedDashState(char cc);
    bool handleScriptDataEscapedDashDashState(char cc);
    bool handleScriptDataEscapedLessThanSignState(char cc);
    bool handleScriptDataEscapedEndTagOpenState(char cc);
    bool handleScriptDataEscapedEndTagNameState(char cc);
    bool handleScriptDataDoubleEscapeStartState(char cc);
    bool handleScriptDataDoubleEscapedState(char cc);
    bool handleScriptDataDoubleEscapedDashState(char cc);
    bool handleScriptDataDoubleEscapedDashDashState(char cc);
    bool handleScriptDataDoubleEscapedLessThanSignState(char cc);
    bool handleScriptDataDoubleEscapeEndState(char cc);
    bool handleBeforeAttributeNameState(char cc);
    bool handleAttributeNameState(char cc);
    bool handleAfterAttributeNameState(char cc);
    bool handleBeforeAttributeValueState(char cc);
    bool handleAttributeValueDoubleQuotedState(char cc);
    bool handleAttributeValueSingleQuotedState(char cc);
    bool handleAttributeValueUnquotedState(char cc);
    bool handleCharacterReferenceInAttributeValueState(char cc);
    bool handleAfterAttributeValueQuotedState(char cc);
    bool handleSelfClosingStartTagState(char cc);
    bool handleBogusCommentState(char cc);
    bool handleMarkupDeclarationOpenState(char cc);
    bool handleCommentStartState(char cc);
    bool handleCommentStartDashState(char cc);
    bool handleCommentState(char cc);
    bool handleCommentEndDashState(char cc);
    bool handleCommentEndState(char cc);
    bool handleCommentEndBangState(char cc);
    bool handleDOCTYPEState(char cc);
    bool handleBeforeDOCTYPENameState(char cc);
    bool handleDOCTYPENameState(char cc);
    bool handleAfterDOCTYPENameState(char cc);
    bool handleAfterDOCTYPEPublicKeywordState(char cc);
    bool handleBeforeDOCTYPEPublicIdentifierState(char cc);
    bool handleDOCTYPEPublicIdentifierDoubleQuotedState(char cc);
    bool handleDOCTYPEPublicIdentifierSingleQuotedState(char cc);
    bool handleAfterDOCTYPEPublicIdentifierState(char cc);
    bool handleBetweenDOCTYPEPublicAndSystemIdentifiersState(char cc);
    bool handleAfterDOCTYPESystemKeywordState(char cc);
    bool handleBeforeDOCTYPESystemIdentifierState(char cc);
    bool handleDOCTYPESystemIdentifierDoubleQuotedState(char cc);
    bool handleDOCTYPESystemIdentifierSingleQuotedState(char cc);
    bool handleAfterDOCTYPESystemIdentifierState(char cc);
    bool handleBogusDOCTYPEState(char cc);
    bool handleCDATASectionState(char cc);
    bool handleCDATASectionRightSquareBracketState(char cc);
    bool handleCDATASectionDoubleRightSquareBracketState(char cc);

    bool advanceTo(State state);
    bool switchTo(State state);

    bool emitCurrentToken();
    bool emitEOFToken();
    bool emitEndTagToken();

    bool flushCharacterBuffer();
    bool flushEndTagNameBuffer();
    bool flushTemporaryBuffer();

    bool isAppropriateEndTag() const { return m_appropriateEndTagName == m_endTagNameBuffer; }
    bool temporaryBufferIs(const std::string_view& value) const { return m_temporaryBuffer == value; }

    char nextInputCharacter();
    char handleInputCharacter(char inputCharacter);

    bool consumeCharacterReference(std::string& output, bool inAttributeValue);
    bool consumeString(const std::string_view& value, bool caseSensitive);

    std::string_view m_input;
    std::string m_entityBuffer;
    std::string m_characterBuffer;
    std::string m_temporaryBuffer;
    std::string m_endTagNameBuffer;
    std::string m_appropriateEndTagName;
    State m_state{State::Data};
    bool m_reconsumeCurrentCharacter{true};
    char m_additionalAllowedCharacter{0};
    HTMLToken m_currentToken;
};

inline bool HTMLTokenizer::advanceTo(State state)
{
    m_state = state;
    m_reconsumeCurrentCharacter = false;
    return true;
}

inline bool HTMLTokenizer::switchTo(State state)
{
    m_state = state;
    m_reconsumeCurrentCharacter = true;
    return true;
}

inline char HTMLTokenizer::nextInputCharacter()
{
    if(!m_input.empty()) {
        if(m_reconsumeCurrentCharacter)
            return handleInputCharacter(m_input.front());
        m_input.remove_prefix(1);
    }

    if(!m_input.empty())
        return handleInputCharacter(m_input.front());
    return 0;
}

inline char HTMLTokenizer::handleInputCharacter(char inputCharacter)
{
    if(inputCharacter != '\r')
        return inputCharacter;
    if(m_input.size() > 1 && m_input[1] == '\n')
        m_input.remove_prefix(1);
    return '\n';
}

} // namespace plutobook

#endif // PLUTOBOOK_HTMLTOKENIZER_H
