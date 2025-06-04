#include "globalstring.h"
#include "stringutils.h"

#include <mutex>
#include <set>

namespace plutobook {

class GlobalStringTable {
public:
    GlobalStringTable();

    const HeapString* add(const std::string_view& value);

private:
    using StringSet = std::pmr::set<HeapString, std::less<>>;
    Heap m_heap;
    StringSet m_table;
    std::mutex m_mutex;
};

GlobalStringTable::GlobalStringTable()
    : m_heap(1024 * 24)
    , m_table(&m_heap)
{
}

const HeapString* GlobalStringTable::add(const std::string_view& value)
{
    std::lock_guard guard(m_mutex);
    auto lb = m_table.lower_bound(value);
    if(lb != m_table.end() && *lb == value)
        return &*lb;
    return &*m_table.emplace_hint(lb, m_heap.createString(value));
}

GlobalStringTable* globalStringTable()
{
    static GlobalStringTable table;
    return &table;
}

GlobalString::GlobalString(const std::string_view& value)
    : m_entry(globalStringTable()->add(value))
{
}

GlobalString GlobalString::foldCase() const
{
    if(m_entry == nullptr)
        return nullGlo;
    auto size = m_entry->size();
    auto data = m_entry->data();

    size_t index = 0;
    while(index < size && !isUpper(data[index]))
        ++index;
    if(index == size) {
        return *this;
    }

    constexpr auto kBufferSize = 128;
    if(size <= kBufferSize) {
        char buffer[kBufferSize];
        for(size_t i = 0; i < index; i++)
            buffer[i] = data[i];
        for(size_t i = index; i < size; i++) {
            buffer[i] = toLower(data[i]);
        }

        return GlobalString({buffer, size});
    }

    std::string value(data, size);
    for(size_t i = index; i < size; i++) {
        value[i] = toLower(data[i]);
    }

    return GlobalString(value);
}

const HeapString GlobalString::nullString;

const GlobalString nullGlo;
const GlobalString emptyGlo("");
const GlobalString starGlo("*");

const GlobalString xhtmlNs("http://www.w3.org/1999/xhtml");
const GlobalString mathmlNs("http://www.w3.org/1998/Math/MathML");
const GlobalString svgNs("http://www.w3.org/2000/svg");

const GlobalString aTag("a");
const GlobalString abbrTag("abbr");
const GlobalString acronymTag("acronym");
const GlobalString addressTag("address");
const GlobalString altGlyphDefTag("altGlyphDef");
const GlobalString altGlyphItemTag("altGlyphItem");
const GlobalString altGlyphTag("altGlyph");
const GlobalString animateColorTag("animateColor");
const GlobalString animateMotionTag("animateMotion");
const GlobalString animateTransformTag("animateTransform");
const GlobalString annotation_xmlTag("annotation-xml");
const GlobalString appletTag("applet");
const GlobalString areaTag("area");
const GlobalString articleTag("article");
const GlobalString asideTag("aside");
const GlobalString audioTag("audio");
const GlobalString bTag("b");
const GlobalString baseTag("base");
const GlobalString basefontTag("basefont");
const GlobalString bgsoundTag("bgsound");
const GlobalString bigTag("big");
const GlobalString blockquoteTag("blockquote");
const GlobalString bodyTag("body");
const GlobalString brTag("br");
const GlobalString buttonTag("button");
const GlobalString canvasTag("canvas");
const GlobalString captionTag("caption");
const GlobalString centerTag("center");
const GlobalString circleTag("circle");
const GlobalString citeTag("cite");
const GlobalString clipPathTag("clipPath");
const GlobalString codeTag("code");
const GlobalString colTag("col");
const GlobalString colgroupTag("colgroup");
const GlobalString commandTag("command");
const GlobalString datagridTag("datagrid");
const GlobalString datalistTag("datalist");
const GlobalString dcellTag("dcell");
const GlobalString dcolTag("dcol");
const GlobalString ddTag("dd");
const GlobalString defsTag("defs");
const GlobalString delTag("del");
const GlobalString descTag("desc");
const GlobalString detailsTag("details");
const GlobalString dfnTag("dfn");
const GlobalString dirTag("dir");
const GlobalString divTag("div");
const GlobalString dlTag("dl");
const GlobalString drowTag("drow");
const GlobalString dtTag("dt");
const GlobalString ellipseTag("ellipse");
const GlobalString emTag("em");
const GlobalString embedTag("embed");
const GlobalString feBlendTag("feBlend");
const GlobalString feColorMatrixTag("feColorMatrix");
const GlobalString feComponentTransferTag("feComponentTransfer");
const GlobalString feCompositeTag("feComposite");
const GlobalString feConvolveMatrixTag("feConvolveMatrix");
const GlobalString feDiffuseLightingTag("feDiffuseLighting");
const GlobalString feDisplacementMapTag("feDisplacementMap");
const GlobalString feDistantLightTag("feDistantLight");
const GlobalString feDropShadowTag("feDropShadow");
const GlobalString feFloodTag("feFlood");
const GlobalString feFuncATag("feFuncA");
const GlobalString feFuncBTag("feFuncB");
const GlobalString feFuncGTag("feFuncG");
const GlobalString feFuncRTag("feFuncR");
const GlobalString feGaussianBlurTag("feGaussianBlur");
const GlobalString feImageTag("feImage");
const GlobalString feMergeNodeTag("");
const GlobalString feMergeTag("feMergeNode");
const GlobalString feMorphologyTag("feMorphology");
const GlobalString feOffsetTag("feOffset");
const GlobalString fePointLightTag("fePointLight");
const GlobalString feSpecularLightingTag("feSpecularLighting");
const GlobalString feSpotLightTag("feSpotLight");
const GlobalString fieldsetTag("fieldset");
const GlobalString figcaptionTag("figcaption");
const GlobalString figureTag("figure");
const GlobalString fontTag("font");
const GlobalString footerTag("footer");
const GlobalString foreignObjectTag("foreignObject");
const GlobalString formTag("form");
const GlobalString frameTag("frame");
const GlobalString framesetTag("frameset");
const GlobalString gTag("g");
const GlobalString glyphRefTag("glyphRef");
const GlobalString h1Tag("h1");
const GlobalString h2Tag("h2");
const GlobalString h3Tag("h3");
const GlobalString h4Tag("h4");
const GlobalString h5Tag("h5");
const GlobalString h6Tag("h6");
const GlobalString headTag("head");
const GlobalString headerTag("header");
const GlobalString hgroupTag("hgroup");
const GlobalString hrTag("hr");
const GlobalString htmlTag("html");
const GlobalString iTag("i");
const GlobalString iframeTag("iframe");
const GlobalString imageTag("image");
const GlobalString imgTag("img");
const GlobalString inputTag("input");
const GlobalString insTag("ins");
const GlobalString isindexTag("isindex");
const GlobalString kbdTag("kbd");
const GlobalString keygenTag("keygen");
const GlobalString labelTag("lebel");
const GlobalString layerTag("layer");
const GlobalString legendTag("legend");
const GlobalString liTag("li");
const GlobalString lineTag("line");
const GlobalString linearGradientTag("linearGradient");
const GlobalString linkTag("link");
const GlobalString listingTag("listing");
const GlobalString mainTag("main");
const GlobalString malignmarkTag("malignmark");
const GlobalString mapTag("map");
const GlobalString markTag("mark");
const GlobalString markerTag("marker");
const GlobalString marqueeTag("marquee");
const GlobalString maskTag("mask");
const GlobalString mathTag("math");
const GlobalString menuTag("menu");
const GlobalString metaTag("meta");
const GlobalString metadataTag("metadata");
const GlobalString meterTag("meter");
const GlobalString mglyphTag("mglyph");
const GlobalString miTag("mi");
const GlobalString mnTag("mn");
const GlobalString moTag("mo");
const GlobalString msTag("ms");
const GlobalString mtextTag("mtext");
const GlobalString navTag("nav");
const GlobalString nobrTag("nobr");
const GlobalString noembedTag("noembed");
const GlobalString noframesTag("noframes");
const GlobalString nolayerTag("nolayer");
const GlobalString noscriptTag("noscript");
const GlobalString objectTag("object");
const GlobalString olTag("ol");
const GlobalString optgroupTag("optgroup");
const GlobalString optionTag("option");
const GlobalString pTag("p");
const GlobalString paramTag("param");
const GlobalString pathTag("path");
const GlobalString patternTag("pattern");
const GlobalString plaintextTag("plaintext");
const GlobalString polygonTag("polygon");
const GlobalString polylineTag("polyline");
const GlobalString preTag("pre");
const GlobalString progressTag("progress");
const GlobalString qTag("q");
const GlobalString radialGradientTag("radialGradient");
const GlobalString rectTag("rect");
const GlobalString rpTag("rp");
const GlobalString rtTag("rt");
const GlobalString rubyTag("ruby");
const GlobalString sTag("s");
const GlobalString sampTag("samp");
const GlobalString scriptTag("script");
const GlobalString sectionTag("section");
const GlobalString selectTag("select");
const GlobalString smallTag("small");
const GlobalString sourceTag("source");
const GlobalString spanTag("span");
const GlobalString stopTag("stop");
const GlobalString strikeTag("strike");
const GlobalString strongTag("strong");
const GlobalString styleTag("style");
const GlobalString subTag("sub");
const GlobalString summaryTag("summary");
const GlobalString supTag("sup");
const GlobalString svgTag("svg");
const GlobalString switchTag("switch");
const GlobalString symbolTag("symbol");
const GlobalString tableTag("table");
const GlobalString tbodyTag("tbody");
const GlobalString tdTag("td");
const GlobalString textPathTag("textPath");
const GlobalString textTag("text");
const GlobalString textareaTag("textarea");
const GlobalString tfootTag("tfoot");
const GlobalString thTag("th");
const GlobalString theadTag("thead");
const GlobalString titleTag("title");
const GlobalString trTag("tr");
const GlobalString trackTag("track");
const GlobalString trefTag("tref");
const GlobalString tspanTag("tspan");
const GlobalString ttTag("tt");
const GlobalString uTag("u");
const GlobalString ulTag("ul");
const GlobalString useTag("use");
const GlobalString varTag("var");
const GlobalString videoTag("video");
const GlobalString wbrTag("wbr");
const GlobalString xmpTag("xmp");

const GlobalString alignAttr("align");
const GlobalString alignment_baselineAttr("alignment-baseline");
const GlobalString altAttr("alt");
const GlobalString attributeNameAttr("attributeName");
const GlobalString attributeTypeAttr("attributeType");
const GlobalString backgroundAttr("background");
const GlobalString baseFrequencyAttr("baseFrequency");
const GlobalString baseProfileAttr("baseProfile");
const GlobalString baseline_shiftAttr("baseline-shift");
const GlobalString bgcolorAttr("bgcolor");
const GlobalString borderAttr("border");
const GlobalString bordercolorAttr("bordercolor");
const GlobalString calcModeAttr("calcMode");
const GlobalString cellpaddingAttr("cellpadding");
const GlobalString cellspacingAttr("cellspacing");
const GlobalString checkedAttr("checked");
const GlobalString classAttr("class");
const GlobalString clipAttr("clip");
const GlobalString clipPathUnitsAttr("clipPathUnits");
const GlobalString clip_pathAttr("clip-path");
const GlobalString clip_ruleAttr("clip-rule");
const GlobalString colorAttr("color");
const GlobalString colsAttr("cols");
const GlobalString colspanAttr("colspan");
const GlobalString contentAttr("content");
const GlobalString cxAttr("cx");
const GlobalString cyAttr("cy");
const GlobalString dAttr("d");
const GlobalString definitionUrlAttr("definitionUrl");
const GlobalString diffuseConstantAttr("diffuseConstant");
const GlobalString directionAttr("direction");
const GlobalString disabledAttr("disabled");
const GlobalString displayAttr("display");
const GlobalString dominant_baselineAttr("dominant-baseline");
const GlobalString dxAttr("dx");
const GlobalString dyAttr("dy");
const GlobalString edgeModeAttr("edgeMode");
const GlobalString enabledAttr("enabled");
const GlobalString encodingAttr("encoding");
const GlobalString faceAttr("face");
const GlobalString fillAttr("fill");
const GlobalString fill_opacityAttr("fill-opacity");
const GlobalString fill_ruleAttr("fill-rule");
const GlobalString filterUnitsAttr("filterUnits");
const GlobalString font_familyAttr("font-family");
const GlobalString font_sizeAttr("font-size");
const GlobalString font_size_adjustAttr("font-size-adjust");
const GlobalString font_stretchAttr("font-stretch");
const GlobalString font_styleAttr("font-style");
const GlobalString font_variantAttr("font-variant");
const GlobalString font_weightAttr("font-weight");
const GlobalString fxAttr("fx");
const GlobalString fyAttr("fy");
const GlobalString glyphRefAttr("glyphRef");
const GlobalString gradientTransformAttr("gradientTransform");
const GlobalString gradientUnitsAttr("gradientUnits");
const GlobalString heightAttr("height");
const GlobalString hrefAttr("href");
const GlobalString hspaceAttr("hspace");
const GlobalString idAttr("id");
const GlobalString kernelMatrixAttr("kernelMatrix");
const GlobalString kernelUnitLengthAttr("kernelUnitLength");
const GlobalString keyPointsAttr("keyPoints");
const GlobalString keySplinesAttr("keySplines");
const GlobalString keyTimesAttr("keyTimes");
const GlobalString langAttr("lang");
const GlobalString lengthAdjustAttr("lengthAdjust");
const GlobalString letter_spacingAttr("letter-spacing");
const GlobalString limitingConeAngleAttr("limitingConeAngle");
const GlobalString markerHeightAttr("markerHeight");
const GlobalString markerUnitsAttr("markerUnits");
const GlobalString markerWidthAttr("markerWidth");
const GlobalString marker_endAttr("marker-end");
const GlobalString marker_midAttr("marker-mid");
const GlobalString marker_startAttr("marker-start");
const GlobalString maskAttr("mask");
const GlobalString maskContentUnitsAttr("maskContentUnits");
const GlobalString maskUnitsAttr("maskUnits");
const GlobalString mask_typeAttr("mask-type");
const GlobalString mediaAttr("media");
const GlobalString multipleAttr("multiple");
const GlobalString nameAttr("name");
const GlobalString numOctavesAttr("numOctaves");
const GlobalString offsetAttr("offset");
const GlobalString opacityAttr("opacity");
const GlobalString orientAttr("orient");
const GlobalString overflowAttr("overflow");
const GlobalString overline_positionAttr("overline-position");
const GlobalString overline_thicknessAttr("overline-thickness");
const GlobalString paint_orderAttr("paint-order");
const GlobalString pathAttr("path");
const GlobalString pathLengthAttr("pathLength");
const GlobalString patternContentUnitsAttr("patternContentUnits");
const GlobalString patternTransformAttr("patternTransform");
const GlobalString patternUnitsAttr("patternUnits");
const GlobalString pointsAtXAttr("pointsAtX");
const GlobalString pointsAtYAttr("pointsAtY");
const GlobalString pointsAtZAttr("pointsAtZ");
const GlobalString pointsAttr("points");
const GlobalString preserveAlphaAttr("preserveAlpha");
const GlobalString preserveAspectRatioAttr("preserveAspectRatio");
const GlobalString primitiveUnitsAttr("primitiveUnits");
const GlobalString rAttr("r");
const GlobalString refXAttr("refX");
const GlobalString refYAttr("refY");
const GlobalString relAttr("rel");
const GlobalString repeatCountAttr("repeatCount");
const GlobalString repeatDurAttr("repeatDur");
const GlobalString requiredExtensionsAttr("requiredExtensions");
const GlobalString requiredFeaturesAttr("requiredFeatures");
const GlobalString rotateAttr("rotate");
const GlobalString rowsAttr("rows");
const GlobalString rowspanAttr("rowspan");
const GlobalString rxAttr("rx");
const GlobalString ryAttr("ry");
const GlobalString sizeAttr("size");
const GlobalString spacingAttr("spacing");
const GlobalString spanAttr("span");
const GlobalString specularConstantAttr("specularConstant");
const GlobalString specularExponentAttr("specularExponent");
const GlobalString spreadMethodAttr("spreadMethod");
const GlobalString srcAttr("src");
const GlobalString startAttr("start");
const GlobalString startOffsetAttr("startOffset");
const GlobalString stdDeviationAttr("stdDeviation");
const GlobalString stitchTilesAttr("stitchTiles");
const GlobalString stop_colorAttr("stop-color");
const GlobalString stop_opacityAttr("stop-opacity");
const GlobalString strikethrough_positionAttr("strikethrough-position");
const GlobalString strikethrough_thicknessAttr("strikethrough-thickness");
const GlobalString strokeAttr("stroke");
const GlobalString stroke_dasharrayAttr("stroke-dasharray");
const GlobalString stroke_dashoffsetAttr("stroke-dashoffset");
const GlobalString stroke_linecapAttr("stroke-linecap");
const GlobalString stroke_linejoinAttr("stroke-linejoin");
const GlobalString stroke_miterlimitAttr("stroke-miterlimit");
const GlobalString stroke_opacityAttr("stroke-opacity");
const GlobalString stroke_widthAttr("stroke-width");
const GlobalString styleAttr("style");
const GlobalString surfaceScaleAttr("surfaceScale");
const GlobalString systemLanguageAttr("systemLanguage");
const GlobalString tableValuesAttr("tableValues");
const GlobalString targetXAttr("targetX");
const GlobalString targetYAttr("targetY");
const GlobalString textAttr("text");
const GlobalString textLengthAttr("textLength");
const GlobalString text_anchorAttr("text-anchor");
const GlobalString text_decorationAttr("text-decoration");
const GlobalString titleAttr("title");
const GlobalString transformAttr("transform");
const GlobalString transform_originAttr("transform-origin");
const GlobalString typeAttr("type");
const GlobalString underline_positionAttr("underline-position");
const GlobalString underline_thicknessAttr("underline-thickness");
const GlobalString unicodeAttr("unicode");
const GlobalString unicode_bidiAttr("unicode-bidi");
const GlobalString unicode_rangeAttr("unicode-range");
const GlobalString valignAttr("valign");
const GlobalString valueAttr("value");
const GlobalString vector_effectAttr("vector-effect");
const GlobalString viewBoxAttr("viewBox");
const GlobalString viewTargetAttr("viewTarget");
const GlobalString visibilityAttr("visibility");
const GlobalString vspaceAttr("vspace");
const GlobalString widthAttr("width");
const GlobalString word_spacingAttr("word-spacing");
const GlobalString writing_modeAttr("writing-mode");
const GlobalString x1Attr("x1");
const GlobalString x2Attr("x2");
const GlobalString xAttr("x");
const GlobalString xChannelSelectorAttr("xChannelSelector");
const GlobalString y1Attr("y1");
const GlobalString y2Attr("y2");
const GlobalString yAttr("y");
const GlobalString yChannelSelectorAttr("yChannelSelector");
const GlobalString zoomAndPanAttr("zoomAndPan");

} // namespace plutobook
