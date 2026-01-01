/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_HPP
#define PLUTOBOOK_HPP

#include <cstdint>
#include <cstddef>
#include <string>
#include <memory>

#include "plutobook.h"

namespace plutobook {

/**
 * This constant defines an index that is guaranteed to be greater than any valid page count.
 * It is typically used as a sentinel value to represent an unbounded upper limit or the maximum possible value,
 * indicating that no specific upper bound is set.
 */
constexpr uint32_t kMaxPageCount = PLUTOBOOK_MAX_PAGE_COUNT;

/**
 * This constant defines an index that is guaranteed to be less than any valid page count.
 * It is typically used as a sentinel value to represent an unbounded lower limit or the minimum possible value,
 * indicating that no specific lower bound is set.
 */
constexpr uint32_t kMinPageCount = PLUTOBOOK_MIN_PAGE_COUNT;

/**
 * @brief The PageSize class represents the dimensions of a page in points (1/72 inch).
 */
class PageSize {
public:
    /**
     * @brief Constructs a PageSize object with width and height set to 0.
     */
    constexpr PageSize() = default;

    /**
     * @brief Constructs a PageSize object from a `plutobook_page_size_t` object.
     *
     * @param size A `plutobook_page_size_t` object.
     */
    constexpr PageSize(plutobook_page_size_t size) : PageSize(size.width, size.height) {}

    /**
     * @brief Constructs a PageSize object with the same width and height.
     *
     * @param size The value to set for both width and height in points.
     */
    constexpr explicit PageSize(float size) : PageSize(size, size) {}

    /**
     * @brief Constructs a PageSize object with the specified width and height.
     *
     * @param width The width of the page in points.
     * @param height The height of the page in points.
     */
    constexpr PageSize(float width, float height) : m_width(width), m_height(height) {}

    /**
     * @brief Sets the width of the page.
     *
     * @param width The width of the page in points.
     */
    constexpr void setWidth(float width) { m_width = width; }

    /**
     * @brief Sets the height of the page.
     *
     * @param height The height of the page in points.
     */
    constexpr void setHeight(float height) { m_height = height; }

    /**
     * @brief Gets the width of the page.
     *
     * @return The width of the page in points.
     */
    constexpr float width() const { return m_width; }

    /**
     * @brief Gets the height of the page.
     *
     * @return The height of the page in points.
     */
    constexpr float height() const { return m_height; }

    /**
     * @brief Returns the page size in landscape orientation.
     *
     * @return A `PageSize` object in landscape orientation.
     */
    constexpr PageSize landscape() const;

    /**
     * @brief Returns the page size in portrait orientation.
     *
     * @return A `PageSize` object in portrait orientation.
     */
    constexpr PageSize portrait() const;

    /**
     * @brief Converts the `PageSize` object to a `plutobook_page_size_t` object.
     *
     * @return A `plutobook_page_size_t` object with the same width and height values.
     */
    constexpr operator plutobook_page_size_t() const { return PLUTOBOOK_MAKE_PAGE_SIZE(m_width, m_height); }

    static const PageSize A3;     ///< Represents the A3 page size (297 x 420 mm).
    static const PageSize A4;     ///< Represents the A4 page size (210 x 297 mm).
    static const PageSize A5;     ///< Represents the A5 page size (148 x 210 mm).
    static const PageSize B4;     ///< Represents the B4 page size (250 x 353 mm).
    static const PageSize B5;     ///< Represents the B5 page size (176 x 250 mm).
    static const PageSize Letter; ///< Represents the Letter page size (8.5 x 11 inches).
    static const PageSize Legal;  ///< Represents the Legal page size (8.5 x 14 inches).
    static const PageSize Ledger; ///< Represents the Ledger page size (11 x 17 inches).

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
 * @brief The PageMargins class represents the margins of a page in points (1/72 inch).
 */
class PageMargins {
public:
    /**
     * @brief Constructs a PageMargins object with all margins set to 0.
     */
    constexpr PageMargins() = default;

