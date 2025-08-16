#ifndef PLUTOBOOK_RESOURCE_H
#define PLUTOBOOK_RESOURCE_H

#include "pointer.h"
#include "heapstring.h"

namespace plutobook {

class Resource : public HeapMember, public RefCounted<Resource> {
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

class Url;
class ResourceData;
class ResourceFetcher;

class ResourceLoader {
public:
    static ResourceData loadUrl(const Url& url, ResourceFetcher* customFetcher = nullptr);
    static Url completeUrl(const std::string_view& value);
    static Url baseUrl();
};

} // namespace plutobook

#endif // PLUTOBOOK_RESOURCE_H
