#ifndef PLUTOBOOK_XMLDOCUMENT_H
#define PLUTOBOOK_XMLDOCUMENT_H

#include "document.h"

namespace plutobook {

class XMLDocument : public Document {
public:
    static std::unique_ptr<XMLDocument> create(Book* book, Heap* heap, Url url);

    bool isXMLDocument() const final { return true; }
    bool load(const std::string_view& content) override;

protected:
    XMLDocument(Book* book, Heap* heap, Url url);
};

template<>
struct is_a<XMLDocument> {
    static bool check(const Node& value) { return value.isXMLDocument(); }
};

} // namespace plutobook

#endif // PLUTOBOOK_XMLDOCUMENT_H