    /**
     * @brief Constructs a PageMargins object from a `plutobook_page_margins_t` object.
     *
     * @param margins A `plutobook_page_margins_t` object.
     */
    constexpr PageMargins(plutobook_page_margins_t margins)
        : PageMargins(margins.top, margins.right, margins.bottom, margins.left)
    {}

    /**
     * @brief Constructs a PageMargins object with the same margin on all sides.
     *
     * @param margin The margin value to apply to all four sides in points.
     */
    constexpr explicit PageMargins(float margin)
        : PageMargins(margin, margin, margin, margin)
    {}

    /**
     * @brief Constructs a PageMargins object with vertical and horizontal margins.
     *
     * @param vertical The margin for the top and bottom sides, in points.
     * @param horizontal The margin for the left and right sides, in points.
     */
    constexpr PageMargins(float vertical, float horizontal)
        : PageMargins(vertical, horizontal, vertical, horizontal)
    {}

    /**
     * @brief Constructs a PageMargins object with vertical and horizontal margins.
     *
     * @param top The top margin in points.
     * @param horizontal The margin for the left and right sides, in points.
     * @param bottom The bottom margin in points.
     */
    constexpr PageMargins(float top, float horizontal, float bottom)
        : PageMargins(top, horizontal, bottom, horizontal)
    {}

    /**
     * @brief Constructs a PageMargins object with specific margins for each side.
     *
     * @param top The margin for the top side, in points.
     * @param right The margin for the right side, in points.
     * @param bottom The margin for the bottom side, in points.
     * @param left The margin for the left side, in points.
     */
    constexpr PageMargins(float top, float right, float bottom, float left)
        : m_top(top), m_right(right), m_bottom(bottom), m_left(left)
    {}

    /**
     * @brief Sets the top margin.
     *
     * @param top The margin value for the top side, in points.
     */
    constexpr void setTop(float top) { m_top = top; }

    /**
     * @brief Sets the right margin.
     *
     * @param right The margin value for the right side, in points.
     */
    constexpr void setRight(float right) { m_right = right; }

    /**
     * @brief Sets the bottom margin.
     *
     * @param bottom The margin value for the bottom side, in points.
     */
    constexpr void setBottom(float bottom) { m_bottom = bottom; }

    /**
     * @brief Sets the left margin.
     *
     * @param left The margin value for the left side, in points.
     */
    constexpr void setLeft(float left) { m_left = left; }

    /**
     * @brief Gets the top margin.
     *
     * @return The margin value for the top side, in points.
     */
    constexpr float top() const { return m_top; }

    /**
     * @brief Gets the right margin.
     *
     * @return The margin value for the right side, in points.
     */
    constexpr float right() const { return m_right; }

    /**
     * @brief Gets the bottom margin.
     *
     * @return The margin value for the bottom side, in points.
     */
    constexpr float bottom() const { return m_bottom; }

    /**
     * @brief Gets the left margin.
     *
     * @return The margin value for the left side, in points.
     */
    constexpr float left() const { return m_left; }

    /**
     * @brief Converts the `PageMargins` object to a `plutobook_page_margins_t` object.
     *
     * @return A `plutobook_page_margins_t` object with the same margin values.
     */
    constexpr operator plutobook_page_margins_t() const { return PLUTOBOOK_MAKE_PAGE_MARGINS(m_top, m_right, m_bottom, m_left); }

    /**
     * @brief Represents page margins with zero dimensions on all sides.
     *
     * - Top: 0 points
     * - Right: 0 points
     * - Bottom: 0 points
     * - Left: 0 points
     */
    static const PageMargins None;

    /**
     * @brief Represents normal page margins (72 points or 1 inch on all sides).
     *
     * - Top: 72 points (1 inch)
     * - Right: 72 points (1 inch)
     * - Bottom: 72 points (1 inch)
     * - Left: 72 points (1 inch)
     */
    static const PageMargins Normal;

    /**
     * @brief Represents narrow page margins (36 points or 0.5 inches on all sides).
     *
     * - Top: 36 points (0.5 inches)
     * - Right: 36 points (0.5 inches)
     * - Bottom: 36 points (0.5 inches)
     * - Left: 36 points (0.5 inches)
     */
    static const PageMargins Narrow;

