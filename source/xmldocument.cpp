/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "xmldocument.h"
#include "xmlparser.h"

#include "plutobook.hpp"

namespace plutobook {

std::unique_ptr<XMLDocument> XMLDocument::create(Book* book, Url baseUrl)
{
    return std::unique_ptr<XMLDocument>(new (book->heap()) XMLDocument(book, std::move(baseUrl)));
}

bool XMLDocument::parse(std::string_view content)
{
    return XMLParser(this).parse(content);
}

XMLDocument::XMLDocument(Book* book, Url baseUrl)
    : Document(book, std::move(baseUrl))
{
}

} // namespace plutobook
