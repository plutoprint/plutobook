/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "resource.h"
#include "stringutils.h"
#include "plutobook.hpp"

#include <filesystem>
#include <vector>

#ifdef PLUTOBOOK_HAS_CURL
#include <curl/curl.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

namespace plutobook {

static FILE* openStream(const std::string& filename, FileMode mode)
{
#ifdef _WIN32
    wchar_t wfilename[1024];
    MultiByteToWideChar(CP_UTF8, 0, filename.data(), -1, wfilename, sizeof(wfilename) / sizeof(wchar_t));

    return _wfopen(wfilename, mode == FileMode::Read ? L"rb" : L"wb");
#else
    return fopen(filename.data(), mode == FileMode::Read ? "rb" : "wb");
#endif
}

FILE* openFile(const std::string& filename, FileMode mode)
{
    auto stream = openStream(filename, mode);
    if(stream == nullptr)
        plutobook_set_error_message("Unable to open file '%s': %s", filename.data(), strerror(errno));
    return stream;
}

static bool loadStream(FILE* stream, ByteArray& output)
{
    if(fseek(stream, 0, SEEK_END) != 0)
        return false;
    auto size = ftell(stream);
    if(size == -1L) {
        return false;
    }

    output.resize(size);

    if(fseek(stream, 0, SEEK_SET) != 0)
        return false;
    fread(output.data(), 1, size, stream);
    return ferror(stream) == 0;
}

struct FileCloser {
    void operator()(FILE* file) const { fclose(file); }
};

bool loadFile(const std::string& filename, ByteArray& output)
{
    auto stream = openFile(filename, FileMode::Read);
    if(stream == nullptr) {
        return false;
    }

    std::unique_ptr<FILE, FileCloser> guard(stream);

    if(loadStream(stream, output))
        return true;
    plutobook_set_error_message("Unable to load file '%s': %s", filename.data(), strerror(errno));
    return false;
}

static ByteArray* ByteArrayCreate(void)
{
    return new ByteArray();
}

static void ByteArrayDestroy(void* data)
{
    delete (ByteArray*)(data);
}

#define MAX_RESOURCE_SIZE 0xFFFFFFFFU

static ResourceData createResourceData(ByteArray* content, const std::string& mimeType, const std::string& textEncoding)
{
    auto data = content->data();
    auto size = content->size();
    if(size > MAX_RESOURCE_SIZE) {
        plutobook_set_error_message("maximum resource size exceeded");
        ByteArrayDestroy(content);
        return ResourceData();
    }

    return ResourceData(data, size, mimeType, textEncoding, ByteArrayDestroy, content);
}

static void parseContentType(std::string_view input, std::string& mimeType, std::string& textEncoding)
{
    auto type = input.substr(0, input.find(';'));
    input.remove_prefix(type.size());
    stripLeadingAndTrailingSpaces(type);
    mimeType.assign(type);
    while(!input.empty()) {
        input.remove_prefix(1);
        auto parameter = input.substr(0, input.find(';'));
        input.remove_prefix(parameter.size());
        stripLeadingAndTrailingSpaces(parameter);
        auto name = parameter.substr(0, parameter.find('='));
        parameter.remove_prefix(name.size());
        stripLeadingAndTrailingSpaces(name);
        if(!parameter.empty() && equals(name, "charset", false)) {
            parameter.remove_prefix(1);
            if(!parameter.empty() && parameter.front() == '\"')
                parameter.remove_prefix(1);
            if(!parameter.empty() && parameter.back() == '\"')
                parameter.remove_suffix(1);
            auto value = parameter.substr(0, parameter.size());
            stripLeadingAndTrailingSpaces(value);
            textEncoding.assign(value);
            break;
        }
    }
}

static const char base64DecMap[128] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x3F,
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
    0x3C, 0x3D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
    0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
    0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
    0x17, 0x18, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20,
    0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30,
    0x31, 0x32, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00
};

