#ifndef PLUTOBOOK_COUNTERS_H
#define PLUTOBOOK_COUNTERS_H

#include <map>
#include <set>
#include <vector>

namespace plutobook {

class Box;
class Document;
class GlobalString;
class HeapString;

class Counters {
public:
    explicit Counters(Document* document);

    void push();
    void pop();

    void reset(const GlobalString& name, int value);
    void increment(const GlobalString& name, int value);
    void set(const GlobalString& name, int value);

    void increaseQuoteDepth() { ++m_quoteDepth; }
    void decreaseQuoteDepth() { --m_quoteDepth; }
    size_t quoteDepth() const { return m_quoteDepth; }

    void update(const Box* box);

    HeapString counterText(const GlobalString& name, const GlobalString& listStyle, const HeapString& separator) const;
    HeapString markerText(const GlobalString& listStyle) const;

private:
    Document* m_document;
    std::vector<std::set<GlobalString>> m_scopes;
    std::map<GlobalString, std::vector<int>> m_values;
    size_t m_quoteDepth{0};
};

} // namespace plutobook

#endif // PLUTOBOOK_COUNTERS_H
