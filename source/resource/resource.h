#ifndef PLUTOBOOK_RESOURCE_H
#define PLUTOBOOK_RESOURCE_H

#include "pointer.h"

#include <string>
#include <vector>

namespace plutobook {

class Url;

class Resource : public RefCounted<Resource> {
public:
    enum class Type {
        Text,
        Image,
        Font
    };

    virtual ~Resource() = default;
    virtual Type type() const = 0;

protected:
    Resource() = default;
};

class ResourceFetcher;

class ResourceLoader {
public:
    static bool loadUrl(const Url& url, std::string& mimeType, std::string& textEncoding, std::vector<char>& content, ResourceFetcher* customFetcher = nullptr);
    static Url completeUrl(const std::string_view& value);
    static Url baseUrl();
};

ResourceFetcher* defaultResourceFetcher();

} // namespace plutobook

#endif // PLUTOBOOK_RESOURCE_H
