#include "textresource.h"
#include "stringutils.h"

namespace plutobook {

RefPtr<TextResource> TextResource::create(const Url& url, const std::string& mimeType, const std::string& textEncoding, std::vector<char> content)
{
    auto text = decode(content.data(), content.size(), mimeType, textEncoding);
    if(text.empty())
        return nullptr;
    return adoptPtr(new TextResource(std::move(text)));
}

std::string TextResource::decode(const char* data, size_t length, const std::string_view& mimeType, const std::string_view& textEncoding)
{
    return std::string(data, length);
}

bool TextResource::isXMLMIMEType(const std::string_view& mimeType)
{
    if(equals(mimeType, "text/xml", false)
        || equals(mimeType, "application/xml", false)
        || equals(mimeType, "text/xsl", false)) {
        return true;
    }

    auto length = mimeType.length();
    if(length < 7) {
        return false;
    }

    if(mimeType[0] == '/' || mimeType[length - 5] == '/' || !endswith(mimeType, "+xml", false)) {
        return false;
    }

    bool hasSlash = false;
    for(int i = 0; i < length - 4; ++i) {
        auto cc = mimeType[i];
        if(isAlnum(cc))
            continue;
        switch (cc) {
        case '_':
        case '-':
        case '+':
        case '~':
        case '!':
        case '$':
        case '^':
        case '{':
        case '}':
        case '|':
        case '.':
        case '%':
        case '\'':
        case '`':
        case '#':
        case '&':
        case '*':
            continue;
        case '/':
            if(hasSlash)
                return false;
            hasSlash = true;
            continue;
        default:
            return false;
        }
    }

    return true;
}

} // namespace plutobook
