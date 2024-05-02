#ifndef PLUTOBOOK_TEXTRESOURCE_H
#define PLUTOBOOK_TEXTRESOURCE_H

#include "resource.h"

#include <string>

namespace plutobook {

class TextResource final : public Resource {
public:
    static RefPtr<TextResource> create(const Url& url, const std::string& mimeType, const std::string& textEncoding, std::vector<char> content);
    static std::string decode(const char* data, size_t length, const std::string_view& mimeType, const std::string_view& textEncoding);
    static bool isXMLMIMEType(const std::string_view& mimeType);
    const std::string& text() const { return m_text; }
    Type type() const final { return Type::Text; }

private:
    TextResource(std::string text) : m_text(std::move(text)) {}
    std::string m_text;
};

template<>
struct is_a<TextResource> {
    static bool check(const Resource& value) { return value.type() == Resource::Type::Text; }
};

} // namespace plutobook

#endif // PLUTOBOOK_TEXTRESOURCE_H
