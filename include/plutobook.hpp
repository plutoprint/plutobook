/*
 * Copyright (c) 2022 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef PLUTOBOOK_HPP
#define PLUTOBOOK_HPP

#include <cstdint>
#include <cstddef>
#include <string>
#include <memory>
#include <vector>

#include "plutobook.h"

namespace plutobook {

/**
 * Defines an index that is guaranteed to exceed the valid page count.
 * It is typically utilized as a sentinel value to represent an unbounded or maximum value, indicating
 * that there is no limit or that the maximum possible value is intended.
 */
constexpr uint32_t kMaxPageCount = PLUTOBOOK_MAX_PAGE_COUNT;
constexpr uint32_t kMinPageCount = PLUTOBOOK_MIN_PAGE_COUNT;

/**
 * @brief The PageSize class represents a page size in points (1/72 inch), providing functionality
 * to manipulate and retrieve width and height.
 */
class PageSize {
public:
    constexpr PageSize() = default;
    constexpr PageSize(plutobook_page_size_t size) : PageSize(size.width, size.height) {}
    constexpr explicit PageSize(float size) : PageSize(size, size) {}
    constexpr PageSize(float width, float height) : m_width(width), m_height(height) {}

    constexpr void setWidth(float width) { m_width = width; }
    constexpr void setHeight(float height) { m_height = height; }

    constexpr float width() const { return m_width; }
    constexpr float height() const { return m_height; }

    constexpr PageSize landscape() const;
    constexpr PageSize portrait() const;

    constexpr operator plutobook_page_size_t() const { return {m_width, m_height}; }

    /**
     * @brief Predefined PageSize objects for common paper sizes.
     */
    static const PageSize A3;
    static const PageSize A4;
    static const PageSize A5;
    static const PageSize B4;
    static const PageSize B5;
    static const PageSize Letter;
    static const PageSize Legal;
    static const PageSize Ledger;

private:
    float m_width{0};
    float m_height{0};
};

constexpr PageSize PageSize::landscape() const
{
    if(m_width < m_height)
        return PageSize(m_height, m_width);
    return PageSize(m_width, m_height);
}

constexpr PageSize PageSize::portrait() const
{
    if(m_width > m_height)
        return PageSize(m_height, m_width);
    return PageSize(m_width, m_height);
}

inline const PageSize PageSize::A3 = PLUTOBOOK_PAGE_SIZE_A3;
inline const PageSize PageSize::A4 = PLUTOBOOK_PAGE_SIZE_A4;
inline const PageSize PageSize::A5 = PLUTOBOOK_PAGE_SIZE_A5;
inline const PageSize PageSize::B4 = PLUTOBOOK_PAGE_SIZE_B4;
inline const PageSize PageSize::B5 = PLUTOBOOK_PAGE_SIZE_B5;
inline const PageSize PageSize::Letter = PLUTOBOOK_PAGE_SIZE_LETTER;
inline const PageSize PageSize::Legal = PLUTOBOOK_PAGE_SIZE_LEGAL;
inline const PageSize PageSize::Ledger = PLUTOBOOK_PAGE_SIZE_LEDGER;

/**
 * @brief The PageMargins class represents the margins of a page in points (1/72 inch), providing functionality
 * to manipulate and retrieve top, right, bottom, and left margins.
 */
class PageMargins {
public:
    constexpr PageMargins() = default;
    constexpr PageMargins(plutobook_page_margins_t margins)
        : PageMargins(margins.top, margins.right, margins.bottom, margins.left)
    {}

    constexpr explicit PageMargins(float margin)
        : PageMargins(margin, margin, margin, margin)
    {}

    constexpr PageMargins(float vertical, float horizontal)
        : PageMargins(vertical, horizontal, horizontal, vertical)
    {}

    constexpr PageMargins(float top, float rightAndLeft, float bottom)
        : PageMargins(top, rightAndLeft, bottom, rightAndLeft)
    {}

