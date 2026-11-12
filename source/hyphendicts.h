/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_HYPHENDICTS_H
#define PLUTOBOOK_HYPHENDICTS_H

namespace plutobook {

// A libhyphen dictionary embedded into the library at build time. The registry
// below is produced by tools/embed_dicts.py from the languages selected with
// the `hyphen-embed-dicts` build option (en-US only by default).
struct EmbeddedHyphenationDict {
    const char* name;            // locale name, e.g. "en_US"
    const unsigned char* data;
    unsigned int size;
};

extern const EmbeddedHyphenationDict kEmbeddedHyphenationDicts[];
extern const unsigned int kEmbeddedHyphenationDictCount;

} // namespace plutobook

#endif // PLUTOBOOK_HYPHENDICTS_H
