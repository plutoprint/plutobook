/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "textresource.h"
#include "document.h"
#include "stringutils.h"

#include "plutobook.hpp"

namespace plutobook {

RefPtr<TextResource> TextResource::create(Document* document, const Url& url)
{
    auto resource = ResourceLoader::loadUrl(url, document->customResourceFetcher());
    if(resource.isNull())
        return nullptr;
    return adoptPtr(new (document->heap()) TextResource(decode(resource.content(), resource.contentLength(), resource.mimeType(), resource.textEncoding())));
}

std::string_view TextResource::decode(const char* data, size_t length, const std::string_view& mimeType, const std::string_view& textEncoding)
{
    std::string_view output(data, length);
    if(length >= 3) {
        auto buffer = (const uint8_t*)(data);

        const auto c1 = buffer[0];
        const auto c2 = buffer[1];
        const auto c3 = buffer[2];
        if(c1 == 0xEF && c2 == 0xBB && c3 == 0xBF) {
            output.remove_prefix(3);
        }
    }

    return output;
}

bool TextResource::isXMLMIMEType(const std::string_view& mimeType)
{
    if(equals(mimeType, "text/xml", false)
        || equals(mimeType, "application/xml", false)
        || equals(mimeType, "text/xsl", false)) {
        return true;
    }

    auto length = mimeType.length();
    if(length < 7) {
        return false;
    }

    if(mimeType[0] == '/' || mimeType[length - 5] == '/' || !endswith(mimeType, "+xml", false)) {
        return false;
    }

    bool hasSlash = false;
    for(int i = 0; i < length - 4; ++i) {
        auto cc = mimeType[i];
        if(isAlnum(cc))
            continue;
        switch(cc) {
        case '_':
        case '-':
        case '+':
        case '~':
        case '!':
        case '$':
        case '^':
        case '{':
        case '}':
        case '|':
        case '.':
        case '%':
        case '\'':
        case '`':
        case '#':
        case '&':
        case '*':
            continue;
        case '/':
            if(hasSlash)
                return false;
            hasSlash = true;
            continue;
        default:
            return false;
        }
    }

    return true;
}

} // namespace plutobook