    /**
     * @brief Represents moderate page margins.
     *
     * - Top: 72 points (1 inch)
     * - Right: 54 points (0.75 inches)
     * - Bottom: 72 points (1 inch)
     * - Left: 54 points (0.75 inches)
     */
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

/**
 * @brief The ResourceData class represents a piece of fetched data (resource).
 */
class PLUTOBOOK_API ResourceData {
public:
    /**
     * @brief Constructs a ResourceData object by initializing the underlying resource with a null pointer.
     */
    ResourceData() : m_data(nullptr) {}

    /**
     * @brief Constructs a `ResourceData` object by adopting a bare pointer to the resource without modifying the reference count.
     *
     * @param data A bare pointer to a `plutobook_resource_data_t` object.
     */
    explicit ResourceData(plutobook_resource_data_t* data) : m_data(data) {}

    /**
     * @brief Constructs a ResourceData object by copying the provided content.
     *
     * @param content The content of the resource.
     * @param contentLength The length of the content in bytes.
     * @param mimeType The MIME type of the content.
     * @param textEncoding The text encoding used for the content.
     */
    ResourceData(const char* content, size_t contentLength, const std::string& mimeType, const std::string& textEncoding);

    /**
     * @brief Constructs a ResourceData object using the provided content "as is", and uses the provided callback to clean up the resource when it is no longer needed.
     *
     * This method does not copy the content. It takes ownership and ensures cleanup via the specified callback.
     *
     * @param content The content of the resource.
     * @param contentLength The length of the content in bytes.
     * @param mimeType The MIME type of the content.
     * @param textEncoding The text encoding used for the content.
     * @param destroyCallback A callback function that is called when the resource is destroyed.
     * @param closure A pointer to user-defined data that is passed to the destroy callback.
     */
    ResourceData(const char* content, size_t contentLength, const std::string& mimeType, const std::string& textEncoding, plutobook_resource_destroy_callback_t destroyCallback, void* closure);

    /**
     * @brief Constructs a ResourceData object by transferring ownership from another `ResourceData` object.
     *
     * The move constructor releases the underlying resource from the provided `ResourceData` object and takes ownership of it,
     * effectively leaving the original object in a null state.
     *
     * @param resource A `ResourceData` object.
     */
    ResourceData(ResourceData&& resource) : m_data(resource.release()) {}

    /**
     * @brief Constructs a ResourceData object by sharing the underlying resource.
     *
     * This constructor shares the underlying resource from the provided `ResourceData` object
     * by incrementing its reference count, allowing multiple `ResourceData` objects to share ownership.
     *
     * @param resource A `ResourceData` object to share the resource from.
     */
    ResourceData(const ResourceData& resource);

    /**
     * @brief Destroys the `ResourceData` object by decreasing the reference count of the underlying resource.
     *
     * When the `ResourceData` object is destroyed, it decreases the reference count of the underlying
     * resource. If the reference count reaches zero, the resource is released and its memory is freed.
     */
    ~ResourceData();

    /**
     * @brief Copy assignment operator that shares ownership of the underlying resource.
     *
     * This operator performs a deep copy of the underlying resource by sharing ownership with the `resource`
     * object. The reference count of the underlying resource is incremented to reflect the new ownership.
     *
     * @param resource A `ResourceData` object to copy the resource from.
     * @return A reference to the current `ResourceData` object after the assignment.
     */
    ResourceData& operator=(const ResourceData& resource);

    /**
     * @brief Move assignment operator that transfers ownership of the underlying resource.
     * This operator releases the underlying resource from the `resource` object and takes ownership of it,
     * leaving the original object in a null state.
     *
     * @param resource A `ResourceData` object to move the resource from.
     * @return A reference to the current `ResourceData` object after the assignment.
     */
    ResourceData& operator=(ResourceData&& resource);

    /**
     * @brief Swaps the underlying resources of two `ResourceData` objects.
     *
     * This function swaps the resource data between the current object and the provided `resource` object.
     * After the swap, both objects will share their respective resources.
     *
     * @param resource The `ResourceData` object to swap with.
     */
    void swap(ResourceData& resource);

