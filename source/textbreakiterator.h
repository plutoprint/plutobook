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

#include <unicode/brkiter.h>

namespace plutobook {

class LocaleData;

class CharacterBreakIterator {
public:
    explicit CharacterBreakIterator(const UString& text, const LocaleData* locale);

    int nextBreakOpportunity(int pos, int end) const;

private:
    icu::BreakIterator* m_iterator;
};

class LineBreakIterator {
public:
    explicit LineBreakIterator(const UString& text, const LocaleData* locale);

    uint32_t nextBreakOpportunity(uint32_t pos) const { return nextBreakOpportunity(pos, m_text.length()); }
    uint32_t nextBreakOpportunity(uint32_t pos, uint32_t end) const;
    uint32_t previousBreakOpportunity(uint32_t offset, uint32_t start = 0) const;

    bool isBreakable(uint32_t pos) const;

private:
    const UString m_text;
    const LocaleData* m_locale;
    mutable icu::BreakIterator* m_iterator = nullptr;
};

} // namespace plutobook

#endif // PLUTOBOOK_TEXTBREAKITERATOR_H
