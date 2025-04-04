#ifndef PLUTOBOOK_FONTRESOURCE_H
#define PLUTOBOOK_FONTRESOURCE_H

#include "resource.h"
#include "globalstring.h"

#include <forward_list>
#include <map>
#include <set>
#include <mutex>

typedef struct hb_font_t hb_font_t;
typedef struct _cairo_font_face cairo_font_face_t;
typedef struct _cairo_scaled_font cairo_scaled_font_t;
typedef struct _FcCharSet FcCharSet;
typedef struct _FcConfig FcConfig;

namespace plutobook {

class FontResource final : public Resource {
public:
    static RefPtr<FontResource> create(ResourceFetcher* fetcher, const Url& url);
    static bool supportsFormat(const std::string_view& format);
    cairo_font_face_t* face() const { return m_face; }
    Type type() const final { return Type::Font; }

    ~FontResource() final;

private:
    FontResource(cairo_font_face_t* face) : m_face(face) {}
    cairo_font_face_t* m_face;
};

template<>
struct is_a<FontResource> {
    static bool check(const Resource& value) { return value.type() == Resource::Type::Font; }
};

using FontSelectionValue = float;

constexpr FontSelectionValue kNormalFontWeight = 400.f;
constexpr FontSelectionValue kBoldFontWeight = 700.f;
constexpr FontSelectionValue kLightFontWeight = 200.f;
constexpr FontSelectionValue kMinFontWeight = 1.f;
constexpr FontSelectionValue kMaxFontWeight = 1000.f;

constexpr FontSelectionValue kNormalFontWidth = 100.f;
constexpr FontSelectionValue kUltraCondensedFontWidth = 50.f;
constexpr FontSelectionValue kExtraCondensedFontWidth = 62.5f;
constexpr FontSelectionValue kCondensedFontWidth = 75.f;
constexpr FontSelectionValue kSemiCondensedFontWidth = 85.5f;
constexpr FontSelectionValue kSemiExpandedFontWidth = 112.5f;
constexpr FontSelectionValue kExpandedFontWidth = 125.f;
constexpr FontSelectionValue kExtraExpandedFontWidth = 150.f;
constexpr FontSelectionValue kUltraExpandedFontWidth = 200.f;

constexpr FontSelectionValue kNormalFontSlope = 0.f;
constexpr FontSelectionValue kItalicFontSlope = 14.f;
constexpr FontSelectionValue kObliqueFontSlope = 20.f;
constexpr FontSelectionValue kMinFontSlope = -90.f;
constexpr FontSelectionValue kMaxFontSlope = 90.f;

struct FontSelectionRequest {
    constexpr FontSelectionRequest() = default;
    constexpr FontSelectionRequest(FontSelectionValue weight, FontSelectionValue width, FontSelectionValue slope)
        : weight(weight), width(width), slope(slope)
    {}

    FontSelectionValue weight = kNormalFontWeight;
    FontSelectionValue width = kNormalFontWidth;
    FontSelectionValue slope = kNormalFontSlope;
};

constexpr bool operator==(const FontSelectionRequest& a, const FontSelectionRequest& b)
{
    return std::tie(a.weight, a.width, a.slope) == std::tie(b.weight, b.width, b.slope);
}

constexpr bool operator!=(const FontSelectionRequest& a, const FontSelectionRequest& b)
{
    return std::tie(a.weight, a.width, a.slope) != std::tie(b.weight, b.width, b.slope);
}

constexpr bool operator<(const FontSelectionRequest& a, const FontSelectionRequest& b)
{
    return std::tie(a.weight, a.width, a.slope) < std::tie(b.weight, b.width, b.slope);
}

constexpr bool operator>(const FontSelectionRequest& a, const FontSelectionRequest& b)
{
    return std::tie(a.weight, a.width, a.slope) > std::tie(b.weight, b.width, b.slope);
}

struct FontSelectionRange {
    constexpr FontSelectionRange() = default;
    constexpr explicit FontSelectionRange(FontSelectionValue value)
        : minimum(value), maximum(value)
    {}

    constexpr FontSelectionRange(FontSelectionValue minimum, FontSelectionValue maximum)
        : minimum(minimum), maximum(maximum)
    {}

    constexpr explicit operator bool() const { return maximum >= minimum; }

