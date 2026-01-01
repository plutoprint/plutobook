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
    explicit GlobalString(const std::string_view& value);

    const char* data() const { return value().data(); }
    size_t size() const { return value().size(); }

    const char* begin() const { return value().begin(); }
    const char* end() const { return value().end(); }

    const char& at(size_t index) const { return value().at(index); }
    const char& operator[](size_t index) const { return value().operator[](index); }

    const char& front() const { return value().front(); }
    const char& back() const { return value().back(); }

    bool isEmpty() const { return value().empty(); }
    bool isNull() const { return !m_entry; }

    GlobalString foldCase() const;

    operator const std::string_view&() const { return value(); }
    operator const HeapString&() const { return value(); }

    bool operator==(const GlobalString& o) const { return m_entry == o.m_entry; }
    bool operator!=(const GlobalString& o) const { return m_entry != o.m_entry; }

    const HeapString& value() const;

private:
    static const HeapString nullString;
    const HeapString* m_entry{nullptr};
};

inline const HeapString& GlobalString::value() const
{
    return m_entry ? *m_entry : nullString;
}

inline GlobalString operator""_glo(const char* data, size_t length)
{
    return GlobalString({data, length});
}

inline std::ostream& operator<<(std::ostream& o, const GlobalString& in) { return o << in.value(); }

inline bool operator<(const GlobalString& a, const GlobalString& b) { return a.value() < b.value(); }
inline bool operator>(const GlobalString& a, const GlobalString& b) { return a.value() > b.value(); }

inline bool operator==(const GlobalString& a, const std::string_view& b) { return a.value() == b; }
inline bool operator!=(const GlobalString& a, const std::string_view& b) { return a.value() != b; }

inline bool operator==(const std::string_view& a, const GlobalString& b) { return a == b.value(); }
inline bool operator!=(const std::string_view& a, const GlobalString& b) { return a != b.value(); }

inline bool operator<(const GlobalString& a, const std::string_view& b) { return a.value() < b; }
inline bool operator>(const GlobalString& a, const std::string_view& b) { return a.value() > b; }

inline bool operator<(const std::string_view& a, const GlobalString& b) { return a < b.value(); }
inline bool operator>(const std::string_view& a, const GlobalString& b) { return a > b.value(); }

inline bool operator==(const GlobalString& a, const HeapString& b) { return a.value() == b; }
inline bool operator!=(const GlobalString& a, const HeapString& b) { return a.value() != b; }

inline bool operator==(const HeapString& a, const GlobalString& b) { return a == b.value(); }
inline bool operator!=(const HeapString& a, const GlobalString& b) { return a != b.value(); }

inline bool operator<(const GlobalString& a, const HeapString& b) { return a.value() < b; }
inline bool operator>(const GlobalString& a, const HeapString& b) { return a.value() > b; }

inline bool operator<(const HeapString& a, const GlobalString& b) { return a < b.value(); }
inline bool operator>(const HeapString& a, const GlobalString& b) { return a > b.value(); }

extern const GlobalString nullGlo;
extern const GlobalString emptyGlo;
extern const GlobalString starGlo;

extern const GlobalString newLineGlo;
extern const GlobalString listItemGlo;
extern const GlobalString pageGlo;
extern const GlobalString pagesGlo;

extern const GlobalString xhtmlNs;
extern const GlobalString mathmlNs;
extern const GlobalString svgNs;

