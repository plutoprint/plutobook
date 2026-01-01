/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "xmlparser.h"
#include "xmldocument.h"
#include "plutobook.h"

#include <expat.h>

namespace plutobook {

XMLParser::XMLParser(XMLDocument* document)
    : m_document(document)
    , m_currentNode(document)
{
}

inline XMLParser* getParser(void* userData)
{
    return (XMLParser*)(userData);
}

static void startElementCallback(void* userData, const XML_Char* name, const XML_Char** attrs)
{
    getParser(userData)->handleStartElement((const char*)(name), (const char**)(attrs));
}

static void endElementCallback(void* userData, const XML_Char* name)
{
    getParser(userData)->handleEndElement((const char*)(name));
}

static void characterDataCallback(void* userData, const XML_Char* data, int length)
{
    getParser(userData)->handleCharacterData((const char*)(data), (size_t)(length));
}

constexpr XML_Char kXmlNamespaceSep = '|';

bool XMLParser::parse(const std::string_view& content)
{
    auto parser = XML_ParserCreateNS(NULL, kXmlNamespaceSep);
    XML_SetUserData(parser, this);
    XML_SetElementHandler(parser, startElementCallback, endElementCallback);
    XML_SetCharacterDataHandler(parser, characterDataCallback);
    auto status = XML_Parse(parser, content.data(), content.length(), XML_TRUE);
    if(status == XML_STATUS_OK) {
        m_document->finishParsingDocument();
        XML_ParserFree(parser);
        return true;
    }

    auto errorString = (const char*)(XML_ErrorString(XML_GetErrorCode(parser)));
    auto lineNumber = (int)(XML_GetCurrentLineNumber(parser));
    auto columnNumber = (int)(XML_GetCurrentColumnNumber(parser));
    plutobook_set_error_message("xml parse error: %s on line %d column %d", errorString, lineNumber, columnNumber);
    XML_ParserFree(parser);
    return false;
}

class QualifiedName {
public:
    QualifiedName(const GlobalString& namespaceURI, const GlobalString& localName)
        : m_namespaceURI(namespaceURI), m_localName(localName)
    {}

    const GlobalString& namespaceURI() const { return m_namespaceURI; }
    const GlobalString& localName() const { return m_localName; }

    static QualifiedName parse(const std::string_view& name);

private:
    GlobalString m_namespaceURI;
    GlobalString m_localName;
};

QualifiedName QualifiedName::parse(const std::string_view& name)
{
    auto index = name.rfind(kXmlNamespaceSep);
    if(index == std::string_view::npos)
        return QualifiedName(emptyGlo, GlobalString(name));
    GlobalString namespaceURI(name.substr(0, index));
    GlobalString localName(name.substr(index + 1));
    return QualifiedName(namespaceURI, localName);
}

void XMLParser::handleStartElement(const char* name, const char** attrs)
{
    auto tagName = QualifiedName::parse(name);
    auto element = m_document->createElement(tagName.namespaceURI(), tagName.localName());
    while(attrs && *attrs) {
        auto attrName = QualifiedName::parse(attrs[0]);
        auto attrValue = m_document->heap()->createString(attrs[1]);
        element->setAttribute(attrName.localName(), attrValue);
        attrs += 2;
    }

    element->setIsCaseSensitive(true);
    m_currentNode->appendChild(element);
    m_currentNode = element;
}

void XMLParser::handleEndElement(const char* name)
{
    m_currentNode = m_currentNode->parentNode();
}

void XMLParser::handleCharacterData(const char* data, size_t length)
{
    std::string_view content(data, length);
    if(auto lastTextNode = to<TextNode>(m_currentNode->lastChild())) {
        lastTextNode->appendData(content);
    } else {
        m_currentNode->appendChild(m_document->createTextNode(content));
    }
}

} // namespace plutobook