    constexpr PageMargins(float top, float right, float bottom, float left)
        : m_top(top), m_right(right), m_bottom(bottom), m_left(left)
    {}

    constexpr void setTop(float top) { m_top = top; }
    constexpr void setRight(float right) { m_right = right; }
    constexpr void setBottom(float bottom) { m_bottom = bottom; }
    constexpr void setLeft(float left) { m_left = left; }

    constexpr float top() const { return m_top; }
    constexpr float right() const { return m_right; }
    constexpr float bottom() const { return m_bottom; }
    constexpr float left() const { return m_left; }

    constexpr operator plutobook_page_margins_t() const { return {m_top, m_right, m_bottom, m_left}; }

    /**
     * @brief Predefined PageMargins objects for common margin settings.
     */
    static const PageMargins None;
    static const PageMargins Normal;
    static const PageMargins Narrow;
    static const PageMargins Moderate;

private:
    float m_top{0};
    float m_right{0};
    float m_bottom{0};
    float m_left{0};
};

inline const PageMargins PageMargins::None = PLUTOBOOK_PAGE_MARGINS_NONE;
inline const PageMargins PageMargins::Normal = PLUTOBOOK_PAGE_MARGINS_NORMAL;
inline const PageMargins PageMargins::Narrow = PLUTOBOOK_PAGE_MARGINS_NARROW;
inline const PageMargins PageMargins::Moderate = PLUTOBOOK_PAGE_MARGINS_MODERATE;

/**
 * Defines conversion factors for various units to points (pt) and vice versa.
 * These conversion factors allow easy conversion between different units and points.
 *
 * Example Usage:
 *   - To convert 12 inches to points: 12 * units::in
 *   - To convert 12 points to inches: 12 / units::in
 */
namespace units {

constexpr float pt = PLUTOBOOK_UNITS_PT;
constexpr float pc = PLUTOBOOK_UNITS_PC;
constexpr float in = PLUTOBOOK_UNITS_IN;
constexpr float cm = PLUTOBOOK_UNITS_CM;
constexpr float mm = PLUTOBOOK_UNITS_MM;
constexpr float px = PLUTOBOOK_UNITS_PX;

} // units

class PLUTOBOOK_API ResourceData {
public:
    /**
     * @brief createWithCopy
     * @param content
     * @param contentLength
     * @param mimeType
     * @param textEncoding
     * @return
     */
    static ResourceData createWithCopy(const char* content, size_t contentLength, const std::string& mimeType, const std::string& textEncoding);

    /**
     * @brief createWithoutCopy
     * @param content
     * @param contentLength
     * @param mimeType
     * @param textEncoding
     * @param destroyCallback
     * @param closure
     * @return
     */
    static ResourceData createWithoutCopy(const char* content, size_t contentLength, const std::string& mimeType, const std::string& textEncoding,
        plutobook_resource_destroy_callback_t destroyCallback, void* closure);

    /**
     * @brief ResourceData
     */
    ResourceData() = default;

    /**
     * @brief ResourceData
     * @param data
     */
    explicit ResourceData(plutobook_resource_data_t* data) : m_data(data) {}

    /**
     * @brief ResourceData
     * @param resource
     */
    ResourceData(ResourceData&& resource) : m_data(resource.release()) {}

    /**
     * @brief ResourceData
     * @param resource
     */
    ResourceData(const ResourceData& resource);

    /**
     * @brief ~ResourceData
     */
    ~ResourceData();

    /**
     * @brief Copy assignment operator
     * @param resource
     * @return
     */
    ResourceData& operator=(const ResourceData& resource);

    /**
     * @brief Move assignment operator
     * @param resource
     * @return
     */
    ResourceData& operator=(ResourceData&& resource);

    /**
     * @brief swap
     * @param resource
     */
    void swap(ResourceData& resource);

    /**
     * @brief content
     * @return
     */
    const char* content() const;

