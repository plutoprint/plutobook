#ifndef PLUTOBOOK_HTMLPARSER_H
#define PLUTOBOOK_HTMLPARSER_H

#include "htmltokenizer.h"

namespace plutobook {

class HTMLElementStack {
public:
    HTMLElementStack() = default;

    void push(Element* element);
    void pushHTMLHtmlElement(Element* element);
    void pushHTMLHeadElement(Element* element);
    void pushHTMLBodyElement(Element* element);

    void pop();
    void popHTMLHeadElement();
    void popHTMLBodyElement();
    void popUntil(const GlobalString& tagName);
    void popUntil(const Element* element);
    void popUntilNumberedHeaderElement();
    void popUntilTableScopeMarker();
    void popUntilTableBodyScopeMarker();
    void popUntilTableRowScopeMarker();
    void popUntilForeignContentScopeMarker();
    void popUntilPopped(const GlobalString& tagName);
    void popUntilPopped(const Element* element);
    void popUntilNumberedHeaderElementPopped();
    void popAll();

    void generateImpliedEndTags();
    void generateImpliedEndTagsExcept(const GlobalString& tagName);

    void removeHTMLHeadElement(const Element* element);
    void removeHTMLBodyElement();

    Element* furthestBlockForFormattingElement(const Element* formattingElement) const;
    Element* topmost(const GlobalString& tagName) const;
    Element* previous(const Element* element) const;

    Element* htmlElement() const { return m_htmlElement; }
    Element* headElement() const { return m_headElement; }
    Element* bodyElement() const { return m_bodyElement; }

    template<bool isMarker(const Element*)>
    bool inScopeTemplate(const GlobalString& tagName) const;
    bool inScope(const Element* element) const;
    bool inScope(const GlobalString& tagName) const;
    bool inButtonScope(const GlobalString& tagName) const;
    bool inListItemScope(const GlobalString& tagName) const;
    bool inTableScope(const GlobalString& tagName) const;
    bool inSelectScope(const GlobalString& tagName) const;
    bool isNumberedHeaderElementInScope() const;

    void remove(const Element* element);
    void remove(int index);
    void replace(const Element* element, Element* item);
    void replace(int index, Element* element);
    void insertAfter(const Element* element, Element* item);
    void insert(int index, Element* element);

    int index(const Element* element) const;
    bool contains(const Element* element) const;
    Element* at(int index) const { return m_elements.at(index); }
    Element* top() const { return m_elements.back(); }

    bool empty() const { return m_elements.empty(); }
    int size() const { return m_elements.size(); }

private:
    std::vector<Element*> m_elements;
    Element* m_htmlElement{nullptr};
    Element* m_headElement{nullptr};
    Element* m_bodyElement{nullptr};
};

class HTMLFormattingElementList {
public:
    HTMLFormattingElementList() = default;

    void append(Element* element);
    void appendMarker();
    void clearToLastMarker();

    void remove(const Element* element);
    void remove(int index);
    void replace(const Element* element, Element* item);
    void replace(int index, Element* element);
    void insert(int index, Element* element);

    int index(const Element* element) const;
    bool contains(const Element* element) const;
    Element* at(int index) const { return m_elements.at(index); }
    Element* closestElementInScope(const GlobalString& tagName);

    bool empty() const { return m_elements.empty(); }
    int size() const { return m_elements.size(); }

private:
    std::vector<Element*> m_elements;
};

class HTMLDocument;

class HTMLParser {
public:
    HTMLParser(HTMLDocument* document, const std::string_view& content);

    void parse();

private:
    Element* createHTMLElement(HTMLTokenView& token) const;
    Element* createElement(HTMLTokenView& token, const GlobalString& namespaceURI) const;
    Element* cloneElement(const Element* element) const;
    Element* currentElement() const { return m_openElements.top(); }

    struct InsertionLocation {
        ContainerNode* parent{nullptr};
        Node* nextChild{nullptr};
    };

    void insertNode(const InsertionLocation& location, Node* child);
    void insertElement(Element* child, ContainerNode* parent);
    void insertElement(Element* child);

    bool shouldFosterParent() const;
    void findFosterLocation(InsertionLocation& location) const;
    void fosterParent(Node* child);

    void reconstructActiveFormattingElements();
    void flushPendingTableCharacters();
    void closeCell();

    static void adjustSVGTagNames(HTMLTokenView& token);
    static void adjustSVGAttributes(HTMLTokenView& token);
    static void adjustMathMLAttributes(HTMLTokenView& token);
    static void adjustForeignAttributes(HTMLTokenView& token);

