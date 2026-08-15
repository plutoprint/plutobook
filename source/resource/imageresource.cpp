/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "imageresource.h"
#include "textresource.h"
#include "svgdocument.h"
#include "graphicscontext.h"
#include "stringutils.h"

#include "plutobook.hpp"

#include <cairo.h>
#ifdef PLUTOBOOK_HAS_WEBP
#include <webp/decode.h>
#endif

#ifdef PLUTOBOOK_HAS_TURBOJPEG
#define STBI_NO_JPEG
#include <turbojpeg.h>
#endif

#ifdef CAIRO_HAS_PNG_FUNCTIONS
#define STBI_NO_PNG
#endif

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#include "stb_image.h"

#include <cstring>
#include <cmath>

namespace plutobook {

RefPtr<ImageResource> ImageResource::create(Document* document, const Url& url)
{
    auto resource = ResourceLoader::loadUrl(url, document->customResourceFetcher());
    if(resource.isNull())
        return nullptr;
    auto image = decode(document->book(), resource.content(), resource.contentLength(), resource.mimeType(), resource.textEncoding(), url.base());
    if(image == nullptr) {
        plutobook_set_error_message("Unable to load image '%s': %s", url.value().data(), plutobook_get_error_message());
        return nullptr;
    }

    return adoptPtr(new (document->heap()) ImageResource(std::move(image)));
}

RefPtr<Image> ImageResource::decode(Book* book, const char* data, size_t size, std::string_view mimeType, std::string_view textEncoding, std::string_view baseUrl)
{
    if(equals(mimeType, "image/svg+xml", false))
        return SVGImage::create(book, TextResource::decode(data, size, mimeType, textEncoding), baseUrl);
    return BitmapImage::create(book, data, size);
}

bool ImageResource::supportsMimeType(std::string_view mimeType)
{
    return equals(mimeType, "image/jpeg", false)
        || equals(mimeType, "image/png", false)
#ifdef PLUTOBOOK_HAS_WEBP
        || equals(mimeType, "image/webp", false)
#endif
        || equals(mimeType, "image/svg+xml", false)
        || equals(mimeType, "image/gif", false)
        || equals(mimeType, "image/bmp", false);
}

static cairo_surface_t* createImageSurface(cairo_format_t format, int width, int height)
{
    auto surface = cairo_image_surface_create(format, width, height);
    if(auto status = cairo_surface_status(surface)) {
        plutobook_set_error_message("image decode error: %s", cairo_status_to_string(status));
        return nullptr;
    }

    return surface;
}

#ifdef CAIRO_HAS_PNG_FUNCTIONS
struct png_read_stream_t {
    const char* data;
    size_t size;
};

static cairo_status_t png_read_function(void* closure, uint8_t* data, uint32_t length)
{
    auto stream = static_cast<png_read_stream_t*>(closure);
    if(length > stream->size)
        return CAIRO_STATUS_READ_ERROR;
    std::memcpy(data, stream->data, length);
    stream->data += length;
    stream->size -= length;
    return CAIRO_STATUS_SUCCESS;
}

static cairo_surface_t* decodePngImage(const char* data, size_t size)
{
    png_read_stream_t stream = { data, size };
    return cairo_image_surface_create_from_png_stream(png_read_function, &stream);
}
#endif // CAIRO_HAS_PNG_FUNCTIONS

#ifdef PLUTOBOOK_HAS_TURBOJPEG
static cairo_surface_t* decodeJpegImage(const char* data, size_t size)
{
    auto tj = tjInitDecompress();
    if(tj == nullptr) {
        plutobook_set_error_message("image decode error: %s", tjGetErrorStr());
        return nullptr;
    }

    int width, height;
    if(tjDecompressHeader(tj, (uint8_t*)(data), size, &width, &height) == -1) {
        plutobook_set_error_message("image decode error: %s", tjGetErrorStr());
        tjDestroy(tj);
        return nullptr;
    }

    auto surface = createImageSurface(CAIRO_FORMAT_RGB24, width, height);
    if(surface == nullptr) {
        tjDestroy(tj);
        return nullptr;
    }

    auto pixels = cairo_image_surface_get_data(surface);
    auto stride = cairo_image_surface_get_stride(surface);
    if(tjDecompress2(tj, (uint8_t*)(data), size, pixels, width, stride, height, TJPF_BGRX, 0) == -1) {
        plutobook_set_error_message("image decode error: %s", tjGetErrorStr());
        cairo_surface_destroy(surface);
        tjDestroy(tj);
        return nullptr;
    }

    cairo_surface_mark_dirty(surface);
    tjDestroy(tj);

    auto mimeData = (uint8_t*)std::malloc(size);
    std::memcpy(mimeData, data, size);
    cairo_surface_set_mime_data(surface, CAIRO_MIME_TYPE_JPEG, mimeData, size, std::free, mimeData);
    return surface;
}
#endif // PLUTOBOOK_HAS_TURBOJPEG

#ifdef PLUTOBOOK_HAS_WEBP
static cairo_surface_t* decodeWebpImage(const char* data, size_t size)
{
    WebPDecoderConfig config;
    if(!WebPInitDecoderConfig(&config)) {
        plutobook_set_error_message("image decode error: WebPInitDecoderConfig failed");
        return nullptr;
    }

    if(WebPGetFeatures((const uint8_t*)(data), size, &config.input) != VP8_STATUS_OK) {
        plutobook_set_error_message("image decode error: WebPGetFeatures failed");
        return nullptr;
    }

    auto surface = createImageSurface(CAIRO_FORMAT_ARGB32, config.input.width, config.input.height);
    if(surface == nullptr)
        return nullptr;
    auto surfaceData = cairo_image_surface_get_data(surface);
    auto surfaceWidth = cairo_image_surface_get_width(surface);
    auto surfaceHeight = cairo_image_surface_get_height(surface);
    auto surfaceStride = cairo_image_surface_get_stride(surface);

    config.output.colorspace = MODE_bgrA;
    config.output.u.RGBA.rgba = surfaceData;
    config.output.u.RGBA.stride = surfaceStride;
    config.output.u.RGBA.size = surfaceStride * surfaceHeight;
    config.output.width = surfaceWidth;
    config.output.height = surfaceHeight;
    config.output.is_external_memory = 1;
    if(WebPDecode((const uint8_t*)(data), size, &config) != VP8_STATUS_OK) {
        plutobook_set_error_message("image decode error: WebPDecode failed");
        cairo_surface_destroy(surface);
        return nullptr;
    }

    cairo_surface_mark_dirty(surface);
    return surface;
}
#endif // PLUTOBOOK_HAS_WEBP

static cairo_surface_t* decodeGenericImage(const char* data, size_t size)
{
    int width, height, channels;
    auto imageData = stbi_load_from_memory((const stbi_uc*)(data), size, &width, &height, &channels, STBI_rgb_alpha);
    if(imageData == nullptr) {
        plutobook_set_error_message("image decode error: %s", stbi_failure_reason());
        return nullptr;
    }

    auto surface = createImageSurface(CAIRO_FORMAT_ARGB32, width, height);
    if(surface == nullptr) {
        stbi_image_free(imageData);
        return nullptr;
    }

    auto surfaceData = cairo_image_surface_get_data(surface);
    auto surfaceStride = cairo_image_surface_get_stride(surface);

    const auto imageStride = width * 4;
    for(int y = 0; y < height; y++) {
        auto src = (const uint32_t*)(imageData + imageStride * y);
        auto dst = (uint32_t*)(surfaceData + surfaceStride * y);
        for(int x = 0; x < width; x++) {
            uint32_t pixel = src[x];
            uint32_t r = (pixel >> 0) & 0xFF;
            uint32_t g = (pixel >> 8) & 0xFF;
            uint32_t b = (pixel >> 16) & 0xFF;
            uint32_t a = (pixel >> 24) & 0xFF;

            uint32_t pr = (r * a) / 255;
            uint32_t pg = (g * a) / 255;
            uint32_t pb = (b * a) / 255;
            dst[x] = (a << 24) | (pr << 16) | (pg << 8) | pb;
        }
    }

    stbi_image_free(imageData);
    cairo_surface_mark_dirty(surface);
    return surface;
}

static cairo_surface_t* decodeBitmapImage(const char* data, size_t size)
{
#ifdef CAIRO_HAS_PNG_FUNCTIONS
    if(size > 8 && std::memcmp(data, "\x89PNG\r\n\x1A\n", 8) == 0)
        return decodePngImage(data, size);
#endif
#ifdef PLUTOBOOK_HAS_TURBOJPEG
    if(size > 3 && std::memcmp(data, "\xFF\xD8\xFF", 3) == 0)
        return decodeJpegImage(data, size);
#endif
#ifdef PLUTOBOOK_HAS_WEBP
    if(size > 14 && std::memcmp(data, "RIFF", 4) == 0 && std::memcmp(data + 8, "WEBPVP", 6) == 0)
        return decodeWebpImage(data, size);
#endif
    return decodeGenericImage(data, size);
}

RefPtr<BitmapImage> BitmapImage::create(Book* book, const char* data, size_t size)
{
    auto surface = decodeBitmapImage(data, size);
    if(surface == nullptr)
        return nullptr;
    if(auto status = cairo_surface_status(surface)) {
        plutobook_set_error_message("image decode error: %s", cairo_status_to_string(status));
        cairo_surface_destroy(surface);
        return nullptr;
    }

    return adoptPtr(new (book->heap()) BitmapImage(surface));
}

void Image::drawTiled(GraphicsContext& context, const Rect& destRect, const Rect& tileRect)
{
    const Size imageSize(size());
    if(imageSize.isEmpty() || destRect.isEmpty() || tileRect.isEmpty()) {
        return;
    }

    const Size scale(tileRect.w / imageSize.w, tileRect.h / imageSize.h);
    const Point phase = {
        destRect.x + std::fmod(std::fmod(-tileRect.x, tileRect.w) - tileRect.w, tileRect.w),
        destRect.y + std::fmod(std::fmod(-tileRect.y, tileRect.h) - tileRect.h, tileRect.h)
    };

    const Rect oneTileRect(phase, tileRect.size());
    if(!oneTileRect.contains(destRect)) {
        drawPattern(context, destRect, imageSize, scale, phase);
    } else {
        const Rect srcRect = {
            (destRect.x - oneTileRect.x) / scale.w,
            (destRect.y - oneTileRect.y) / scale.h,
            (destRect.w / scale.w),
            (destRect.h / scale.h)
        };

        draw(context, destRect, srcRect);
    }
}

void BitmapImage::draw(GraphicsContext& context, const Rect& dstRect, const Rect& srcRect)
{
    if(dstRect.isEmpty() || srcRect.isEmpty()) {
        return;
    }

    auto xScale = srcRect.w / dstRect.w;
    auto yScale = srcRect.h / dstRect.h;
    cairo_matrix_t matrix = {xScale, 0, 0, yScale, srcRect.x, srcRect.y};

    auto pattern = cairo_pattern_create_for_surface(m_surface);
    cairo_pattern_set_matrix(pattern, &matrix);
    cairo_pattern_set_extend(pattern, CAIRO_EXTEND_NONE);

    auto canvas = context.canvas();
    cairo_save(canvas);
    cairo_set_fill_rule(canvas, CAIRO_FILL_RULE_WINDING);
    cairo_translate(canvas, dstRect.x, dstRect.y);
    cairo_rectangle(canvas, 0, 0, dstRect.w, dstRect.h);
    cairo_set_source(canvas, pattern);
    cairo_fill(canvas);
    cairo_restore(canvas);
    cairo_pattern_destroy(pattern);
}

void BitmapImage::drawPattern(GraphicsContext& context, const Rect& destRect, const Size& size, const Size& scale, const Point& phase)
{
    assert(!destRect.isEmpty() && !size.isEmpty() && !scale.isEmpty());

    cairo_matrix_t matrix;
    cairo_matrix_init(&matrix, scale.w, 0, 0, scale.h, phase.x, phase.y);
    cairo_matrix_invert(&matrix);

    auto pattern = cairo_pattern_create_for_surface(m_surface);
    cairo_pattern_set_matrix(pattern, &matrix);
    cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT);

