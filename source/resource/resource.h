#ifndef PLUTOBOOK_RESOURCE_H
#define PLUTOBOOK_RESOURCE_H

#include "pointer.h"

#include <string>
#include <vector>
#include <memory>

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
    bool loadUrl(const Url& url, std::string& mimeType, std::string& textEncoding, std::vector<char>& content) const;

    void setCustomFetcher(ResourceFetcher* fetcher) { m_customFetcher = fetcher; }
    ResourceFetcher* customFetcher() const { return m_customFetcher; }
    ResourceFetcher* defaultFetcher() const { return m_defaultFetcher.get(); }

    static Url baseUrl();
    static Url completeUrl(const std::string_view& value);

private:
    ResourceLoader();
    ResourceFetcher* m_customFetcher{nullptr};
    std::unique_ptr<ResourceFetcher> m_defaultFetcher;
    friend ResourceLoader* resourceLoader();
};

ResourceLoader* resourceLoader();

} // namespace plutobook

#endif // PLUTOBOOK_RESOURCE_H