    /**
     * @brief Retrieves the content of the resource.
     * @return A pointer to the content of the resource.
     */
    const char* content() const;

    /**
     * @brief Retrieves the length of the resource content.
     * @return The length of the resource content in bytes.
     */
    size_t contentLength() const;

    /**
     * @brief Retrieves the MIME type of the resource content.
     * @return The MIME type of the resource content.
     */
    std::string_view mimeType() const;

    /**
     * @brief Retrieves the text encoding used for the resource content.
     * @return The text encoding used for the resource content.
     */
    std::string_view textEncoding() const;

    /**
     * @brief Releases the underlying resource and transfers ownership.
     * @return A pointer to the underlying `plutobook_resource_data_t` object, which is now owned by the caller.
     */
    plutobook_resource_data_t* release();

    /**
     * @brief Retrieves the underlying resource.
     * @return A pointer to the underlying `plutobook_resource_data_t` resource.
     */
    plutobook_resource_data_t* get() const { return m_data; }

    /**
     * @brief Checks if the resource is null.
     *
     * This function checks if the resource data is null (i.e., not initialized or invalid).
     *
     * @return `true` if the resource is null, otherwise `false`.
     */
    bool isNull() const { return m_data == nullptr; }

private:
    plutobook_resource_data_t* m_data;
};

/**
 * @brief The ResourceFetcher class is an abstract base class for fetching resources from a URL.
 */
class PLUTOBOOK_API ResourceFetcher {
public:
    /**
     * @brief Destructor
     */
    virtual ~ResourceFetcher() = default;

    /**
     * @brief Fetches a resource from the specified URL.
     * @param url The URL of the resource to fetch.
     * @return A `ResourceData` object containing the fetched resource, or a null `ResourceData` if an error occurs.
     */
    virtual ResourceData fetchUrl(const std::string& url) = 0;
};

/**
 * @brief The DefaultResourceFetcher class provides a default implementation of ResourceFetcher.
 */
class PLUTOBOOK_API DefaultResourceFetcher final : public ResourceFetcher {
public:
    /**
     * @brief Destroys the DefaultResourceFetcher instance.
     */
    ~DefaultResourceFetcher() final;

    /**
     * @brief Sets the path to a file containing trusted CA certificates.
     *
     * If not set, no custom CA file is used.
     *
     * @param path Path to the CA certificate bundle file.
     */
    void setCAInfo(std::string path) { m_caInfo = std::move(path); }

    /**
     * @brief Sets the path to a directory containing trusted CA certificates.
     *
     * If not set, no custom CA path is used.
     *
     * @param path Path to the directory with CA certificates.
     */
    void setCAPath(std::string path) { m_caPath = std::move(path); }

    /**
     * @brief Enables or disables SSL peer certificate verification.
     *
     * If not set, verification is enabled by default.
     *
     * @param verify Set to true to verify the peer, false to disable verification.
     */
    void setVerifyPeer(bool verify) { m_verifyPeer = verify; }

    /**
     * @brief Enables or disables SSL host name verification.
     *
     * If not set, verification is enabled by default.
     *
     * @param verify Set to true to verify the host, false to disable verification.
     */
    void setVerifyHost(bool verify) { m_verifyHost = verify; }

    /**
     * @brief Enables or disables automatic following of HTTP redirects.
     *
     * If not set, following redirects is enabled by default.
     *
     * @param follow Set to true to follow redirects, false to disable.
     */
    void setFollowRedirects(bool follow) { m_followRedirects = follow; }

    /**
     * @brief Sets the maximum number of redirects to follow.
     *
     * If not set, the default maximum is 30.
     *
     * @param amount The maximum number of redirects.
     */
    void setMaxRedirects(int amount) { m_maxRedirects = amount; }

    /**
     * @brief Sets the maximum time allowed for a request.
     *
     * If not set, the default timeout is 300 seconds.
     *
     * @param timeout Timeout duration in seconds.
     */
    void setTimeout(int timeout) { m_timeout = timeout; }

