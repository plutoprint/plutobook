/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_COUNTERS_H
#define PLUTOBOOK_COUNTERS_H

#include <map>
#include <set>
#include <vector>
#include <cstdint>

namespace plutobook {

class Box;
class Document;
class GlobalString;
class HeapString;

class Counters {
public:
    Counters(Document* document, uint32_t pageCount);

    void push();
    void pop();

    void reset(const GlobalString& name, int value);
    void increment(const GlobalString& name, int value);
    void set(const GlobalString& name, int value);

    void increaseQuoteDepth() { ++m_quoteDepth; }
    void decreaseQuoteDepth() { --m_quoteDepth; }

    uint32_t pageCount() const { return m_pageCount; }
    uint32_t quoteDepth() const { return m_quoteDepth; }

    void update(const Box* box);

    HeapString counterText(const GlobalString& name, const GlobalString& listStyle, const HeapString& separator) const;
    HeapString markerText(const GlobalString& listStyle) const;

private:
    Document* m_document;
    std::vector<std::set<GlobalString>> m_scopes;
    std::map<GlobalString, std::vector<int>> m_values;
    uint32_t m_pageCount;
    uint32_t m_quoteDepth{0};
};

} // namespace plutobook

#endif // PLUTOBOOK_COUNTERS_H