    /**
     * @brief contentLength
     * @return
     */
    size_t contentLength() const;

    /**
     * @brief mimeType
     * @return
     */
    std::string_view mimeType() const;

    /**
     * @brief textEncoding
     * @return
     */
    std::string_view textEncoding() const;

    /**
     * @brief release
     * @return
     */
    plutobook_resource_data_t* release();

    /**
     * @brief data
     * @return
     */
    plutobook_resource_data_t* data() const { return m_data; }

    /**
     * @brief isNull
     * @return
     */
    bool isNull() const { return m_data == nullptr; }

private:
    plutobook_resource_data_t* m_data{nullptr};
};

/**
 * @brief The ResourceFetcher class is an abstract base class for fetching resources from a URL.
 */
class PLUTOBOOK_API ResourceFetcher {
public:
    /**
     * @brief Virtual destructor
     */
    virtual ~ResourceFetcher() = default;

    /**
     * @brief loadUrl
     * @param url
     * @return
     */
    virtual ResourceData loadUrl(const std::string& url) = 0;
};

/**
 * @brief defaultResourceFetcher
 * @return
 */
ResourceFetcher* defaultResourceFetcher();

/**
 * @brief The OutputStream is an abstract base class for writing data to an output stream.
 */
class PLUTOBOOK_API OutputStream {
public:
    /**
     * @brief Virtual destructor
     */
    virtual ~OutputStream() = default;

    /**
     * @brief Writes data to the output stream.
     * @param data A pointer to the buffer containing the data to be written.
     * @param length The length of the data to be written, in bytes.
     * @return true if the data was written successfully, false otherwise.
     */
    virtual bool write(const char* data, size_t length) = 0;
};

/**
 * @brief The Canvas class provides a basic interface for manipulating a 2D drawing surface.
 */
class PLUTOBOOK_API Canvas {
public:
    /**
     * @brief Destructor
     */
    virtual ~Canvas();

    /**
     * @brief Flushes any pending drawing operations.
     */
    void flush();

    /**
     * @brief Finishes all drawing operations and cleans up the canvas.
     */
    void finish();

    /**
     * @brief Translates the origin of the coordinate system.
     * @param tx The horizontal translation distance.
     * @param ty The vertical translation distance.
     */
    void translate(float tx, float ty);

    /**
     * @brief Scales the coordinate system.
     * @param sx The horizontal scaling factor.
     * @param sy The vertical scaling factor.
     */
    void scale(float sx, float sy);

    /**
     * @brief Rotates the coordinate system.
     * @param angle The rotation angle in radians.
     */
    void rotate(float angle);

    /**
     * @brief Multiplies the current transformation matrix with the specified matrix.
     * @param a The horizontal scaling factor.
     * @param b The horizontal skewing factor.
     * @param c The vertical skewing factor.
     * @param d The vertical scaling factor.
     * @param e The horizontal translation distance.
     * @param f The vertical translation distance.
     */
    void transform(float a, float b, float c, float d, float e, float f);

    /**
     * @brief Resets the current transformation to the identity matrix.
     * @param a The horizontal scaling factor.
     * @param b The horizontal skewing factor.
     * @param c The vertical skewing factor.
     * @param d The vertical scaling factor.
     * @param e The horizontal translation distance.
     * @param f The vertical translation distance.
     */
    void setMatrix(float a, float b, float c, float d, float e, float f);

    /**
     * @brief Resets the transformation matrix to the identity matrix.
     */
    void resetMatrix();

    /**
     * @brief Intersects the current clip with the specified rectangle.
     * @param x The x-coordinate of the top-left corner of the rectangle.
     * @param y The y-coordinate of the top-left corner of the rectangle.
     * @param width width The width of the rectangle.
     * @param height height The height of the rectangle.
     */
    void clipRect(float x, float y, float width, float height);

