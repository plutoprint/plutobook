#include "xmldocument.h"
#include "xmlparser.h"

namespace plutobook {

std::unique_ptr<XMLDocument> XMLDocument::create(Book* book, Heap* heap, ResourceFetcher* fetcher, Url url)
{
    return std::unique_ptr<XMLDocument>(new (heap) XMLDocument(book, heap, fetcher, std::move(url)));
}

bool XMLDocument::load(const std::string_view& content)
{
    return XMLParser(this).parse(content);
}

XMLDocument::XMLDocument(Book* book, Heap* heap, ResourceFetcher* fetcher, Url url)
    : Document(book, heap, fetcher, std::move(url))
{
}

} // namespace plutobook
