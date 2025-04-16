#ifndef PLUTOBOOK_IMAGERESOURCE_H
#define PLUTOBOOK_IMAGERESOURCE_H

#include "resource.h"

#include <memory>

typedef struct _cairo_surface cairo_surface_t;

namespace plutobook {

class Image;

class ImageResource final : public Resource {
public:
    static RefPtr<ImageResource> create(ResourceFetcher* fetcher, const Url& url);
    static RefPtr<Image> decode(const char* data, size_t size, const std::string_view& mimeType, const std::string_view& textEncoding, ResourceFetcher* fetcher, const std::string_view& baseUrl);
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
class Point;
class Size;
class Rect;

class Image : public RefCounted<Image> {
public:
    Image() = default;
    virtual ~Image() = default;

    virtual bool isBitmapImage() const { return false; }
    virtual bool isSVGImage() const { return false; }

    void drawTiled(GraphicsContext& context, const Rect& destRect, const Rect& tileRect);

    virtual void draw(GraphicsContext& context, const Rect& dstRect, const Rect& srcRect) = 0;
    virtual void drawPattern(GraphicsContext& context, const Rect& destRect, const Size& size, const Size& scale, const Point& phase) = 0;
    virtual void computeIntrinsicDimensions(float& intrinsicWidth, float& intrinsicHeight, double& intrinsicRatio) = 0;

    virtual void setContainerSize(const Size& size) = 0;
    virtual Size intrinsicSize() const = 0;
    virtual Size size() const = 0;
};

class BitmapImage final : public Image {
public:
    static RefPtr<BitmapImage> create(const char* data, size_t size);

    bool isBitmapImage() const final { return true; }

    void draw(GraphicsContext& context, const Rect& dstRect, const Rect& srcRect) final;
    void drawPattern(GraphicsContext& context, const Rect& destRect, const Size& size, const Size& scale, const Point& phase) final;
    void computeIntrinsicDimensions(float& intrinsicWidth, float& intrinsicHeight, double& intrinsicRatio) final;

    void setContainerSize(const Size& size) final;
    Size intrinsicSize() const final;
    Size size() const final;

    ~BitmapImage() final;

private:
    BitmapImage(cairo_surface_t* surface) : m_surface(surface) {}
    cairo_surface_t* m_surface;
};

template<>
struct is_a<BitmapImage> {
    static bool check(const Image& value) { return value.isBitmapImage(); }
};

class SVGDocument;
class Heap;

class SVGImage final : public Image {
public:
    static RefPtr<SVGImage> create(const std::string_view& content, ResourceFetcher* fetcher, Url baseUrl);

    bool isSVGImage() const final { return true; }

    void draw(GraphicsContext& context, const Rect& dstRect, const Rect& srcRect) final;
    void drawPattern(GraphicsContext& context, const Rect& destRect, const Size& size, const Size& scale, const Point& phase) final;
    void computeIntrinsicDimensions(float& intrinsicWidth, float& intrinsicHeight, double& intrinsicRatio) final;

    void setContainerSize(const Size& size) final;
    Size intrinsicSize() const final;
    Size size() const final;

private:
    SVGImage(std::unique_ptr<Heap> heap, std::unique_ptr<SVGDocument> document);
    std::unique_ptr<Heap> m_heap;
    std::unique_ptr<SVGDocument> m_document;
};

template<>
struct is_a<SVGImage> {
    static bool check(const Image& value) { return value.isSVGImage(); }
};

} // namespace plutobook

#endif // PLUTOBOOK_IMAGERESOURCE_H