extern const GlobalString aTag;
extern const GlobalString addressTag;
extern const GlobalString annotation_xmlTag;
extern const GlobalString appletTag;
extern const GlobalString areaTag;
extern const GlobalString articleTag;
extern const GlobalString asideTag;
extern const GlobalString bTag;
extern const GlobalString baseTag;
extern const GlobalString basefontTag;
extern const GlobalString bgsoundTag;
extern const GlobalString bigTag;
extern const GlobalString blockquoteTag;
extern const GlobalString bodyTag;
extern const GlobalString brTag;
extern const GlobalString buttonTag;
extern const GlobalString captionTag;
extern const GlobalString centerTag;
extern const GlobalString circleTag;
extern const GlobalString clipPathTag;
extern const GlobalString codeTag;
extern const GlobalString colTag;
extern const GlobalString colgroupTag;
extern const GlobalString commandTag;
extern const GlobalString ddTag;
extern const GlobalString defsTag;
extern const GlobalString descTag;
extern const GlobalString detailsTag;
extern const GlobalString dirTag;
extern const GlobalString divTag;
extern const GlobalString dlTag;
extern const GlobalString dtTag;
extern const GlobalString ellipseTag;
extern const GlobalString emTag;
extern const GlobalString embedTag;
extern const GlobalString fieldsetTag;
extern const GlobalString figcaptionTag;
extern const GlobalString figureTag;
extern const GlobalString fontTag;
extern const GlobalString footerTag;
extern const GlobalString foreignObjectTag;
extern const GlobalString formTag;
extern const GlobalString frameTag;
extern const GlobalString framesetTag;
extern const GlobalString gTag;
extern const GlobalString h1Tag;
extern const GlobalString h2Tag;
extern const GlobalString h3Tag;
extern const GlobalString h4Tag;
extern const GlobalString h5Tag;
extern const GlobalString h6Tag;
extern const GlobalString headTag;
extern const GlobalString headerTag;
extern const GlobalString hgroupTag;
extern const GlobalString hrTag;
extern const GlobalString htmlTag;
extern const GlobalString iTag;
extern const GlobalString iframeTag;
extern const GlobalString imageTag;
extern const GlobalString imgTag;
extern const GlobalString inputTag;
extern const GlobalString keygenTag;
extern const GlobalString liTag;
extern const GlobalString lineTag;
extern const GlobalString linearGradientTag;
extern const GlobalString linkTag;
extern const GlobalString listingTag;
extern const GlobalString mainTag;
extern const GlobalString malignmarkTag;
extern const GlobalString markerTag;
extern const GlobalString marqueeTag;
extern const GlobalString maskTag;
extern const GlobalString mathTag;
extern const GlobalString menuTag;
extern const GlobalString metaTag;
extern const GlobalString metadataTag;
extern const GlobalString mglyphTag;
extern const GlobalString miTag;
extern const GlobalString mnTag;
extern const GlobalString moTag;
extern const GlobalString msTag;
extern const GlobalString mtextTag;
extern const GlobalString navTag;
extern const GlobalString nobrTag;
extern const GlobalString noembedTag;
extern const GlobalString noframesTag;
extern const GlobalString noscriptTag;
extern const GlobalString objectTag;
extern const GlobalString olTag;
extern const GlobalString optgroupTag;
extern const GlobalString optionTag;
extern const GlobalString pTag;
extern const GlobalString paramTag;
extern const GlobalString pathTag;
extern const GlobalString patternTag;
extern const GlobalString plaintextTag;
extern const GlobalString polygonTag;
extern const GlobalString polylineTag;
extern const GlobalString preTag;
extern const GlobalString radialGradientTag;
extern const GlobalString rectTag;
extern const GlobalString rpTag;
extern const GlobalString rtTag;
extern const GlobalString rubyTag;
extern const GlobalString sTag;
extern const GlobalString scriptTag;
extern const GlobalString sectionTag;
extern const GlobalString selectTag;
extern const GlobalString smallTag;
extern const GlobalString sourceTag;
extern const GlobalString spanTag;
extern const GlobalString stopTag;
extern const GlobalString strikeTag;
extern const GlobalString strongTag;
extern const GlobalString styleTag;
extern const GlobalString subTag;
extern const GlobalString summaryTag;
extern const GlobalString supTag;
extern const GlobalString svgTag;
extern const GlobalString switchTag;
extern const GlobalString symbolTag;
extern const GlobalString tableTag;
extern const GlobalString tbodyTag;
extern const GlobalString tdTag;
extern const GlobalString textPathTag;
extern const GlobalString textTag;
extern const GlobalString textareaTag;
extern const GlobalString tfootTag;
extern const GlobalString thTag;
extern const GlobalString theadTag;
extern const GlobalString titleTag;
extern const GlobalString trTag;
extern const GlobalString trackTag;
extern const GlobalString tspanTag;
extern const GlobalString ttTag;
extern const GlobalString uTag;
extern const GlobalString ulTag;
extern const GlobalString useTag;
extern const GlobalString varTag;
extern const GlobalString wbrTag;
extern const GlobalString xmpTag;

