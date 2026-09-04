/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_TEXTBREAKITERATOR_H
#define PLUTOBOOK_TEXTBREAKITERATOR_H

#include "ustring.h"

namespace plutobook {

class LocaleData;

class TextBreakIterator {
public:
    TextBreakIterator(const TextBreakIterator&) = delete;
    TextBreakIterator& operator=(const TextBreakIterator&) = delete;

protected:
    TextBreakIterator(const UString& text, const LocaleData* locale);

    const UString m_text;
    const LocaleData* m_locale;
    const uint64_t m_id;
};

class CharacterBreakIterator final : public TextBreakIterator {
public:
    explicit CharacterBreakIterator(const UString& text, const LocaleData* locale);

    uint32_t nextBreakOpportunity(uint32_t pos, uint32_t end) const;
};

class LineBreakIterator final : public TextBreakIterator {
public:
    explicit LineBreakIterator(const UString& text, const LocaleData* locale);

    static bool isBreakableSpace(UChar cc);

    uint32_t nextBreakOpportunity(uint32_t pos) const { return nextBreakOpportunity(pos, m_text.length()); }
    uint32_t nextBreakOpportunity(uint32_t pos, uint32_t end) const;
    uint32_t previousBreakOpportunity(uint32_t offset, uint32_t start = 0) const;

    bool isBreakable(uint32_t pos) const;
};

inline bool LineBreakIterator::isBreakableSpace(UChar cc)
{
    return cc == kSpaceCharacter || cc == kTabulationCharacter || cc == kNewlineCharacter;
}

} // namespace plutobook

#endif // PLUTOBOOK_TEXTBREAKITERATOR_H