    auto canvas = context.canvas();
    cairo_save(canvas);
    cairo_set_fill_rule(canvas, CAIRO_FILL_RULE_WINDING);
    cairo_rectangle(canvas, destRect.x, destRect.y, destRect.w, destRect.h);
    cairo_set_source(canvas, pattern);
    cairo_fill(canvas);
    cairo_restore(canvas);
    cairo_pattern_destroy(pattern);
}

void BitmapImage::computeIntrinsicDimensions(float& intrinsicWidth, float& intrinsicHeight, double& intrinsicRatio)
{
    intrinsicWidth = cairo_image_surface_get_width(m_surface);
    intrinsicHeight = cairo_image_surface_get_height(m_surface);
    if(intrinsicHeight > 0) {
        intrinsicRatio = intrinsicWidth / intrinsicHeight;
    } else {
        intrinsicRatio = 0;
    }
}

Size BitmapImage::intrinsicSize() const
{
    auto width = cairo_image_surface_get_width(m_surface);
    auto height = cairo_image_surface_get_height(m_surface);

    return Size(width, height);
}

Size BitmapImage::size() const
{
    return intrinsicSize();
}

BitmapImage::~BitmapImage()
{
    cairo_surface_destroy(m_surface);
}

BitmapImage::BitmapImage(cairo_surface_t* surface)
    : m_surface(surface)
{
}

