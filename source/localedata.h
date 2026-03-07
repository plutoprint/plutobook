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

#include <unicode/locid.h>
#include <unicode/brkiter.h>
#include <memory>

namespace plutobook {

class LocaleData {
public:
    static const LocaleData* get(const GlobalString& lang);

    icu::BreakIterator* characterIterator() const;
    icu::BreakIterator* lineIterator() const;

    const GlobalString& getQuote(bool open, size_t depth) const;

public:
    LocaleData(const icu::Locale& locale) : m_locale(locale) {}

    icu::Locale m_locale;

    mutable std::unique_ptr<icu::BreakIterator> m_characterIterator;
    mutable std::unique_ptr<icu::BreakIterator> m_lineIterator;

    class Quotes {
    public:
        static std::unique_ptr<Quotes> create(const char* locale);

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
