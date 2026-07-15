/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_HYPHENATOR_H
#define PLUTOBOOK_HYPHENATOR_H

#include "ustring.h"

#include <string>
#include <vector>

namespace plutobook {

// Wraps a libhyphen dictionary for automatic (`hyphens: auto`) hyphenation.
//
// When PlutoBook is built without libhyphen, `get()` always returns nullptr so
// callers transparently fall back to manual hyphenation. Dictionaries are
// resolved once per locale and cached; the system location is tried first,
// then the dictionaries embedded into the library at build time.
class Hyphenator {
public:
    // Returns a cached hyphenator for `localeName` (e.g. "en_US"), or nullptr
    // when no dictionary can be found for it.
    static const Hyphenator* get(const std::string& localeName);

    // Appends to `breaks` the UTF-16 offsets, relative to the start of `word`,
    // after which a hyphenation break is allowed. `leftmin`/`rightmin` are the
    // minimum number of characters that must stay before/after a break.
    void hyphenate(const UChar* word, size_t length, std::vector<uint32_t>& breaks,
                   int leftmin = 2, int rightmin = 2) const;

    ~Hyphenator();

private:
    explicit Hyphenator(void* dict) : m_dict(dict) {}
    Hyphenator(const Hyphenator&) = delete;
    Hyphenator& operator=(const Hyphenator&) = delete;

    void* m_dict; // HyphenDict* (opaque so libhyphen stays out of the header)
};

} // namespace plutobook

#endif // PLUTOBOOK_HYPHENATOR_H