    FontSelectionValue minimum = FontSelectionValue(0);
    FontSelectionValue maximum = FontSelectionValue(0);
};

constexpr FontSelectionRange kInvalidFontSelectionRange = FontSelectionRange(1, 0);

constexpr bool operator==(const FontSelectionRange& a, const FontSelectionRange& b)
{
    return std::tie(a.minimum, a.maximum) == std::tie(b.minimum, b.maximum);
}

constexpr bool operator!=(const FontSelectionRange& a, const FontSelectionRange& b)
{
    return std::tie(a.minimum, a.maximum) != std::tie(b.minimum, b.maximum);
}

constexpr bool operator<(const FontSelectionRange& a, const FontSelectionRange& b)
{
    return std::tie(a.minimum, a.maximum) < std::tie(b.minimum, b.maximum);
}

constexpr bool operator>(const FontSelectionRange& a, const FontSelectionRange& b)
{
    return std::tie(a.minimum, a.maximum) > std::tie(b.minimum, b.maximum);
}

struct FontSelectionDescription {
    constexpr FontSelectionDescription() = default;
    constexpr explicit FontSelectionDescription(const FontSelectionRequest& request)
        : weight(request.weight), width(request.width), slope(request.slope)
    {}

    constexpr FontSelectionDescription(FontSelectionRange weight, FontSelectionRange width, FontSelectionRange slope)
        : weight(weight), width(width), slope(slope)
    {}

    FontSelectionRange weight = kInvalidFontSelectionRange;
    FontSelectionRange width = kInvalidFontSelectionRange;
    FontSelectionRange slope = kInvalidFontSelectionRange;
};

constexpr bool operator==(const FontSelectionDescription& a, const FontSelectionDescription& b)
{
    return std::tie(a.weight, a.width, a.slope) == std::tie(b.weight, b.width, b.slope);
}

constexpr bool operator!=(const FontSelectionDescription& a, const FontSelectionDescription& b)
{
    return std::tie(a.weight, a.width, a.slope) != std::tie(b.weight, b.width, b.slope);
}

constexpr bool operator<(const FontSelectionDescription& a, const FontSelectionDescription& b)
{
    return std::tie(a.weight, a.width, a.slope) < std::tie(b.weight, b.width, b.slope);
}

constexpr bool operator>(const FontSelectionDescription& a, const FontSelectionDescription& b)
{
    return std::tie(a.weight, a.width, a.slope) > std::tie(b.weight, b.width, b.slope);
}

class FontSelectionAlgorithm {
public:
    explicit FontSelectionAlgorithm(const FontSelectionRequest& request);

    void addCandidate(const FontSelectionDescription& description);

    FontSelectionValue widthDistance(const FontSelectionRange& width) const;
    FontSelectionValue slopeDistance(const FontSelectionRange& slope) const;
    FontSelectionValue weightDistance(const FontSelectionRange& weight) const;

    bool isCandidateBetter(const FontSelectionDescription& currentCandidate, const FontSelectionDescription& previousCandidate) const;

private:
    FontSelectionRequest m_request;
    FontSelectionRange m_weight;
    FontSelectionRange m_width;
    FontSelectionRange m_slope;
};

class FontTag {
public:
    constexpr explicit FontTag(uint32_t value) : m_value(value) {}
    constexpr explicit FontTag(const std::string_view& tag)
        : m_value((tag[0] << 24) | (tag[1] << 16) | (tag[2] << 8) | (tag[3]))
    {
        assert(tag.length() == 4);
    }

    constexpr uint32_t value() const { return m_value; }

private:
    uint32_t m_value;
};

constexpr bool operator==(const FontTag& a, const FontTag& b) { return a.value() == b.value(); }
constexpr bool operator!=(const FontTag& a, const FontTag& b) { return a.value() != b.value(); }
constexpr bool operator<(const FontTag& a, const FontTag& b) { return a.value() < b.value(); }
constexpr bool operator>(const FontTag& a, const FontTag& b) { return a.value() > b.value(); }

using FontFeature = std::pair<FontTag, int>;
using FontVariation = std::pair<FontTag, float>;

using FontFeatureList = std::forward_list<FontFeature>;
using FontVariationList = std::forward_list<FontVariation>;

constexpr FontSelectionValue kMediumFontSize = 16.f;

struct FontDataDescription {
    FontSelectionValue size = kMediumFontSize;
    FontSelectionRequest request;
    FontVariationList variations;
};

constexpr bool operator==(const FontDataDescription& a, const FontDataDescription& b)
{
    return std::tie(a.size, a.request, a.variations) == std::tie(b.size, b.request, b.variations);
}

constexpr bool operator!=(const FontDataDescription& a, const FontDataDescription& b)
{
    return std::tie(a.size, a.request, a.variations) != std::tie(b.size, b.request, b.variations);
}

constexpr bool operator<(const FontDataDescription& a, const FontDataDescription& b)
{
    return std::tie(a.size, a.request, a.variations) < std::tie(b.size, b.request, b.variations);
}

constexpr bool operator>(const FontDataDescription& a, const FontDataDescription& b)
{
    return std::tie(a.size, a.request, a.variations) > std::tie(b.size, b.request, b.variations);
}

using FontFamilyList = std::forward_list<GlobalString>;

struct FontDescription {
    FontFamilyList families;
    FontDataDescription data;
};

constexpr bool operator==(const FontDescription& a, const FontDescription& b)
{
    return std::tie(a.families, a.data) == std::tie(b.families, b.data);
}

constexpr bool operator!=(const FontDescription& a, const FontDescription& b)
{
    return std::tie(a.families, a.data) != std::tie(b.families, b.data);
}

constexpr bool operator<(const FontDescription& a, const FontDescription& b)
{
    return std::tie(a.families, a.data) < std::tie(b.families, b.data);
}

constexpr bool operator>(const FontDescription& a, const FontDescription& b)
{
    return std::tie(a.families, a.data) > std::tie(b.families, b.data);
}

class FontData;

class FontFace : public RefCounted<FontFace> {
public:
    enum class Type {
        Local,
        Remote,
        Segmented
    };