    /**
     * @brief Fetches the resource at the specified URL.
     *
     * @param url The resource URL to fetch.
     * @return The fetched resource data.
     */
    ResourceData fetchUrl(const std::string& url) final;

private:
    DefaultResourceFetcher();

    std::string m_caInfo;
    std::string m_caPath;

    bool m_verifyPeer = true;
    bool m_verifyHost = true;

    bool m_followRedirects = true;
    int m_maxRedirects = 30;
    int m_timeout = 300;

    friend DefaultResourceFetcher* defaultResourceFetcher();
};

/**
 * @brief Returns a singleton instance of DefaultResourceFetcher.
 * @return A pointer to the singleton DefaultResourceFetcher instance.
 */
DefaultResourceFetcher* defaultResourceFetcher();

/**
 * @brief The OutputStream is an abstract base class for writing data to an output stream.
 */
class PLUTOBOOK_API OutputStream {
public:
    /**
     * @brief Destructor
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
     * @brief Gets the underlying cairo surface associated with the canvas.
     * @return A pointer to the underlying `cairo_surface_t` object.
     */
    cairo_surface_t* surface() const;

    /**
     * @brief Gets the underlying cairo context associated with the canvas.
     * @return A pointer to the underlying `cairo_t` object.
     */
    cairo_t* context() const;

    /**
     * @brief Gets the underlying canvas.
     * @return A pointer to the underlying `plutobook_canvas_t` object.
     */
    plutobook_canvas_t* canvas() const { return m_canvas; }

    /**
     * @brief Checks if the canvas is null.
     * @return `true` if the canvas is null, otherwise `false`.
     */
    bool isNull() const { return m_canvas == nullptr; }

protected:
    Canvas(const Canvas&) = delete;
    Canvas& operator=(const Canvas&) = delete;
    Canvas(plutobook_canvas_t* canvas) : m_canvas(canvas) {}
    plutobook_canvas_t* m_canvas;
};

/**
 * @brief Defines different memory formats for image data.
 */
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
     *
     * The date must be in ISO-8601 format: YYYY-MM-DDThh:mm:ss, with an optional timezone "[+/-]hh:mm" or "Z" for UTC.
     *
     * @param creationDate The creation date of the PDF document.
     */
    void setCreationDate(const std::string& creationDate);

    /**
     * @brief Sets the modification date of the PDF document.
     *
     * The date must be in ISO-8601 format: YYYY-MM-DDThh:mm:ss, with an optional timezone "[+/-]hh:mm" or "Z" for UTC.
     *
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

/**
 * @brief Defines the different media types used for CSS @media queries.
 */
enum class MediaType {
    Print = PLUTOBOOK_MEDIA_TYPE_PRINT,
    Screen = PLUTOBOOK_MEDIA_TYPE_SCREEN
};

class PLUTOBOOK_API Book {
public:
    /**
     * @brief Constructs a Book object with the given page size, margins, and media type.
     *
     * @param size The initial page size.
     * @param margins The initial page margins.
     * @param media The media type used for media queries.
     */
    Book(const PageSize& size, const PageMargins& margins = PageMargins::Normal, MediaType media = MediaType::Print);

    /**
     * @brief Destructor
     */
    ~Book();

    /**
     * @brief Sets the title of the document.
     * @param title The title of the document.
     */
    void setTitle(std::string title) { m_title = std::move(title); }

    /**
     * @brief Gets the title of the document.
     * @return The title of the document.
     */
    const std::string& title() const { return m_title; }

    /**
     * @brief Sets the author of the document.
     * @param author The author of the document.
     */
    void setAuthor(std::string author) { m_author = std::move(author); }

    /**
     * @brief Gets the author of the document.
     * @return The author of the document.
     */
    const std::string& author() const { return m_author; }

    /**
     * @brief Sets the subject of the document.
     * @param subject The subject of the document.
     */
    void setSubject(std::string subject) { m_subject = std::move(subject); }

    /**
     * @brief Gets the subject of the document.
     * @return The subject of the document.
     */
    const std::string& subject() const { return m_subject; }