    /**
     * @brief Clears the canvas surface with the specified color.
     * @param red The red component of the color.
     * @param green The green component of the color.
     * @param blue The blue component of the color.
     * @param alpha The alpha component of the color.
     */
    void clearSurface(float red, float green, float blue, float alpha);

    /**
     * @brief Saves the current drawing state.
     */
    void saveState();

    /**
     * @brief Restores the most recently saved drawing state.
     */
    void restoreState();

    /**
     * @brief surface
     * @return
     */
    cairo_surface_t* surface() const;

    /**
     * @brief context
     * @return
     */
    cairo_t* context() const;

    /**
     * @brief canvas
     * @return
     */
    plutobook_canvas_t* canvas() const { return m_canvas; }

    /**
     * @brief isNull
     * @return
     */
    bool isNull() const { return !m_canvas; }

    /**
     * @brief isGood
     * @return
     */
    bool isGood() const;

protected:
    Canvas(const Canvas&) = delete;
    Canvas& operator=(const Canvas&) = delete;
    Canvas(plutobook_canvas_t* canvas) : m_canvas(canvas) {}
    plutobook_canvas_t* m_canvas;
};

enum class ImageFormat {
    Invalid = PLUTOBOOK_IMAGE_FORMAT_INVALID,
    ARGB32 = PLUTOBOOK_IMAGE_FORMAT_ARGB32,
    RGB24 = PLUTOBOOK_IMAGE_FORMAT_RGB24,
    A8 = PLUTOBOOK_IMAGE_FORMAT_A8,
    A1 = PLUTOBOOK_IMAGE_FORMAT_A1
};

class PLUTOBOOK_API ImageCanvas final : public Canvas {
public:
    /**
     * @brief Constructs an ImageCanvas with the specified width, height, and optional image format.
     * @param width The width of the canvas.
     * @param height The height of the canvas.
     * @param format The format of the image
     */
    ImageCanvas(int width, int height, ImageFormat format = ImageFormat::ARGB32);

    /**
     * @brief Constructs an ImageCanvas with the provided image data, width, height, stride, and optional image format.
     * @param data A pointer to the image data.
     * @param width The width of the image data.
     * @param height The height of the image data.
     * @param stride The stride of the image data.
     * @param format The format of the image data
     */
    ImageCanvas(uint8_t* data, int width, int height, int stride, ImageFormat format = ImageFormat::ARGB32);

    /**
     * @brief Retrieves a pointer to the image data.
     * @return A pointer to the image data.
     */
    uint8_t* data() const;

    /**
     * @brief Retrieves the width of the canvas.
     * @return The width of the canvas.
     */
    int width() const;

    /**
     * @brief Retrieves the height of the canvas.
     * @return The height of the canvas.
     */
    int height() const;

    /**
     * @brief Retrieves the stride of the image data.
     * @return The stride of the image data.
     */
    int stride() const;

    /**
     * @brief Retrieves the format of the image data.
     * @return The format of the image data.
     */
    ImageFormat format() const;

    /**
     * @brief Writes the image data to a PNG file.
     * @param filename The name of the PNG file to write to.
     * @return true if the image was successfully written to the file, false otherwise.
     */
    bool writeToPng(const std::string& filename) const;

    /**
     * @brief Writes the image data to an output stream in PNG format.
     * @param output The output stream to write to.
     * @return true if the image was successfully written to the output stream, false otherwise.
     */
    bool writeToPng(OutputStream& output) const;

    /**
     * @brief Writes the image data to a PNG file using a custom write callback.
     * @param callback The callback function for writing data
     * @param closure A pointer to user-defined data to be passed to the callback.
     * @return true if the image was successfully written using the callback, false otherwise.
     */
    bool writeToPng(plutobook_stream_write_callback_t callback, void* closure) const;
};

/**
 * @brief The PDFCanvas class represents a canvas for creating PDF documents.
 */
