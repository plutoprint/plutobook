/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "plutobook.hpp"
#include "argparser.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

using namespace plutobook;

enum class PageSizeType {
    A3,
    A4,
    A5,
    B4,
    B5,
    Letter,
    Legal,
    Ledger
};

static PageSize getPageSize(PageSizeType sizeType)
{
    switch(sizeType) {
    case PageSizeType::A3:
        return PageSize::A3;
    case PageSizeType::A4:
        return PageSize::A4;
    case PageSizeType::A5:
        return PageSize::A5;
    case PageSizeType::B4:
        return PageSize::B4;
    case PageSizeType::B5:
        return PageSize::B5;
    case PageSizeType::Letter:
        return PageSize::Letter;
    case PageSizeType::Legal:
        return PageSize::Legal;
    case PageSizeType::Ledger:
        return PageSize::Ledger;
    }

    return PageSize::A4;
}

static bool size_func(void* closure, const char* value)
{
    static const ArgEnum<PageSizeType> choices[] = {
        {"a3", PageSizeType::A3},
        {"a4", PageSizeType::A4},
        {"a5", PageSizeType::A5},
        {"b4", PageSizeType::B4},
        {"b5", PageSizeType::B5},
        {"letter", PageSizeType::Letter},
        {"legal", PageSizeType::Legal},
        {"ledger", PageSizeType::Ledger},
    };

    return parseArgChoices(closure, value, choices, std::size(choices));
}

static bool media_func(void* closure, const char* value)
{
    static const ArgEnum<MediaType> choices[] = {
        {"print", MediaType::Print},
        {"screen", MediaType::Screen}
    };

    return parseArgChoices(closure, value, choices, std::size(choices));
}

enum class KindFilter {
    Blocks,
    TextLines,
    All
};

static bool kind_func(void* closure, const char* value)
{
    static const ArgEnum<KindFilter> choices[] = {
        {"blocks", KindFilter::Blocks},
        {"textlines", KindFilter::TextLines},
        {"all", KindFilter::All}
    };

    return parseArgChoices(closure, value, choices, std::size(choices));
}

// The class list is fixed up front when the caller supplies one, so that a dataset
// built from many documents keeps stable class ids; otherwise it grows as labels
// are discovered and is written out alongside the labels.
class ClassIndex {
public:
    explicit ClassIndex(bool fixed) : m_fixed(fixed) {}

    void add(std::string name) { m_names.push_back(std::move(name)); }

    int lookup(const std::string& name)
    {
        auto it = std::find(m_names.begin(), m_names.end(), name);
        if(it != m_names.end())
            return static_cast<int>(it - m_names.begin());
        if(m_fixed)
            return -1;
        m_names.push_back(name);
        return static_cast<int>(m_names.size() - 1);
    }

    const std::vector<std::string>& names() const { return m_names; }

private:
    std::vector<std::string> m_names;
    bool m_fixed;
};

static bool wanted(const Annotation& annotation, KindFilter filter)
{
    switch(filter) {
    case KindFilter::Blocks:
        return annotation.kind != AnnotationKind::TextLine;
    case KindFilter::TextLines:
        return annotation.kind == AnnotationKind::TextLine;
    case KindFilter::All:
        return true;
    }

    return true;
}

// YOLO wants the box centre and extent as fractions of the image, and refuses
// anything outside the image, so clamp before normalizing.
static bool normalize(const Annotation& annotation, float scale, float imageWidth, float imageHeight,
    float& cx, float& cy, float& w, float& h)
{
    auto left = std::max(0.f, annotation.x * scale);
    auto top = std::max(0.f, annotation.y * scale);
    auto right = std::min(imageWidth, (annotation.x + annotation.width) * scale);
    auto bottom = std::min(imageHeight, (annotation.y + annotation.height) * scale);
    if(right <= left || bottom <= top)
        return false;

    cx = ((left + right) / 2.f) / imageWidth;
    cy = ((top + bottom) / 2.f) / imageHeight;
    w = (right - left) / imageWidth;
    h = (bottom - top) / imageHeight;
    return true;
}

static void writeJsonString(std::ostream& o, const std::string& value)
{
    o << '"';
    for(auto cc : value) {
        switch(cc) {
        case '"':
            o << "\\\"";
            break;
        case '\\':
            o << "\\\\";
            break;
        case '\n':
            o << "\\n";
            break;
        case '\r':
            o << "\\r";
            break;
        case '\t':
            o << "\\t";
            break;
        default:
            if(static_cast<unsigned char>(cc) < 0x20) {
                char buffer[8];
                snprintf(buffer, sizeof(buffer), "\\u%04x", cc);
                o << buffer;
            } else {
                o << cc;
            }

            break;
        }
    }

    o << '"';
}

