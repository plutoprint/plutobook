/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "plutobook.hpp"
#include "htmldocument.h"
#include "xmldocument.h"
#include "textresource.h"
#include "imageresource.h"
#include "fontresource.h"
#include "replacedbox.h"
#include "graphicscontext.h"

#include <cmath>
#include <utility>
#include <cstdio>

#ifdef _WIN32
#include <windows.h>
#endif

namespace plutobook {

class FileOutputStream final : public OutputStream {
public:
    explicit FileOutputStream(const std::string& filename);

    bool isOpen() const { return m_handle; }
    bool write(const char* data, size_t length) final;

    ~FileOutputStream() final;

private:
    FILE* m_handle;
};

static FILE* output_stream_fopen(const char* filename)
{
#ifdef _WIN32
    wchar_t wfilename[1024];
    if(!MultiByteToWideChar(CP_UTF8, 0, filename, -1, wfilename, sizeof(wfilename) / sizeof(wchar_t))) {
        errno = EINVAL;
        return NULL;
    }

    return _wfopen(wfilename, L"wb");
#else
    return fopen(filename, "wb");
#endif
}

FileOutputStream::FileOutputStream(const std::string& filename)
    : m_handle(output_stream_fopen(filename.data()))
{
    if(m_handle == NULL) {
        plutobook_set_error_message("Unable to open file '%s': %s", filename.data(), std::strerror(errno));
    }
}

bool FileOutputStream::write(const char* data, size_t length)
{
    return length == fwrite(data, 1, length, m_handle);
}

FileOutputStream::~FileOutputStream()
{
    if(m_handle) {
        fclose(m_handle);
    }
}

static plutobook_stream_status_t stream_write_func(void* closure, const char* data, unsigned int length)
{
    auto output = static_cast<OutputStream*>(closure);
    if(output->write(data, length))
        return PLUTOBOOK_STREAM_STATUS_SUCCESS;
    return PLUTOBOOK_STREAM_STATUS_WRITE_ERROR;
}

Canvas::~Canvas()
{
    plutobook_canvas_destroy(m_canvas);
}

void Canvas::flush()
{
    plutobook_canvas_flush(m_canvas);
}

void Canvas::finish()
{
    plutobook_canvas_finish(m_canvas);
}

void Canvas::translate(float tx, float ty)
{
    plutobook_canvas_translate(m_canvas, tx, ty);
}

void Canvas::scale(float sx, float sy)
{
    plutobook_canvas_scale(m_canvas, sx, sy);
}

void Canvas::rotate(float angle)
{
    plutobook_canvas_rotate(m_canvas, angle);
}

void Canvas::transform(float a, float b, float c, float d, float e, float f)
{
    plutobook_canvas_transform(m_canvas, a, b, c, d, e, f);
}

void Canvas::setMatrix(float a, float b, float c, float d, float e, float f)
{
    plutobook_canvas_set_matrix(m_canvas, a, b, c, d, e, f);
}

void Canvas::resetMatrix()
{
    plutobook_canvas_reset_matrix(m_canvas);
}

void Canvas::clipRect(float x, float y, float width, float height)
{
    plutobook_canvas_clip_rect(m_canvas, x, y, width, height);
}

void Canvas::clearSurface(float red, float green, float blue, float alpha)
{
    plutobook_canvas_clear_surface(m_canvas, red, green, blue, alpha);
}

void Canvas::saveState()
{
    plutobook_canvas_save_state(m_canvas);
}

void Canvas::restoreState()
{
    plutobook_canvas_restore_state(m_canvas);
}

cairo_surface_t* Canvas::surface() const
{
    return plutobook_canvas_get_surface(m_canvas);
}

cairo_t* Canvas::context() const
{
    return plutobook_canvas_get_context(m_canvas);
}

ImageCanvas::ImageCanvas(int width, int height, ImageFormat format)
    : Canvas(plutobook_image_canvas_create(width, height, (plutobook_image_format_t)(format)))
{
}

ImageCanvas::ImageCanvas(uint8_t* data, int width, int height, int stride, ImageFormat format)
    : Canvas(plutobook_image_canvas_create_for_data(data, width, height, stride, (plutobook_image_format_t)(format)))
{
}

uint8_t* ImageCanvas::data() const
{
    return plutobook_image_canvas_get_data(m_canvas);
}

int ImageCanvas::width() const
{
    return plutobook_image_canvas_get_width(m_canvas);
}

int ImageCanvas::height() const
{
    return plutobook_image_canvas_get_height(m_canvas);
}

int ImageCanvas::stride() const
{
    return plutobook_image_canvas_get_stride(m_canvas);
}

ImageFormat ImageCanvas::format() const
{
    return (ImageFormat)(plutobook_image_canvas_get_format(m_canvas));
}

bool ImageCanvas::writeToPng(const std::string& filename) const
{
    FileOutputStream output(filename);
    if(!output.isOpen())
        return false;
    return writeToPng(output);
}

bool ImageCanvas::writeToPng(OutputStream& output) const
{
    return writeToPng(stream_write_func, &output);
}

bool ImageCanvas::writeToPng(plutobook_stream_write_callback_t callback, void* closure) const
{
    return plutobook_image_canvas_write_to_png_stream(m_canvas, callback, closure);
}

PDFCanvas::PDFCanvas(const std::string& filename, const PageSize& pageSize)
    : Canvas(plutobook_pdf_canvas_create(filename.data(), (plutobook_page_size_t)(pageSize)))
{
}

PDFCanvas::PDFCanvas(OutputStream& output, const PageSize& pageSize)
    : PDFCanvas(stream_write_func, &output, pageSize)
{
}

PDFCanvas::PDFCanvas(plutobook_stream_write_callback_t callback, void* closure, const PageSize& pageSize)
    : Canvas(plutobook_pdf_canvas_create_for_stream(callback, closure, pageSize))
{
}

void PDFCanvas::setTitle(const std::string& title)
{
    plutobook_pdf_canvas_set_metadata(m_canvas, PLUTOBOOK_PDF_METADATA_TITLE, title.data());
}

void PDFCanvas::setAuthor(const std::string& author)
{
    plutobook_pdf_canvas_set_metadata(m_canvas, PLUTOBOOK_PDF_METADATA_AUTHOR, author.data());
}

void PDFCanvas::setSubject(const std::string& subject)
{
    plutobook_pdf_canvas_set_metadata(m_canvas, PLUTOBOOK_PDF_METADATA_SUBJECT, subject.data());
}

void PDFCanvas::setKeywords(const std::string& keywords)
{
    plutobook_pdf_canvas_set_metadata(m_canvas, PLUTOBOOK_PDF_METADATA_KEYWORDS, keywords.data());
}

void PDFCanvas::setCreator(const std::string& creator)
{
    plutobook_pdf_canvas_set_metadata(m_canvas, PLUTOBOOK_PDF_METADATA_CREATOR, creator.data());
}

void PDFCanvas::setCreationDate(const std::string& creationDate)
{
    plutobook_pdf_canvas_set_metadata(m_canvas, PLUTOBOOK_PDF_METADATA_CREATION_DATE, creationDate.data());
}

void PDFCanvas::setModificationDate(const std::string& modificationDate)
{
    plutobook_pdf_canvas_set_metadata(m_canvas, PLUTOBOOK_PDF_METADATA_MODIFICATION_DATE, modificationDate.data());
}

void PDFCanvas::setPageSize(const PageSize& pageSize)
{
    plutobook_pdf_canvas_set_size(m_canvas, pageSize);
}

void PDFCanvas::showPage()
{
    plutobook_pdf_canvas_show_page(m_canvas);
}

ResourceData::ResourceData(const char* content, size_t contentLength, const std::string& mimeType, const std::string& textEncoding)
    : m_data(plutobook_resource_data_create(content, contentLength, mimeType.data(), textEncoding.data()))
{
}

ResourceData::ResourceData(const char* content, size_t contentLength, const std::string& mimeType, const std::string& textEncoding, plutobook_resource_destroy_callback_t destroyCallback, void* closure)
    : m_data(plutobook_resource_data_create_without_copy(content, contentLength, mimeType.data(), textEncoding.data(), destroyCallback, closure))
{
}

ResourceData::ResourceData(const ResourceData& resource)
    : m_data(plutobook_resource_data_reference(resource.get()))
{
}

ResourceData::~ResourceData()
{
    plutobook_resource_data_destroy(m_data);
}

ResourceData& ResourceData::operator=(const ResourceData& resource)
{
    ResourceData(resource).swap(*this);
    return *this;
}

ResourceData& ResourceData::operator=(ResourceData&& resource)
{
    ResourceData(std::move(resource)).swap(*this);
    return *this;
}

void ResourceData::swap(ResourceData& resource)
{
    std::swap(m_data, resource.m_data);
}

const char* ResourceData::content() const
{
    return plutobook_resource_data_get_content(m_data);
}

size_t ResourceData::contentLength() const
{
    return plutobook_resource_data_get_content_length(m_data);
}

std::string_view ResourceData::mimeType() const
{
    return plutobook_resource_data_get_mime_type(m_data);
}

std::string_view ResourceData::textEncoding() const
{
    return plutobook_resource_data_get_text_encoding(m_data);
}

plutobook_resource_data_t* ResourceData::release()
{
    return std::exchange(m_data, nullptr);
}

Book::Book(const PageSize& size, const PageMargins& margins, MediaType media)
    : m_pageSize(size)
    , m_pageMargins(margins)
    , m_mediaType(media)
    , m_heap(new Heap(1024 * 32))
{
}

Book::~Book() = default;

float Book::viewportWidth() const
{
    return std::max(0.f, m_pageSize.width() - m_pageMargins.left() - m_pageMargins.right()) / units::px;
}

float Book::viewportHeight() const
{
    return std::max(0.f, m_pageSize.height() - m_pageMargins.top() - m_pageMargins.bottom()) / units::px;
}

float Book::documentWidth() const
{
    if(auto document = layoutIfNeeded())
        return document->width();
    return 0.f;
}

float Book::documentHeight() const
{
    if(auto document = layoutIfNeeded())
        return document->height();
    return 0.f;
}

uint32_t Book::pageCount() const
{
    if(auto document = paginateIfNeeded())
        return document->pageCount();
    return 0;
}

PageSize Book::pageSizeAt(uint32_t pageIndex) const
{
    if(auto document = paginateIfNeeded())
        return document->pageSizeAt(pageIndex);
    return m_pageSize;
}

bool Book::loadUrl(const std::string_view& url, const std::string_view& userStyle, const std::string_view& userScript)
{
    auto completeUrl = ResourceLoader::completeUrl(url);
    auto resource = ResourceLoader::loadUrl(completeUrl, m_customResourceFetcher);
    if(resource.isNull())
        return false;
    if(loadData(resource.content(), resource.contentLength(), resource.mimeType(), resource.textEncoding(), userStyle, userScript, completeUrl.base())) {
        return true;
    }

    plutobook_set_error_message("Unable to load URL '%s': %s", completeUrl.value().data(), plutobook_get_error_message());
    return false;
}

bool Book::loadData(const char* data, size_t length, const std::string_view& mimeType, const std::string_view& textEncoding, const std::string_view& userStyle, const std::string_view& userScript, const std::string_view& baseUrl)
{
    if(TextResource::isXMLMIMEType(mimeType))
        return loadXml(TextResource::decode(data, length, mimeType, textEncoding), userStyle, userScript, baseUrl);
    if(ImageResource::supportsMimeType(mimeType))
        return loadImage(data, length, mimeType, textEncoding, userStyle, userScript, baseUrl);
    return loadHtml(TextResource::decode(data, length, mimeType, textEncoding), userStyle, userScript, baseUrl);
}

bool Book::loadImage(const char* data, size_t length, const std::string_view& mimeType, const std::string_view& textEncoding, const std::string_view& userStyle, const std::string_view& userScript, const std::string_view& baseUrl)
{
    auto image = ImageResource::decode(data, length, mimeType, textEncoding, baseUrl, m_customResourceFetcher);
    if(image == nullptr) {
        return false;
    }

    loadHtml("<img>", userStyle, userScript, baseUrl);

    auto document = buildIfNeeded();
    assert(document && document->isHTMLDocument());
    auto htmlElement = to<HTMLElement>(document->firstChild());
    assert(htmlElement && htmlElement->tagName() == htmlTag);
    auto headElement = to<HTMLElement>(htmlElement->firstChild());
    assert(headElement && headElement->tagName() == headTag);
    auto bodyElement = to<HTMLElement>(headElement->nextSibling());
    assert(bodyElement && bodyElement->tagName() == bodyTag);
    auto imageElement = to<HTMLElement>(bodyElement->firstChild());
    assert(imageElement && imageElement->tagName() == imgTag);

    auto box = imageElement->box();
    if(box == nullptr) {
        plutobook_set_error_message("image element has no associated layout box");
        return false;
    }

    auto& imageBox = to<ImageBox>(*box);
    imageBox.setImage(std::move(image));
    return true;
}

bool Book::loadXml(const std::string_view& content, const std::string_view& userStyle, const std::string_view& userScript, const std::string_view& baseUrl)
{
    clearContent();
    m_document = XMLDocument::create(this, m_heap.get(), m_customResourceFetcher, ResourceLoader::completeUrl(baseUrl));
    if(!m_document->parse(content)) {
        clearContent();
        return false;
    }

    m_document->addUserStyleSheet(userStyle);
    m_document->runJavaScript(userScript);
    return true;
}

bool Book::loadHtml(const std::string_view& content, const std::string_view& userStyle, const std::string_view& userScript, const std::string_view& baseUrl)
{
    clearContent();
    m_document = HTMLDocument::create(this, m_heap.get(), m_customResourceFetcher, ResourceLoader::completeUrl(baseUrl));
    if(!m_document->parse(content)) {
        assert(false);
    }

    m_document->addUserStyleSheet(userStyle);
    m_document->runJavaScript(userScript);
    return true;
}

void Book::clearContent()
{
    m_document.reset();
    m_heap->release();
    m_needsBuild = true;
    m_needsLayout = true;
    m_needsPagination = true;
}

void Book::renderPage(Canvas& canvas, uint32_t pageIndex) const
{
    if(auto context = canvas.context()) {
        renderPage(context, pageIndex);
    }
}

void Book::renderPage(plutobook_canvas_t* canvas, uint32_t pageIndex) const
{
    if(auto context = plutobook_canvas_get_context(canvas)) {
        renderPage(context, pageIndex);
    }
}

void Book::renderPage(cairo_t* canvas, uint32_t pageIndex) const
{
    if(auto document = paginateIfNeeded()) {
        GraphicsContext context(canvas);
        document->renderPage(context, pageIndex);
    }
}

void Book::renderDocument(Canvas& canvas) const
{
    if(auto context = canvas.context()) {
        renderDocument(context);
    }
}

void Book::renderDocument(Canvas& canvas, float x, float y, float width, float height) const
{
    if(auto context = canvas.context()) {
        renderDocument(context, x, y, width, height);
    }
}

void Book::renderDocument(plutobook_canvas_t* canvas) const
{
    if(auto context = plutobook_canvas_get_context(canvas)) {
        renderDocument(context);
    }
}

void Book::renderDocument(plutobook_canvas_t* canvas, float x, float y, float width, float height) const
{
    if(auto context = plutobook_canvas_get_context(canvas)) {
        renderDocument(context, x, y, width, height);
    }
}

void Book::renderDocument(cairo_t* canvas) const
{
    if(auto document = layoutIfNeeded()) {
        GraphicsContext context(canvas);
        document->render(context, Rect::Infinite);
    }
}

void Book::renderDocument(cairo_t* canvas, float x, float y, float width, float height) const
{
    if(auto document = layoutIfNeeded()) {
        GraphicsContext context(canvas);
        document->render(context, Rect(x, y, width, height));
    }
}

bool Book::writeToPdf(const std::string& filename, uint32_t pageStart, uint32_t pageEnd, int pageStep) const
{
    FileOutputStream output(filename);
    if(!output.isOpen())
        return false;
    if(writeToPdf(output, pageStart, pageEnd, pageStep)) {
        return true;
    }

    plutobook_set_error_message("Unable to write PDF '%s': %s", filename.data(), plutobook_get_error_message());
    return false;
}

bool Book::writeToPdf(OutputStream& output, uint32_t pageStart, uint32_t pageEnd, int pageStep) const
{
    return writeToPdf(stream_write_func, &output, pageStart, pageEnd, pageStep);
}

bool Book::writeToPdf(plutobook_stream_write_callback_t callback, void* closure, uint32_t pageStart, uint32_t pageEnd, int pageStep) const
{
    if(pageStep == 0) {
        plutobook_set_error_message("invalid page range: step cannot be zero");
        return false;
    }

    pageStart = std::max(1u, std::min(pageStart, pageCount()));
    pageEnd = std::max(1u, std::min(pageEnd, pageCount()));
    if((pageStep > 0 && pageStart > pageEnd) || (pageStep < 0 && pageStart < pageEnd)) {
        plutobook_set_error_message("invalid page range: step direction does not match range (from=%u to=%u step=%d)", pageStart, pageEnd, pageStep);
        return false;
    }

    PDFCanvas canvas(callback, closure, pageSizeAt(pageStart - 1));
    if(canvas.isNull())
        return false;
    canvas.scale(PLUTOBOOK_UNITS_PX, PLUTOBOOK_UNITS_PX);
    canvas.setTitle(m_title);
    canvas.setSubject(m_subject);
    canvas.setAuthor(m_author);
    canvas.setCreator("PlutoBook " PLUTOBOOK_VERSION_STRING " (https://github.com/plutoprint)");
    canvas.setKeywords(m_keywords);
    canvas.setCreationDate(m_creationDate);
    canvas.setModificationDate(m_modificationDate);
    for(auto pageNum = pageStart; pageStep > 0 ? pageNum <= pageEnd : pageNum >= pageEnd; pageNum += pageStep) {
        canvas.setPageSize(pageSizeAt(pageNum - 1));
        renderPage(canvas, pageNum - 1);
        canvas.showPage();
    }

    canvas.finish();
    return true;
}

bool Book::writeToPng(const std::string& filename, int width, int height) const
{
    FileOutputStream output(filename);
    if(!output.isOpen())
        return false;
    if(writeToPng(output, width, height)) {
        return true;
    }

    plutobook_set_error_message("Unable to write PNG '%s': %s", filename.data(), plutobook_get_error_message());
    return false;
}

bool Book::writeToPng(OutputStream& output, int width, int height) const
{
    return writeToPng(stream_write_func, &output, width, height);
}

bool Book::writeToPng(plutobook_stream_write_callback_t callback, void* closure, int width, int height) const
{
    const auto docWidth = documentWidth();
    const auto docHeight = documentHeight();
    if(docWidth <= 0 || docHeight <= 0) {
        plutobook_set_error_message("invalid document size: width=%g height=%g", docWidth, docHeight);
        return false;
    }

    if(width <= 0 && height <= 0) {
        width = static_cast<int>(std::ceil(docWidth));
        height = static_cast<int>(std::ceil(docHeight));
    } else if(width > 0 && height <= 0) {
        height = static_cast<int>(std::ceil(width * docHeight / docWidth));
    } else if(height > 0 && width <= 0) {
        width = static_cast<int>(std::ceil(height * docWidth / docHeight));
    }

    const auto xScale = width / docWidth;
    const auto yScale = height / docHeight;

    ImageCanvas canvas(width, height, ImageFormat::ARGB32);
    if(canvas.isNull())
        return false;
    canvas.scale(xScale, yScale);
    renderDocument(canvas, 0, 0, docWidth, docHeight);
    return canvas.writeToPng(callback, closure);
}

Document* Book::buildIfNeeded() const
{
    auto document = m_document.get();
    if(document && m_needsBuild) {
        document->build();
        m_needsBuild = false;
    }

    return document;
}

Document* Book::layoutIfNeeded() const
{
    auto document = buildIfNeeded();
    if(document && m_needsLayout) {
        document->setContainerSize(viewportWidth(), viewportHeight());
        document->layout();
        m_needsPagination = true;
        m_needsLayout = false;
    }

    return document;
}

Document* Book::paginateIfNeeded() const
{
    auto document = buildIfNeeded();
    if(document && m_needsPagination) {
        document->paginate();
        m_needsPagination = false;
        m_needsLayout = true;
    }

    return document;
}

} // namespace plutobook
