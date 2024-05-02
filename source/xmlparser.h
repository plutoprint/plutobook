#ifndef PLUTOBOOK_XMLPARSER_H
#define PLUTOBOOK_XMLPARSER_H

#include <string_view>

namespace plutobook {

class XMLDocument;
class ContainerNode;

class XMLParser {
public:
    explicit XMLParser(XMLDocument* document);

    bool parse(const std::string_view& content);

    void handleStartNamespace(const char* prefix, const char* uri);
    void handleEndNamespace(const char* prefix);

    void handleStartElement(const char* name, const char** attrs);
    void handleEndElement(const char* name);
    void handleCharacterData(const char* data, size_t length);

private:
    XMLDocument* m_document;
    ContainerNode* m_currentNode;
};

} // namespace plutobook

#endif // PLUTOBOOK_SVGPARSER_H