RefPtr<SVGImage> SVGImage::create(Book* book, std::string_view content, std::string_view baseUrl)
{
    auto document = SVGDocument::create(book, ResourceLoader::completeUrl(baseUrl));
    if(!document->parse(content))
        return nullptr;
    if(!document->rootElement()->isOfType(svgNs, svgTag)) {
        plutobook_set_error_message("invalid SVG image: root element must be <svg> in the \"http://www.w3.org/2000/svg\" namespace");
        return nullptr;
    }

    return adoptPtr(new (book->heap()) SVGImage(std::move(document)));
}

void SVGImage::draw(GraphicsContext& context, const Rect& dstRect, const Rect& srcRect)
{
    if(dstRect.isEmpty() || srcRect.isEmpty()) {
        return;
    }

    auto xScale = dstRect.w / srcRect.w;
    auto yScale = dstRect.h / srcRect.h;

    auto xOffset = dstRect.x - (srcRect.x * xScale);
    auto yOffset = dstRect.y - (srcRect.y * yScale);

    context.save();
    context.clipRect(dstRect);
    context.translate(xOffset, yOffset);
    context.scale(xScale, yScale);
    m_document->render(context, srcRect);
    context.restore();
}

void SVGImage::drawPattern(GraphicsContext& context, const Rect& destRect, const Size& size, const Size& scale, const Point& phase)
{
    assert(!destRect.isEmpty() && !size.isEmpty() && !scale.isEmpty());

    cairo_matrix_t pattern_matrix;
    cairo_matrix_init(&pattern_matrix, 1, 0, 0, 1, -phase.x, -phase.y);

    cairo_rectangle_t pattern_rectangle = {0, 0, size.w * scale.w, size.h * scale.h};
    auto pattern_surface = cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA, &pattern_rectangle);
    auto pattern_canvas = cairo_create(pattern_surface);

    auto pattern = cairo_pattern_create_for_surface(pattern_surface);
    cairo_pattern_set_matrix(pattern, &pattern_matrix);
    cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT);

    GraphicsContext pattern_context(pattern_canvas);
    m_document->render(pattern_context, Rect::Infinite);

    auto canvas = context.canvas();
    cairo_save(canvas);
    cairo_set_fill_rule(canvas, CAIRO_FILL_RULE_WINDING);
    cairo_rectangle(canvas, destRect.x, destRect.y, destRect.w, destRect.h);
    cairo_set_source(canvas, pattern);
    cairo_fill(canvas);
    cairo_restore(canvas);

    cairo_pattern_destroy(pattern);
    cairo_destroy(pattern_canvas);
    cairo_surface_destroy(pattern_surface);
}