class PLUTOBOOK_API PDFCanvas final : public Canvas {
public:
    /**
     * @brief Constructs a PDFCanvas that writes to a PDF file with the specified filename and page size.
     * @param filename The name of the PDF file to create.
     * @param pageSize The size of the pages in the PDF.
     */
    PDFCanvas(const std::string& filename, const PageSize& pageSize);

    /**
     * @brief Constructs a PDFCanvas that writes to an output stream with the specified page size.
     * @param output The output stream to write the PDF to.
     * @param pageSize The size of the pages in the PDF.
     */
    PDFCanvas(OutputStream& output, const PageSize& pageSize);

    /**
     * @brief Constructs a PDFCanvas that uses a custom write callback and closure with the specified page size.
     * @param callback The callback function for writing PDF data.
     * @param closure A pointer to user-defined data to be passed to the callback.
     * @param pageSize The size of the pages in the PDF.
     */
    PDFCanvas(plutobook_stream_write_callback_t callback, void* closure, const PageSize& pageSize);

    /**
     * @brief Sets the title of the PDF document.
     * @param title The title of the PDF document.
     */
    void setTitle(const std::string& title);

    /**
     * @brief Sets the author of the PDF document.
     * @param author The author of the PDF document.
     */
    void setAuthor(const std::string& author);

    /**
     * @brief Sets the subject of the PDF document.
     * @param subject The subject of the PDF document.
     */
    void setSubject(const std::string& subject);

    /**
     * @brief Sets the keywords associated with the PDF document.
     * @param keywords The keywords associated with the PDF document.
     */
    void setKeywords(const std::string& keywords);

    /**
     * @brief Sets the creator of the PDF document.
     * @param creator The creator of the PDF document.
     */
    void setCreator(const std::string& creator);

    /**
     * @brief Sets the creation date of the PDF document.
     * @param creationDate The creation date of the PDF document.
     */
    void setCreationDate(const std::string& creationDate);

    /**
     * @brief Sets the modification date of the PDF document.
     * @param modificationDate The modification date of the PDF document.
     */
    void setModificationDate(const std::string& modificationDate);

    /**
     * @brief Sets the page size of the PDF document.
     * @param pageSize The page size of the PDF document.
     */
    void setPageSize(const PageSize& pageSize);

    /**
     * @brief Signals the end of a page and starts a new page.
     */
    void showPage();
};

class Heap;
class Document;

enum class MediaType {
    Print = PLUTOBOOK_MEDIA_TYPE_PRINT,
    Screen = PLUTOBOOK_MEDIA_TYPE_SCREEN
};

class PLUTOBOOK_API Book {
public:
    /**
     * @brief Book
     * @param size
     * @param margins
     * @param media
     */
    Book(const PageSize& size, const PageMargins& margins = PageMargins::Normal, MediaType media = MediaType::Print);

    /**
     * @brief ~Book
     */
    ~Book();

    /**
     * @brief setTitle
     * @param title
     */
    void setTitle(std::string title) { m_title = std::move(title); }

    /**
     * @brief title
     * @return
     */
    const std::string& title() const { return m_title; }

    /**
     * @brief setAuthor
     * @param author
     */
    void setAuthor(std::string author) { m_author = std::move(author); }

    /**
     * @brief author
     * @return
     */
    const std::string& author() const { return m_author; }

    /**
     * @brief setSubject
     * @param subject
     */
    void setSubject(std::string subject) { m_subject = std::move(subject); }

    /**
     * @brief subject
     * @return
     */
    const std::string& subject() const { return m_subject; }

    /**
     * @brief setKeywords
     * @param keywords
     */
    void setKeywords(std::string keywords) { m_keywords = std::move(keywords); }

    /**
     * @brief keywords
     * @return
     */
    const std::string& keywords() const { return m_keywords; }

    /**
     * @brief setCreator
     * @param creator
     */
    void setCreator(std::string creator) { m_creator = std::move(creator); }

    /**
     * @brief creator
     * @return
     */
    const std::string& creator() const { return m_creator; }

