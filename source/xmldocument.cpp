#include "xmldocument.h"
#include "xmlparser.h"

namespace plutobook {

std::unique_ptr<XMLDocument> XMLDocument::create(Book* book, Heap* heap, ResourceFetcher* fetcher, Url baseUrl)
{
    return std::unique_ptr<XMLDocument>(new (heap) XMLDocument(book, heap, fetcher, std::move(baseUrl)));
}

bool XMLDocument::parse(const std::string_view& content)
{
    return XMLParser(this).parse(content);
}

XMLDocument::XMLDocument(Book* book, Heap* heap, ResourceFetcher* fetcher, Url baseUrl)
    : Document(book, heap, fetcher, std::move(baseUrl))
{
}

} // namespace plutobook
