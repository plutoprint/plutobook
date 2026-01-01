/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "htmltokenizer.h"
#include "htmlentityparser.h"
#include "stringutils.h"

namespace plutobook {

HTMLTokenView HTMLTokenizer::nextToken()
{
    m_currentToken.reset();
    if(!m_characterBuffer.empty()) {
        flushCharacterBuffer();
        assert(m_characterBuffer.empty());
        return m_currentToken;
    }

    if(!m_endTagNameBuffer.empty()) {
        flushEndTagNameBuffer();
        assert(m_endTagNameBuffer.empty());
        if(m_state == State::Data) {
            return m_currentToken;
        }
    }

    while(handleState(nextInputCharacter()));
    return m_currentToken;
}

bool HTMLTokenizer::handleState(char cc)
{
    switch(m_state) {
    case State::Data:
        return handleDataState(cc);
    case State::CharacterReferenceInData:
        return handleCharacterReferenceInDataState(cc);
    case State::RCDATA:
        return handleRCDATAState(cc);
    case State::CharacterReferenceInRCDATA:
        return handleCharacterReferenceInRCDATAState(cc);
    case State::RAWTEXT:
        return handleRAWTEXTState(cc);
    case State::ScriptData:
        return handleScriptDataState(cc);
    case State::PLAINTEXT:
        return handlePLAINTEXTState(cc);
    case State::TagOpen:
        return handleTagOpenState(cc);
    case State::EndTagOpen:
        return handleEndTagOpenState(cc);
    case State::TagName:
        return handleTagNameState(cc);
    case State::RCDATALessThanSign:
        return handleRCDATALessThanSignState(cc);
    case State::RCDATAEndTagOpen:
        return handleRCDATAEndTagOpenState(cc);
    case State::RCDATAEndTagName:
        return handleRCDATAEndTagNameState(cc);
    case State::RAWTEXTLessThanSign:
        return handleRAWTEXTLessThanSignState(cc);
    case State::RAWTEXTEndTagOpen:
        return handleRAWTEXTEndTagOpenState(cc);
    case State::RAWTEXTEndTagName:
        return handleRAWTEXTEndTagNameState(cc);
    case State::ScriptDataLessThanSign:
        return handleScriptDataLessThanSignState(cc);
    case State::ScriptDataEndTagOpen:
        return handleScriptDataEndTagOpenState(cc);
    case State::ScriptDataEndTagName:
        return handleScriptDataEndTagNameState(cc);
    case State::ScriptDataEscapeStart:
        return handleScriptDataEscapeStartState(cc);
    case State::ScriptDataEscapeStartDash:
        return handleScriptDataEscapeStartDashState(cc);
    case State::ScriptDataEscaped:
        return handleScriptDataEscapedState(cc);
    case State::ScriptDataEscapedDash:
        return handleScriptDataEscapedDashState(cc);
    case State::ScriptDataEscapedDashDash:
        return handleScriptDataEscapedDashDashState(cc);
    case State::ScriptDataEscapedLessThanSign:
        return handleScriptDataEscapedLessThanSignState(cc);
    case State::ScriptDataEscapedEndTagOpen:
        return handleScriptDataEscapedEndTagOpenState(cc);
    case State::ScriptDataEscapedEndTagName:
        return handleScriptDataEscapedEndTagNameState(cc);
    case State::ScriptDataDoubleEscapeStart:
        return handleScriptDataDoubleEscapeStartState(cc);
    case State::ScriptDataDoubleEscaped:
        return handleScriptDataDoubleEscapedState(cc);
    case State::ScriptDataDoubleEscapedDash:
        return handleScriptDataDoubleEscapedDashState(cc);
    case State::ScriptDataDoubleEscapedDashDash:
        return handleScriptDataDoubleEscapedDashDashState(cc);
    case State::ScriptDataDoubleEscapedLessThanSign:
        return handleScriptDataDoubleEscapedLessThanSignState(cc);
    case State::ScriptDataDoubleEscapeEnd:
        return handleScriptDataDoubleEscapeEndState(cc);
    case State::BeforeAttributeName:
        return handleBeforeAttributeNameState(cc);
    case State::AttributeName:
        return handleAttributeNameState(cc);
    case State::AfterAttributeName:
        return handleAfterAttributeNameState(cc);
    case State::BeforeAttributeValue:
        return handleBeforeAttributeValueState(cc);
    case State::AttributeValueDoubleQuoted:
        return handleAttributeValueDoubleQuotedState(cc);
    case State::AttributeValueSingleQuoted:
        return handleAttributeValueSingleQuotedState(cc);
    case State::AttributeValueUnquoted:
        return handleAttributeValueUnquotedState(cc);
    case State::CharacterReferenceInAttributeValue:
        return handleCharacterReferenceInAttributeValueState(cc);
    case State::AfterAttributeValueQuoted:
        return handleAfterAttributeValueQuotedState(cc);
    case State::SelfClosingStartTag:
        return handleSelfClosingStartTagState(cc);
    case State::BogusComment:
        return handleBogusCommentState(cc);
    case State::MarkupDeclarationOpen:
        return handleMarkupDeclarationOpenState(cc);
    case State::CommentStart:
        return handleCommentStartState(cc);
    case State::CommentStartDash:
        return handleCommentStartDashState(cc);
    case State::Comment:
        return handleCommentState(cc);
    case State::CommentEndDash:
        return handleCommentEndDashState(cc);
    case State::CommentEnd:
        return handleCommentEndState(cc);
    case State::CommentEndBang:
        return handleCommentEndBangState(cc);
    case State::DOCTYPE:
        return handleDOCTYPEState(cc);
    case State::BeforeDOCTYPEName:
        return handleBeforeDOCTYPENameState(cc);
    case State::DOCTYPEName:
        return handleDOCTYPENameState(cc);
    case State::AfterDOCTYPEName:
        return handleAfterDOCTYPENameState(cc);
    case State::AfterDOCTYPEPublicKeyword:
        return handleAfterDOCTYPEPublicKeywordState(cc);
    case State::BeforeDOCTYPEPublicIdentifier:
        return handleBeforeDOCTYPEPublicIdentifierState(cc);
    case State::DOCTYPEPublicIdentifierDoubleQuoted:
        return handleDOCTYPEPublicIdentifierDoubleQuotedState(cc);
    case State::DOCTYPEPublicIdentifierSingleQuoted:
        return handleDOCTYPEPublicIdentifierSingleQuotedState(cc);
    case State::AfterDOCTYPEPublicIdentifier:
        return handleAfterDOCTYPEPublicIdentifierState(cc);
    case State::BetweenDOCTYPEPublicAndSystemIdentifiers:
        return handleBetweenDOCTYPEPublicAndSystemIdentifiersState(cc);
    case State::AfterDOCTYPESystemKeyword:
        return handleAfterDOCTYPESystemKeywordState(cc);
    case State::BeforeDOCTYPESystemIdentifier:
        return handleBeforeDOCTYPESystemIdentifierState(cc);
    case State::DOCTYPESystemIdentifierDoubleQuoted:
        return handleDOCTYPESystemIdentifierDoubleQuotedState(cc);
    case State::DOCTYPESystemIdentifierSingleQuoted:
        return handleDOCTYPESystemIdentifierSingleQuotedState(cc);
    case State::AfterDOCTYPESystemIdentifier:
        return handleAfterDOCTYPESystemIdentifierState(cc);
    case State::BogusDOCTYPE:
        return handleBogusDOCTYPEState(cc);
    case State::CDATASection:
        return handleCDATASectionState(cc);
    case State::CDATASectionRightSquareBracket:
        return handleCDATASectionRightSquareBracketState(cc);
    case State::CDATASectionDoubleRightSquareBracket:
        return handleCDATASectionDoubleRightSquareBracketState(cc);
    }

    return false;
}

bool HTMLTokenizer::handleDataState(char cc)
{
    if(cc == '&')
        return advanceTo(State::CharacterReferenceInData);
    if(cc == '<') {
        if(!m_characterBuffer.empty())
            return advanceTo(State::TagOpen) && flushCharacterBuffer();
        return advanceTo(State::TagOpen);
    }

    if(cc == 0)
        return emitEOFToken();

    m_characterBuffer += cc;
    return advanceTo(State::Data);
}

bool HTMLTokenizer::handleCharacterReferenceInDataState(char cc)
{
    m_entityBuffer.clear();
    if(consumeCharacterReference(m_entityBuffer, false)) {
        m_characterBuffer += m_entityBuffer;
    } else {
        m_characterBuffer += '&';
    }

    return switchTo(State::Data);
}

bool HTMLTokenizer::handleRCDATAState(char cc)
{
    if(cc == '&')
        return advanceTo(State::CharacterReferenceInRCDATA);
    if(cc == '<')
        return advanceTo(State::RCDATALessThanSign);

    if(cc == 0)
        return emitEOFToken();

    m_characterBuffer += cc;
    return advanceTo(State::RCDATA);
}

bool HTMLTokenizer::handleCharacterReferenceInRCDATAState(char cc)
{
    m_entityBuffer.clear();
    if(consumeCharacterReference(m_entityBuffer, false)) {
        m_characterBuffer += m_entityBuffer;
    } else {
        m_characterBuffer += '&';
    }

    return switchTo(State::RCDATA);
}

bool HTMLTokenizer::handleRAWTEXTState(char cc)
{
    if(cc == '<')
        return advanceTo(State::RAWTEXTLessThanSign);

   if(cc == 0)
        return emitEOFToken();

    m_characterBuffer += cc;
    return advanceTo(State::RAWTEXT);
}

bool HTMLTokenizer::handleScriptDataState(char cc)
{
    if(cc == '<')
        return advanceTo(State::ScriptDataLessThanSign);

    if(cc == 0)
        return emitEOFToken();

    m_characterBuffer += cc;
    return advanceTo(State::ScriptData);
}

bool HTMLTokenizer::handlePLAINTEXTState(char cc)
{
    if(cc == 0)
        return emitEOFToken();

    m_characterBuffer += cc;
    return advanceTo(State::PLAINTEXT);
}

bool HTMLTokenizer::handleTagOpenState(char cc)
{
    if(cc == '!')
        return advanceTo(State::MarkupDeclarationOpen);
    if(cc == '/')
        return advanceTo(State::EndTagOpen);
    if(cc == '?') {
        m_currentToken.beginComment();
        return switchTo(State::BogusComment);
    }

    if(isAlpha(cc)) {
        m_currentToken.beginStartTag();
        m_currentToken.addToTagName(toLower(cc));
        return advanceTo(State::TagName);
    }

    m_characterBuffer += '<';
    return switchTo(State::Data);
}

bool HTMLTokenizer::handleEndTagOpenState(char cc)
{
    if(isAlpha(cc)) {
        m_currentToken.beginEndTag();
        m_currentToken.addToTagName(toLower(cc));
        m_appropriateEndTagName.clear();
        return advanceTo(State::TagName);
    }

    if(cc == '>')
        return advanceTo(State::Data);

    if(cc == 0) {
        m_characterBuffer += '<';
        m_characterBuffer += '/';
        return switchTo(State::Data);
    }

    m_currentToken.beginComment();
    return switchTo(State::BogusComment);
}

bool HTMLTokenizer::handleTagNameState(char cc)
{
    if(isSpace(cc))
        return advanceTo(State::BeforeAttributeName);
    if(cc == '/')
        return advanceTo(State::SelfClosingStartTag);
    if(cc == '>')
        return advanceTo(State::Data) && emitCurrentToken();

    if(cc == 0)
        return switchTo(State::Data);

    m_currentToken.addToTagName(toLower(cc));
    return advanceTo(State::TagName);
}

bool HTMLTokenizer::handleRCDATALessThanSignState(char cc)
{
    if(cc == '/') {
        m_temporaryBuffer.clear();
        return advanceTo(State::RCDATAEndTagOpen);
    }

    m_characterBuffer += '<';
    return switchTo(State::RCDATA);
}

bool HTMLTokenizer::handleRCDATAEndTagOpenState(char cc)
{
    if(isAlpha(cc)) {
        assert(m_endTagNameBuffer.empty());
        assert(m_temporaryBuffer.empty());
        m_temporaryBuffer += cc;
        m_endTagNameBuffer += toLower(cc);
        return advanceTo(State::RCDATAEndTagName);
    }

    m_characterBuffer += '<';
    m_characterBuffer += '/';
    return switchTo(State::RCDATA);
}

bool HTMLTokenizer::handleRCDATAEndTagNameState(char cc)
{
    if(isSpace(cc) && isAppropriateEndTag())
        return advanceTo(State::BeforeAttributeName) && flushEndTagNameBuffer();

    if(cc == '/' && isAppropriateEndTag())
        return advanceTo(State::SelfClosingStartTag) && flushEndTagNameBuffer();

    if(cc == '>' && isAppropriateEndTag())
        return advanceTo(State::Data) && emitEndTagToken();

    if(isAlpha(cc)) {
        m_temporaryBuffer += cc;
        m_endTagNameBuffer += toLower(cc);
        return advanceTo(State::RCDATAEndTagName);
    }

    m_characterBuffer += '<';
    m_characterBuffer += '/';
    return switchTo(State::RCDATA) && flushTemporaryBuffer();
}

bool HTMLTokenizer::handleRAWTEXTLessThanSignState(char cc)
{
    if(cc == '/') {
        m_temporaryBuffer.clear();
        return advanceTo(State::RAWTEXTEndTagOpen);
    }

    m_characterBuffer += '<';
    return switchTo(State::RAWTEXT);
}

bool HTMLTokenizer::handleRAWTEXTEndTagOpenState(char cc)
{
    if(isAlpha(cc)) {
        assert(m_endTagNameBuffer.empty());
        assert(m_temporaryBuffer.empty());
        m_temporaryBuffer += cc;
        m_endTagNameBuffer += toLower(cc);
        return advanceTo(State::RAWTEXTEndTagName);
    }

    m_characterBuffer += '<';
    m_characterBuffer += '/';
    return switchTo(State::RAWTEXT);
}

bool HTMLTokenizer::handleRAWTEXTEndTagNameState(char cc)
{
    if(isSpace(cc) && isAppropriateEndTag())
        return advanceTo(State::BeforeAttributeName) && flushEndTagNameBuffer();

    if(cc == '/' && isAppropriateEndTag())
        return advanceTo(State::SelfClosingStartTag) && flushEndTagNameBuffer();

    if(cc == '>' && isAppropriateEndTag())
        return advanceTo(State::Data) && emitEndTagToken();

    if(isAlpha(cc)) {
        m_temporaryBuffer += cc;
        m_endTagNameBuffer += toLower(cc);
        return advanceTo(State::RAWTEXTEndTagName);
    }

    m_characterBuffer += '<';
    m_characterBuffer += '/';
    return switchTo(State::RAWTEXT) && flushTemporaryBuffer();
}

bool HTMLTokenizer::handleScriptDataLessThanSignState(char cc)
{
    if(cc == '/') {
        m_temporaryBuffer.clear();
        return advanceTo(State::ScriptDataEndTagOpen);
    }

    if(cc == '!') {
        m_characterBuffer += '<';
        m_characterBuffer += '!';
        return advanceTo(State::ScriptDataEscapeStart);
    }

    m_characterBuffer += '<';
    return switchTo(State::ScriptData);
}

bool HTMLTokenizer::handleScriptDataEndTagOpenState(char cc)
{
    if(isAlpha(cc)) {
        assert(m_endTagNameBuffer.empty());
        assert(m_temporaryBuffer.empty());
        m_temporaryBuffer += cc;
        m_endTagNameBuffer += toLower(cc);
        return advanceTo(State::ScriptDataEndTagName);
    }

    m_characterBuffer += '<';
    m_characterBuffer += '/';
    return switchTo(State::ScriptData);
}

bool HTMLTokenizer::handleScriptDataEndTagNameState(char cc)
{
    if(isSpace(cc) && isAppropriateEndTag())
        return advanceTo(State::BeforeAttributeName) && flushEndTagNameBuffer();

    if(cc == '/' && isAppropriateEndTag())
        return advanceTo(State::SelfClosingStartTag) && flushEndTagNameBuffer();

    if(cc == '>' && isAppropriateEndTag())
        return advanceTo(State::Data) && emitEndTagToken();

    if(isAlpha(cc)) {
        m_temporaryBuffer += cc;
        m_endTagNameBuffer += toLower(cc);
        return advanceTo(State::ScriptDataEndTagName);
    }

    m_characterBuffer += '<';
    m_characterBuffer += '/';
    return switchTo(State::ScriptData) && flushTemporaryBuffer();
}

bool HTMLTokenizer::handleScriptDataEscapeStartState(char cc)
{
    if(cc == '-') {
        m_characterBuffer += cc;
        return advanceTo(State::ScriptDataEscapeStartDash);
    }

    return switchTo(State::ScriptData);
}

bool HTMLTokenizer::handleScriptDataEscapeStartDashState(char cc)
{
    if(cc == '-') {
        m_characterBuffer += cc;
        return advanceTo(State::ScriptDataEscapedDashDash);
    }

    return switchTo(State::ScriptData);
}

bool HTMLTokenizer::handleScriptDataEscapedState(char cc)
{
    if(cc == '-') {
        m_characterBuffer += cc;
        return advanceTo(State::ScriptDataEscapedDash);
    }

    if(cc == '<')
        return advanceTo(State::ScriptDataEscapedLessThanSign);

    if(cc == 0)
        return switchTo(State::Data);

    m_characterBuffer += cc;
    return advanceTo(State::ScriptDataEscaped);
}

bool HTMLTokenizer::handleScriptDataEscapedDashState(char cc)
{
    if(cc == '-') {
        m_characterBuffer += cc;
        return advanceTo(State::ScriptDataEscapedDashDash);
    }

    if(cc == '<')
        return advanceTo(State::ScriptDataEscapedLessThanSign);

    if(cc == 0)
        return switchTo(State::Data);

    m_characterBuffer += cc;
    return advanceTo(State::ScriptDataEscaped);
}

bool HTMLTokenizer::handleScriptDataEscapedDashDashState(char cc)
{
    if(cc == '-') {
        m_characterBuffer += cc;
        return advanceTo(State::ScriptDataEscapedDashDash);
    }

    if(cc == '<')
        return advanceTo(State::ScriptDataEscapedLessThanSign);

    if(cc == '>') {
        m_characterBuffer += cc;
        return advanceTo(State::ScriptData);
    }

    if(cc == 0)
        return switchTo(State::Data);

    m_characterBuffer += cc;
    return advanceTo(State::ScriptDataEscaped);
}

bool HTMLTokenizer::handleScriptDataEscapedLessThanSignState(char cc)
{
    if(cc == '/') {
        m_temporaryBuffer.clear();
        return advanceTo(State::ScriptDataEscapedEndTagOpen);
    }

    if(isAlpha(cc)) {
        m_temporaryBuffer.clear();
        m_temporaryBuffer += toLower(cc);
        m_characterBuffer += '<';
        m_characterBuffer += cc;
        return advanceTo(State::ScriptDataDoubleEscapeStart);
    }

    m_characterBuffer += '<';
    return switchTo(State::ScriptDataEscaped);
}

bool HTMLTokenizer::handleScriptDataEscapedEndTagOpenState(char cc)
{
    if(isAlpha(cc)) {
        assert(m_endTagNameBuffer.empty());
        assert(m_temporaryBuffer.empty());
        m_temporaryBuffer += cc;
        m_endTagNameBuffer += toLower(cc);
        return advanceTo(State::ScriptDataEscapedEndTagName);
    }

    m_characterBuffer += '<';
    m_characterBuffer += '/';
    return switchTo(State::ScriptDataEscaped);
}

bool HTMLTokenizer::handleScriptDataEscapedEndTagNameState(char cc)
{
    if(isSpace(cc) && isAppropriateEndTag())
        return advanceTo(State::BeforeAttributeName) && flushEndTagNameBuffer();

    if(cc == '/' && isAppropriateEndTag())
        return advanceTo(State::SelfClosingStartTag) && flushEndTagNameBuffer();

    if(cc == '>' && isAppropriateEndTag())
        return advanceTo(State::Data) && emitEndTagToken();

    if(isAlpha(cc)) {
        m_temporaryBuffer += cc;
        m_endTagNameBuffer += toLower(cc);
        return advanceTo(State::ScriptDataEscapedEndTagName);
    }

    m_characterBuffer += '<';
    m_characterBuffer += '/';
    return switchTo(State::ScriptDataEscaped) && flushTemporaryBuffer();
}

bool HTMLTokenizer::handleScriptDataDoubleEscapeStartState(char cc)
{
    if(isSpace(cc) || cc == '/' || cc == '>') {
        m_characterBuffer += cc;
        if(temporaryBufferIs(scriptTag.value()))
            return advanceTo(State::ScriptDataDoubleEscaped);
        return advanceTo(State::ScriptDataEscaped);
    }

    if(isAlpha(cc)) {
        m_characterBuffer += cc;
        m_temporaryBuffer += toLower(cc);
        return advanceTo(State::ScriptDataDoubleEscapeStart);
    }

    return switchTo(State::ScriptDataEscaped);
}

bool HTMLTokenizer::handleScriptDataDoubleEscapedState(char cc)
{
    if(cc == '-') {
        m_characterBuffer += cc;
        return advanceTo(State::ScriptDataDoubleEscapedDash);
    }

    if(cc == '<') {
        m_characterBuffer += cc;
        return advanceTo(State::ScriptDataDoubleEscapedLessThanSign);
    }

    if(cc == 0)
        return switchTo(State::Data);

    m_characterBuffer += cc;
    return advanceTo(State::ScriptDataDoubleEscaped);
}

bool HTMLTokenizer::handleScriptDataDoubleEscapedDashState(char cc)
{
    if(cc == '-') {
        m_characterBuffer += cc;
        return advanceTo(State::ScriptDataDoubleEscapedDashDash);
    }

    if(cc == '<') {
        m_characterBuffer += cc;
        return advanceTo(State::ScriptDataDoubleEscapedLessThanSign);
    }

    if(cc == 0)
        return switchTo(State::Data);

    m_characterBuffer += cc;
    return advanceTo(State::ScriptDataDoubleEscaped);
}

bool HTMLTokenizer::handleScriptDataDoubleEscapedDashDashState(char cc)
{
    if(cc == '-') {
        m_characterBuffer += cc;
        return advanceTo(State::ScriptDataDoubleEscapedDashDash);
    }

    if(cc == '<') {
        m_characterBuffer += cc;
        return advanceTo(State::ScriptDataDoubleEscapedLessThanSign);
    }

    if(cc == '>') {
        m_characterBuffer += cc;
        return advanceTo(State::ScriptData);
    }

    if(cc == 0)
        return switchTo(State::Data);

    m_characterBuffer += cc;
    return advanceTo(State::ScriptDataDoubleEscaped);
}

bool HTMLTokenizer::handleScriptDataDoubleEscapedLessThanSignState(char cc)
{
    if(cc == '/') {
        m_characterBuffer += cc;
        m_temporaryBuffer.clear();
        return advanceTo(State::ScriptDataDoubleEscapeEnd);
    }

    return switchTo(State::ScriptDataDoubleEscaped);
}

bool HTMLTokenizer::handleScriptDataDoubleEscapeEndState(char cc)
{
    if(isSpace(cc) || cc == '/' || cc == '>') {
        m_characterBuffer += cc;
        if(temporaryBufferIs(scriptTag.value()))
            return advanceTo(State::ScriptDataEscaped);
        return advanceTo(State::ScriptDataDoubleEscaped);
    }

    if(isAlpha(cc)) {
        m_characterBuffer += cc;
        m_temporaryBuffer += toLower(cc);
        return advanceTo(State::ScriptDataDoubleEscapeEnd);
    }

    return switchTo(State::ScriptDataDoubleEscaped);
}

bool HTMLTokenizer::handleBeforeAttributeNameState(char cc)
{
    if(isSpace(cc))
        return advanceTo(State::BeforeAttributeName);

    if(cc == '/')
        return advanceTo(State::SelfClosingStartTag);

    if(cc == '>')
        return advanceTo(State::Data) && emitCurrentToken();

    if(isAlpha(cc)) {
        m_currentToken.beginAttribute();
        m_currentToken.addToAttributeName(toLower(cc));
        return advanceTo(State::AttributeName);
    }

    if(cc == 0)
        return switchTo(State::Data);

    m_currentToken.beginAttribute();
    m_currentToken.addToAttributeName(cc);
    return advanceTo(State::AttributeName);
}

bool HTMLTokenizer::handleAttributeNameState(char cc)
{
    if(isSpace(cc))
        return advanceTo(State::AfterAttributeName);

    if(cc == '/') {
        m_currentToken.endAttribute();
        return advanceTo(State::SelfClosingStartTag);
    }

    if(cc == '=')
        return advanceTo(State::BeforeAttributeValue);

    if(cc == '>') {
        m_currentToken.endAttribute();
        return advanceTo(State::Data) && emitCurrentToken();
    }

    if(isAlpha(cc)) {
        m_currentToken.addToAttributeName(toLower(cc));
        return advanceTo(State::AttributeName);
    }

    if(cc == 0) {
        m_currentToken.endAttribute();
        return switchTo(State::Data);
    }

    m_currentToken.addToAttributeName(cc);
    return advanceTo(State::AttributeName);
}

bool HTMLTokenizer::handleAfterAttributeNameState(char cc)
{
    if(isSpace(cc))
        return advanceTo(State::AfterAttributeName);

    if(cc == '/') {
        m_currentToken.endAttribute();
        return advanceTo(State::SelfClosingStartTag);
    }

    if(cc == '=')
        return advanceTo(State::BeforeAttributeValue);

    if(cc == '>') {
        m_currentToken.endAttribute();
        return advanceTo(State::Data) && emitCurrentToken();
    }

    if(isAlpha(cc)) {
        m_currentToken.endAttribute();
        m_currentToken.beginAttribute();
        m_currentToken.addToAttributeName(toLower(cc));
        return advanceTo(State::AttributeName);
    }

    if(cc == 0) {
        m_currentToken.endAttribute();
        return switchTo(State::Data);
    }

    m_currentToken.endAttribute();
    m_currentToken.beginAttribute();
    m_currentToken.addToAttributeName(cc);
    return advanceTo(State::AttributeName);
}

bool HTMLTokenizer::handleBeforeAttributeValueState(char cc)
{
    if(isSpace(cc))
        return advanceTo(State::BeforeAttributeValue);

    if(cc == '"')
        return advanceTo(State::AttributeValueDoubleQuoted);

    if(cc == '&')
        return switchTo(State::AttributeValueUnquoted);

    if(cc == '\'')
        return advanceTo(State::AttributeValueSingleQuoted);

    if(cc == '>') {
        m_currentToken.endAttribute();
        return advanceTo(State::Data) && emitCurrentToken();
    }

    if(cc == 0) {
        m_currentToken.endAttribute();
        return switchTo(State::Data);
    }

    m_currentToken.addToAttributeValue(cc);
    return advanceTo(State::AttributeValueUnquoted);
}

bool HTMLTokenizer::handleAttributeValueDoubleQuotedState(char cc)
{
    if(cc == '"')
        return advanceTo(State::AfterAttributeValueQuoted);

    if(cc == '&') {
        m_additionalAllowedCharacter = '"';
        return advanceTo(State::CharacterReferenceInAttributeValue);
    }

    if(cc == 0) {
        m_currentToken.endAttribute();
        return switchTo(State::Data);
    }

    m_currentToken.addToAttributeValue(cc);
    return advanceTo(State::AttributeValueDoubleQuoted);
}

bool HTMLTokenizer::handleAttributeValueSingleQuotedState(char cc)
{
    if(cc == '\'')
        return advanceTo(State::AfterAttributeValueQuoted);

    if(cc == '&') {
        m_additionalAllowedCharacter = '\'';
        return advanceTo(State::CharacterReferenceInAttributeValue);
    }

    if(cc == 0) {
        m_currentToken.endAttribute();
        return switchTo(State::Data);
    }

    m_currentToken.addToAttributeValue(cc);
    return advanceTo(State::AttributeValueSingleQuoted);
}

bool HTMLTokenizer::handleAttributeValueUnquotedState(char cc)
{
    if(isSpace(cc)) {
        m_currentToken.endAttribute();
        return advanceTo(State::BeforeAttributeName);
    }

    if(cc == '&') {
        m_additionalAllowedCharacter = '>';
        return advanceTo(State::CharacterReferenceInAttributeValue);
    }

    if(cc == '>') {
        m_currentToken.endAttribute();
        return advanceTo(State::Data) && emitCurrentToken();
    }

    if(cc == 0) {
        m_currentToken.endAttribute();
        return switchTo(State::Data);
    }

    m_currentToken.addToAttributeValue(cc);
    return advanceTo(State::AttributeValueUnquoted);
}

bool HTMLTokenizer::handleCharacterReferenceInAttributeValueState(char cc)
{
    m_entityBuffer.clear();
    if(consumeCharacterReference(m_entityBuffer, true)) {
        m_currentToken.addToAttributeValue(m_entityBuffer);
    } else {
        m_currentToken.addToAttributeValue('&');
    }

    if(m_additionalAllowedCharacter == '"')
        return switchTo(State::AttributeValueDoubleQuoted);
    if(m_additionalAllowedCharacter == '\'')
        return switchTo(State::AttributeValueSingleQuoted);

    assert(m_additionalAllowedCharacter == '>');
    return switchTo(State::AttributeValueUnquoted);
}

bool HTMLTokenizer::handleAfterAttributeValueQuotedState(char cc)
{
    m_currentToken.endAttribute();
    if(isSpace(cc))
        return advanceTo(State::BeforeAttributeName);

    if(cc == '/')
        return advanceTo(State::SelfClosingStartTag);

    if(cc == '>')
        return advanceTo(State::Data) && emitCurrentToken();

    if(cc == 0)
        return switchTo(State::Data);

    return switchTo(State::BeforeAttributeName);
}

bool HTMLTokenizer::handleSelfClosingStartTagState(char cc)
{
    if(cc == '>') {
        m_currentToken.setSelfClosing();
        return advanceTo(State::Data) && emitCurrentToken();
    }

    if(cc == 0)
        return switchTo(State::Data);

    return switchTo(State::BeforeAttributeName);
}

bool HTMLTokenizer::handleBogusCommentState(char cc)
{
    if(cc == '>')
        return advanceTo(State::Data) && emitCurrentToken();

    if(cc == 0)
        return switchTo(State::Data) && emitCurrentToken();

    m_currentToken.addToComment(cc);
    return advanceTo(State::BogusComment);
}

bool HTMLTokenizer::handleMarkupDeclarationOpenState(char cc)
{
    constexpr std::string_view dashdash("--");
    constexpr std::string_view doctype("DOCTYPE");
    constexpr std::string_view cdata("[CDATA[");
    if(consumeString(dashdash, true)) {
        m_currentToken.beginComment();
        return switchTo(State::CommentStart);
    }

    if(consumeString(doctype, false))
        return switchTo(State::DOCTYPE);

    if(consumeString(cdata, true))
        return switchTo(State::CDATASection);

    m_currentToken.beginComment();
    return switchTo(State::BogusComment);
}

bool HTMLTokenizer::handleCommentStartState(char cc)
{
    if(cc == '-')
        return advanceTo(State::CommentStartDash);

    if(cc == '>')
        return advanceTo(State::Data) && emitCurrentToken();

    if(cc == 0)
        return switchTo(State::Data) && emitCurrentToken();

    m_currentToken.addToComment(cc);
    return advanceTo(State::Comment);
}

bool HTMLTokenizer::handleCommentStartDashState(char cc)
{
    if(cc == '-')
        return advanceTo(State::CommentEnd);

    if(cc == '>')
        return advanceTo(State::Data) && emitCurrentToken();

    if(cc == 0)
        return switchTo(State::Data) && emitCurrentToken();

    m_currentToken.addToComment('-');
    m_currentToken.addToComment(cc);
    return advanceTo(State::Comment);
}

bool HTMLTokenizer::handleCommentState(char cc)
{
    if(cc == '-')
        return advanceTo(State::CommentEndDash);

    if(cc == 0)
        return switchTo(State::Data) && emitCurrentToken();

    m_currentToken.addToComment(cc);
    return advanceTo(State::Comment);
}

bool HTMLTokenizer::handleCommentEndDashState(char cc)
{
    if(cc == '-')
        return advanceTo(State::CommentEnd);

    if(cc == 0)
        return switchTo(State::Data) && emitCurrentToken();

    m_currentToken.addToComment('-');
    m_currentToken.addToComment(cc);
    return advanceTo(State::Comment);
}

bool HTMLTokenizer::handleCommentEndState(char cc)
{
    if(cc == '>')
        return advanceTo(State::Data) && emitCurrentToken();

    if(cc == '!')
        return advanceTo(State::CommentEndBang);

    if(cc == '-') {
        m_currentToken.addToComment(cc);
        return advanceTo(State::CommentEnd);
    }

    if(cc == 0)
        return switchTo(State::Data) && emitCurrentToken();

    m_currentToken.addToComment('-');
    m_currentToken.addToComment('-');
    m_currentToken.addToComment(cc);
    return advanceTo(State::Comment);
}

bool HTMLTokenizer::handleCommentEndBangState(char cc)
{
    if(cc == '-') {
        m_currentToken.addToComment('-');
        m_currentToken.addToComment('-');
        m_currentToken.addToComment('!');
        return advanceTo(State::CommentEndDash);
    }

    if(cc == '>')
        return advanceTo(State::Data) && emitCurrentToken();

    if(cc == 0)
        return switchTo(State::Data) && emitCurrentToken();

    m_currentToken.addToComment('-');
    m_currentToken.addToComment('-');
    m_currentToken.addToComment('!');
    m_currentToken.addToComment(cc);
    return advanceTo(State::Comment);
}

bool HTMLTokenizer::handleDOCTYPEState(char cc)
{
    if(isSpace(cc))
        return advanceTo(State::BeforeDOCTYPEName);

    if(cc == 0) {
        m_currentToken.beginDOCTYPE();
        m_currentToken.setForceQuirks();
        return switchTo(State::Data) && emitCurrentToken();
    }

    return switchTo(State::BeforeDOCTYPEName);
}

bool HTMLTokenizer::handleBeforeDOCTYPENameState(char cc)
{
    if(isSpace(cc))
        return advanceTo(State::BeforeDOCTYPEName);

    if(cc == '>') {
        m_currentToken.beginDOCTYPE();
        m_currentToken.setForceQuirks();
        return advanceTo(State::Data) && emitCurrentToken();
    }

    if(cc == 0) {
        m_currentToken.beginDOCTYPE();
        m_currentToken.setForceQuirks();
        return switchTo(State::Data) && emitCurrentToken();
    }

    m_currentToken.beginDOCTYPE();
    m_currentToken.addToDOCTYPEName(toLower(cc));
    return advanceTo(State::DOCTYPEName);
}

bool HTMLTokenizer::handleDOCTYPENameState(char cc)
{
    if(isSpace(cc))
        return advanceTo(State::AfterDOCTYPEName);

    if(cc == '>')
        return advanceTo(State::Data) && emitCurrentToken();

    if(cc == 0) {
        m_currentToken.setForceQuirks();
        return switchTo(State::Data) && emitCurrentToken();
    }

    m_currentToken.addToDOCTYPEName(toLower(cc));
    return advanceTo(State::DOCTYPEName);
}

bool HTMLTokenizer::handleAfterDOCTYPENameState(char cc)
{
    if(isSpace(cc))
        return advanceTo(State::AfterDOCTYPEName);
    if(cc == '>')
        return advanceTo(State::Data) && emitCurrentToken();
    if(cc == 0) {
        m_currentToken.setForceQuirks();
        return switchTo(State::Data) && emitCurrentToken();
    }

    constexpr std::string_view publicKeyword("public");
    constexpr std::string_view systemKeyword("system");
    if(consumeString(publicKeyword, false))
        return switchTo(State::AfterDOCTYPEPublicKeyword);

    if(consumeString(systemKeyword, false))
        return switchTo(State::AfterDOCTYPESystemKeyword);

    m_currentToken.setForceQuirks();
    return advanceTo(State::BogusDOCTYPE);
}

bool HTMLTokenizer::handleAfterDOCTYPEPublicKeywordState(char cc)
{
    if(isSpace(cc))
        return advanceTo(State::BeforeDOCTYPEPublicIdentifier);

    if(cc == '"') {
        m_currentToken.setPublicIdentifier();
        return advanceTo(State::DOCTYPEPublicIdentifierDoubleQuoted);
    }

    if(cc == '\'') {
        m_currentToken.setPublicIdentifier();
        return advanceTo(State::DOCTYPEPublicIdentifierSingleQuoted);
    }

    if(cc == '>') {
        m_currentToken.setForceQuirks();
        return advanceTo(State::Data) && emitCurrentToken();
    }

    if(cc == 0) {
        m_currentToken.setForceQuirks();
        return switchTo(State::Data) && emitCurrentToken();
    }

    m_currentToken.setForceQuirks();
    return advanceTo(State::BogusDOCTYPE);
}

bool HTMLTokenizer::handleBeforeDOCTYPEPublicIdentifierState(char cc)
{
    if(isSpace(cc))
        return advanceTo(State::BeforeDOCTYPEPublicIdentifier);

    if(cc == '"') {
        m_currentToken.setPublicIdentifier();
        return advanceTo(State::DOCTYPEPublicIdentifierDoubleQuoted);
    }

    if(cc == '\'') {
        m_currentToken.setPublicIdentifier();
        return advanceTo(State::DOCTYPEPublicIdentifierSingleQuoted);
    }

    if(cc == '>') {
        m_currentToken.setForceQuirks();
        return advanceTo(State::Data) && emitCurrentToken();
    }

    if(cc == 0) {
        m_currentToken.setForceQuirks();
        return switchTo(State::Data) && emitCurrentToken();
    }

    m_currentToken.setForceQuirks();
    return advanceTo(State::BogusDOCTYPE);
}

bool HTMLTokenizer::handleDOCTYPEPublicIdentifierDoubleQuotedState(char cc)
{
    if(cc == '"')
        return advanceTo(State::AfterDOCTYPEPublicIdentifier);

    if(cc == '>') {
        m_currentToken.setForceQuirks();
        return advanceTo(State::Data) && emitCurrentToken();
    }

    if(cc == 0) {
        m_currentToken.setForceQuirks();
        return switchTo(State::Data) && emitCurrentToken();
    }

    m_currentToken.addToPublicIdentifier(cc);
    return advanceTo(State::DOCTYPEPublicIdentifierDoubleQuoted);
}

bool HTMLTokenizer::handleDOCTYPEPublicIdentifierSingleQuotedState(char cc)
{
    if(cc == '\'')
        return advanceTo(State::AfterDOCTYPEPublicIdentifier);

    if(cc == '>') {
        m_currentToken.setForceQuirks();
        return advanceTo(State::Data) && emitCurrentToken();
    }

    if(cc == 0) {
        m_currentToken.setForceQuirks();
        return switchTo(State::Data) && emitCurrentToken();
    }

    m_currentToken.addToPublicIdentifier(cc);
    return advanceTo(State::DOCTYPEPublicIdentifierSingleQuoted);
}

bool HTMLTokenizer::handleAfterDOCTYPEPublicIdentifierState(char cc)
{
    if(isSpace(cc))
        return advanceTo(State::BetweenDOCTYPEPublicAndSystemIdentifiers);

    if(cc == '>')
        return advanceTo(State::Data) && emitCurrentToken();

    if(cc == '"') {
        m_currentToken.setSystemIdentifier();
        return advanceTo(State::DOCTYPESystemIdentifierDoubleQuoted);
    }

    if(cc == '\'') {
        m_currentToken.setSystemIdentifier();
        return advanceTo(State::DOCTYPESystemIdentifierSingleQuoted);
    }

    if(cc == 0) {
        m_currentToken.setForceQuirks();
        return switchTo(State::Data) && emitCurrentToken();
    }

    m_currentToken.setForceQuirks();
    return advanceTo(State::BogusDOCTYPE);
}

bool HTMLTokenizer::handleBetweenDOCTYPEPublicAndSystemIdentifiersState(char cc)
{
    if(isSpace(cc))
        return advanceTo(State::BetweenDOCTYPEPublicAndSystemIdentifiers);

    if(cc == '>')
        return advanceTo(State::Data) && emitCurrentToken();

    if(cc == '"') {
        m_currentToken.setSystemIdentifier();
        return advanceTo(State::DOCTYPESystemIdentifierDoubleQuoted);
    }

    if(cc == '\'') {
        m_currentToken.setSystemIdentifier();
        return advanceTo(State::DOCTYPESystemIdentifierSingleQuoted);
    }

    if(cc == 0) {
        m_currentToken.setForceQuirks();
        return switchTo(State::Data) && emitCurrentToken();
    }

    m_currentToken.setForceQuirks();
    return advanceTo(State::BogusDOCTYPE);
}

bool HTMLTokenizer::handleAfterDOCTYPESystemKeywordState(char cc)
{
    if(isSpace(cc))
        return advanceTo(State::BeforeDOCTYPESystemIdentifier);

    if(cc == '"') {
        m_currentToken.setSystemIdentifier();
        return advanceTo(State::DOCTYPESystemIdentifierDoubleQuoted);
    }

    if(cc == '\'') {
        m_currentToken.setSystemIdentifier();
        return advanceTo(State::DOCTYPESystemIdentifierSingleQuoted);
    }

    if(cc == '>') {
        m_currentToken.setForceQuirks();
        return advanceTo(State::Data) && emitCurrentToken();
    }

    if(cc == 0) {
        m_currentToken.setForceQuirks();
        return switchTo(State::Data) && emitCurrentToken();
    }

    m_currentToken.setForceQuirks();
    return advanceTo(State::BogusDOCTYPE);
}

bool HTMLTokenizer::handleBeforeDOCTYPESystemIdentifierState(char cc)
{
    if(isSpace(cc))
        return advanceTo(State::BeforeDOCTYPESystemIdentifier);

    if(cc == '"') {
        m_currentToken.setSystemIdentifier();
        return advanceTo(State::DOCTYPESystemIdentifierDoubleQuoted);
    }

    if(cc == '\'') {
        m_currentToken.setSystemIdentifier();
        return advanceTo(State::DOCTYPESystemIdentifierSingleQuoted);
    }

    if(cc == '>') {
        m_currentToken.setForceQuirks();
        return advanceTo(State::Data) && emitCurrentToken();
    }

    if(cc == 0) {
        m_currentToken.setForceQuirks();
        return switchTo(State::Data) && emitCurrentToken();
    }

    m_currentToken.setForceQuirks();
    return advanceTo(State::BogusDOCTYPE);
}

bool HTMLTokenizer::handleDOCTYPESystemIdentifierDoubleQuotedState(char cc)
{
    if(cc == '"')
        return advanceTo(State::AfterDOCTYPESystemIdentifier);

    if(cc == '>') {
        m_currentToken.setForceQuirks();
        return advanceTo(State::Data) && emitCurrentToken();
    }

    if(cc == 0) {
        m_currentToken.setForceQuirks();
        return switchTo(State::Data) && emitCurrentToken();
    }

    m_currentToken.addToSystemIdentifier(cc);
    return advanceTo(State::DOCTYPESystemIdentifierDoubleQuoted);
}

bool HTMLTokenizer::handleDOCTYPESystemIdentifierSingleQuotedState(char cc)
{
    if(cc == '\'')
        return advanceTo(State::AfterDOCTYPESystemIdentifier);

    if(cc == '>') {
        m_currentToken.setForceQuirks();
        return advanceTo(State::Data) && emitCurrentToken();
    }

    if(cc == 0) {
        m_currentToken.setForceQuirks();
        return switchTo(State::Data) && emitCurrentToken();
    }

    m_currentToken.addToSystemIdentifier(cc);
    return advanceTo(State::DOCTYPESystemIdentifierSingleQuoted);
}

bool HTMLTokenizer::handleAfterDOCTYPESystemIdentifierState(char cc)
{
    if(isSpace(cc))
        return advanceTo(State::AfterDOCTYPESystemIdentifier);

    if(cc == '>')
        return advanceTo(State::Data) && emitCurrentToken();

    if(cc == 0) {
        m_currentToken.setForceQuirks();
        return switchTo(State::Data) && emitCurrentToken();
    }

    return advanceTo(State::BogusDOCTYPE);
}

bool HTMLTokenizer::handleBogusDOCTYPEState(char cc)
{
    if(cc == '>')
        return advanceTo(State::Data) && emitCurrentToken();

    if(cc == 0)
        return switchTo(State::Data) && emitCurrentToken();

    return advanceTo(State::BogusDOCTYPE);
}

bool HTMLTokenizer::handleCDATASectionState(char cc)
{
    if(cc == ']')
        return advanceTo(State::CDATASectionRightSquareBracket);
    if(cc == 0)
        return switchTo(State::Data);

    m_characterBuffer += cc;
    return advanceTo(State::CDATASection);
}

bool HTMLTokenizer::handleCDATASectionRightSquareBracketState(char cc)
{
    if(cc == ']')
        return advanceTo(State::CDATASectionDoubleRightSquareBracket);

    m_characterBuffer += cc;
    return switchTo(State::CDATASection);
}

bool HTMLTokenizer::handleCDATASectionDoubleRightSquareBracketState(char cc)
{
    if(cc == '>')
        return advanceTo(State::Data);

    m_characterBuffer += ']';
    m_characterBuffer += ']';
    return switchTo(State::CDATASection);
}

bool HTMLTokenizer::emitCurrentToken()
{
    assert(m_currentToken.type() != HTMLToken::Type::Unknown);
    assert(m_characterBuffer.empty());
    if(m_currentToken.type() == HTMLToken::Type::StartTag)
        m_appropriateEndTagName = m_currentToken.data();
    return false;
}

bool HTMLTokenizer::emitEOFToken()
{
    if(!m_characterBuffer.empty()) {
        m_reconsumeCurrentCharacter = true;
        return flushCharacterBuffer();
    }

    m_state = State::Data;
    m_currentToken.setEndOfFile();
    return false;
}

bool HTMLTokenizer::emitEndTagToken()
{
    flushEndTagNameBuffer();
    return false;
}

bool HTMLTokenizer::flushCharacterBuffer()
{
    assert(!m_characterBuffer.empty());
    if(!isSpace(m_characterBuffer.front())) {
        m_currentToken.beginCharacter();
        m_currentToken.addToCharacter(m_characterBuffer);
        m_characterBuffer.clear();
        return false;
    }

    m_currentToken.beginSpaceCharacter();
    for(auto cc : m_characterBuffer) {
        if(!isSpace(cc))
            break;
        m_currentToken.addToSpaceCharacter(cc);
    }

    const auto& data = m_currentToken.data();
    m_characterBuffer.erase(0, data.length());
    return false;
}

bool HTMLTokenizer::flushEndTagNameBuffer()
{
    if(!m_characterBuffer.empty())
        return flushCharacterBuffer();
    m_currentToken.beginEndTag();
    for(auto cc : m_endTagNameBuffer)
        m_currentToken.addToTagName(cc);
    m_appropriateEndTagName.clear();
    m_endTagNameBuffer.clear();
    m_temporaryBuffer.clear();
    return true;
}

bool HTMLTokenizer::flushTemporaryBuffer()
{
    m_characterBuffer += m_temporaryBuffer;
    m_temporaryBuffer.clear();
    m_endTagNameBuffer.clear();
    return true;
}

bool HTMLTokenizer::consumeCharacterReference(std::string& output, bool inAttributeValue)
{
    HTMLEntityParser entityParser(m_input, output, inAttributeValue);
    if(!entityParser.parse())
        return false;
    m_input.remove_prefix(entityParser.offset());
    return true;
}

bool HTMLTokenizer::consumeString(const std::string_view& value, bool caseSensitive)
{
    if(startswith(m_input, value, caseSensitive)) {
        m_input.remove_prefix(value.size());
        return true;
    }

    return false;
}

} // namespace plutobook