    /**
     * @brief Sets the keywords associated with the document.
     * @param keywords The keywords associated with the document.
     */
    void setKeywords(std::string keywords) { m_keywords = std::move(keywords); }

    /**
     * @brief Gets the keywords associated with the document.
     * @return The keywords associated with the document.
     */
    const std::string& keywords() const { return m_keywords; }

    /**
     * @brief Sets the creator of the document.
     * @param creator The creator of the document.
     */
    void setCreator(std::string creator) { m_creator = std::move(creator); }

    /**
     * @brief Gets the creator of the document.
     * @return The creator of the document.
     */
    const std::string& creator() const { return m_creator; }

    /**
     * @brief Sets the creation date of the document.
     *
     * The date must be in ISO-8601 format: YYYY-MM-DDThh:mm:ss, with an optional timezone "[+/-]hh:mm" or "Z" for UTC.
     *
     * @param creationDate The creation date of the document.
     */
    void setCreationDate(std::string creationDate) { m_creationDate = std::move(creationDate); }

    /**
     * @brief Gets the creation date of the document.
     * @return The creation date of the document.
     */
    const std::string& creationDate() const { return m_creationDate; }

    /**
     * @brief Sets the modification date of the document.
     *
     * The date must be in ISO-8601 format: YYYY-MM-DDThh:mm:ss, with an optional timezone "[+/-]hh:mm" or "Z" for UTC.
     *
     * @param modificationDate The modification date of the document.
     */
    void setModificationDate(std::string modificationDate) { m_modificationDate = std::move(modificationDate); }

    /**
     * @brief Gets the modification date of the document.
     * @return The modification date of the document.
     */
    const std::string& modificationDate() const { return m_modificationDate; }

    /**
     * @brief Returns the width of the viewport.
     * @return The width of the viewport in pixels.
     */
    float viewportWidth() const;

    /**
     * @brief Returns the height of the viewport.
     * @return The height of the viewport in pixels.
     */
    float viewportHeight() const;

    /**
     * @brief Returns the width of the document.
     * @return The width of the document in pixels.
     */
    float documentWidth() const;

    /**
     * @brief Returns the height of the document.
     * @return The height of the document in pixels.
     */
    float documentHeight() const;

    /**
     * @brief Returns the initial page size.
     * @return The initial page size.
     */
    const PageSize& pageSize() const { return m_pageSize; }

    /**
     * @brief Returns the initial page margins.
     * @return The initial page margins.
     */
    const PageMargins& pageMargins() const { return m_pageMargins; }

    /**
     * @brief Returns the media type used for media queries.
     * @return The media type used for media queries.
     */
    MediaType mediaType() const { return m_mediaType; }

    /**
     * @brief Returns the number of pages in the document.
     * @return The number of pages in the document.
     */
    uint32_t pageCount() const;

    /**
     * @brief Returns the page size at the specified index.
     * @param pageIndex The index of the page.
     * @return The size of the page at the specified index.
     */
    PageSize pageSizeAt(uint32_t pageIndex) const;

    /**
     * @brief Loads the document from the specified URL.
     * @param url The URL to load the document from.
     * @param userStyle An optional user-defined style to apply.
     * @param userScript An optional user-defined script to run after the document has loaded.
     * @return `true` on success, or `false` on failure.
     */
    bool loadUrl(const std::string_view& url, const std::string_view& userStyle = {}, const std::string_view& userScript = {});

    /**
     * @brief Loads the document from the specified data.
     * @param data The data to load the document from.
     * @param length The length of the data in bytes.
     * @param mimeType The MIME type of the data.
     * @param textEncoding The text encoding of the data.
     * @param userStyle An optional user-defined style to apply.
     * @param userScript An optional user-defined script to run after the document has loaded.
     * @param baseUrl The base URL for resolving relative URLs.
     * @return `true` on success, or `false` on failure.
     */
    bool loadData(const char* data, size_t length, const std::string_view& mimeType = {}, const std::string_view& textEncoding = {},
        const std::string_view& userStyle = {}, const std::string_view& userScript = {}, const std::string_view& baseUrl = {});

