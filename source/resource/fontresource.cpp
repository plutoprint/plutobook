#include "fontresource.h"
#include "plutobook.hpp"
#include "document.h"
#include "stringutils.h"
#include "boxstyle.h"

#include <fontconfig/fontconfig.h>
#include <fontconfig/fcfreetype.h>

#include <cairo-ft.h>
#include <hb-ft.h>

#include <iostream>
#include <numbers>
#include <cmath>

namespace plutobook {

class FTFontData {
public:
    static FTFontData* create(ResourceData resource);
    FT_Face face() const { return m_face; }
    ~FTFontData() { FT_Done_Face(m_face); }

private:
    FTFontData(FT_Face face, ResourceData resource) : m_face(face), m_resource(std::move(resource)) {}
    FT_Face m_face;
    ResourceData m_resource;
};

FTFontData* FTFontData::create(ResourceData resource)
{
    static FT_Library ftLibrary;
    if(ftLibrary == nullptr) {
        if(auto error = FT_Init_FreeType(&ftLibrary)) {
            std::cerr << "freetype error: " << FT_Error_String(error) << std::endl;
        }
    }

    FT_Face ftFace = nullptr;
    if(auto error = FT_New_Memory_Face(ftLibrary, (FT_Byte*)(resource.content()), resource.contentLength(), 0, &ftFace)) {
        std::cerr << "freetype error: " << FT_Error_String(error) << std::endl;
        return nullptr;
    }

    return new FTFontData(ftFace, std::move(resource));
}

static void FTFontDataDestroy(void* data)
{
    delete (FTFontData*)(data);
}

RefPtr<FontResource> FontResource::create(ResourceFetcher* fetcher, const Url& url)
{
    auto resource = ResourceLoader::loadUrl(url, fetcher);
    if(resource.isNull())
        return nullptr;
    auto fontData = FTFontData::create(std::move(resource));
    if(fontData == nullptr) {
        std::cerr << "unable to decode font: " << url << std::endl;
        return nullptr;
    }

    static cairo_user_data_key_t key;
    auto face = cairo_ft_font_face_create_for_ft_face(fontData->face(), FT_LOAD_DEFAULT);
    cairo_font_face_set_user_data(face, &key, fontData, FTFontDataDestroy);
    if(auto status = cairo_font_face_status(face)) {
        std::cerr << "cairo error: " << cairo_status_to_string(status) << std::endl;
        FTFontDataDestroy(fontData);
        return nullptr;
    }

    return adoptPtr(new FontResource(face));
}

bool FontResource::supportsFormat(const std::string_view& format)
{
    return true;
}

FontResource::~FontResource()
{
    cairo_font_face_destroy(m_face);
}

FontSelectionAlgorithm::FontSelectionAlgorithm(const FontSelectionRequest& request)
    : m_request(request)
{
}

void FontSelectionAlgorithm::addCandidate(const FontSelectionDescription& description)
{
    assert(description.weight && description.width && description.slope);

    m_weight.minimum = std::min(m_weight.minimum, description.weight.minimum);
    m_weight.maximum = std::max(m_weight.maximum, description.weight.maximum);

    m_width.minimum = std::min(m_width.minimum, description.width.minimum);
    m_width.maximum = std::max(m_width.maximum, description.width.maximum);

    m_slope.minimum = std::min(m_slope.minimum, description.slope.minimum);
    m_slope.maximum = std::max(m_slope.maximum, description.slope.maximum);
}

FontSelectionValue FontSelectionAlgorithm::widthDistance(const FontSelectionRange& width) const
{
    if(m_request.width >= width.minimum && m_request.width <= width.maximum)
        return FontSelectionValue(0);
    if(m_request.width > kNormalFontWidth) {
        if(width.minimum > m_request.width)
            return width.minimum - m_request.width;
        assert(width.maximum < m_request.width);
        auto threshold = std::max(m_request.width, m_width.maximum);
        return threshold - width.maximum;
    }

    if(width.maximum < m_request.width)
        return m_request.width - width.maximum;
    assert(width.minimum > m_request.width);
    auto threshold = std::min(m_request.width, m_width.minimum);
    return width.minimum - threshold;
}

FontSelectionValue FontSelectionAlgorithm::slopeDistance(const FontSelectionRange& slope) const
{
    if(m_request.slope >= slope.minimum && m_request.slope <= slope.maximum)
        return FontSelectionValue(0);
    if(m_request.slope >= kItalicFontSlope) {
        if(slope.minimum > m_request.slope)
            return slope.minimum - m_request.slope;
        assert(m_request.slope > slope.maximum);
        auto threshold = std::max(m_request.slope, m_slope.maximum);
        return threshold - slope.maximum;
    }

    if(m_request.slope >= FontSelectionValue(0)) {
        if(slope.maximum >= FontSelectionValue(0) && slope.maximum < m_request.slope)
            return m_request.slope - slope.maximum;
        if(slope.minimum > m_request.slope)
            return slope.minimum;
        assert(slope.maximum < FontSelectionValue(0));
        auto threshold = std::max(m_request.slope, m_slope.maximum);
        return threshold - slope.maximum;
    }

    if(m_request.slope > -kItalicFontSlope) {
        if(slope.minimum > m_request.slope && slope.minimum <= FontSelectionValue(0))
            return slope.minimum - m_request.slope;
        if(slope.maximum < m_request.slope)
            return -slope.maximum;
        assert(slope.minimum > FontSelectionValue(0));
        auto threshold = std::min(m_request.slope, m_slope.minimum);
        return slope.minimum - threshold;
    }

    if(slope.maximum < m_request.slope)
        return m_request.slope - slope.maximum;
    assert(slope.minimum > m_request.slope);
    auto threshold = std::min(m_request.slope, m_slope.minimum);
    return slope.minimum - threshold;
}

FontSelectionValue FontSelectionAlgorithm::weightDistance(const FontSelectionRange& weight) const
{
    constexpr FontSelectionValue kUpperWeightSearchThreshold = 500.f;
    constexpr FontSelectionValue kLowerWeightSearchThreshold = 400.f;
    if(m_request.weight >= weight.minimum && m_request.weight <= weight.maximum)
        return FontSelectionValue(0);
    if(m_request.weight >= kLowerWeightSearchThreshold && m_request.weight <= kUpperWeightSearchThreshold) {
        if(weight.minimum > m_request.weight && weight.minimum <= kUpperWeightSearchThreshold)
            return weight.minimum - m_request.weight;
        if(weight.maximum < m_request.weight)
            return kUpperWeightSearchThreshold - weight.maximum;
        assert(weight.minimum > kUpperWeightSearchThreshold);
        auto threshold = std::min(m_request.weight, m_weight.minimum);
        return weight.minimum - threshold;
    }

    if(m_request.weight < kLowerWeightSearchThreshold) {
        if(weight.maximum < m_request.weight)
            return m_request.weight - weight.maximum;
        assert(weight.minimum > m_request.weight);
        auto threshold = std::min(m_request.weight, m_weight.minimum);
        return weight.minimum - threshold;
    }

    assert(m_request.weight >= kUpperWeightSearchThreshold);
    if(weight.minimum > m_request.weight)
        return weight.minimum - m_request.weight;
    assert(weight.maximum < m_request.weight);
    auto threshold = std::max(m_request.weight, m_weight.maximum);
    return threshold - weight.maximum;
}

bool FontSelectionAlgorithm::isCandidateBetter(const FontSelectionDescription& currentCandidate, const FontSelectionDescription& previousCandidate) const
{
    auto widthDelta = widthDistance(currentCandidate.width) - widthDistance(previousCandidate.width);
    if(widthDelta < FontSelectionValue(0))
        return true;
    if(widthDelta > FontSelectionValue(0))
        return false;
    auto slopeDelta = slopeDistance(currentCandidate.slope) - slopeDistance(previousCandidate.slope);
    if(slopeDelta < FontSelectionValue(0))
        return true;
    if(slopeDelta > FontSelectionValue(0))
        return false;
    return weightDistance(currentCandidate.weight) < weightDistance(previousCandidate.weight);
}

RefPtr<FontData> LocalFontFace::getFontData(const FontDataDescription& description)
{
    return fontDataCache()->getFontData(m_family, description);
}

static std::string buildVariationSettings(const FontDataDescription& description, FontVariationList variations)
{
    constexpr FontTag wghtTag("wght");
    constexpr FontTag wdthTag("wdth");
    constexpr FontTag slntTag("slnt");

    variations.emplace_front(wghtTag, description.request.weight);
    variations.emplace_front(wdthTag, description.request.width);
    variations.emplace_front(slntTag, description.request.slope);
    for(auto& variation : description.variations) {
        variations.push_front(variation);
    }

    variations.sort();
    variations.unique();

    std::string output;
    for(auto& [tag, value] : variations) {
        const char name[4] = {
            static_cast<char>(0xFF & (tag.value() >> 24)),
            static_cast<char>(0xFF & (tag.value() >> 16)),
            static_cast<char>(0xFF & (tag.value() >> 8)),
            static_cast<char>(0xFF & (tag.value() >> 0))
        };

        if(!output.empty())
            output += ',';
        output += std::string_view(name, 4);
        output += '=';
        output += std::to_string(value);
    }

    return output;
}

RefPtr<FontData> RemoteFontFace::getFontData(const FontDataDescription& description)
{
    const float slopeAngle = -tanf(description.request.slope * std::numbers::pi / 180.f);

    cairo_matrix_t ctm;
    cairo_matrix_init_identity(&ctm);

    cairo_matrix_t ftm;
    cairo_matrix_init(&ftm, 1, 0, slopeAngle, 1, 0, 0);
    cairo_matrix_scale(&ftm, description.size, description.size);

    auto options = cairo_font_options_create();
    auto variations = buildVariationSettings(description, m_variations);
    cairo_font_options_set_variations(options, variations.data());
    cairo_font_options_set_hint_metrics(options, CAIRO_HINT_METRICS_OFF);

    auto face = cairo_font_face_reference(m_resource->face());
    auto font = cairo_scaled_font_create(face, &ftm, &ctm, options);

    cairo_font_face_destroy(face);
    cairo_font_options_destroy(options);
    return SimpleFontData::create(font, m_features);
}

RefPtr<FontData> SegmentedFontFace::getFontData(const FontDataDescription& description)
{
    auto& fontData = m_table[description];
    if(fontData != nullptr)
        return fontData;
    FontDataSet fonts;
    for(auto& face : m_faces) {
        if(auto font = face->getFontData(description)) {
            fonts.insert(std::move(font));
        }
    }

    if(!fonts.empty())
        fontData = SegmentedFontData::create(std::move(fonts));
    return fontData;
}

RefPtr<SimpleFontData> SimpleFontData::create(cairo_scaled_font_t* font, FontFeatureList features)
{
    auto face = cairo_ft_scaled_font_lock_face(font);
    if(face == nullptr) {
        auto status = cairo_scaled_font_status(font);
        std::cerr << "cairo error: " << cairo_status_to_string(status) << std::endl;
        cairo_scaled_font_destroy(font);
        return nullptr;
    }

    auto charSet = FcFreeTypeCharSet(face, nullptr);
    auto zeroGlyph = FcFreeTypeCharIndex(face, '0');
    auto spaceGlyph = FcFreeTypeCharIndex(face, ' ');
    auto xGlyph = FcFreeTypeCharIndex(face, 'x');
    auto glyph_extents = [font](auto index) {
        cairo_text_extents_t extents;
        cairo_glyph_t glyph = {index, 0, 0};
        cairo_scaled_font_glyph_extents(font, &glyph, 1, &extents);
        return extents;
    };

    cairo_font_extents_t font_extents;
    cairo_scaled_font_extents(font, &font_extents);

    FontDataInfo info;
    info.ascent = font_extents.ascent;
    info.descent = font_extents.descent;
    info.lineGap = font_extents.height - font_extents.ascent - font_extents.descent;
    info.xHeight = glyph_extents(xGlyph).height;
    info.spaceWidth = glyph_extents(spaceGlyph).x_advance;
    info.zeroWidth = glyph_extents(zeroGlyph).x_advance;
    info.zeroGlyph = zeroGlyph;
    info.spaceGlyph = spaceGlyph;
    cairo_ft_scaled_font_unlock_face(font);
    return adoptPtr(new SimpleFontData(font, charSet, info, std::move(features)));
}

#define FLT_TO_HB(v) static_cast<hb_position_t>(v * (1 << 16))

hb_font_t* SimpleFontData::hbFont() const
{
    if(m_hbFont != nullptr)
        return m_hbFont;
    auto ftFace = cairo_ft_scaled_font_lock_face(m_font);
    auto hbFace = hb_ft_face_create_referenced(ftFace);
    auto hbFont = hb_font_create(hbFace);

    cairo_matrix_t scale_matrix;
    cairo_scaled_font_get_scale_matrix(m_font, &scale_matrix);
    hb_font_set_scale(hbFont, FLT_TO_HB(scale_matrix.xx), FLT_TO_HB(scale_matrix.yy));

    cairo_font_options_t* font_options = cairo_font_options_create();
    cairo_scaled_font_get_font_options(m_font, font_options);
    std::string_view variations(cairo_font_options_get_variations(font_options));
    std::vector<hb_variation_t> settings;
    while(!variations.empty()) {
        hb_variation_t setting;
        auto variation = variations.substr(0, variations.find(','));
        if(hb_variation_from_string(variation.data(), variation.size(), &setting))
            settings.push_back(setting);
        variations.remove_prefix(std::min(variations.size(), variation.size() + 1));
    }

    hb_font_set_variations(hbFont, settings.data(), settings.size());
    cairo_font_options_destroy(font_options);

    static hb_font_funcs_t* hbFunctions = nullptr;
    if(hbFunctions == nullptr) {
        auto nominal_glyph_func = [](hb_font_t*, void* context, hb_codepoint_t unicode, hb_codepoint_t* glyph, void*) -> hb_bool_t {
            auto font = static_cast<cairo_scaled_font_t*>(context);
            if(auto face = cairo_ft_scaled_font_lock_face(font)) {
                *glyph = FcFreeTypeCharIndex(face, unicode);
                cairo_ft_scaled_font_unlock_face(font);
                return !!*glyph;
            }

            return false;
        };

        auto variation_glyph_func = [](hb_font_t*, void* context, hb_codepoint_t unicode, hb_codepoint_t variation, hb_codepoint_t* glyph, void*) -> hb_bool_t {
            auto font = static_cast<cairo_scaled_font_t*>(context);
            if(auto face = cairo_ft_scaled_font_lock_face(font)) {
                *glyph = FT_Face_GetCharVariantIndex(face, unicode, variation);
                cairo_ft_scaled_font_unlock_face(font);
                return !!*glyph;
            }

            return false;
        };

        auto glyph_h_advance_func = [](hb_font_t*, void* context, hb_codepoint_t index, void*) -> hb_position_t {
            auto font = static_cast<cairo_scaled_font_t*>(context);
            cairo_text_extents_t extents;
            cairo_glyph_t glyph = {index, 0, 0};
            cairo_scaled_font_glyph_extents(font, &glyph, 1, &extents);
            return FLT_TO_HB(extents.x_advance);
        };

        auto glyph_extents_func = [](hb_font_t*, void* context, hb_codepoint_t index, hb_glyph_extents_t* extents, void*) -> hb_bool_t {
            auto font = static_cast<cairo_scaled_font_t*>(context);
            cairo_text_extents_t glyph_extents;
            cairo_glyph_t glyph = {index, 0, 0};
            cairo_scaled_font_glyph_extents(font, &glyph, 1, &glyph_extents);

            extents->x_bearing = FLT_TO_HB(glyph_extents.x_bearing);
            extents->y_bearing = FLT_TO_HB(glyph_extents.y_bearing);
            extents->width = FLT_TO_HB(glyph_extents.width);
            extents->height = FLT_TO_HB(glyph_extents.height);
            return true;
        };

        hbFunctions = hb_font_funcs_create();
        hb_font_funcs_set_nominal_glyph_func(hbFunctions, nominal_glyph_func, nullptr, nullptr);
        hb_font_funcs_set_variation_glyph_func(hbFunctions, variation_glyph_func, nullptr, nullptr);
        hb_font_funcs_set_glyph_h_advance_func(hbFunctions, glyph_h_advance_func, nullptr, nullptr);
        hb_font_funcs_set_glyph_extents_func(hbFunctions, glyph_extents_func, nullptr, nullptr);
        hb_font_funcs_make_immutable(hbFunctions);
    }

    hb_font_set_funcs(hbFont, hbFunctions, m_font, nullptr);
    hb_font_make_immutable(hbFont);
    hb_face_destroy(hbFace);
    cairo_ft_scaled_font_unlock_face(m_font);

    m_hbFont = hbFont;
    return hbFont;
}

const SimpleFontData* SimpleFontData::getFontData(uint32_t codepoint) const
{
    if(FcCharSetHasChar(m_charSet, codepoint))
        return this;
    return nullptr;
}

float SimpleFontData::tabWidth(const TabSize& tabSize) const
{
    if(tabSize.type() == TabSize::Type::Space)
        return tabSize.value() * spaceWidth();
    return tabSize.value();
}

SimpleFontData::~SimpleFontData()
{
    hb_font_destroy(m_hbFont);
    cairo_scaled_font_destroy(m_font);
    FcCharSetDestroy(m_charSet);
}

const SimpleFontData* SegmentedFontData::getFontData(uint32_t codepoint) const
{
    for(auto& font : m_fonts) {
        if(auto fontData = font->getFontData(codepoint)) {
            return fontData;
        }
    }

    return nullptr;
}

RefPtr<SimpleFontData> FontDataCache::getFontData(const FontDataDescription& description)
{
    return getFontData(emptyGlo, description);
}

constexpr int fcWeight(FontSelectionValue weight)
{
    if(weight < FontSelectionValue(150))
        return FC_WEIGHT_THIN;
    if(weight < FontSelectionValue(250))
        return FC_WEIGHT_ULTRALIGHT;
    if(weight < FontSelectionValue(350))
        return FC_WEIGHT_LIGHT;
    if(weight < FontSelectionValue(450))
        return FC_WEIGHT_REGULAR;
    if(weight < FontSelectionValue(550))
        return FC_WEIGHT_MEDIUM;
    if(weight < FontSelectionValue(650))
        return FC_WEIGHT_SEMIBOLD;
    if(weight < FontSelectionValue(750))
        return FC_WEIGHT_BOLD;
    if(weight < FontSelectionValue(850))
        return FC_WEIGHT_EXTRABOLD;
    return FC_WEIGHT_ULTRABLACK;
}

constexpr int fcWidth(FontSelectionValue width)
{
    if(width <= kUltraCondensedFontWidth)
        return FC_WIDTH_ULTRACONDENSED;
    if(width <= kExtraCondensedFontWidth)
        return FC_WIDTH_EXTRACONDENSED;
    if(width <= kCondensedFontWidth)
        return FC_WIDTH_CONDENSED;
    if(width <= kSemiCondensedFontWidth)
        return FC_WIDTH_SEMICONDENSED;
    if(width <= kNormalFontWidth)
        return FC_WIDTH_NORMAL;
    if(width <= kSemiExpandedFontWidth)
        return FC_WIDTH_SEMIEXPANDED;
    if(width <= kExpandedFontWidth)
        return FC_WIDTH_EXPANDED;
    if(width <= kExtraExpandedFontWidth)
        return FC_WIDTH_EXTRAEXPANDED;
    return FC_WIDTH_ULTRAEXPANDED;
}

constexpr int fcSlant(FontSelectionValue slope)
{
    if(slope <= kNormalFontSlope)
        return FC_SLANT_ROMAN;
    if(slope <= kItalicFontSlope)
        return FC_SLANT_ITALIC;
    return FC_SLANT_OBLIQUE;
}

static RefPtr<SimpleFontData> createFontDataFromPattern(FcPattern* pattern, const FontDataDescription& description)
{
    if(pattern == nullptr)
        return nullptr;
    FcMatrix matrix;
    FcMatrixInit(&matrix);

    int matchMatrixIndex = 0;
    FcMatrix* matchMatrix = nullptr;
    while(FcPatternGetMatrix(pattern, FC_MATRIX, matchMatrixIndex, &matchMatrix) == FcResultMatch) {
        FcMatrixMultiply(&matrix, &matrix, matchMatrix);
        ++matchMatrixIndex;
    }

    cairo_matrix_t ctm;
    cairo_matrix_init_identity(&ctm);

    cairo_matrix_t ftm;
    cairo_matrix_init(&ftm, 1.0, -matrix.yx, -matrix.xy, 1.0, 0.0, 0.0);
    cairo_matrix_scale(&ftm, description.size, description.size);

    FontFeatureList featureSettings;
    FontVariationList variationSettings;

    int matchFeatureIndex = 0;
    char* matchFeatureName = nullptr;
    while(FcPatternGetString(pattern, FC_FONT_FEATURES, matchFeatureIndex, (FcChar8**)(&matchFeatureName)) == FcResultMatch) {
        hb_feature_t feature;
        if(hb_feature_from_string(matchFeatureName, -1, &feature))
            featureSettings.emplace_front(feature.tag, feature.value);
        ++matchFeatureIndex;
    }

    int matchVariationIndex = 0;
    char* matchVariationName = nullptr;
    while(FcPatternGetString(pattern, FC_FONT_VARIATIONS, matchVariationIndex, (FcChar8**)(&matchVariationName)) == FcResultMatch) {
        hb_variation_t variation;
        if(hb_variation_from_string(matchVariationName, -1, &variation))
            variationSettings.emplace_front(variation.tag, variation.value);
        ++matchVariationIndex;
    }

    auto options = cairo_font_options_create();
    auto variations = buildVariationSettings(description, variationSettings);
    cairo_font_options_set_variations(options, variations.data());
    cairo_font_options_set_hint_metrics(options, CAIRO_HINT_METRICS_OFF);

    auto face = cairo_ft_font_face_create_for_pattern(pattern);
    auto font = cairo_scaled_font_create(face, &ftm, &ctm, options);

    cairo_font_face_destroy(face);
    cairo_font_options_destroy(options);
    FcPatternDestroy(pattern);
    return SimpleFontData::create(font, std::move(featureSettings));
}

constexpr bool isGenericFamilyName(const std::string_view& familyName)
{
    return equals(familyName, "sans", false)
        || equals(familyName, "sans-serif", false)
        || equals(familyName, "serif", false)
        || equals(familyName, "monospace", false)
        || equals(familyName, "fantasy", false)
        || equals(familyName, "cursive", false)
        || equals(familyName, "emoji", false);
}

RefPtr<SimpleFontData> FontDataCache::getFontData(const GlobalString& family, const FontDataDescription& description)
{
    std::lock_guard guard(m_mutex);
    auto& fontData = m_table[family][description];
    if(fontData != nullptr)
        return fontData;
    auto pattern = FcPatternCreate();
    FcPatternAddDouble(pattern, FC_PIXEL_SIZE, description.size);
    FcPatternAddInteger(pattern, FC_WEIGHT, fcWeight(description.request.weight));
    FcPatternAddInteger(pattern, FC_WIDTH, fcWidth(description.request.width));
    FcPatternAddInteger(pattern, FC_SLANT, fcSlant(description.request.slope));

    std::string familyName(family.value());
    FcPatternAddString(pattern, FC_FAMILY, (FcChar8*)(familyName.data()));
    FcPatternAddBool(pattern, FC_SCALABLE, FcTrue);

    FcConfigSubstitute(m_config, pattern, FcMatchPattern);
    FcDefaultSubstitute(pattern);

    char* configFamilyName = nullptr;
    FcPatternGetString(pattern, FC_FAMILY, 0, (FcChar8**)(&configFamilyName));

    FcResult matchResult;
    auto matchPattern = FcFontMatch(m_config, pattern, &matchResult);
    if(matchResult == FcResultMatch && !familyName.empty() && !isGenericFamilyName(familyName)) {
        matchResult = FcResultNoMatch;
        FcValue matchValue;
        FcValueBinding matchBinding;
        int matchFamilyIndex = 0;
        while(FcPatternGetWithBinding(matchPattern, FC_FAMILY, matchFamilyIndex, &matchValue, &matchBinding) == FcResultMatch) {
            auto matchFamilyName = (const char*)(matchValue.u.s);
            if(matchBinding == FcValueBindingStrong || equals(configFamilyName, matchFamilyName, false) || equals(familyName, matchFamilyName, false)) {
               matchResult = FcResultMatch;
               break;
            }

            ++matchFamilyIndex;
        }
    }

    FcPatternDestroy(pattern);
    if(matchResult == FcResultMatch)
        return (fontData = createFontDataFromPattern(matchPattern, description));
    std::cerr << "unable to load font: family=" << family << " size=" << description.size
        << " weight=" << description.request.weight << " width=" << description.request.width << " slope=" << description.request.slope << std::endl;
    return nullptr;
}

RefPtr<SimpleFontData> FontDataCache::getFontData(uint32_t codepoint, const FontDataDescription& description)
{
    std::lock_guard guard(m_mutex);
    auto pattern = FcPatternCreate();
    auto charSet = FcCharSetCreate();

    FcCharSetAddChar(charSet, codepoint);
    FcPatternAddCharSet(pattern, FC_CHARSET, charSet);
    FcPatternAddDouble(pattern, FC_PIXEL_SIZE, description.size);
    FcPatternAddInteger(pattern, FC_WEIGHT, fcWeight(description.request.weight));
    FcPatternAddInteger(pattern, FC_WIDTH, fcWidth(description.request.width));
    FcPatternAddInteger(pattern, FC_SLANT, fcSlant(description.request.slope));

    FcConfigSubstitute(m_config, pattern, FcMatchPattern);
    FcDefaultSubstitute(pattern);
    FcResult matchResult;
    auto matchPattern = FcFontMatch(m_config, pattern, &matchResult);
    if(matchResult == FcResultMatch) {
        matchResult = FcResultNoMatch;
        int matchCharSetIndex = 0;
        FcCharSet* matchCharSet = nullptr;
        while(FcPatternGetCharSet(matchPattern, FC_CHARSET, matchCharSetIndex, &matchCharSet) == FcResultMatch) {
            if(FcCharSetHasChar(matchCharSet, codepoint)) {
                matchResult = FcResultMatch;
                break;
            }

            ++matchCharSetIndex;
        }
    }

    FcCharSetDestroy(charSet);
    FcPatternDestroy(pattern);
    if(matchResult == FcResultMatch)
        return createFontDataFromPattern(matchPattern, description);
    FcPatternDestroy(matchPattern);
    std::cerr << "unable to load font: codepoint=" << codepoint << " size=" << description.size
        << " weight=" << description.request.weight << " width=" << description.request.width << " slope=" << description.request.slope << std::endl;
    return nullptr;
}

FontDataCache::~FontDataCache()
{
    FcConfigDestroy(m_config);
}

FontDataCache::FontDataCache()
    : m_config(FcInitLoadConfigAndFonts())
{
    for(auto nameSet : { FcSetSystem, FcSetApplication }) {
        auto allFonts = FcConfigGetFonts(m_config, nameSet);
        if(allFonts == nullptr)
            continue;
        for(int fontIndex = 0; fontIndex < allFonts->nfont; ++fontIndex) {
            auto matchPattern = allFonts->fonts[fontIndex];
            int matchFamilyIndex = 0;
            char* matchFamilyName = nullptr;
            while(FcPatternGetString(matchPattern, FC_FAMILY, matchFamilyIndex, (FcChar8**)(&matchFamilyName)) == FcResultMatch) {
                m_families.emplace(matchFamilyName);
                ++matchFamilyIndex;
            }
        }
    }
}

FontDataCache* fontDataCache()
{
    static FontDataCache fontCache;
    return &fontCache;
}

RefPtr<Font> Font::create(Document* document, const FontDescription& description)
{
    return adoptPtr(new (document->heap()) Font(document, description));
}

Heap* Font::heap() const
{
    return m_document->heap();
}

const SimpleFontData* Font::getFontData(uint32_t codepoint) const
{
    for(auto& font : m_fonts) {
        if(auto fontData = font->getFontData(codepoint)) {
            return fontData;
        }
    }

    if(auto fontData = fontDataCache()->getFontData(codepoint, m_description.data)) {
        m_fonts.push_back(fontData);
        return fontData.get();
    }

    return m_primaryFont;
}

Font::Font(Document* document, const FontDescription& description)
    : m_document(document)
    , m_description(description)
    , m_fonts(document->heap())
{
    for(auto& family : description.families) {
        if(auto font = document->getFontData(family, description.data)) {
            m_fonts.push_back(std::move(font));
        }
    }

    for(auto& font : m_fonts) {
        if(auto fontData = font->getFontData(' ')) {
            m_primaryFont = fontData;
            break;
        }
    }

    if(m_primaryFont == nullptr) {
        if(auto fontData = fontDataCache()->getFontData(description.data)) {
            m_fonts.push_back(fontData);
            m_primaryFont = fontData.get();
        }
    }
}

} // namespace plutobook