    /**
     * @brief setCreationDate
     * @param creationDate
     */
    void setCreationDate(std::string creationDate) { m_creationDate = std::move(creationDate); }

    /**
     * @brief creationDate
     * @return
     */
    const std::string& creationDate() const { return m_creationDate; }

    /**
     * @brief setModificationDate
     * @param modificationDate
     */
    void setModificationDate(std::string modificationDate) { m_modificationDate = std::move(modificationDate); }

    /**
     * @brief modificationDate
     * @return
     */
    const std::string& modificationDate() const { return m_modificationDate; }

    /**
     * @brief viewportWidth
     * @return
     */
    float viewportWidth() const;

    /**
     * @brief viewportHeight
     * @return
     */
    float viewportHeight() const;

    /**
     * @brief documentWidth
     * @return
     */
    float documentWidth() const;

    /**
     * @brief documentHeight
     * @return
     */
    float documentHeight() const;

    /**
     * @brief pageSize
     * @return
     */
    const PageSize& pageSize() const { return m_pageSize; }

    /**
     * @brief pageMargins
     * @return
     */
    const PageMargins& pageMargins() const { return m_pageMargins; }

    /**
     * @brief mediaType
     * @return
     */
    MediaType mediaType() const { return m_mediaType; }

    /**
     * @brief pageCount
     * @return
     */
    uint32_t pageCount() const;

    /**
     * @brief pageSizeAt
     * @param pageIndex
     * @return
     */
    PageSize pageSizeAt(uint32_t pageIndex) const;

    /**
     * @brief loadUrl
     * @param url
     * @param userStyle
     * @param userScript
     * @return
     */
    bool loadUrl(const std::string_view& url, const std::string_view& userStyle = {}, const std::string_view& userScript = {});

    /**
     * @brief loadData
     * @param data
     * @param length
     * @param mimeType
     * @param textEncoding
     * @param userStyle
     * @param userScript
     * @param baseUrl
     * @return
     */
    bool loadData(const char* data, size_t length, const std::string_view& mimeType = {}, const std::string_view& textEncoding = {},
        const std::string_view& userStyle = {}, const std::string_view& userScript = {}, const std::string_view& baseUrl = {});

    /**
     * @brief loadImage
     * @param data
     * @param length
     * @param mimeType
     * @param textEncoding
     * @param userStyle
     * @param userScript
     * @param baseUrl
     * @return
     */
    bool loadImage(const char* data, size_t length, const std::string_view& mimeType = {}, const std::string_view& textEncoding = {},
        const std::string_view& userStyle = {}, const std::string_view& userScript = {}, const std::string_view& baseUrl = {});

    /**
     * @brief loadXml
     * @param content
     * @param userStyle
     * @param userScript
     * @param baseUrl
     * @return
     */
    bool loadXml(const std::string_view& content, const std::string_view& userStyle = {}, const std::string_view& userScript = {}, const std::string_view& baseUrl = {});

    /**
     * @brief loadHtml
     * @param content
     * @param userStyle
     * @param userScript
     * @param baseUrl
     * @return
     */
    bool loadHtml(const std::string_view& content, const std::string_view& userStyle = {}, const std::string_view& userScript = {}, const std::string_view& baseUrl = {});

    /**
     * @brief clearContent
     */
    void clearContent();

    /**
     * @brief renderPage
     * @param canvas
     * @param pageIndex
     */
    void renderPage(Canvas& canvas, uint32_t pageIndex) const;

    /**
     * @brief renderPage
     * @param canvas
     * @param pageIndex
     */
    void renderPage(plutobook_canvas_t* canvas, uint32_t pageIndex) const;

    /**
     * @brief renderPage
     * @param canvas
     * @param pageIndex
     */
    void renderPage(cairo_t* canvas, uint32_t pageIndex) const;

    /**
     * @brief renderDocument
     * @param canvas
     */
    void renderDocument(Canvas& canvas) const;