extern const GlobalString alignAttr;
extern const GlobalString altAttr;
extern const GlobalString backgroundAttr;
extern const GlobalString bgcolorAttr;
extern const GlobalString borderAttr;
extern const GlobalString bordercolorAttr;
extern const GlobalString cellpaddingAttr;
extern const GlobalString cellspacingAttr;
extern const GlobalString checkedAttr;
extern const GlobalString classAttr;
extern const GlobalString clipPathUnitsAttr;
extern const GlobalString colorAttr;
extern const GlobalString colsAttr;
extern const GlobalString colspanAttr;
extern const GlobalString cxAttr;
extern const GlobalString cyAttr;
extern const GlobalString dAttr;
extern const GlobalString disabledAttr;
extern const GlobalString dxAttr;
extern const GlobalString dyAttr;
extern const GlobalString enabledAttr;
extern const GlobalString encodingAttr;
extern const GlobalString faceAttr;
extern const GlobalString frameAttr;
extern const GlobalString fxAttr;
extern const GlobalString fyAttr;
extern const GlobalString gradientTransformAttr;
extern const GlobalString gradientUnitsAttr;
extern const GlobalString heightAttr;
extern const GlobalString hiddenAttr;
extern const GlobalString hrefAttr;
extern const GlobalString hspaceAttr;
extern const GlobalString idAttr;
extern const GlobalString langAttr;
extern const GlobalString markerHeightAttr;
extern const GlobalString markerUnitsAttr;
extern const GlobalString markerWidthAttr;
extern const GlobalString maskContentUnitsAttr;
extern const GlobalString maskUnitsAttr;
extern const GlobalString mediaAttr;
extern const GlobalString multipleAttr;
extern const GlobalString noshadeAttr;
extern const GlobalString offsetAttr;
extern const GlobalString orientAttr;
extern const GlobalString patternContentUnitsAttr;
extern const GlobalString patternTransformAttr;
extern const GlobalString patternUnitsAttr;
extern const GlobalString pointsAttr;
extern const GlobalString preserveAspectRatioAttr;
extern const GlobalString rAttr;
extern const GlobalString refXAttr;
extern const GlobalString refYAttr;
extern const GlobalString relAttr;
extern const GlobalString rotateAttr;
extern const GlobalString rowsAttr;
extern const GlobalString rowspanAttr;
extern const GlobalString rulesAttr;
extern const GlobalString rxAttr;
extern const GlobalString ryAttr;
extern const GlobalString sizeAttr;
extern const GlobalString spanAttr;
extern const GlobalString spreadMethodAttr;
extern const GlobalString srcAttr;
extern const GlobalString startAttr;
extern const GlobalString styleAttr;
extern const GlobalString textAttr;
extern const GlobalString transformAttr;
extern const GlobalString typeAttr;
extern const GlobalString valignAttr;
extern const GlobalString valueAttr;
extern const GlobalString viewBoxAttr;
extern const GlobalString vspaceAttr;
extern const GlobalString widthAttr;
extern const GlobalString x1Attr;
extern const GlobalString x2Attr;
extern const GlobalString xAttr;
extern const GlobalString y1Attr;
extern const GlobalString y2Attr;
extern const GlobalString yAttr;

} // namespace plutobook

#endif // PLUTOBOOK_GLOBALSTRING_H
