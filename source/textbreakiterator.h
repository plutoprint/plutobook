#ifndef PLUTOBOOK_TEXTBREAKITERATOR_H
#define PLUTOBOOK_TEXTBREAKITERATOR_H

#include "ustring.h"

#include <unicode/brkiter.h>

namespace plutobook {

class CharacterBreakIterator {
public:
    explicit CharacterBreakIterator(const UString& text);

    int nextBreakOpportunity(int pos, int end) const;

private:
    static icu::BreakIterator* getIterator();
    icu::BreakIterator* m_iterator;
};

class LineBreakIterator {
public:
    explicit LineBreakIterator(const UString& text);

    uint32_t nextBreakOpportunity(uint32_t pos) const { return nextBreakOpportunity(pos, m_text.length()); }
    uint32_t nextBreakOpportunity(uint32_t pos, uint32_t end) const;
    uint32_t previousBreakOpportunity(uint32_t offset, uint32_t start = 0) const;
    bool isBreakable(uint32_t pos) const;

    static bool isBreakableSpace(UChar cc) { return cc == kSpaceCharacter || cc == kTabulationCharacter || cc == kNewlineCharacter; }

private:
    static icu::BreakIterator* getIterator();
    mutable icu::BreakIterator* m_iterator{nullptr};
    const UString m_text;
};

} // namespace plutobook

#endif // PLUTOBOOK_TEXTBREAKITERATOR_H