    /**
     * @brief Loads the document from the specified image data.
     * @param data The image data to load the document from.
     * @param length The length of the image data in bytes.
     * @param mimeType The MIME type of the image data.
     * @param textEncoding The text encoding of the image data.
     * @param userStyle An optional user-defined style to apply.
     * @param userScript An optional user-defined script to run after the document has loaded.
     * @param baseUrl The base URL for resolving relative URLs.
     * @return `true` on success, or `false` on failure.
     */
    bool loadImage(const char* data, size_t length, const std::string_view& mimeType = {}, const std::string_view& textEncoding = {},
        const std::string_view& userStyle = {}, const std::string_view& userScript = {}, const std::string_view& baseUrl = {});

    /**
     * @brief Loads the document from the specified XML data.
     * @param content The XML data to load the document from, encoded in UTF-8.
     * @param userStyle An optional user-defined style to apply.
     * @param userScript An optional user-defined script to run after the document has loaded.
     * @param baseUrl The base URL for resolving relative URLs.
     * @return `true` on success, or `false` on failure.
     */
    bool loadXml(const std::string_view& content, const std::string_view& userStyle = {}, const std::string_view& userScript = {}, const std::string_view& baseUrl = {});

    /**
     * @brief Loads the document from the specified HTML data.
     * @param content The HTML data to load the document from, encoded in UTF-8.
     * @param userStyle An optional user-defined style to apply.
     * @param userScript An optional user-defined script to run after the document has loaded.
     * @param baseUrl The base URL for resolving relative URLs.
     * @return `true` on success, or `false` on failure.
     */
    bool loadHtml(const std::string_view& content, const std::string_view& userStyle = {}, const std::string_view& userScript = {}, const std::string_view& baseUrl = {});

    /**
     * @brief Clears the content of the document.
     */
    void clearContent();

    /**
     * @brief Renders the specified page to the given canvas.
     * @param canvas The canvas to render the page on.
     * @param pageIndex The index of the page to render.
     */
    void renderPage(Canvas& canvas, uint32_t pageIndex) const;

    /**
     * @brief Renders the specified page to the given canvas.
     * @param canvas The canvas to render the page on.
     * @param pageIndex The index of the page to render.
     */
    void renderPage(plutobook_canvas_t* canvas, uint32_t pageIndex) const;

    /**
     * @brief Renders the specified page to the given canvas.
     * @param canvas The canvas to render the page on.
     * @param pageIndex The index of the page to render.
     */
    void renderPage(cairo_t* canvas, uint32_t pageIndex) const;

    /**
     * @brief Renders the entire document to the given canvas.
     * @param canvas The canvas to render the entire document on.
     */
    void renderDocument(Canvas& canvas) const;

    /**
     * @brief Renders a specific rectangular portion of the document to the given canvas.
     * @param canvas The canvas to render the document portion on.
     * @param x The x-coordinate of the top-left corner of the rectangle.
     * @param y The y-coordinate of the top-left corner of the rectangle.
     * @param width The width of the rectangle to render.
     * @param height The height of the rectangle to render.
     */
    void renderDocument(Canvas& canvas, float x, float y, float width, float height) const;

    /**
     * @brief Renders the entire document to the given canvas.
     * @param canvas The canvas to render the entire document on.
     */
    void renderDocument(plutobook_canvas_t* canvas) const;

    /**
     * @brief Renders a specific rectangular portion of the document to the given canvas.
     * @param canvas The canvas to render the document portion on.
     * @param x The x-coordinate of the top-left corner of the rectangle.
     * @param y The y-coordinate of the top-left corner of the rectangle.
     * @param width The width of the rectangle to render.
     * @param height The height of the rectangle to render.
     */
    void renderDocument(plutobook_canvas_t* canvas, float x, float y, float width, float height) const;

    /**
     * @brief Renders the entire document to the given canvas.
     * @param canvas The canvas to render the entire document on.
     */
    void renderDocument(cairo_t* canvas) const;

