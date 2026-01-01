/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

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

const GlobalString newLineGlo("\n");
const GlobalString listItemGlo("list-item");
const GlobalString pageGlo("page");
const GlobalString pagesGlo("pages");

const GlobalString xhtmlNs("http://www.w3.org/1999/xhtml");
const GlobalString mathmlNs("http://www.w3.org/1998/Math/MathML");
const GlobalString svgNs("http://www.w3.org/2000/svg");

const GlobalString aTag("a");
const GlobalString abbrTag("abbr");
const GlobalString addressTag("address");
const GlobalString annotation_xmlTag("annotation-xml");
const GlobalString appletTag("applet");
const GlobalString areaTag("area");
const GlobalString articleTag("article");
const GlobalString asideTag("aside");
const GlobalString bTag("b");
const GlobalString baseTag("base");
const GlobalString basefontTag("basefont");
const GlobalString bgsoundTag("bgsound");
const GlobalString bigTag("big");
const GlobalString blockquoteTag("blockquote");
const GlobalString bodyTag("body");
const GlobalString brTag("br");
const GlobalString buttonTag("button");
const GlobalString captionTag("caption");
const GlobalString centerTag("center");
const GlobalString circleTag("circle");
const GlobalString clipPathTag("clipPath");
const GlobalString codeTag("code");
const GlobalString colTag("col");
const GlobalString colgroupTag("colgroup");
const GlobalString commandTag("command");
const GlobalString ddTag("dd");
const GlobalString defsTag("defs");
const GlobalString descTag("desc");
const GlobalString detailsTag("details");
const GlobalString dirTag("dir");
const GlobalString divTag("div");
const GlobalString dlTag("dl");
const GlobalString dtTag("dt");
const GlobalString ellipseTag("ellipse");
const GlobalString emTag("em");
const GlobalString embedTag("embed");
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
const GlobalString keygenTag("keygen");
const GlobalString liTag("li");
const GlobalString lineTag("line");
const GlobalString linearGradientTag("linearGradient");
const GlobalString linkTag("link");
const GlobalString listingTag("listing");
const GlobalString mainTag("main");
const GlobalString malignmarkTag("malignmark");
const GlobalString markerTag("marker");
const GlobalString marqueeTag("marquee");
const GlobalString maskTag("mask");
const GlobalString mathTag("math");
const GlobalString menuTag("menu");
const GlobalString metaTag("meta");
const GlobalString metadataTag("metadata");
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
const GlobalString radialGradientTag("radialGradient");
const GlobalString rectTag("rect");
const GlobalString rpTag("rp");
const GlobalString rtTag("rt");
const GlobalString rubyTag("ruby");
const GlobalString sTag("s");
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
const GlobalString tspanTag("tspan");
const GlobalString ttTag("tt");
const GlobalString uTag("u");
const GlobalString ulTag("ul");
const GlobalString useTag("use");
const GlobalString varTag("var");
const GlobalString wbrTag("wbr");
const GlobalString xmpTag("xmp");

const GlobalString alignAttr("align");
const GlobalString altAttr("alt");
const GlobalString backgroundAttr("background");
const GlobalString bgcolorAttr("bgcolor");
const GlobalString borderAttr("border");
const GlobalString bordercolorAttr("bordercolor");
const GlobalString cellpaddingAttr("cellpadding");
const GlobalString cellspacingAttr("cellspacing");
const GlobalString checkedAttr("checked");
const GlobalString classAttr("class");
const GlobalString clipPathUnitsAttr("clipPathUnits");
const GlobalString colorAttr("color");
const GlobalString colsAttr("cols");
const GlobalString colspanAttr("colspan");
const GlobalString cxAttr("cx");
const GlobalString cyAttr("cy");
const GlobalString dAttr("d");
const GlobalString disabledAttr("disabled");
const GlobalString dxAttr("dx");
const GlobalString dyAttr("dy");
const GlobalString enabledAttr("enabled");
const GlobalString encodingAttr("encoding");
const GlobalString faceAttr("face");
const GlobalString frameAttr("frame");
const GlobalString fxAttr("fx");
const GlobalString fyAttr("fy");
const GlobalString gradientTransformAttr("gradientTransform");
const GlobalString gradientUnitsAttr("gradientUnits");
const GlobalString heightAttr("height");
const GlobalString hiddenAttr("hidden");
const GlobalString hrefAttr("href");
const GlobalString hspaceAttr("hspace");
const GlobalString idAttr("id");
const GlobalString langAttr("lang");
const GlobalString markerHeightAttr("markerHeight");
const GlobalString markerUnitsAttr("markerUnits");
const GlobalString markerWidthAttr("markerWidth");
const GlobalString maskContentUnitsAttr("maskContentUnits");
const GlobalString maskUnitsAttr("maskUnits");
const GlobalString mediaAttr("media");
const GlobalString multipleAttr("multiple");
const GlobalString noshadeAttr("noshade");
const GlobalString offsetAttr("offset");
const GlobalString orientAttr("orient");
const GlobalString patternContentUnitsAttr("patternContentUnits");
const GlobalString patternTransformAttr("patternTransform");
const GlobalString patternUnitsAttr("patternUnits");
const GlobalString pointsAttr("points");
const GlobalString preserveAspectRatioAttr("preserveAspectRatio");
const GlobalString rAttr("r");
const GlobalString refXAttr("refX");
const GlobalString refYAttr("refY");
const GlobalString relAttr("rel");
const GlobalString rotateAttr("rotate");
const GlobalString rowsAttr("rows");
const GlobalString rowspanAttr("rowspan");
const GlobalString rulesAttr("rules");
const GlobalString rxAttr("rx");
const GlobalString ryAttr("ry");
const GlobalString sizeAttr("size");
const GlobalString spanAttr("span");
const GlobalString spreadMethodAttr("spreadMethod");
const GlobalString srcAttr("src");
const GlobalString startAttr("start");
const GlobalString styleAttr("style");
const GlobalString textAttr("text");
const GlobalString transformAttr("transform");
const GlobalString typeAttr("type");
const GlobalString valignAttr("valign");
const GlobalString valueAttr("value");
const GlobalString viewBoxAttr("viewBox");
const GlobalString vspaceAttr("vspace");
const GlobalString widthAttr("width");
const GlobalString x1Attr("x1");
const GlobalString x2Attr("x2");
const GlobalString xAttr("x");
const GlobalString y1Attr("y1");
const GlobalString y2Attr("y2");
const GlobalString yAttr("y");

} // namespace plutobook
