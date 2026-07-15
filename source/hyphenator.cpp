/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "hyphenator.h"

#ifdef PLUTOBOOK_HAS_HYPHEN

#include <hyphen.h>

#include <unicode/utf8.h>
#include <unicode/utf16.h>

#include <cstdlib>
#include <map>
#include <memory>
#include <string_view>

namespace plutobook {

static HyphenDict* loadDictFrom(const std::string& directory, const std::string& localeName)
{
    auto prefix = directory + "/hyph_";
    if(auto* dict = hnj_hyphen_load((prefix + localeName + ".dic").c_str()))
        return dict;
    // Fall back from a full locale (e.g. "en_US") to the bare language ("en").
    auto separator = localeName.find('_');
    if(separator != std::string::npos) {
        auto language = localeName.substr(0, separator);
        if(auto* dict = hnj_hyphen_load((prefix + language + ".dic").c_str()))
            return dict;
    }

    return nullptr;
}

static HyphenDict* loadSystemDict(const std::string& localeName)
{
    // Directories from PLUTOBOOK_HYPHEN_PATH (colon-separated) are searched
    // before the standard system location.
    if(const char* path = std::getenv("PLUTOBOOK_HYPHEN_PATH")) {
        std::string_view remaining(path);
        while(!remaining.empty()) {
            auto separator = remaining.find(':');
            auto directory = remaining.substr(0, separator);
            if(!directory.empty()) {
                if(auto* dict = loadDictFrom(std::string(directory), localeName))
                    return dict;
            }

            if(separator == std::string_view::npos)
                break;
            remaining.remove_prefix(separator + 1);
        }
    }

    return loadDictFrom("/usr/share/hyphen", localeName);
}

const Hyphenator* Hyphenator::get(const std::string& localeName)
{
    thread_local std::map<std::string, std::unique_ptr<Hyphenator>> cache;
    auto it = cache.find(localeName);
    if(it != cache.end())
        return it->second.get();
    std::unique_ptr<Hyphenator> hyphenator;
    if(auto* dict = loadSystemDict(localeName))
        hyphenator.reset(new Hyphenator(dict));
    auto* result = hyphenator.get();
    cache.emplace(localeName, std::move(hyphenator));
    return result;
}

void Hyphenator::hyphenate(const UChar* word, size_t length, std::vector<uint32_t>& breaks, int leftmin, int rightmin) const
{
    // libhyphen needs a full word and cannot break anything shorter than
    // leftmin + rightmin characters; skip pathological lengths outright.
    if(length < 4 || length > 100)
        return;

    // Encode the word as UTF-8 for libhyphen while recording, for every UTF-8
    // byte, the UTF-16 offset of the code point it belongs to.
    std::string utf8;
    std::vector<uint32_t> byteToUtf16;
    utf8.reserve(length * 2);
    byteToUtf16.reserve(length * 2);
    for(uint32_t offset = 0; offset < length;) {
        const uint32_t start = offset;
        UChar32 character;
        U16_NEXT(word, offset, length, character);

        uint8_t buffer[4];
        int count = 0;
        UBool error = false;
        U8_APPEND(buffer, count, 4, character, error);
        if(error)
            return;
        for(int i = 0; i < count; ++i) {
            utf8.push_back(static_cast<char>(buffer[i]));
            byteToUtf16.push_back(start);
        }
    }

    const int wordSize = static_cast<int>(utf8.size());
    std::vector<char> hyphens(wordSize + 5, 0);
    std::vector<char> hyphenated(wordSize * 2 + 5, 0);
    char** rep = nullptr;
    int* pos = nullptr;
    int* cut = nullptr;
    if(hnj_hyphen_hyphenate3(static_cast<HyphenDict*>(m_dict), utf8.data(), wordSize,
        hyphens.data(), hyphenated.data(), &rep, &pos, &cut, leftmin, rightmin, leftmin, rightmin) == 0) {
        for(int i = 0; i + 1 < wordSize; ++i) {
            // Odd value == hyphenation allowed after this byte; map the boundary
            // that follows back to a UTF-16 offset.
            if(hyphens[i] & 1) {
                auto offset = byteToUtf16[i + 1];
                if(offset > 0 && offset < length) {
                    if(breaks.empty() || breaks.back() != offset)
                        breaks.push_back(offset);
                }
            }
        }
    }

    if(rep) {
        for(int i = 0; i < wordSize; ++i)
            std::free(rep[i]);
        std::free(rep);
    }

    std::free(pos);
    std::free(cut);
}

Hyphenator::~Hyphenator()
{
    if(m_dict)
        hnj_hyphen_free(static_cast<HyphenDict*>(m_dict));
}

} // namespace plutobook

#else // PLUTOBOOK_HAS_HYPHEN

namespace plutobook {

const Hyphenator* Hyphenator::get(const std::string&)
{
    return nullptr;
}

void Hyphenator::hyphenate(const UChar*, size_t, std::vector<uint32_t>&, int, int) const
{
}

Hyphenator::~Hyphenator() = default;

} // namespace plutobook

#endif // PLUTOBOOK_HAS_HYPHEN
