/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_XMLDOCUMENT_H
#define PLUTOBOOK_XMLDOCUMENT_H

#include "document.h"

namespace plutobook {

class XMLDocument : public Document {
public:
    static std::unique_ptr<XMLDocument> create(Book* book, Heap* heap, ResourceFetcher* fetcher, Url baseUrl);

    bool isXMLDocument() const final { return true; }
    bool parse(const std::string_view& content) override;

protected:
    XMLDocument(Book* book, Heap* heap, ResourceFetcher* fetcher, Url baseUrl);
};

template<>
struct is_a<XMLDocument> {
    static bool check(const Node& value) { return value.isXMLDocument(); }
};

} // namespace plutobook

#endif // PLUTOBOOK_XMLDOCUMENT_H