static bool base64Decode(std::string_view input, ByteArray& output)
{
    output.resize(input.length());
    size_t equalsSignCount = 0;
    size_t outputLength = 0;
    for(int cc : input) {
        if(cc == '=') {
            ++equalsSignCount;
        } else if(cc == '+' || cc == '/' || isAlpha(cc) || isDigit(cc)) {
            if(equalsSignCount > 0)
                return false;
            output[outputLength++] = base64DecMap[cc];
        } else if(!isSpace(cc)) {
            return false;
        }
    }

    if(outputLength == 0 || equalsSignCount > 2 || (outputLength % 4) == 1)
        return false;
    if(outputLength < input.length())
        output.resize(outputLength);
    outputLength -= (outputLength + 3) / 4;
    if(outputLength == 0) {
        return false;
    }

    size_t sidx = 0;
    size_t didx = 0;
    if(outputLength > 1) {
        while(didx < outputLength - 2) {
            output[didx + 0] = (((output[sidx + 0] << 2) & 255) | ((output[sidx + 1] >> 4) & 003));
            output[didx + 1] = (((output[sidx + 1] << 4) & 255) | ((output[sidx + 2] >> 2) & 017));
            output[didx + 2] = (((output[sidx + 2] << 6) & 255) | ((output[sidx + 3] >> 0) & 077));
            sidx += 4;
            didx += 3;
        }
    }

    if(didx < outputLength)
        output[didx] = (((output[sidx + 0] << 2) & 255) | ((output[sidx + 1] >> 4) & 003));
    if(++didx < outputLength)
        output[didx] = (((output[sidx + 1] << 4) & 255) | ((output[sidx + 2] >> 2) & 017));
    if(outputLength < output.size())
        output.resize(outputLength);
    return true;
}

static std::string percentDecode(std::string_view input)
{
    std::string output;
    output.reserve(input.length());
    while(!input.empty()) {
        auto cc = input.front();
        if(cc == '%' && input.length() > 2 && isHexDigit(input[1]) && isHexDigit(input[2])) {
            output.push_back(toHexByte(input[1], input[2]));
            input.remove_prefix(3);
        } else {
            output.push_back(cc);
            input.remove_prefix(1);
        }
    }

    return output;
}

static ResourceData loadDataUrl(std::string_view input)
{
    assert(startswith(input, "data:", false));
    input.remove_prefix(5);

    auto headerEnd = input.find(',');
    if(headerEnd == std::string_view::npos) {
        plutobook_set_error_message("invalid data URL: missing comma separator");
        return ResourceData();
    }

    auto header = input.substr(0, headerEnd);
    input.remove_prefix(headerEnd + 1);

    auto mediaTypeEnd = header.rfind(';');
    if(mediaTypeEnd == std::string_view::npos) {
        mediaTypeEnd = header.length();
    }

    std::string mimeType;
    std::string textEncoding;
    auto mediaType = header.substr(0, mediaTypeEnd);
    parseContentType(mediaType, mimeType, textEncoding);
    if(mimeType.empty() && textEncoding.empty()) {
        mimeType.assign("text/plain");
        textEncoding.assign("US-ASCII");
    }

    std::string_view formatType;
    if(mediaTypeEnd < header.length()) {
        formatType = header.substr(mediaTypeEnd + 1);
        stripLeadingAndTrailingSpaces(formatType);
    }

    auto content = ByteArrayCreate();
    if(!equals(formatType, "base64", false)) {
        content->reserve(input.length());
        content->assign(input.begin(), input.end());
    } else {
        if(!base64Decode(input, *content)) {
            plutobook_set_error_message("invalid data URL: base64 decoding failed");
            ByteArrayDestroy(content);
            return ResourceData();
        }
    }

    return createResourceData(content, mimeType, textEncoding);
}

static bool mimeTypeFromPath(std::string& mimeType, std::string_view path)
{
    auto dot = path.rfind('.');
    if(dot == std::string_view::npos)
        return false;
    auto slash = path.find_last_of("/\\");
    if(slash != std::string_view::npos && dot < slash) {
        return false;
    }

    static const struct {
        std::string_view ext;
        std::string_view mime;
    } table[] = {
        {"xhtml", "application/xhtml+xml"},
        {"html", "text/html"},
        {"htm", "text/html"},
        {"txt", "text/plain"},
        {"css", "text/css"},
        {"xml", "text/xml"},
        {"jpeg", "image/jpeg"},
        {"jpg", "image/jpeg"},
        {"png", "image/png"},
        {"webp", "image/webp"},
        {"svg", "image/svg+xml"},
        {"gif", "image/gif"},
        {"bmp", "image/bmp"}
    };

    const auto ext = path.substr(dot + 1);
    for(const auto& entry : table) {
        if(equalsIgnoringCase(ext, entry.ext)) {
            mimeType.assign(entry.mime);
            return true;
        }
    }

    return false;
}

static ResourceData loadLocalFile(std::string_view input)
{
    assert(startswith(input, "file://", false));
    input.remove_prefix(7);
    if(input.size() >= 3 && input[0] == '/' && isAlpha(input[1]) && input[2] == ':') {
        input.remove_prefix(1);
    }

    auto filename = percentDecode(input.substr(0, input.find('?')));
#ifdef _WIN32
    std::replace(filename.begin(), filename.end(), '/', '\\');
#endif

    auto content = ByteArrayCreate();
    if(!loadFile(filename, *content)) {
        ByteArrayDestroy(content);
        return ResourceData();
    }

    std::string mimeType;
    std::string textEncoding;
    mimeTypeFromPath(mimeType, filename);

    return createResourceData(content, mimeType, textEncoding);
}