    /**
     * @brief Renders a specific rectangular portion of the document to the given canvas.
     * @param canvas The canvas to render the document portion on.
     * @param x The x-coordinate of the top-left corner of the rectangle.
     * @param y The y-coordinate of the top-left corner of the rectangle.
     * @param width The width of the rectangle to render.
     * @param height The height of the rectangle to render.
     */
    void renderDocument(cairo_t* canvas, float x, float y, float width, float height) const;

    /**
     * @brief Writes a range of pages from the document to a PDF file.
     * @param filename The file path where the PDF document will be written.
     * @param pageStart The first page in the range to be written (inclusive).
     * @param pageEnd The last page in the range to be written (inclusive).
     * @param pageStep The increment used to advance through pages in the range.
     * @return `true` on success, or `false` on failure.
     */
    bool writeToPdf(const std::string& filename, uint32_t pageStart = kMinPageCount, uint32_t pageEnd = kMaxPageCount, int pageStep = 1) const;

    /**
     * @brief Writes the entire document to a PDF stream using a callback function.
     * @param output The output stream where the PDF document will be written.
     * @param pageStart The first page in the range to be written (inclusive).
     * @param pageEnd The last page in the range to be written (inclusive).
     * @param pageStep The increment used to advance through pages in the range.
     * @return `true` on success, or `false` on failure.
     */
    bool writeToPdf(OutputStream& output, uint32_t pageStart = kMinPageCount, uint32_t pageEnd = kMaxPageCount, int pageStep = 1) const;

    /**
     * @brief Writes the entire document to a PDF stream using a callback function.
     * @param callback A callback function used for writing the PDF stream.
     * @param closure A user-defined pointer passed to the callback function for additional data.
     * @param pageStart The first page in the range to be written (inclusive).
     * @param pageEnd The last page in the range to be written (inclusive).
     * @param pageStep The increment used to advance through pages in the range.
     * @return `true` on success, or `false` on failure.
     */
    bool writeToPdf(plutobook_stream_write_callback_t callback, void* closure, uint32_t pageStart = kMinPageCount, uint32_t pageEnd = kMaxPageCount, int pageStep = 1) const;

    /**
     * @brief Writes the entire document to a PNG image file.
     * @param filename The file path where the PNG image will be written.
     * @param width The desired width in pixels, or -1 to auto-scale based on the document size.
     * @param height The desired height in pixels, or -1 to auto-scale based on the document size.
     * @return `true` on success, or `false` on failure.
     */
    bool writeToPng(const std::string& filename, int width = -1, int height = -1) const;

    /**
     * @brief Writes the entire document to a PNG image stream.
     * @param output The output stream where the PNG image will be written.
     * @param width The desired width in pixels, or -1 to auto-scale based on the document size.
     * @param height The desired height in pixels, or -1 to auto-scale based on the document size.
     * @return `true` on success, or `false` on failure.
     */
    bool writeToPng(OutputStream& output, int width = -1, int height = -1) const;

    /**
     * @brief Writes the entire document to a PNG image stream using a callback function.
     * @param callback A callback function to handle the image data stream.
     * @param closure A pointer to user-defined data to pass to the callback function.
     * @param width The desired width in pixels, or -1 to auto-scale based on the document size.
     * @param height The desired height in pixels, or -1 to auto-scale based on the document size.
     * @return `true` on success, or `false` on failure.
     */
    bool writeToPng(plutobook_stream_write_callback_t callback, void* closure, int width = -1, int height = -1) const;

    /**
     * @brief Sets a custom resource fetcher to be used for fetching external resources.
     *
     * @param fetcher A pointer to a `ResourceFetcher` object. Pass `nullptr` to clear the custom fetcher.
     */
    void setCustomResourceFetcher(ResourceFetcher* fetcher) { m_customResourceFetcher = fetcher; }

    /**
     * @brief Retrieves the currently set custom resource fetcher.
     *
     * @return A pointer to the `ResourceFetcher` instance, or `nullptr` if none is set.
     */
    ResourceFetcher* customResourceFetcher() const { return m_customResourceFetcher; }

    /**
     * @internal
     */
    Heap* heap() const { return m_heap.get(); }

    /**
     * @internal
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