static SVGSVGElement* toSVGRootElement(Element* element)
{
    assert(element && element->isOfType(svgNs, svgTag));
    return static_cast<SVGSVGElement*>(element);
}

void SVGImage::computeIntrinsicDimensions(float& intrinsicWidth, float& intrinsicHeight, double& intrinsicRatio)
{
    auto rootElement = toSVGRootElement(m_document->rootElement());
    rootElement->computeIntrinsicDimensions(intrinsicWidth, intrinsicHeight, intrinsicRatio);
}

void SVGImage::setContainerSize(const Size& size)
{
    m_containerSize = size;
    if(m_document->setContainerSize(size.w, size.h)) {
        m_document->layout(nullptr);
    }
}

Size SVGImage::intrinsicSize() const
{
    float intrinsicWidth = 0.f;
    float intrinsicHeight = 0.f;
    double intrinsicRatio = 0.0;

    auto rootElement = toSVGRootElement(m_document->rootElement());
    rootElement->computeIntrinsicDimensions(intrinsicWidth, intrinsicHeight, intrinsicRatio);
    if(intrinsicRatio && (!intrinsicWidth || !intrinsicHeight)) {
        if(!intrinsicWidth && intrinsicHeight)
            intrinsicWidth = intrinsicHeight * intrinsicRatio;
        else if(intrinsicWidth && !intrinsicHeight) {
            intrinsicHeight = intrinsicWidth / intrinsicRatio;
        }
    }

    if(intrinsicWidth > 0 && intrinsicHeight > 0) {
        return Size(intrinsicWidth, intrinsicHeight);
    }

    const auto& viewBoxRect = rootElement->viewBox();
    if(viewBoxRect.isValid())
        return viewBoxRect.size();
    return Size(300, 150);
}

Size SVGImage::size() const
{
    return m_containerSize;
}

SVGImage::~SVGImage() = default;

SVGImage::SVGImage(std::unique_ptr<SVGDocument> document)
    : m_document(std::move(document))
{
    m_document->build();
}

} // namespace plutobook
