/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_RESOURCE_H
#define PLUTOBOOK_RESOURCE_H

#include "pointer.h"
#include "heapstring.h"
#include "url.h"

#include <cstdio>
#include <memory>
#include <vector>

namespace plutobook {

class Resource : public HeapMember, public RefCounted<Resource> {
public:
    enum class Type {
        Text,
        Image,
        Font
    };

    virtual ~Resource() = default;
    virtual Type type() const = 0;

protected:
    Resource() = default;
};

struct FileCloser {
    void operator()(FILE* file) const { fclose(file); }
};

using File = std::unique_ptr<FILE, FileCloser>;

enum class FileMode {
    Read,
    Write
};

using ByteArray = std::vector<char>;

File openFile(const std::string& filename, FileMode mode);
bool loadFile(const std::string& filename, ByteArray& output);

class ResourceData;
class ResourceFetcher;

class ResourceLoader {
public:
    static ResourceData loadUrl(const Url& url, ResourceFetcher* customFetcher = nullptr);
    static Url completeUrl(std::string_view value);
    static Url baseUrl();
};

} // namespace plutobook

#endif // PLUTOBOOK_RESOURCE_H