    virtual ~FontFace() = default;
    virtual RefPtr<FontData> getFontData(const FontDataDescription& description) = 0;
    virtual Type type() const = 0;

protected:
    FontFace() = default;
};

class LocalFontFace final : public FontFace {
public:
    static RefPtr<LocalFontFace> create(const GlobalString& family);

    RefPtr<FontData> getFontData(const FontDataDescription& description) final;
    Type type() const final { return Type::Local; }

private:
    LocalFontFace(const GlobalString& family) : m_family(family) {}
    GlobalString m_family;
};

inline RefPtr<LocalFontFace> LocalFontFace::create(const GlobalString& family)
{
    return adoptPtr(new LocalFontFace(family));
}

template<>
struct is_a<LocalFontFace> {
    static bool check(const FontFace& value) { return value.type() == FontFace::Type::Local; }
};

class RemoteFontFace final : public FontFace {
public:
    static RefPtr<RemoteFontFace> create(FontFeatureList features, FontVariationList variations, RefPtr<FontResource> resource);

    RefPtr<FontData> getFontData(const FontDataDescription& description) final;
    Type type() const final { return Type::Remote; }

private:
    RemoteFontFace(FontFeatureList features, FontVariationList variations, RefPtr<FontResource> resource)
        : m_features(std::move(features)), m_variations(std::move(variations)), m_resource(std::move(resource))
    {}

    FontFeatureList m_features;
    FontVariationList m_variations;
    RefPtr<FontResource> m_resource;
};

inline RefPtr<RemoteFontFace> RemoteFontFace::create(FontFeatureList features, FontVariationList variations, RefPtr<FontResource> resource)
{
    return adoptPtr(new RemoteFontFace(std::move(features), std::move(variations), std::move(resource)));
}

template<>
struct is_a<RemoteFontFace> {
    static bool check(const FontFace& value) { return value.type() == FontFace::Type::Local; }
};

class SegmentedFontData;

class SegmentedFontFace final : public FontFace {
public:
    static RefPtr<SegmentedFontFace> create(const FontSelectionDescription& description);

    const FontSelectionDescription& description() const { return m_description; }
    const std::set<RefPtr<FontFace>>& faces() const { return m_faces; }
    RefPtr<FontData> getFontData(const FontDataDescription& description) final;
    Type type() const final { return Type::Segmented; }

    void add(RefPtr<FontFace> face) { m_faces.insert(std::move(face)); }

private:
    SegmentedFontFace(const FontSelectionDescription& description) : m_description(description) {}
    FontSelectionDescription m_description;
    std::set<RefPtr<FontFace>> m_faces;
    std::map<FontDataDescription, RefPtr<SegmentedFontData>> m_table;
};

inline RefPtr<SegmentedFontFace> SegmentedFontFace::create(const FontSelectionDescription& description)
{
    return adoptPtr(new SegmentedFontFace(description));
}

template<>
struct is_a<SegmentedFontFace> {
    static bool check(const FontFace& value) { return value.type() == FontFace::Type::Segmented; }
};

class SimpleFontData;

class FontData : public RefCounted<FontData> {
public:
    virtual ~FontData() = default;
    virtual const SimpleFontData* getFontData(uint32_t codepoint) const = 0;
    virtual bool isSegmented() const = 0;

protected:
    FontData() = default;
};

struct FontDataInfo {
    float ascent;
    float descent;
    float lineGap;
    float xHeight;
    float zeroWidth;
    float spaceWidth;
    uint16_t zeroGlyph;
    uint16_t spaceGlyph;
};

class TabSize;

class SimpleFontData final : public FontData {
public:
    static RefPtr<SimpleFontData> create(cairo_scaled_font_t* font, FontFeatureList features);

