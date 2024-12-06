#include "imageresource.h"
#include "textresource.h"
#include "svgdocument.h"
#include "stringutils.h"
#include "graphicscontext.h"
#include "geometry.h"

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
#include "stb_image.h"

#include <cstring>
#include <cmath>

namespace plutobook {

RefPtr<ImageResource> ImageResource::create(ResourceFetcher* fetcher, const Url& url)
{
    auto resource = ResourceLoader::loadUrl(url, fetcher);
    if(resource.isNull())
        return nullptr;
    auto image = decode(resource.content(), resource.contentLength(), resource.mimeType(), resource.textEncoding(), fetcher, url.base());
    if(image == nullptr)
        return nullptr;
    return adoptPtr(new ImageResource(std::move(image)));
}

RefPtr<Image> ImageResource::decode(const char* data, size_t size, const std::string_view& mimeType, const std::string_view& textEncoding, ResourceFetcher* fetcher, const std::string_view& baseUrl)
{
    if(equals(mimeType, "image/svg+xml", false))
        return SVGImage::create(TextResource::decode(data, size, mimeType, textEncoding), fetcher, ResourceLoader::completeUrl(baseUrl));
    return BitmapImage::create(data, size);
}

bool ImageResource::supportsMimeType(const std::string_view& mimeType)
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

Image::~Image()
{
    cairo_surface_destroy(m_surface);
}

void Image::draw(GraphicsContext& context, const Rect& dstRect, const Rect& srcRect)
{
    if(dstRect.isEmpty() || srcRect.isEmpty()) {
        return;
    }

    assert(m_width > 0.f && m_height > 0.f);
    auto xScale = srcRect.w / dstRect.w;
    auto yScale = srcRect.h / dstRect.h;
    cairo_matrix_t matrix = {xScale, 0, 0, yScale, srcRect.x, srcRect.y};

    auto pattern = cairo_pattern_create_for_surface(m_surface);
    cairo_pattern_set_matrix(pattern, &matrix);
    cairo_pattern_set_extend(pattern, CAIRO_EXTEND_PAD);

    auto canvas = context.canvas();
    cairo_save(canvas);
    cairo_translate(canvas, dstRect.x, dstRect.y);
    cairo_rectangle(canvas, 0, 0, dstRect.w, dstRect.h);
    cairo_set_fill_rule(canvas, CAIRO_FILL_RULE_WINDING);
    cairo_clip(canvas);
    cairo_set_source(canvas, pattern);
    cairo_paint(canvas);
    cairo_restore(canvas);
    cairo_pattern_destroy(pattern);
}

void Image::drawTiled(GraphicsContext& context, const Rect& destRect, const Rect& tileRect)
{
    if(destRect.isEmpty() || tileRect.isEmpty()) {
        return;
    }

    assert(m_width > 0.f && m_height > 0.f);
    auto xScale = tileRect.w / m_width;
    auto yScale = tileRect.h / m_height;

    Rect oneTileRect(destRect.x, destRect.y, tileRect.w, tileRect.h);
    oneTileRect.x += std::fmod(std::fmod(-tileRect.x, tileRect.w) - tileRect.w, tileRect.w);
    oneTileRect.y += std::fmod(std::fmod(-tileRect.y, tileRect.h) - tileRect.h, tileRect.h);
    if(oneTileRect.contains(destRect)) {
        Rect srcRect = {
            (destRect.x - oneTileRect.x) / xScale,
            (destRect.y - oneTileRect.y) / yScale,
            (destRect.w / xScale),
            (destRect.h / yScale)
        };

        draw(context, destRect, srcRect);
        return;
    }

    cairo_matrix_t matrix = {xScale, 0, 0, yScale, oneTileRect.x, oneTileRect.y};
    cairo_matrix_invert(&matrix);

    auto pattern = cairo_pattern_create_for_surface(m_surface);
    cairo_pattern_set_matrix(pattern, &matrix);
    cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT);

    auto canvas = context.canvas();
    cairo_save(canvas);
    cairo_rectangle(canvas, destRect.x, destRect.y, destRect.w, destRect.h);
    cairo_set_source(canvas, pattern);
    cairo_set_fill_rule(canvas, CAIRO_FILL_RULE_WINDING);
    cairo_fill(canvas);
    cairo_restore(canvas);
    cairo_pattern_destroy(pattern);
}

Image::Image(cairo_surface_t* surface, float width, float height)
    : m_surface(surface), m_width(width), m_height(height)
{
}

RefPtr<BitmapImage> BitmapImage::create(const char* data, size_t size)
{
    auto surface = decode(data, size);
    if(surface == nullptr)
        return nullptr;
    if(cairo_surface_status(surface)) {
        cairo_surface_destroy(surface);
        return nullptr;
    }

    return adoptPtr(new BitmapImage(surface, cairo_image_surface_get_width(surface), cairo_image_surface_get_height(surface)));
}

#ifdef CAIRO_HAS_PNG_FUNCTIONS