    /**
     * @brief renderDocument
     * @param canvas
     * @param x
     * @param y
     * @param width
     * @param height
     */
    void renderDocument(Canvas& canvas, float x, float y, float width, float height) const;

    /**
     * @brief renderDocument
     * @param canvas
     */
    void renderDocument(plutobook_canvas_t* canvas) const;

    /**
     * @brief renderDocument
     * @param canvas
     * @param x
     * @param y
     * @param width
     * @param height
     */
    void renderDocument(plutobook_canvas_t* canvas, float x, float y, float width, float height) const;

    /**
     * @brief renderDocument
     * @param canvas
     */
    void renderDocument(cairo_t* canvas) const;

    /**
     * @brief renderDocument
     * @param canvas
     * @param x
     * @param y
     * @param width
     * @param height
     */
    void renderDocument(cairo_t* canvas, float x, float y, float width, float height) const;

    /**
     * @brief writeToPdf
     * @param filename
     * @param fromPage
     * @param toPage
     * @param pageStep
     * @return
     */
    bool writeToPdf(const std::string& filename, uint32_t fromPage = kMinPageCount, uint32_t toPage = kMaxPageCount, int pageStep = 1) const;

    /**
     * @brief writeToPdf
     * @param output
     * @param fromPage
     * @param toPage
     * @param pageStep
     * @return
     */
    bool writeToPdf(OutputStream& output, uint32_t fromPage = kMinPageCount, uint32_t toPage = kMaxPageCount, int pageStep = 1) const;

    /**
     * @brief writeToPdf
     * @param callback
     * @param closure
     * @param fromPage
     * @param toPage
     * @param pageStep
     * @return
     */
    bool writeToPdf(plutobook_stream_write_callback_t callback, void* closure, uint32_t fromPage = kMinPageCount, uint32_t toPage = kMaxPageCount, int pageStep = 1) const;

    /**
     * @brief writeToPng
     * @param filename
     * @param format
     * @return
     */
    bool writeToPng(const std::string& filename, ImageFormat format = ImageFormat::ARGB32) const;

    /**
     * @brief writeToPng
     * @param output
     * @param format
     * @return
     */
    bool writeToPng(OutputStream& output, ImageFormat format = ImageFormat::ARGB32) const;

    /**
     * @brief writeToPng
     * @param callback
     * @param closure
     * @param format
     * @return
     */
    bool writeToPng(plutobook_stream_write_callback_t callback, void* closure, ImageFormat format = ImageFormat::ARGB32) const;

    /**
     * @brief setCustomResourceFetcher
     * @param fetcher
     */
    void setCustomResourceFetcher(ResourceFetcher* fetcher) { m_customResourceFetcher = fetcher; }

    /**
     * @brief customResourceFetcher
     * @return
     */
    ResourceFetcher* customResourceFetcher() const { return m_customResourceFetcher; }

    /**
     * @brief heap
     * @return
     */
    Heap* heap() const { return m_heap.get(); }

    /**
     * @brief document
     * @return
     */
    Document* document() const { return m_document.get(); }

private:
    Document* buildIfNeeded() const;
    Document* layoutIfNeeded() const;
    Document* paginateIfNeeded() const;

    PageSize m_pageSize;
    PageMargins m_pageMargins;
    MediaType m_mediaType;

    mutable bool m_needsBuild{true};
    mutable bool m_needsLayout{true};
    mutable bool m_needsPagination{true};

    std::string m_title;
    std::string m_author;
    std::string m_subject;
    std::string m_keywords;
    std::string m_creator;
    std::string m_creationDate;
    std::string m_modificationDate;

    ResourceFetcher* m_customResourceFetcher{nullptr};
    std::unique_ptr<Heap> m_heap;
    std::unique_ptr<Document> m_document;
};

} // namespace plutobook

#endif // PLUTOBOOK_HPP