    void insertDoctype(HTMLTokenView& token);
    void insertComment(HTMLTokenView& token, ContainerNode* parent);
    void insertHTMLHtmlElement(HTMLTokenView& token);
    void insertHeadElement(HTMLTokenView& token);
    void insertHTMLBodyElement(HTMLTokenView& token);
    void insertHTMLFormElement(HTMLTokenView& token);
    void insertSelfClosingHTMLElement(HTMLTokenView& token);
    void insertHTMLElement(HTMLTokenView& token);
    void insertHTMLFormattingElement(HTMLTokenView& token);
    void insertForeignElement(HTMLTokenView& token, const GlobalString& namespaceURI);
    void insertTextNode(const std::string_view& data);

    enum class InsertionMode {
        Initial,
        BeforeHTML,
        BeforeHead,
        InHead,
        InHeadNoscript,
        AfterHead,
        InBody,
        Text,
        InTable,
        InTableText,
        InCaption,
        InColumnGroup,
        InTableBody,
        InRow,
        InCell,
        InSelect,
        InSelectInTable,
        AfterBody,
        InFrameset,
        AfterFrameset,
        AfterAfterBody,
        AfterAfterFrameset,
        InForeignContent
    };

    void setInsertionMode(InsertionMode mode) { m_insertionMode = mode; }
    InsertionMode insertionMode() const { return m_insertionMode; }

    void resetInsertionMode();
    InsertionMode currentInsertionMode(HTMLTokenView& token) const;

    void handleInitialMode(HTMLTokenView& token);
    void handleBeforeHTMLMode(HTMLTokenView& token);
    void handleBeforeHeadMode(HTMLTokenView& token);
    void handleInHeadMode(HTMLTokenView& token);
    void handleInHeadNoscriptMode(HTMLTokenView& token);
    void handleAfterHeadMode(HTMLTokenView& token);
    void handleInBodyMode(HTMLTokenView& token);
    void handleTextMode(HTMLTokenView& token);
    void handleInTableMode(HTMLTokenView& token);
    void handleInTableTextMode(HTMLTokenView& token);
    void handleInCaptionMode(HTMLTokenView& token);
    void handleInColumnGroupMode(HTMLTokenView& token);
    void handleInTableBodyMode(HTMLTokenView& token);
    void handleInRowMode(HTMLTokenView& token);
    void handleInCellMode(HTMLTokenView& token);
    void handleInSelectMode(HTMLTokenView& token);
    void handleInSelectInTableMode(HTMLTokenView& token);
    void handleAfterBodyMode(HTMLTokenView& token);
    void handleInFramesetMode(HTMLTokenView& token);
    void handleAfterFramesetMode(HTMLTokenView& token);
    void handleAfterAfterBodyMode(HTMLTokenView& token);
    void handleAfterAfterFramesetMode(HTMLTokenView& token);
    void handleInForeignContentMode(HTMLTokenView& token);

    void handleFakeStartTagToken(const GlobalString& tagName);
    void handleFakeEndTagToken(const GlobalString& tagName);

    void handleFormattingEndTagToken(HTMLTokenView& token);
    void handleOtherFormattingEndTagToken(HTMLTokenView& token);

    void handleErrorToken(HTMLTokenView& token);
    void handleRCDataToken(HTMLTokenView& token);
    void handleRawTextToken(HTMLTokenView& token);
    void handleScriptDataToken(HTMLTokenView& token);

    void handleDoctypeToken(HTMLTokenView& token);
    void handleCommentToken(HTMLTokenView& token);
    void handleToken(HTMLTokenView& token, InsertionMode mode);
    void handleToken(HTMLTokenView& token) { handleToken(token, m_insertionMode); }

    HTMLDocument* m_document;
    Element* m_form{nullptr};
    Element* m_head{nullptr};

    HTMLTokenizer m_tokenizer;
    HTMLElementStack m_openElements;
    HTMLFormattingElementList m_activeFormattingElements;
    std::string m_pendingTableCharacters;

    InsertionMode m_insertionMode{InsertionMode::Initial};
    InsertionMode m_originalInsertionMode{InsertionMode::Initial};
    bool m_inQuirksMode{false};
    bool m_framesetOk{false};
    bool m_fosterRedirecting{false};
    bool m_skipLeadingNewline{false};
};

} // namespace plutobook

#endif // PLUTOBOOK_HTMLPARSER_H
