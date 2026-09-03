/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_LOCALE_DATA_H
#define PLUTOBOOK_LOCALE_DATA_H

#include "globalstring.h"
#include "ustring.h"

#include <unicode/locid.h>
#include <unicode/brkiter.h>

#include <memory>

typedef const struct hb_language_impl_t* hb_language_t;

namespace plutobook {

class LocaleData {
public:
    static std::unique_ptr<LocaleData> create(const GlobalString& lang);

    uint64_t nextIteratorId() const { return ++m_iteratorIdCounter; }
    int nextCharacterBreak(uint64_t id, const UString& text, int offset) const;
    int nextLineBreak(uint64_t id, const UString& text, int offset) const;

    hb_language_t language() const { return m_language; }
    const GlobalString& getQuote(bool open, size_t depth) const;
    const char* lang() const;

private:
    LocaleData(hb_language_t language) : m_language(language) {}

    hb_language_t m_language;

    mutable std::unique_ptr<icu::BreakIterator> m_characterIterator;
    mutable std::unique_ptr<icu::BreakIterator> m_lineIterator;

    mutable uint64_t m_iteratorIdCounter = 0;
    mutable uint64_t m_characterIteratorId = 0;
    mutable uint64_t m_lineIteratorId = 0;

    icu::Locale locale() const;

    class Quotes {
    public:
        static std::unique_ptr<Quotes> create(std::string_view lang);

        const GlobalString& getQuote(bool open, size_t depth) const;

    private:
        Quotes(const char* open1, const char* close1, const char* open2, const char* close2)
            : m_open1(open1), m_close1(close1), m_open2(open2), m_close2(close2)
        {}

        GlobalString m_open1;
        GlobalString m_close1;
        GlobalString m_open2;
        GlobalString m_close2;
    };

    mutable std::unique_ptr<Quotes> m_quotes;
};

} // namespace plutobook

#endif // PLUTOBOOK_LOCALE_DATA_H
