/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_GLOBALSTRING_H
#define PLUTOBOOK_GLOBALSTRING_H

#include "heapstring.h"

namespace plutobook {

class GlobalString {
public:
    GlobalString() = default;
    explicit GlobalString(std::string_view value);

    const HeapString& value() const;

    const char* data() const { return value().data(); }
    size_t size() const { return value().size(); }

    const char* begin() const { return value().begin(); }
    const char* end() const { return value().end(); }

    const char& at(size_t index) const { return value().at(index); }
    const char& operator[](size_t index) const { return value().operator[](index); }

    const char& front() const { return value().front(); }
    const char& back() const { return value().back(); }

    bool isEmpty() const { return value().empty(); }
    bool isNull() const { return m_entry == nullptr; }

    bool operator==(const GlobalString& o) const { return m_entry == o.m_entry; }

    operator std::string_view() const { return value(); }
    operator const HeapString&() const { return value(); }

    GlobalString foldCase() const;

private:
    static const HeapString nullString;
    const HeapString* m_entry{nullptr};
};

inline const HeapString GlobalString::nullString;

inline const HeapString& GlobalString::value() const
{
    return m_entry ? *m_entry : nullString;
}

inline GlobalString operator""_glo(const char* data, size_t length)
{
    return GlobalString({data, length});
}

inline std::ostream& operator<<(std::ostream& o, const GlobalString& in) { return o << in.value(); }

inline bool operator==(const GlobalString& a, std::string_view b) { return a.value() == b; }
inline bool operator==(std::string_view a, const GlobalString& b) { return a == b.value(); }
inline bool operator==(const GlobalString& a, const HeapString& b) { return a.value() == b; }
inline bool operator==(const HeapString& a, const GlobalString& b) { return a == b.value(); }

inline bool operator<(const GlobalString& a, const GlobalString& b) { return a.value() < b.value(); }
inline bool operator<(const GlobalString& a, std::string_view b) { return a.value() < b; }
inline bool operator<(std::string_view a, const GlobalString& b) { return a < b.value(); }
inline bool operator<(const GlobalString& a, const HeapString& b) { return a.value() < b; }
inline bool operator<(const HeapString& a, const GlobalString& b) { return a < b.value(); }

inline const GlobalString nullGlo;
inline const GlobalString emptyGlo("");
inline const GlobalString starGlo("*");

inline const GlobalString newLineGlo("\n");
inline const GlobalString listItemGlo("list-item");
inline const GlobalString pageGlo("page");
inline const GlobalString pagesGlo("pages");

inline const GlobalString xhtmlNs("http://www.w3.org/1999/xhtml");
inline const GlobalString mathmlNs("http://www.w3.org/1998/Math/MathML");
inline const GlobalString svgNs("http://www.w3.org/2000/svg");

inline const GlobalString aTag("a");
inline const GlobalString abbrTag("abbr");
inline const GlobalString addressTag("address");
inline const GlobalString annotation_xmlTag("annotation-xml");
inline const GlobalString appletTag("applet");
inline const GlobalString areaTag("area");
inline const GlobalString articleTag("article");
inline const GlobalString asideTag("aside");
inline const GlobalString bTag("b");
inline const GlobalString baseTag("base");
inline const GlobalString basefontTag("basefont");
inline const GlobalString bgsoundTag("bgsound");
inline const GlobalString bigTag("big");
inline const GlobalString blockquoteTag("blockquote");
inline const GlobalString bodyTag("body");
inline const GlobalString brTag("br");
inline const GlobalString buttonTag("button");
inline const GlobalString captionTag("caption");
inline const GlobalString centerTag("center");
inline const GlobalString circleTag("circle");
inline const GlobalString clipPathTag("clipPath");
inline const GlobalString codeTag("code");
inline const GlobalString colTag("col");
inline const GlobalString colgroupTag("colgroup");
inline const GlobalString commandTag("command");
inline const GlobalString ddTag("dd");
inline const GlobalString defsTag("defs");
inline const GlobalString descTag("desc");
inline const GlobalString detailsTag("details");
inline const GlobalString dirTag("dir");
inline const GlobalString divTag("div");
inline const GlobalString dlTag("dl");
inline const GlobalString dtTag("dt");
inline const GlobalString ellipseTag("ellipse");
inline const GlobalString emTag("em");
inline const GlobalString embedTag("embed");
inline const GlobalString fieldsetTag("fieldset");
inline const GlobalString figcaptionTag("figcaption");
inline const GlobalString figureTag("figure");
inline const GlobalString fontTag("font");
inline const GlobalString footerTag("footer");
inline const GlobalString foreignObjectTag("foreignObject");
inline const GlobalString formTag("form");
inline const GlobalString frameTag("frame");
inline const GlobalString framesetTag("frameset");
inline const GlobalString gTag("g");
inline const GlobalString h1Tag("h1");
inline const GlobalString h2Tag("h2");
inline const GlobalString h3Tag("h3");
inline const GlobalString h4Tag("h4");
inline const GlobalString h5Tag("h5");
inline const GlobalString h6Tag("h6");
inline const GlobalString headTag("head");
inline const GlobalString headerTag("header");
inline const GlobalString hgroupTag("hgroup");
inline const GlobalString hrTag("hr");
inline const GlobalString htmlTag("html");
inline const GlobalString iTag("i");
inline const GlobalString iframeTag("iframe");
inline const GlobalString imageTag("image");
inline const GlobalString imgTag("img");
inline const GlobalString inputTag("input");
inline const GlobalString keygenTag("keygen");
inline const GlobalString liTag("li");
inline const GlobalString lineTag("line");
inline const GlobalString linearGradientTag("linearGradient");
inline const GlobalString linkTag("link");
inline const GlobalString listingTag("listing");
inline const GlobalString mainTag("main");
inline const GlobalString malignmarkTag("malignmark");
inline const GlobalString markerTag("marker");
inline const GlobalString marqueeTag("marquee");
inline const GlobalString maskTag("mask");
inline const GlobalString mathTag("math");
inline const GlobalString menuTag("menu");
inline const GlobalString metaTag("meta");
inline const GlobalString metadataTag("metadata");
inline const GlobalString mglyphTag("mglyph");
inline const GlobalString miTag("mi");
inline const GlobalString mnTag("mn");
inline const GlobalString moTag("mo");
inline const GlobalString msTag("ms");
inline const GlobalString mtextTag("mtext");
inline const GlobalString navTag("nav");
inline const GlobalString nobrTag("nobr");
inline const GlobalString noembedTag("noembed");
inline const GlobalString noframesTag("noframes");
inline const GlobalString noscriptTag("noscript");
inline const GlobalString objectTag("object");
inline const GlobalString olTag("ol");
inline const GlobalString optgroupTag("optgroup");
inline const GlobalString optionTag("option");
inline const GlobalString pTag("p");
inline const GlobalString paramTag("param");
inline const GlobalString pathTag("path");
inline const GlobalString patternTag("pattern");
inline const GlobalString plaintextTag("plaintext");
inline const GlobalString polygonTag("polygon");
inline const GlobalString polylineTag("polyline");
inline const GlobalString preTag("pre");
inline const GlobalString radialGradientTag("radialGradient");
inline const GlobalString rectTag("rect");
inline const GlobalString rpTag("rp");
inline const GlobalString rtTag("rt");
inline const GlobalString rubyTag("ruby");
inline const GlobalString sTag("s");
inline const GlobalString scriptTag("script");
inline const GlobalString sectionTag("section");
inline const GlobalString selectTag("select");
inline const GlobalString smallTag("small");
inline const GlobalString sourceTag("source");
inline const GlobalString spanTag("span");
inline const GlobalString stopTag("stop");
inline const GlobalString strikeTag("strike");
inline const GlobalString strongTag("strong");
inline const GlobalString styleTag("style");
inline const GlobalString subTag("sub");
inline const GlobalString summaryTag("summary");
inline const GlobalString supTag("sup");
inline const GlobalString svgTag("svg");
inline const GlobalString switchTag("switch");
inline const GlobalString symbolTag("symbol");
inline const GlobalString tableTag("table");
inline const GlobalString tbodyTag("tbody");
inline const GlobalString tdTag("td");
inline const GlobalString textPathTag("textPath");
inline const GlobalString textTag("text");
inline const GlobalString textareaTag("textarea");
inline const GlobalString tfootTag("tfoot");
inline const GlobalString thTag("th");
inline const GlobalString theadTag("thead");
inline const GlobalString titleTag("title");
inline const GlobalString trTag("tr");
inline const GlobalString trackTag("track");
inline const GlobalString tspanTag("tspan");
inline const GlobalString ttTag("tt");
inline const GlobalString uTag("u");
inline const GlobalString ulTag("ul");
inline const GlobalString useTag("use");
inline const GlobalString varTag("var");
inline const GlobalString wbrTag("wbr");
inline const GlobalString xmpTag("xmp");

inline const GlobalString alignAttr("align");
inline const GlobalString altAttr("alt");
inline const GlobalString backgroundAttr("background");
inline const GlobalString bgcolorAttr("bgcolor");
inline const GlobalString borderAttr("border");
inline const GlobalString bordercolorAttr("bordercolor");
inline const GlobalString cellpaddingAttr("cellpadding");
inline const GlobalString cellspacingAttr("cellspacing");
inline const GlobalString checkedAttr("checked");
inline const GlobalString classAttr("class");
inline const GlobalString clipPathUnitsAttr("clipPathUnits");
inline const GlobalString colorAttr("color");
inline const GlobalString colsAttr("cols");
inline const GlobalString colspanAttr("colspan");
inline const GlobalString contentAttr("content");
inline const GlobalString cxAttr("cx");
inline const GlobalString cyAttr("cy");
inline const GlobalString dAttr("d");
inline const GlobalString disabledAttr("disabled");
inline const GlobalString dxAttr("dx");
inline const GlobalString dyAttr("dy");
inline const GlobalString enabledAttr("enabled");
inline const GlobalString encodingAttr("encoding");
inline const GlobalString faceAttr("face");
inline const GlobalString frameAttr("frame");
inline const GlobalString fxAttr("fx");
inline const GlobalString fyAttr("fy");
inline const GlobalString gradientTransformAttr("gradientTransform");
inline const GlobalString gradientUnitsAttr("gradientUnits");
inline const GlobalString heightAttr("height");
inline const GlobalString hiddenAttr("hidden");
inline const GlobalString hrefAttr("href");
inline const GlobalString hspaceAttr("hspace");
inline const GlobalString idAttr("id");
inline const GlobalString langAttr("lang");
inline const GlobalString lengthAdjustAttr("lengthAdjust");
inline const GlobalString markerHeightAttr("markerHeight");
inline const GlobalString markerUnitsAttr("markerUnits");
inline const GlobalString markerWidthAttr("markerWidth");
inline const GlobalString maskContentUnitsAttr("maskContentUnits");
inline const GlobalString maskUnitsAttr("maskUnits");
inline const GlobalString mediaAttr("media");
inline const GlobalString multipleAttr("multiple");
inline const GlobalString nameAttr("name");
inline const GlobalString noshadeAttr("noshade");
inline const GlobalString nowrapAttr("nowrap");
inline const GlobalString offsetAttr("offset");
inline const GlobalString orientAttr("orient");
inline const GlobalString patternContentUnitsAttr("patternContentUnits");
inline const GlobalString patternTransformAttr("patternTransform");
inline const GlobalString patternUnitsAttr("patternUnits");
inline const GlobalString pointsAttr("points");
inline const GlobalString preserveAspectRatioAttr("preserveAspectRatio");
inline const GlobalString rAttr("r");
inline const GlobalString refXAttr("refX");
inline const GlobalString refYAttr("refY");
inline const GlobalString relAttr("rel");
inline const GlobalString rotateAttr("rotate");
inline const GlobalString rowsAttr("rows");
inline const GlobalString rowspanAttr("rowspan");
inline const GlobalString rulesAttr("rules");
inline const GlobalString rxAttr("rx");
inline const GlobalString ryAttr("ry");
inline const GlobalString sizeAttr("size");
inline const GlobalString spanAttr("span");
inline const GlobalString spreadMethodAttr("spreadMethod");
inline const GlobalString srcAttr("src");
inline const GlobalString startAttr("start");
inline const GlobalString startOffsetAttr("startOffset");
inline const GlobalString styleAttr("style");
inline const GlobalString textAttr("text");
inline const GlobalString textLengthAttr("textLength");
inline const GlobalString transformAttr("transform");
inline const GlobalString typeAttr("type");
inline const GlobalString valignAttr("valign");
inline const GlobalString valueAttr("value");
inline const GlobalString viewBoxAttr("viewBox");
inline const GlobalString vspaceAttr("vspace");
inline const GlobalString widthAttr("width");
inline const GlobalString x1Attr("x1");
inline const GlobalString x2Attr("x2");
inline const GlobalString xAttr("x");
inline const GlobalString y1Attr("y1");
inline const GlobalString y2Attr("y2");
inline const GlobalString yAttr("y");

} // namespace plutobook

#endif // PLUTOBOOK_GLOBALSTRING_H
