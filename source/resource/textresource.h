#ifndef PLUTOBOOK_TEXTRESOURCE_H
#define PLUTOBOOK_TEXTRESOURCE_H

#include "resource.h"

#include <string>

namespace plutobook {

class Document;

class TextResource final : public Resource {
public:
    static RefPtr<TextResource> create(Document* document, const Url& url);
    static std::string_view decode(const char* data, size_t length, const std::string_view& mimeType, const std::string_view& textEncoding);
    static bool isXMLMIMEType(const std::string_view& mimeType);
    const std::string& text() const { return m_text; }
    Type type() const final { return Type::Text; }

private:
    TextResource(const std::string_view& text) : m_text(text) {}
    std::string m_text;
};

template<>
struct is_a<TextResource> {
    static bool check(const Resource& value) { return value.type() == Resource::Type::Text; }
};

} // namespace plutobook

#endif // PLUTOBOOK_TEXTRESOURCE_H
