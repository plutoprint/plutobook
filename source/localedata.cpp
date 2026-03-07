/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "localedata.h"

#include <unicode/ulocdata.h>
#include <cassert>
#include <map>

namespace plutobook {

const LocaleData* LocaleData::get(const GlobalString& lang)
{
    if(lang.isEmpty()) {
        thread_local std::unique_ptr<LocaleData> locale(new LocaleData(icu::Locale::getDefault()));
        return locale.get();
    }

    thread_local std::map<GlobalString, std::unique_ptr<LocaleData>> table;

    auto& locale = table[lang];
    if(!locale) {
        std::string language(lang.value());
        icu::Locale loc(language.data());
        if(loc.isBogus())
            loc = icu::Locale::getDefault();
        locale.reset(new LocaleData(loc));
    }

    return locale.get();
}

icu::BreakIterator* LocaleData::characterIterator() const
{
    if(!m_characterIterator) {
        UErrorCode status = U_ZERO_ERROR;
        m_characterIterator.reset(icu::BreakIterator::createCharacterInstance(m_locale, status));
        assert(m_characterIterator && U_SUCCESS(status));
    }

    return m_characterIterator.get();
}

icu::BreakIterator* LocaleData::lineIterator() const
{
    if(!m_lineIterator) {
        UErrorCode status = U_ZERO_ERROR;
        m_lineIterator.reset(icu::BreakIterator::createLineInstance(m_locale, status));
        assert(m_lineIterator && U_SUCCESS(status));
    }

    return m_lineIterator.get();
}

std::unique_ptr<LocaleData::Quotes> LocaleData::Quotes::create(const char* locale)
{
    constexpr auto MAX_QUOTE_SIZE = U8_MAX_LENGTH + 1;

    char open1[MAX_QUOTE_SIZE] = {};
    char close1[MAX_QUOTE_SIZE] = {};
    char open2[MAX_QUOTE_SIZE] = {};
    char close2[MAX_QUOTE_SIZE] = {};

    UErrorCode status = U_ZERO_ERROR;
    ULocaleData* uld = ulocdata_open(locale, &status);
    if(U_SUCCESS(status)) {
        struct {
            ULocaleDataDelimiterType type;
            char* value;
        } delimiters[] = {
            {ULOCDATA_QUOTATION_START, open1},
            {ULOCDATA_QUOTATION_END, close1},
            {ULOCDATA_ALT_QUOTATION_START, open2},
            {ULOCDATA_ALT_QUOTATION_END, close2}
        };

        for(auto& delim : delimiters) {
            UChar result;
            auto length = ulocdata_getDelimiter(uld, delim.type, &result, 1, &status);
            if(length == 1 && U_SUCCESS(status)) {
                int len = 0;
                U8_APPEND_UNSAFE(delim.value, len, result);
                delim.value[len] = '\0';
            }
        }

        ulocdata_close(uld);
    }

    return std::unique_ptr<Quotes>(new Quotes(open1, close1, open2, close2));
}

const GlobalString& LocaleData::Quotes::getQuote(bool open, size_t depth) const
{
    if(!depth)
        return open ? m_open1 : m_close1;
    return open ? m_open2 : m_close2;
}

const GlobalString& LocaleData::getQuote(bool open, size_t depth) const
{
    if(!m_quotes)
        m_quotes = Quotes::create(m_locale.getName());
    return m_quotes->getQuote(open, depth);
}

} // namespace plutobook