#ifdef PLUTOBOOK_HAS_CURL

DefaultResourceFetcher::DefaultResourceFetcher()
{
    curl_global_init(CURL_GLOBAL_ALL);
#ifdef PLUTOBOOK_AUTODETECT_CA
    static const char* cainfos[] = {
        "/etc/ssl/certs/ca-certificates.crt",
        "/etc/pki/tls/certs/ca-bundle.crt",
        "/usr/share/ssl/certs/ca-bundle.crt",
        "/usr/local/share/certs/ca-root-nss.crt",
        "/etc/ssl/cert.pem"
    };

    static const char* capaths[] = {
        "/etc/ssl/certs"
    };

    for(auto path : cainfos) {
        if(std::filesystem::exists(path)
            && std::filesystem::is_regular_file(path)) {
            m_caInfo.assign(path);
            break;
        }
    }

    for(auto path : capaths) {
        if(std::filesystem::exists(path)
            && std::filesystem::is_directory(path)) {
            m_caPath.assign(path);
            break;
        }
    }
#endif // PLUTOBOOK_AUTODETECT_CA
}

DefaultResourceFetcher::~DefaultResourceFetcher()
{
    curl_global_cleanup();
}

static size_t writeCallback(const char* contents, size_t blockSize, size_t numberOfBlocks, ByteArray* response)
{
    size_t totalSize = blockSize * numberOfBlocks;
    response->insert(response->end(), contents, contents + totalSize);
    return totalSize;
}

ResourceData DefaultResourceFetcher::fetchUrl(const std::string& url)
{
    if(startswith(url, "data:", false))
        return loadDataUrl(percentDecode(url));
    if(startswith(url, "file://", false)) {
        return loadLocalFile(url);
    }

    std::string mimeType;
    std::string textEncoding;
    auto content = ByteArrayCreate();

    auto curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url.data());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, content);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "PlutoBook/" PLUTOBOOK_VERSION_STRING);
    curl_easy_setopt(curl, CURLOPT_MAXFILESIZE_LARGE, MAX_RESOURCE_SIZE);

    if(!m_caInfo.empty())
        curl_easy_setopt(curl, CURLOPT_CAINFO, m_caInfo.data());
    if(!m_caPath.empty()) {
        curl_easy_setopt(curl, CURLOPT_CAPATH, m_caPath.data());
    }

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, m_verifyPeer ? 2L : 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, m_verifyHost ? 1L : 0L);
#ifdef CURLSSLOPT_NATIVE_CA
    curl_easy_setopt(curl, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NATIVE_CA);
#endif

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, m_followRedirects);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, m_maxRedirects);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, m_timeout);

    auto response = curl_easy_perform(curl);
    if(response == CURLE_OK) {
        const char* contentType = nullptr;
        auto response = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &contentType);
        if(response == CURLE_OK && contentType) {
            parseContentType(contentType, mimeType, textEncoding);
        }

        if(mimeType.empty()) {
            mimeTypeFromPath(mimeType, percentDecode(url.substr(0, url.rfind('?'))));
        }
    }

    curl_easy_cleanup(curl);
    if(response == CURLE_OK)
        return createResourceData(content, mimeType, textEncoding);
    plutobook_set_error_message("Unable to fetch URL '%s': %s", url.data(), curl_easy_strerror(response));
    ByteArrayDestroy(content);
    return ResourceData();
}

#else

DefaultResourceFetcher::DefaultResourceFetcher() = default;
DefaultResourceFetcher::~DefaultResourceFetcher() = default;

ResourceData DefaultResourceFetcher::fetchUrl(const std::string& url)
{
    if(startswith(url, "data:", false))
        return loadDataUrl(percentDecode(url));
    if(startswith(url, "file://", false))
        return loadLocalFile(url);
    plutobook_set_error_message("Unable to fetch URL '%s': Unsupported protocol", url.data());
    return ResourceData();
}

#endif // PLUTOBOOK_HAS_CURL

ResourceData ResourceLoader::loadUrl(const Url& url, ResourceFetcher* customFetcher)
{
    if(url.protocolIs("data"))
        return loadDataUrl(percentDecode(url.value()));
    if(customFetcher == nullptr)
        customFetcher = defaultResourceFetcher();
    return customFetcher->fetchUrl(url.value());
}

Url ResourceLoader::baseUrl()
{
    auto path = std::filesystem::current_path();
    auto href = ada::href_from_file(path.generic_string());

    return Url(href + '/');
}

Url ResourceLoader::completeUrl(std::string_view value)
{
    return baseUrl().complete(value);
}

DefaultResourceFetcher* defaultResourceFetcher()
{
    static DefaultResourceFetcher defaultFetcher;
    return &defaultFetcher;
}

} // namespace plutobook