static const char* kindName(AnnotationKind kind)
{
    switch(kind) {
    case AnnotationKind::Block:
        return "block";
    case AnnotationKind::Inline:
        return "inline";
    case AnnotationKind::TextLine:
        return "textline";
    }

    return "block";
}

int main(int argc, char* argv[])
{
    const char* input = "";
    const char* output = "";

    PageSizeType size = PageSizeType::A4;
    MediaType media = MediaType::Print;
    KindFilter kind = KindFilter::All;

    float width = -1;
    float height = -1;
    float margin = 72;

    float dpi = 150;

    const char* name = nullptr;
    const char* classes = nullptr;
    const char* label_attribute = "data-anno";
    const char* id_attribute = "data-anno-id";

    float min_size = 1;

    int page_start = kMinPageCount;
    int page_end = kMaxPageCount;

    bool include_unlabelled = false;
    bool merge_inline = false;
    bool no_images = false;
    bool jsonl = false;

    const char* user_style = "";
    const char* user_script = "";

    ArgDesc args[] = {
        {"input", ArgType::String, &input, nullptr, "Specify the input HTML filename or URL"},
        {"output", ArgType::String, &output, nullptr, "Specify the output dataset directory"},

        {"--size", ArgType::Choice, &size, size_func, "Specify the page size (eg. A4)"},
        {"--media", ArgType::Choice, &media, media_func, "Specify the media type (eg. print, screen)"},
        {"--width", ArgType::Length, &width, nullptr, "Specify the page width (eg. 210mm)"},
        {"--height", ArgType::Length, &height, nullptr, "Specify the page height (eg. 297mm)"},
        {"--margin", ArgType::Length, &margin, nullptr, "Specify the page margin (eg. 72pt)"},

        {"--dpi", ArgType::Float, &dpi, nullptr, "Specify the resolution of the rendered images"},
        {"--name", ArgType::String, &name, nullptr, "Specify the basename used for the generated files"},

        {"--kind", ArgType::Choice, &kind, kind_func, "Specify what to annotate (eg. blocks, textlines, all)"},
        {"--classes", ArgType::String, &classes, nullptr, "Specify a fixed comma-separated class list"},
        {"--label-attr", ArgType::String, &label_attribute, nullptr, "Specify the attribute holding the label"},
        {"--id-attr", ArgType::String, &id_attribute, nullptr, "Specify the attribute holding the element identity"},
        {"--min-size", ArgType::Float, &min_size, nullptr, "Discard boxes smaller than this, in points"},

        {"--page-start", ArgType::Int, &page_start, nullptr, "Specify the first page number to export"},
        {"--page-end", ArgType::Int, &page_end, nullptr, "Specify the last page number to export"},

        {"--include-unlabelled", ArgType::Flag, &include_unlabelled, nullptr, "Annotate unlabelled blocks by tag name"},
        {"--merge-inline", ArgType::Flag, &merge_inline, nullptr, "Merge the line fragments of an inline element"},
        {"--no-images", ArgType::Flag, &no_images, nullptr, "Write labels only, without rendering images"},
        {"--jsonl", ArgType::Flag, &jsonl, nullptr, "Also write a JSONL sidecar with text and identities"},

        {"--user-style", ArgType::String, &user_style, nullptr, "Specify the user-defined CSS style"},
        {"--user-script", ArgType::String, &user_script, nullptr, "Specify the user-defined JavaScript"},
        {nullptr}
    };

    parseArgs("html2yolo", "Export HTML page layout as a YOLO detection dataset", args, argc, argv);

    PageSize pageSize(getPageSize(size));
    if(width >= 0)
        pageSize.setWidth(width);
    if(height >= 0) {
        pageSize.setHeight(height);
    }

    Book book(pageSize, PageMargins(margin), media);
    if(!book.loadUrl(input, user_style, user_script)) {
        fprintf(stderr, "ERROR: %s\n", plutobook_get_error_message());
        return 1;
    }

    AnnotationOptions options;
    options.labelAttribute = label_attribute;
    options.idAttribute = id_attribute;
    options.includeTextLines = kind != KindFilter::Blocks;
    options.includeUnlabelledBlocks = include_unlabelled;
    options.mergeInlineFragments = merge_inline;
    options.minimumSize = min_size;

    ClassIndex classIndex(classes != nullptr);
    if(classes) {
        std::string list(classes);
        size_t offset = 0;
        while(offset <= list.size()) {
            auto end = list.find(',', offset);
            if(end == std::string::npos)
                end = list.size();
            auto value = list.substr(offset, end - offset);
            if(!value.empty())
                classIndex.add(std::move(value));
            offset = end + 1;
        }
    }

    std::filesystem::path root(output);
    std::filesystem::path imageDir(root / "images");
    std::filesystem::path labelDir(root / "labels");

    std::error_code ec;
    std::filesystem::create_directories(labelDir, ec);
    if(!no_images)
        std::filesystem::create_directories(imageDir, ec);
    if(ec) {
        fprintf(stderr, "ERROR: unable to create '%s': %s\n", output, ec.message().c_str());
        return 1;
    }

    std::string stem = name ? name : std::filesystem::path(input).stem().string();
    if(stem.empty())
        stem = "page";

    // Page sizes and annotations are reported in points, while rendering happens in
    // CSS pixels, so the two coordinate systems reach the image by different factors.
    const float scale = dpi / 72.f;
    const float renderScale = dpi / (72.f / units::px);

    std::ofstream manifest;
    if(jsonl) {
        manifest.open(root / (stem + ".jsonl"));
        if(!manifest) {
            fprintf(stderr, "ERROR: unable to write the JSONL sidecar\n");
            return 1;
        }
    }

    auto pageCount = book.pageCount();
    auto firstPage = std::max(1, page_start);
    auto lastPage = std::min<uint32_t>(pageCount, page_end < 0 ? pageCount : uint32_t(page_end));

    uint32_t written = 0;
    for(uint32_t pageNumber = firstPage; pageNumber <= lastPage; ++pageNumber) {
        auto pageIndex = pageNumber - 1;
        auto pageSizeAt = book.pageSizeAt(pageIndex);

        auto imageWidth = std::max(1, int(std::lround(pageSizeAt.width() * scale)));
        auto imageHeight = std::max(1, int(std::lround(pageSizeAt.height() * scale)));

        char suffix[32];
        snprintf(suffix, sizeof(suffix), "_p%04u", pageNumber);
        std::string base = stem + suffix;

        if(!no_images) {
            ImageCanvas canvas(imageWidth, imageHeight);
            canvas.clearSurface(1, 1, 1, 1);
            canvas.scale(renderScale, renderScale);
            book.renderPage(canvas, pageIndex);
            if(!canvas.writeToPng((imageDir / (base + ".png")).string())) {
                fprintf(stderr, "ERROR: %s\n", plutobook_get_error_message());
                return 1;
            }
        }

        std::ofstream labels(labelDir / (base + ".txt"));
        if(!labels) {
            fprintf(stderr, "ERROR: unable to write the labels for page %u\n", pageNumber);
            return 1;
        }

        for(const auto& annotation : book.annotationsAt(pageIndex, options)) {
            if(!wanted(annotation, kind))
                continue;
            auto label = annotation.label.empty() ? std::string("text") : annotation.label;
            auto classId = classIndex.lookup(label);
            if(classId < 0)
                continue;

            float cx, cy, w, h;
            if(!normalize(annotation, scale, float(imageWidth), float(imageHeight), cx, cy, w, h))
                continue;

            labels << classId << ' ' << cx << ' ' << cy << ' ' << w << ' ' << h << '\n';
            ++written;

            if(jsonl) {
                manifest << "{\"image\":";
                writeJsonString(manifest, "images/" + base + ".png");
                manifest << ",\"page\":" << pageNumber
                    << ",\"kind\":\"" << kindName(annotation.kind) << '"'
                    << ",\"class_id\":" << classId
                    << ",\"label\":";
                writeJsonString(manifest, label);
                manifest << ",\"id\":";
                writeJsonString(manifest, annotation.id);
                manifest << ",\"parent_id\":";
                writeJsonString(manifest, annotation.parentId);
                manifest << ",\"tag\":";
                writeJsonString(manifest, annotation.tagName);
                manifest << ",\"bbox\":[" << annotation.x * scale << ',' << annotation.y * scale
                    << ',' << annotation.width * scale << ',' << annotation.height * scale << ']'
                    << ",\"text\":";
                writeJsonString(manifest, annotation.text);
                manifest << "}\n";
            }
        }
    }

    std::ofstream classFile(root / "classes.txt");
    for(const auto& className : classIndex.names())
        classFile << className << '\n';

    std::ofstream dataFile(root / "data.yaml");
    dataFile << "path: " << std::filesystem::absolute(root).string() << '\n'
        << "train: images\n"
        << "val: images\n"
        << "nc: " << classIndex.names().size() << '\n'
        << "names:\n";
    for(size_t i = 0; i < classIndex.names().size(); ++i)
        dataFile << "  " << i << ": " << classIndex.names()[i] << '\n';

    fprintf(stderr, "%u boxes across %u pages in %zu classes\n", written,
        lastPage >= firstPage ? lastPage - firstPage + 1 : 0, classIndex.names().size());
    return 0;
}
