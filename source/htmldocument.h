#ifndef PLUTOBOOK_HTMLDOCUMENT_H
#define PLUTOBOOK_HTMLDOCUMENT_H

#include "document.h"

#include <optional>

namespace plutobook {

class HTMLDocument;

class HTMLElement : public Element {
public:
    HTMLElement(Document* document, const GlobalString& tagName);

    bool isHTMLElement() const final { return true; }

    void buildFirstLetterPseudoBox(Box* parent);
    void buildPseudoBox(Counters& counters, Box* parent, PseudoType pseudoType);
    void buildBox(Counters& counters, Box* parent) override;
};

template<>
struct is_a<HTMLElement> {
    static bool check(const Node& value) { return value.isHTMLElement(); }
};

class HTMLBodyElement final : public HTMLElement {
public:
    HTMLBodyElement(Document* document);

    void collectAttributeStyle(std::string& output, const GlobalString& name, const HeapString& value) const final;
};

class HTMLFontElement final : public HTMLElement {
public:
    HTMLFontElement(Document* document);

    void collectAttributeStyle(std::string& output, const GlobalString& name, const HeapString& value) const final;
};

class Image;

class HTMLImageElement final : public HTMLElement {
public:
    HTMLImageElement(Document* document);

    void collectAttributeStyle(std::string& output, const GlobalString& name, const HeapString& value) const final;
    const HeapString& src() const;
    const HeapString& altText() const;
    RefPtr<Image> srcImage() const;

    Box* createBox(const RefPtr<BoxStyle>& style) final;
};

class HTMLBRElement final : public HTMLElement {
public:
    HTMLBRElement(Document* document);

    Box* createBox(const RefPtr<BoxStyle>& style) final;
};

class HTMLWBRElement final : public HTMLElement {
public:
    HTMLWBRElement(Document* document);

    Box* createBox(const RefPtr<BoxStyle>& style) final;
};

class HTMLLIElement final : public HTMLElement {
public:
    HTMLLIElement(Document* document);

    std::optional<int> value() const;
};

class HTMLOLElement final : public HTMLElement {
public:
    HTMLOLElement(Document* document);

    int start() const;
};

class HTMLTableElement final : public HTMLElement {
public:
    HTMLTableElement(Document* document);

    void collectAttributeStyle(std::string& output, const GlobalString& name, const HeapString& value) const final;
};

class HTMLTablePartElement : public HTMLElement {
public:
    HTMLTablePartElement(Document* document, const GlobalString& tagName);

    void collectAttributeStyle(std::string& output, const GlobalString& name, const HeapString& value) const override;
};

class HTMLTableSectionElement final : public HTMLTablePartElement {
public:
    HTMLTableSectionElement(Document* document, const GlobalString& tagName);
};

class HTMLTableRowElement final : public HTMLTablePartElement {
public:
    HTMLTableRowElement(Document* document);
};

class HTMLTableColElement final : public HTMLTablePartElement {
public:
    HTMLTableColElement(Document* document, const GlobalString& tagName);

    unsigned span() const;

    Box* createBox(const RefPtr<BoxStyle>& style) final;
};

class HTMLTableCellElement final : public HTMLTablePartElement {
public:
    HTMLTableCellElement(Document* document, const GlobalString& tagName);

    unsigned colSpan() const;
    unsigned rowSpan() const;

    Box* createBox(const RefPtr<BoxStyle>& style) final;
};

class HTMLInputElement final : public HTMLElement {
public:
    HTMLInputElement(Document* document);

    unsigned size() const;

    Box* createBox(const RefPtr<BoxStyle>& style) final;
};

class HTMLTextAreaElement final : public HTMLElement {
public:
    HTMLTextAreaElement(Document* document);

    unsigned rows() const;
    unsigned cols() const;

    Box* createBox(const RefPtr<BoxStyle>& style) final;
};

class HTMLSelectElement final : public HTMLElement {
public:
    HTMLSelectElement(Document* document);

    unsigned size() const;

    Box* createBox(const RefPtr<BoxStyle>& style) final;
};

class HTMLStyleElement final : public HTMLElement {
public:
    HTMLStyleElement(Document* document);

    const HeapString& type() const;
    const HeapString& media() const;

    void finishParsingDocument() final;
};

class HTMLLinkElement final : public HTMLElement {
public:
    HTMLLinkElement(Document* document);

    const HeapString& rel() const;
    const HeapString& type() const;
    const HeapString& media() const;
    const HeapString& href() const;

    void finishParsingDocument() final;
};

class HTMLTitleElement final : public HTMLElement {
public:
    HTMLTitleElement(Document* document);

    void finishParsingDocument() final;
};

class HTMLDocument final : public Document {
public:
    static std::unique_ptr<HTMLDocument> create(Book* book, Heap* heap, ResourceFetcher* fetcher, Url url);

    bool isHTMLDocument() const final { return true; }
    bool load(const std::string_view& content) final;

private:
    HTMLDocument(Book* book, Heap* heap, ResourceFetcher* fetcher, Url url);
};

template<>
struct is_a<HTMLDocument> {
    static bool check(const Node& value) { return value.isHTMLDocument(); }
};

} // namespace plutobook

#endif // PLUTOBOOK_HTMLDOCUMENT_H