    hb_font_t* hbFont() const;
    cairo_scaled_font_t* font() const { return m_font; }
    const FontDataInfo& info() const { return m_info; }
    const FontFeatureList& features() const { return m_features; }
    const SimpleFontData* getFontData(uint32_t codepoint) const final;
    bool isSegmented() const final { return false; }

    float ascent() const { return m_info.ascent; }
    float descent() const { return m_info.descent; }
    float height() const { return m_info.ascent + m_info.descent; }
    float xHeight() const { return m_info.xHeight; }
    float lineGap() const { return m_info.lineGap; }
    float lineSpacing() const { return m_info.ascent + m_info.descent + m_info.lineGap; }
    float zeroWidth() const { return m_info.zeroWidth; }
    float spaceWidth() const { return m_info.spaceWidth; }
    uint16_t zeroGlyph() const { return m_info.zeroGlyph; }
    uint16_t spaceGlyph() const { return m_info.spaceGlyph; }
    float tabWidth(const TabSize& tabSize) const;

    ~SimpleFontData() final;

private:
    SimpleFontData(cairo_scaled_font_t* font, FcCharSet* charSet, const FontDataInfo& info, FontFeatureList features)
        : m_font(font), m_charSet(charSet), m_info(info), m_features(std::move(features))
    {}

    mutable hb_font_t* m_hbFont{nullptr};
    cairo_scaled_font_t* m_font;
    FcCharSet* m_charSet;
    FontDataInfo m_info;
    FontFeatureList m_features;
};

template<>
struct is_a<SimpleFontData> {
    static bool check(const FontData& value) { return !value.isSegmented(); }
};

using FontDataSet = std::set<RefPtr<FontData>>;

class SegmentedFontData final : public FontData {
public:
    static RefPtr<SegmentedFontData> create(FontDataSet fonts);

    const FontDataSet& fonts() const { return m_fonts; }
    const SimpleFontData* getFontData(uint32_t codepoint) const final;
    bool isSegmented() const final { return true; }

private:
    SegmentedFontData(FontDataSet fonts) : m_fonts(std::move(fonts)) {}
    FontDataSet m_fonts;
};

inline RefPtr<SegmentedFontData> SegmentedFontData::create(FontDataSet fonts)
{
    return adoptPtr(new SegmentedFontData(std::move(fonts)));
}

template<>
struct is_a<SegmentedFontData> {
    static bool check(const FontData& value) { return value.isSegmented(); }
};

class FontDataCache {
public:
    RefPtr<SimpleFontData> getFontData(const FontDataDescription& description);
    RefPtr<SimpleFontData> getFontData(const GlobalString& family, const FontDataDescription& description);
    RefPtr<SimpleFontData> getFontData(uint32_t codepoint, const FontDataDescription& description);

    bool isFamilyAvailable(const GlobalString& family) const { return m_families.contains(family); }

    ~FontDataCache();

private:
    FontDataCache();
    FcConfig* m_config;
    std::mutex m_mutex;
    std::set<GlobalString> m_families;
    std::map<GlobalString, std::map<FontDataDescription, RefPtr<SimpleFontData>>> m_table;
    friend FontDataCache* fontDataCache();
};

FontDataCache* fontDataCache();

using FontDataList = std::pmr::vector<RefPtr<FontData>>;

class Document;

class Font : public HeapMember, public RefCounted<Font> {
public:
    static RefPtr<Font> create(Document* document, const FontDescription& description);

    Heap* heap() const;
    Document* document() const { return m_document; }
    const FontDescription& description() const { return m_description; }
    const FontDataList& fonts() const { return m_fonts; }
    const SimpleFontData* primaryFont() const { return m_primaryFont; }

    float size() const { return m_description.data.size; }
    float weight() const { return m_description.data.request.weight; }
    float stretch() const { return m_description.data.request.width; }
    float style() const { return m_description.data.request.slope; }

    const FontFamilyList& family() const { return m_description.families; }
    const FontVariationList& variationSettings() const { return m_description.data.variations; }
    const SimpleFontData* getFontData(uint32_t codepoint) const;

private:
    Font(Document* document, const FontDescription& description);
    Document* m_document;
    FontDescription m_description;
    mutable FontDataList m_fonts;
    const SimpleFontData* m_primaryFont{nullptr};
};

} // namespace plutobook

#endif // PLUTOBOOK_FONTRESOURCE_H