struct png_read_stream_t {
    const char* data;
    size_t size;
    size_t read;
};

static cairo_status_t png_read_function(void* closure, uint8_t* data, uint32_t length)
{
    auto stream = static_cast<png_read_stream_t*>(closure);
    for(size_t i = 0; i < length; ++i, ++stream->read) {
        if(stream->read >= stream->size)
            return CAIRO_STATUS_READ_ERROR;
        data[i] = stream->data[stream->read];
    }

    return CAIRO_STATUS_SUCCESS;
}

#endif // CAIRO_HAS_PNG_FUNCTIONS

cairo_surface_t* BitmapImage::decode(const char* data, size_t size)
{
#ifdef CAIRO_HAS_PNG_FUNCTIONS
    if(size > 8 && std::memcmp(data, "\x89PNG\r\n\x1A\n", 8) == 0) {
        png_read_stream_t stream;
        stream.data = data;
        stream.size = size;
        stream.read = 0;
        return cairo_image_surface_create_from_png_stream(png_read_function, &stream);
    }
#endif // CAIRO_HAS_PNG_FUNCTIONS
#ifdef PLUTOBOOK_HAS_TURBOJPEG
    if(size > 3 && std::memcmp(data, "\xFF\xD8\xFF", 3) == 0) {
        int width, height;
        auto tj = tjInitDecompress();
        if(!tj || tjDecompressHeader(tj, (uint8_t*)(data), size, &width, &height) == -1) {
            tjDestroy(tj);
            return nullptr;
        }

        auto surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, width, height);
        auto surfaceData = cairo_image_surface_get_data(surface);
        auto surfaceWidth = cairo_image_surface_get_width(surface);
        auto surfaceStride = cairo_image_surface_get_stride(surface);
        auto surfaceHeight = cairo_image_surface_get_height(surface);
        tjDecompress2(tj, (uint8_t*)(data), size, surfaceData, surfaceWidth, surfaceStride, surfaceHeight, TJPF_BGRX, 0);
        tjDestroy(tj);
        cairo_surface_mark_dirty(surface);
        return surface;
    }
#endif // PLUTOBOOK_HAS_TURBOJPEG
#ifdef PLUTOBOOK_HAS_WEBP
    if(size > 14 && std::memcmp(data, "RIFF", 4) == 0 && std::memcmp(data + 8, "WEBPVP", 6) == 0) {
        WebPDecoderConfig config;
        if(!WebPInitDecoderConfig(&config)) {
            return nullptr;
        }

        if(WebPGetFeatures((const uint8_t*)(data), size, &config.input) != VP8_STATUS_OK)
            return nullptr;
        auto surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, config.input.width, config.input.height);
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
            return nullptr;
        }

        cairo_surface_mark_dirty(surface);
        return surface;
    }
#endif // PLUTOBOOK_HAS_WEBP

    int width, height, channels;
    auto imageData = stbi_load_from_memory((const stbi_uc*)(data), size, &width, &height, &channels, STBI_rgb_alpha);
    if(imageData == nullptr)
        return nullptr;
    auto surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    auto surfaceData = cairo_image_surface_get_data(surface);
    auto surfaceWidth = cairo_image_surface_get_width(surface);
    auto surfaceHeight = cairo_image_surface_get_height(surface);
    auto surfaceStride = cairo_image_surface_get_stride(surface);
    for(int y = 0; y < surfaceHeight; y++) {
        uint32_t* src = (uint32_t*)(imageData + surfaceStride * y);
        uint32_t* dst = (uint32_t*)(surfaceData + surfaceStride * y);
        for(int x = 0; x < surfaceWidth; x++) {
            uint32_t a = (src[x] >> 24) & 0xFF;
            uint32_t b = (src[x] >> 16) & 0xFF;
            uint32_t g = (src[x] >> 8) & 0xFF;
            uint32_t r = (src[x] >> 0) & 0xFF;

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

BitmapImage::BitmapImage(cairo_surface_t* surface, float width, float height)
    : Image(surface, width, height)
{
}

RefPtr<SVGImage> SVGImage::create(const std::string_view& content, ResourceFetcher* fetcher, Url baseUrl)
{
    Heap heap(1024 * 24);
    auto document = SVGDocument::create(nullptr, &heap, fetcher, std::move(baseUrl));
    if(!document->load(content) || !document->rootElement()->isOfType(svgNs, svgTag)) {
        return nullptr;
    }

    document->build();
    document->layout(nullptr);

    cairo_rectangle_t rectangle = {0, 0, document->width(), document->height()};
    auto surface = cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA, &rectangle);
    auto canvas = cairo_create(surface);

    GraphicsContext context(canvas);
    document->render(context, Rect::Infinite);
    cairo_destroy(canvas);
    return adoptPtr(new SVGImage(surface, document->width(), document->height()));
}

SVGImage::SVGImage(cairo_surface_t* surface, float width, float height)
    : Image(surface, width, height)
{
}

} // namespace plutobook
