#ifndef PLUTOBOOK_IMAGERESOURCE_H
#define PLUTOBOOK_IMAGERESOURCE_H

#include "resource.h"

typedef struct _cairo_surface cairo_surface_t;

namespace plutobook {

class Image;

class ImageResource final : public Resource {
public:
    static RefPtr<ImageResource> create(const Url& url, const std::string& mimeType, const std::string& textEncoding, std::vector<char> content);
    static RefPtr<Image> decode(const char* data, size_t size, const std::string_view& mimeType, const std::string_view& textEncoding, const std::string_view& baseUrl);
    static bool supportsMimeType(const std::string_view& mimeType);
    const RefPtr<Image>& image() const { return m_image; }
    Type type() const final { return Type::Image; }

private:
    ImageResource(RefPtr<Image> image) : m_image(std::move(image)) {}
    RefPtr<Image> m_image;
};

template<>
struct is_a<ImageResource> {
    static bool check(const Resource& value) { return value.type() == Resource::Type::Image; }
};

class GraphicsContext;
class Rect;

class Image : public RefCounted<Image> {
public:
    virtual ~Image();
    virtual bool isBitmapImage() const { return false; }
    virtual bool isSVGImage() const { return false; }

    void draw(GraphicsContext& context, const Rect& dstRect, const Rect& srcRect);
    void drawTiled(GraphicsContext& context, const Rect& destRect, const Rect& tileRect);

    float width() const { return m_width; }
    float height() const { return m_height; }

protected:
    Image(cairo_surface_t* surface, float width, float height);
    cairo_surface_t* m_surface;
    const float m_width;
    const float m_height;
};

class BitmapImage final : public Image {
public:
    static RefPtr<BitmapImage> create(const char* data, size_t size);
    static cairo_surface_t* decode(const char* data, size_t size);

    bool isBitmapImage() const final { return true; }

private:
    BitmapImage(cairo_surface_t* surface, float width, float height);
};

template<>
struct is_a<BitmapImage> {
    static bool check(const Image& value) { return value.isBitmapImage(); }
};

class SVGImage final : public Image {
public:
    static RefPtr<SVGImage> create(const std::string_view& content, Url baseUrl);

    bool isSVGImage() const final { return true; }

private:
    SVGImage(cairo_surface_t* surface, float width, float height);
};

template<>
struct is_a<SVGImage> {
    static bool check(const Image& value) { return value.isSVGImage(); }
};

} // namespace plutobook

#endif // PLUTOBOOK_IMAGERESOURCE_H
