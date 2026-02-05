/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "fontresource.h"
#include "document.h"
#include "stringutils.h"

#include "plutobook.hpp"

#include <fontconfig/fontconfig.h>
#include <fontconfig/fcfreetype.h>

#include <cairo-ft.h>
#include <hb-ft.h>

#include <numbers>
#include <cmath>

namespace plutobook {

class FTFontData {
public:
    static FTFontData* create(ResourceData resource);
    FT_Face face() const { return m_face; }
    ~FTFontData() { FT_Done_Face(m_face); }

private:
    FTFontData(FT_Face face, ResourceData resource)
        : m_face(face), m_resource(std::move(resource))
    {}

    FT_Face m_face;
    ResourceData m_resource;
};

FTFontData* FTFontData::create(ResourceData resource)
{
    thread_local FT_Library ftLibrary;
    if(ftLibrary == nullptr) {
        if(auto error = FT_Init_FreeType(&ftLibrary)) {
            plutobook_set_error_message("font decode error: %s", FT_Error_String(error));
            return nullptr;
        }
    }

    FT_Face ftFace = nullptr;
    if(auto error = FT_New_Memory_Face(ftLibrary, (FT_Byte*)(resource.content()), resource.contentLength(), 0, &ftFace)) {
        plutobook_set_error_message("font decode error: %s", FT_Error_String(error));
        return nullptr;
    }

    return new FTFontData(ftFace, std::move(resource));
}

static void FTFontDataDestroy(void* data)
{
    delete (FTFontData*)(data);
}

RefPtr<FontResource> FontResource::create(Document* document, const Url& url)
{
    auto resource = ResourceLoader::loadUrl(url, document->customResourceFetcher());
    if(resource.isNull())
        return nullptr;
    auto fontData = FTFontData::create(std::move(resource));
    if(fontData == nullptr) {
        plutobook_set_error_message("Unable to load font '%s': %s", url.value().data(), plutobook_get_error_message());
        return nullptr;
    }

    static cairo_user_data_key_t key;
    auto face = cairo_ft_font_face_create_for_ft_face(fontData->face(), FT_LOAD_DEFAULT);
    cairo_font_face_set_user_data(face, &key, fontData, FTFontDataDestroy);
    if(auto status = cairo_font_face_status(face)) {
        plutobook_set_error_message("Unable to load font '%s': %s", url.value().data(), cairo_status_to_string(status));
        FTFontDataDestroy(fontData);
        return nullptr;
    }

    return adoptPtr(new (document->heap()) FontResource(face, FcFreeTypeCharSet(fontData->face(), nullptr)));
}

bool FontResource::supportsFormat(std::string_view format)
{
    return equals(format, "opentype", false)
        || equals(format, "opentype-variations", false)
        || equals(format, "truetype", false)
        || equals(format, "truetype-variations", false)
#ifdef FT_CONFIG_OPTION_USE_BROTLI
        || equals(format, "woff2", false)
        || equals(format, "woff2-variations", false)
#endif
        || equals(format, "woff", false)
        || equals(format, "woff-variations", false);
}

FontResource::~FontResource()
{
    FcCharSetDestroy(m_charSet);
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

static std::string buildVariationSettings(const FontDataDescription& description, const FontVariationList& variations)
{
    constexpr FontTag wghtTag("wght");
    constexpr FontTag wdthTag("wdth");
    constexpr FontTag slntTag("slnt");

    std::map<FontTag, float> variationSettings;
    for(const auto& variation : description.variations) {
        variationSettings.insert(variation);
    }

    variationSettings.emplace(wghtTag, description.request.weight);
    variationSettings.emplace(wdthTag, description.request.width);
    variationSettings.emplace(slntTag, description.request.slope);

    std::string output;
    for(const auto& [tag, value] : variationSettings) {
        const char name[4] = {
            static_cast<char>(0xFF & (tag.value() >> 24)),
            static_cast<char>(0xFF & (tag.value() >> 16)),
            static_cast<char>(0xFF & (tag.value() >> 8)),
            static_cast<char>(0xFF & (tag.value() >> 0))
        };

        if(!output.empty())
            output += ',';
        output.append(name, 4);
        output += '=';
        output += toString(value);
    }

    return output;
}

RefPtr<FontData> SimpleFontFace::getFontData(Document* document, const FontDataDescription& description)
{
    while(m_resource == nullptr && !m_sources.empty()) {
        const auto& source = m_sources.front();
        if(auto family = std::get_if<GlobalString>(&source)) {
            return fontDataCache()->getFontData(*family, description);
        }

        const auto& url = std::get<Url>(source);
        m_resource = document->fetchFontResource(url);
        m_sources.pop_front();
    }

    if(m_resource == nullptr) {
        return nullptr;
    }

    const auto slopeAngle = -std::tan(description.request.slope * std::numbers::pi / 180.0);

    cairo_matrix_t ctm;
    cairo_matrix_init_identity(&ctm);

    cairo_matrix_t ftm;
    cairo_matrix_init(&ftm, 1, 0, slopeAngle, 1, 0, 0);
    cairo_matrix_scale(&ftm, description.size, description.size);

    auto options = cairo_font_options_create();
    auto variations = buildVariationSettings(description, m_variations);
    cairo_font_options_set_variations(options, variations.data());
    cairo_font_options_set_hint_metrics(options, CAIRO_HINT_METRICS_OFF);

    auto charSet = FcCharSetCopy(m_resource->charSet());
    auto face = cairo_font_face_reference(m_resource->face());
    auto font = cairo_scaled_font_create(face, &ftm, &ctm, options);

    cairo_font_face_destroy(face);
    cairo_font_options_destroy(options);

    return SimpleFontData::create(font, charSet, m_features);
}

RefPtr<FontData> SegmentedFontFace::getFontData(Document* document, const FontDataDescription& description)
{
    auto& fontData = m_table[description];
    if(fontData != nullptr)
        return fontData;
    FontDataRangeList fonts;
    for(const auto& face : m_faces) {
        if(auto font = face->getFontData(document, description)) {
            const auto& ranges = face->ranges();
            if(ranges.empty()) {
                fonts.emplace_front(0, 0x10FFFF, std::move(font));
            } else {
                for(const auto& range : ranges) {
                    fonts.emplace_front(range.first, range.second, font);
                }
            }
        }
    }

    if(!fonts.empty())
        fontData = SegmentedFontData::create(std::move(fonts));
    return fontData;
}

#define FLT_TO_HB(v) static_cast<hb_position_t>((v) * (1 << 16))

RefPtr<SimpleFontData> SimpleFontData::create(cairo_scaled_font_t* font, FcCharSet* charSet, FontFeatureList features)
{
    auto ftFace = cairo_ft_scaled_font_lock_face(font);
    if(ftFace == nullptr) {
        cairo_scaled_font_destroy(font);
        return nullptr;
    }

    auto zeroGlyph = FcFreeTypeCharIndex(ftFace, '0');
    auto spaceGlyph = FcFreeTypeCharIndex(ftFace, ' ');
    auto xGlyph = FcFreeTypeCharIndex(ftFace, 'x');
    auto glyph_extents = [font](unsigned long index) {
        cairo_glyph_t glyph = { index, 0, 0 };
        cairo_text_extents_t extents;
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
    info.hasColor = FT_HAS_COLOR(ftFace);

    auto hbFace = hb_ft_face_create_referenced(ftFace);
    auto hbFont = hb_font_create(hbFace);

    cairo_matrix_t scale_matrix;
    cairo_scaled_font_get_scale_matrix(font, &scale_matrix);
    hb_font_set_scale(hbFont, FLT_TO_HB(scale_matrix.xx), FLT_TO_HB(scale_matrix.yy));

    cairo_font_options_t* font_options = cairo_font_options_create();
    cairo_scaled_font_get_font_options(font, font_options);

    std::vector<hb_variation_t> settings;
    std::string_view variations(cairo_font_options_get_variations(font_options));
    while(!variations.empty()) {
        auto delim_index = variations.find(',');
        auto variation = variations.substr(0, delim_index);

        hb_variation_t setting;
        if(hb_variation_from_string(variation.data(), variation.size(), &setting)) {
            settings.push_back(setting);
        }

        variations.remove_prefix(variation.size());
        if(delim_index != std::string_view::npos) {
            variations.remove_prefix(1);
        }
    }

    hb_font_set_variations(hbFont, settings.data(), settings.size());
    cairo_font_options_destroy(font_options);

    static hb_font_funcs_t* hbFunctions = []() {
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

        auto hbFunctions = hb_font_funcs_create();
        hb_font_funcs_set_nominal_glyph_func(hbFunctions, nominal_glyph_func, nullptr, nullptr);
        hb_font_funcs_set_variation_glyph_func(hbFunctions, variation_glyph_func, nullptr, nullptr);
        hb_font_funcs_set_glyph_h_advance_func(hbFunctions, glyph_h_advance_func, nullptr, nullptr);
        hb_font_funcs_set_glyph_extents_func(hbFunctions, glyph_extents_func, nullptr, nullptr);
        hb_font_funcs_make_immutable(hbFunctions);
        return hbFunctions;
    }();

    hb_font_set_funcs(hbFont, hbFunctions, font, nullptr);
    hb_font_make_immutable(hbFont);
    hb_face_destroy(hbFace);
    cairo_ft_scaled_font_unlock_face(font);

    return adoptPtr(new SimpleFontData(font, hbFont, charSet, info, std::move(features)));
}

const SimpleFontData* SimpleFontData::getFontData(uint32_t codepoint, bool preferColor) const
{
    if(preferColor && !m_info.hasColor)
        return nullptr;
    if(FcCharSetHasChar(m_charSet, codepoint))
        return this;
    return nullptr;
}

SimpleFontData::~SimpleFontData()
{
    hb_font_destroy(m_hbFont);
    cairo_scaled_font_destroy(m_font);
    FcCharSetDestroy(m_charSet);
}

const SimpleFontData* FontDataRange::getFontData(uint32_t codepoint, bool preferColor) const
{
    if(m_from <= codepoint && m_to >= codepoint)
        return m_data->getFontData(codepoint, preferColor);
    return nullptr;
}

const SimpleFontData* SegmentedFontData::getFontData(uint32_t codepoint, bool preferColor) const
{
    for(const auto& font : m_fonts) {
        if(auto fontData = font.getFontData(codepoint, preferColor)) {
            return fontData;
        }
    }

    return nullptr;
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

    int matchCharSetIndex = 0;
    FcCharSet* matchCharSet = nullptr;

    auto charSet = FcCharSetCreate();
    while(FcPatternGetCharSet(pattern, FC_CHARSET, matchCharSetIndex, &matchCharSet) == FcResultMatch) {
        FcCharSetMerge(charSet, matchCharSet, nullptr);
        ++matchCharSetIndex;
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

    return SimpleFontData::create(font, charSet, std::move(featureSettings));
}

constexpr bool isGenericFamilyName(std::string_view familyName)
{
    return equals(familyName, "sans", false)
        || equals(familyName, "sans-serif", false)
        || equals(familyName, "serif", false)
        || equals(familyName, "monospace", false)
        || equals(familyName, "fantasy", false)
        || equals(familyName, "cursive", false)
        || equals(familyName, "emoji", false);
}

static RefPtr<SimpleFontData> createFontData(FcConfig* config, const GlobalString& family, const FontDataDescription& description)
{
    auto pattern = FcPatternCreate();
    FcPatternAddDouble(pattern, FC_PIXEL_SIZE, description.size);
    FcPatternAddInteger(pattern, FC_WEIGHT, fcWeight(description.request.weight));
    FcPatternAddInteger(pattern, FC_WIDTH, fcWidth(description.request.width));
    FcPatternAddInteger(pattern, FC_SLANT, fcSlant(description.request.slope));

    std::string familyName(family.value());
    FcPatternAddString(pattern, FC_FAMILY, (FcChar8*)(familyName.data()));
    FcPatternAddBool(pattern, FC_SCALABLE, FcTrue);
    if(equalsIgnoringCase(familyName, "emoji")) {
        FcPatternAddBool(pattern, FC_COLOR, FcTrue);
    }

    FcConfigSubstitute(config, pattern, FcMatchPattern);
    FcDefaultSubstitute(pattern);

    char* configFamilyName = nullptr;
    FcPatternGetString(pattern, FC_FAMILY, 0, (FcChar8**)(&configFamilyName));

    FcResult matchResult;
    auto matchPattern = FcFontMatch(config, pattern, &matchResult);
    if(matchResult == FcResultMatch && !isGenericFamilyName(familyName)) {
        matchResult = FcResultNoMatch;
        FcValue matchValue;
        FcValueBinding matchBinding;
        int matchFamilyIndex = 0;
        while(FcPatternGetWithBinding(matchPattern, FC_FAMILY, matchFamilyIndex, &matchValue, &matchBinding) == FcResultMatch) {
            auto matchFamilyName = (const char*)(matchValue.u.s);
            if(matchBinding == FcValueBindingStrong || equalsIgnoringCase(configFamilyName, matchFamilyName) || equalsIgnoringCase(familyName, matchFamilyName)) {
               matchResult = FcResultMatch;
               break;
            }

            ++matchFamilyIndex;
        }
    }

    FcPatternDestroy(pattern);
    if(matchResult == FcResultMatch)
        return createFontDataFromPattern(matchPattern, description);
    FcPatternDestroy(matchPattern);
    return nullptr;
}

RefPtr<SimpleFontData> FontDataCache::getFontData(const GlobalString& family, const FontDataDescription& description)
{
    std::lock_guard guard(m_mutex);
    auto& fontData = m_table[family][description];
    if(fontData == nullptr)
        fontData = createFontData(m_config, family, description);
    return fontData;
}

RefPtr<SimpleFontData> FontDataCache::getFontData(uint32_t codepoint, bool preferColor, const FontDataDescription& description)
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
    FcPatternAddBool(pattern, FC_SCALABLE, FcTrue);
    if(preferColor) {
        FcPatternAddBool(pattern, FC_COLOR, FcTrue);
    }

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
    return nullptr;
}

bool FontDataCache::isFamilyAvailable(const GlobalString& family)
{
    std::lock_guard guard(m_mutex);
    for(auto nameSet : { FcSetSystem, FcSetApplication }) {
        auto allFonts = FcConfigGetFonts(m_config, nameSet);
        if(allFonts == nullptr)
            continue;
        for(int fontIndex = 0; fontIndex < allFonts->nfont; ++fontIndex) {
            auto matchPattern = allFonts->fonts[fontIndex];
            int matchFamilyIndex = 0;
            char* matchFamilyName = nullptr;
            while(FcPatternGetString(matchPattern, FC_FAMILY, matchFamilyIndex, (FcChar8**)(&matchFamilyName)) == FcResultMatch) {
                if(equalsIgnoringCase(family, matchFamilyName))
                    return true;
                ++matchFamilyIndex;
            }
        }
    }

    return false;
}

FontDataCache::~FontDataCache()
{
    FcConfigDestroy(m_config);
}

FontDataCache::FontDataCache()
    : m_config(FcInitLoadConfigAndFonts())
{
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

const SimpleFontData* Font::getFontData(uint32_t codepoint, bool preferColor)
{
    for(const auto& font : m_fonts) {
        if(auto fontData = font->getFontData(codepoint, preferColor)) {
            return fontData;
        }
    }

    if(preferColor) {
        if(m_emojiFont == nullptr) {
            static const GlobalString emoji("emoji");
            if(auto fontData = fontDataCache()->getFontData(emoji, m_description.data)) {
                m_emojiFont = fontData.get();
                m_fonts.push_back(std::move(fontData));
            }
        }

        return m_emojiFont;
    }

    if(auto fontData = fontDataCache()->getFontData(codepoint, preferColor, m_description.data)) {
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
    for(const auto& family : description.families) {
        if(auto font = document->getFontData(family, description.data)) {
            if(m_primaryFont == nullptr)
                m_primaryFont = font->getFontData(' ', false);
            m_fonts.push_back(std::move(font));
        }
    }

    if(m_primaryFont == nullptr) {
        static const GlobalString serif("serif");
        if(auto fontData = fontDataCache()->getFontData(serif, description.data)) {
            m_primaryFont = fontData.get();
            m_fonts.push_back(std::move(fontData));
        }
    }
}

} // namespace plutobook
