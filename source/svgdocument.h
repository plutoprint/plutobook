/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_SVGDOCUMENT_H
#define PLUTOBOOK_SVGDOCUMENT_H

#include "xmldocument.h"
#include "svgproperty.h"

#include <memory>

namespace plutobook {

using SVGPropertyMap = std::pmr::map<GlobalString, SVGProperty*>;

class SVGResourceContainerBox;
class SVGResourceClipperBox;
class SVGResourceMaskerBox;

class SVGElement : public Element {
public:
    SVGElement(Document* document, const GlobalString& tagName);

    bool isSVGElement() const final { return true; }

    void parseAttribute(const GlobalString& name, const HeapString& value) override;
    void collectAttributeStyle(std::string& output, const GlobalString& name, const HeapString& value) const override;
    void addProperty(const GlobalString& name, SVGProperty& value);
    SVGProperty* getProperty(const GlobalString& name) const;
    Size currentViewportSize() const;

    SVGResourceContainerBox* getResourceById(const std::string_view& id) const;
    SVGResourceClipperBox* getClipper(const std::string_view& id) const;
    SVGResourceMaskerBox* getMasker(const std::string_view& id) const;
    Box* createBox(const RefPtr<BoxStyle>& style) override { return nullptr; }

private:
    SVGPropertyMap m_properties;
};

template<>
struct is_a<SVGElement> {
    static bool check(const Node& value) { return value.isSVGElement(); }
};

inline bool Node::isSVGRootNode() const
{
    auto element = to<SVGElement>(this);
    if(element && element->tagName() == svgTag)
        return !m_parentNode->isSVGElement();
    return false;
}

class SVGFitToViewBox {
public:
    SVGFitToViewBox(SVGElement* element);

    const Rect& viewBox() const { return m_viewBox.value(); }
    const SVGPreserveAspectRatio& preserveAspectRatio() const { return m_preserveAspectRatio; }
    Transform viewBoxToViewTransform(const Size& viewportSize) const;
    Rect getClipRect(const Size& viewportSize) const;

private:
    SVGRect m_viewBox;
    SVGPreserveAspectRatio m_preserveAspectRatio;
};

class SVGURIReference {
public:
    SVGURIReference(SVGElement* element);

    const std::string& href() const { return m_href.value(); }
    SVGElement* getTargetElement(const Document* document) const;

private:
    SVGString m_href;
};

class SVGResourcePaintServerBox;
class SVGPaintServer;
class StrokeData;
class Paint;

class SVGGraphicsElement : public SVGElement {
public:
    SVGGraphicsElement(Document* document, const GlobalString& tagName);

    const Transform& transform() const { return m_transform.value(); }
    SVGResourcePaintServerBox* getPainter(const std::string_view& id) const;
    SVGPaintServer getPaintServer(const Paint& paint, float opacity) const;
    StrokeData getStrokeData(const BoxStyle* style) const;

private:
    SVGTransform m_transform;
};

class SVGSVGElement final : public SVGGraphicsElement, public SVGFitToViewBox {
public:
    SVGSVGElement(Document* document);

    const SVGLength& x() const { return m_x; }
    const SVGLength& y() const { return m_y; }
    const SVGLength& width() const { return m_width; }
    const SVGLength& height() const { return m_height; }
    void computeIntrinsicDimensions(float& intrinsicWidth, float& intrinsicHeight, double& intrinsicRatio);
    void collectAttributeStyle(std::string& output, const GlobalString& name, const HeapString& value) const final;
    Box* createBox(const RefPtr<BoxStyle>& style) final;

private:
    SVGLength m_x;
    SVGLength m_y;
    SVGLength m_width;
    SVGLength m_height;
};

class SVGUseElement final : public SVGGraphicsElement, public SVGURIReference {
public:
    SVGUseElement(Document* document);

    const SVGLength& x() const { return m_x; }
    const SVGLength& y() const { return m_y; }
    const SVGLength& width() const { return m_width; }
    const SVGLength& height() const { return m_height; }
    Box* createBox(const RefPtr<BoxStyle>& style) final;
    void finishParsingDocument() final;

private:
    Element* cloneTargetElement(SVGElement* targetElement);
    SVGLength m_x;
    SVGLength m_y;
    SVGLength m_width;
    SVGLength m_height;
};

class Image;

class SVGImageElement final : public SVGGraphicsElement, public SVGURIReference {
public:
    SVGImageElement(Document* document);

    const SVGLength& x() const { return m_x; }
    const SVGLength& y() const { return m_y; }
    const SVGLength& width() const { return m_width; }
    const SVGLength& height() const { return m_height; }
    const SVGPreserveAspectRatio& preserveAspectRatio() const { return m_preserveAspectRatio; }
    Box* createBox(const RefPtr<BoxStyle>& style) final;

    RefPtr<Image> image() const;

private:
    SVGLength m_x;
    SVGLength m_y;
    SVGLength m_width;
    SVGLength m_height;
    SVGPreserveAspectRatio m_preserveAspectRatio;
};

class SVGSymbolElement final : public SVGGraphicsElement, public SVGFitToViewBox {
public:
    SVGSymbolElement(Document* document);

    Box* createBox(const RefPtr<BoxStyle>& style) final;
};

class SVGAElement final : public SVGGraphicsElement, public SVGURIReference {
public:
    SVGAElement(Document* document);

    Box* createBox(const RefPtr<BoxStyle>& style) final;
};

class SVGGElement final : public SVGGraphicsElement {
public:
    SVGGElement(Document* document);

    Box* createBox(const RefPtr<BoxStyle>& style) final;
};

class SVGDefsElement final : public SVGGraphicsElement {
public:
    SVGDefsElement(Document* document);

    Box* createBox(const RefPtr<BoxStyle>& style) final;
};

class SVGResourceMarkerBox;

class SVGGeometryElement : public SVGGraphicsElement {
public:
    SVGGeometryElement(Document* document, const GlobalString& tagName);

    SVGResourceMarkerBox* getMarker(const std::string_view& id) const;
};

class SVGPathElement final : public SVGGeometryElement {
public:
    SVGPathElement(Document* document);

    const Path& path() const { return m_d.value(); }

    Box* createBox(const RefPtr<BoxStyle>& style) final;

private:
    SVGPath m_d;
};

class SVGShapeElement : public SVGGeometryElement {
public:
    SVGShapeElement(Document* document, const GlobalString& tagName);

    virtual Rect getPath(Path& path) const = 0;

    Box* createBox(const RefPtr<BoxStyle>& style) final;
};

class SVGLineElement final : public SVGShapeElement {
public:
    SVGLineElement(Document* document);

    const SVGLength& x1() const { return m_x1; }
    const SVGLength& y1() const { return m_y1; }
    const SVGLength& x2() const { return m_x2; }
    const SVGLength& y2() const { return m_y2; }

    Rect getPath(Path& path) const final;

private:
    SVGLength m_x1;
    SVGLength m_y1;
    SVGLength m_x2;
    SVGLength m_y2;
};

class SVGRectElement final : public SVGShapeElement {
public:
    SVGRectElement(Document* document);

    const SVGLength& x() const { return m_x; }
    const SVGLength& y() const { return m_y; }
    const SVGLength& width() const { return m_width; }
    const SVGLength& height() const { return m_height; }
    const SVGLength& rx() const { return m_rx; }
    const SVGLength& ry() const { return m_ry; }

    Rect getPath(Path& path) const final;

private:
    SVGLength m_x;
    SVGLength m_y;
    SVGLength m_width;
    SVGLength m_height;
    SVGLength m_rx;
    SVGLength m_ry;
};

class SVGEllipseElement final : public SVGShapeElement {
public:
    SVGEllipseElement(Document* document);

    const SVGLength& cx() const { return m_cx; }
    const SVGLength& cy() const { return m_cy; }
    const SVGLength& rx() const { return m_rx; }
    const SVGLength& ry() const { return m_ry; }

    Rect getPath(Path& path) const final;

private:
    SVGLength m_cx;
    SVGLength m_cy;
    SVGLength m_rx;
    SVGLength m_ry;
};

class SVGCircleElement final : public SVGShapeElement {
public:
    SVGCircleElement(Document* document);

    const SVGLength& cx() const { return m_cx; }
    const SVGLength& cy() const { return m_cy; }
    const SVGLength& r() const { return m_r; }

    Rect getPath(Path& path) const final;

private:
    SVGLength m_cx;
    SVGLength m_cy;
    SVGLength m_r;
};

class SVGPolyElement : public SVGShapeElement {
public:
    SVGPolyElement(Document* document, const GlobalString& tagName);

    const SVGPointList& points() const { return m_points; }

    Rect getPath(Path& path) const final;

private:
    SVGPointList m_points;
};

class SVGTextPositioningElement : public SVGGraphicsElement {
public:
    SVGTextPositioningElement(Document* document, const GlobalString& tagName);

    const SVGLengthList& x() const { return m_x; }
    const SVGLengthList& y() const { return m_y; }
    const SVGLengthList& dx() const { return m_dx; }
    const SVGLengthList& dy() const { return m_dy; }
    const SVGNumberList& rotate() const { return m_rotate; }

private:
    SVGLengthList m_x;
    SVGLengthList m_y;
    SVGLengthList m_dx;
    SVGLengthList m_dy;
    SVGNumberList m_rotate;
};

class SVGTSpanElement final : public SVGTextPositioningElement {
public:
    SVGTSpanElement(Document* document);

    Box* createBox(const RefPtr<BoxStyle>& style) final;
};

class SVGTextElement final : public SVGTextPositioningElement {
public:
    SVGTextElement(Document* document);

    Box* createBox(const RefPtr<BoxStyle>& style) final;
};

class SVGMarkerElement final : public SVGElement, public SVGFitToViewBox {
public:
    SVGMarkerElement(Document* document);

    const SVGLength& refX() const { return m_refX; }
    const SVGLength& refY() const { return m_refY; }
    const SVGLength& markerWidth() const { return m_markerWidth; }
    const SVGLength& markerHeight() const { return m_markerHeight; }
    const SVGAngle& orient() const { return m_orient; }
    const SVGMarkerUnitsType markerUnits() const { return m_markerUnits.value(); }
    Box* createBox(const RefPtr<BoxStyle>& style) final;

private:
    SVGLength m_refX;
    SVGLength m_refY;
    SVGLength m_markerWidth;
    SVGLength m_markerHeight;
    SVGEnumeration<SVGMarkerUnitsType> m_markerUnits;
    SVGAngle m_orient;
};

class SVGClipPathElement final : public SVGGraphicsElement {
public:
    SVGClipPathElement(Document* document);

    const SVGUnitsType clipPathUnits() const { return m_clipPathUnits.value(); }
    Box* createBox(const RefPtr<BoxStyle>& style) final;

private:
    SVGEnumeration<SVGUnitsType> m_clipPathUnits;
};

class SVGMaskElement final : public SVGElement {
public:
    SVGMaskElement(Document* document);

    const SVGLength& x() const { return m_x; }
    const SVGLength& y() const { return m_y; }
    const SVGLength& width() const { return m_width; }
    const SVGLength& height() const { return m_height; }
    const SVGUnitsType maskUnits() const { return m_maskUnits.value(); }
    const SVGUnitsType maskContentUnits() const { return m_maskContentUnits.value(); }
    Box* createBox(const RefPtr<BoxStyle>& style) final;

private:
    SVGLength m_x;
    SVGLength m_y;
    SVGLength m_width;
    SVGLength m_height;
    SVGEnumeration<SVGUnitsType> m_maskUnits;
    SVGEnumeration<SVGUnitsType> m_maskContentUnits;
};

class SVGPatternAttributes;

class SVGPatternElement final : public SVGElement, public SVGURIReference, public SVGFitToViewBox {
public:
    SVGPatternElement(Document* document);

    const SVGLength& x() const { return m_x; }
    const SVGLength& y() const { return m_y; }
    const SVGLength& width() const { return m_width; }
    const SVGLength& height() const { return m_height; }
    const Transform& patternTransform() const { return m_patternTransform.value(); }
    const SVGUnitsType patternUnits() const { return m_patternUnits.value(); }
    const SVGUnitsType patternContentUnits() const { return m_patternContentUnits.value(); }
    SVGPatternAttributes collectPatternAttributes() const;
    Box* createBox(const RefPtr<BoxStyle>& style) final;

private:
    SVGLength m_x;
    SVGLength m_y;
    SVGLength m_width;
    SVGLength m_height;
    SVGTransform m_patternTransform;
    SVGEnumeration<SVGUnitsType> m_patternUnits;
    SVGEnumeration<SVGUnitsType> m_patternContentUnits;
};

class SVGPatternAttributes {
public:
    SVGPatternAttributes() = default;

    const SVGLength& x() const { return m_x->x(); }
    const SVGLength& y() const { return m_y->y(); }
    const SVGLength& width() const { return m_width->width(); }
    const SVGLength& height() const { return m_height->height(); }
    const Transform& patternTransform() const { return m_patternTransform->patternTransform(); }
    SVGUnitsType patternUnits() const { return m_patternUnits->patternUnits(); }
    SVGUnitsType patternContentUnits() const { return m_patternContentUnits->patternContentUnits(); }
    const Rect& viewBox() const { return m_viewBox->viewBox(); }
    const SVGPreserveAspectRatio& preserveAspectRatio() const { return m_preserveAspectRatio->preserveAspectRatio(); }
    const SVGPatternElement* patternContentElement() const { return m_patternContentElement; }

    bool hasX() const { return m_x; }
    bool hasY() const { return m_y; }
    bool hasWidth() const { return m_width; }
    bool hasHeight() const { return m_height; }
    bool hasPatternTransform() const { return m_patternTransform; }
    bool hasPatternUnits() const { return m_patternUnits; }
    bool hasPatternContentUnits() const { return m_patternContentUnits; }
    bool hasViewBox() const { return m_viewBox; }
    bool hasPreserveAspectRatio() const { return m_preserveAspectRatio; }
    bool hasPatternContentElement() const { return m_patternContentElement; }

    void setX(const SVGPatternElement* value) { m_x = value; }
    void setY(const SVGPatternElement* value) { m_y = value; }
    void setWidth(const SVGPatternElement* value) { m_width = value; }
    void setHeight(const SVGPatternElement* value) { m_height = value; }
    void setPatternTransform(const SVGPatternElement* value) { m_patternTransform = value; }
    void setPatternUnits(const SVGPatternElement* value) { m_patternUnits = value; }
    void setPatternContentUnits(const SVGPatternElement* value) { m_patternContentUnits = value; }
    void setViewBox(const SVGPatternElement* value) { m_viewBox = value; }
    void setPreserveAspectRatio(const SVGPatternElement* value) { m_preserveAspectRatio = value; }
    void setPatternContentElement(const SVGPatternElement* value) { m_patternContentElement = value; }

    void setDefaultValues(const SVGPatternElement* element) {
        if(!m_x) { m_x = element; }
        if(!m_y) { m_y = element; }
        if(!m_width) { m_width = element; }
        if(!m_height) { m_height = element; }
        if(!m_patternTransform) { m_patternTransform = element; }
        if(!m_patternUnits) { m_patternUnits = element; }
        if(!m_patternContentUnits) { m_patternContentUnits = element; }
        if(!m_viewBox) { m_viewBox = element; }
        if(!m_preserveAspectRatio) { m_preserveAspectRatio = element; }
        if(!m_patternContentElement) { m_patternContentElement = element; }
    }

private:
    const SVGPatternElement* m_x{nullptr};
    const SVGPatternElement* m_y{nullptr};
    const SVGPatternElement* m_width{nullptr};
    const SVGPatternElement* m_height{nullptr};
    const SVGPatternElement* m_patternTransform{nullptr};
    const SVGPatternElement* m_patternUnits{nullptr};
    const SVGPatternElement* m_patternContentUnits{nullptr};
    const SVGPatternElement* m_viewBox{nullptr};
    const SVGPatternElement* m_preserveAspectRatio{nullptr};
    const SVGPatternElement* m_patternContentElement{nullptr};
};

class Color;

class SVGStopElement final : public SVGElement {
public:
    SVGStopElement(Document* document);

    const float offset() const { return m_offset.value(); }
    Color stopColorIncludingOpacity() const;
    Box* createBox(const RefPtr<BoxStyle>& style) final;

private:
    SVGNumberPercentage m_offset;
};

class SVGGradientAttributes;

class SVGGradientElement : public SVGElement, public SVGURIReference {
public:
    SVGGradientElement(Document* document, const GlobalString& tagName);

    const Transform& gradientTransform() const { return m_gradientTransform.value(); }
    const SVGUnitsType gradientUnits() const { return m_gradientUnits.value(); }
    const SVGSpreadMethodType spreadMethod() const { return m_spreadMethod.value(); }
    void collectGradientAttributes(SVGGradientAttributes& attributes) const;

private:
    SVGTransform m_gradientTransform;
    SVGEnumeration<SVGUnitsType> m_gradientUnits;
    SVGEnumeration<SVGSpreadMethodType> m_spreadMethod;
};

class SVGGradientAttributes {
public:
    SVGGradientAttributes() = default;

    const Transform& gradientTransform() const { return m_gradientTransform->gradientTransform(); }
    SVGSpreadMethodType spreadMethod() const { return m_spreadMethod->spreadMethod(); }
    SVGUnitsType gradientUnits() const { return m_gradientUnits->gradientUnits(); }
    const SVGGradientElement* gradientContentElement() const { return m_gradientContentElement; }

    bool hasGradientTransform() const { return m_gradientTransform; }
    bool hasSpreadMethod() const { return m_spreadMethod; }
    bool hasGradientUnits() const { return m_gradientUnits; }
    bool hasGradientContentElement() const { return m_gradientContentElement; }

    void setGradientTransform(const SVGGradientElement* value) { m_gradientTransform = value; }
    void setSpreadMethod(const SVGGradientElement* value) { m_spreadMethod = value; }
    void setGradientUnits(const SVGGradientElement* value) { m_gradientUnits = value; }
    void setGradientContentElement(const SVGGradientElement* value) { m_gradientContentElement = value; }

    void setDefaultValues(const SVGGradientElement* element) {
        if(!m_gradientTransform) { m_gradientTransform = element; }
        if(!m_spreadMethod) { m_spreadMethod = element; }
        if(!m_gradientUnits) { m_gradientUnits = element; }
        if(!m_gradientContentElement) { m_gradientContentElement = element; }
    }

private:
    const SVGGradientElement* m_gradientTransform{nullptr};
    const SVGGradientElement* m_spreadMethod{nullptr};
    const SVGGradientElement* m_gradientUnits{nullptr};
    const SVGGradientElement* m_gradientContentElement{nullptr};
};

class SVGLinearGradientAttributes;

class SVGLinearGradientElement final : public SVGGradientElement {
public:
    SVGLinearGradientElement(Document* document);

    const SVGLength& x1() const { return m_x1; }
    const SVGLength& y1() const { return m_y1; }
    const SVGLength& x2() const { return m_x2; }
    const SVGLength& y2() const { return m_y2; }
    SVGLinearGradientAttributes collectGradientAttributes() const;
    Box* createBox(const RefPtr<BoxStyle>& style) final;

private:
    SVGLength m_x1;
    SVGLength m_y1;
    SVGLength m_x2;
    SVGLength m_y2;
};

class SVGLinearGradientAttributes : public SVGGradientAttributes {
public:
    SVGLinearGradientAttributes() = default;

    const SVGLength& x1() const { return m_x1->x1(); }
    const SVGLength& y1() const { return m_y1->y1(); }
    const SVGLength& x2() const { return m_x2->x2(); }
    const SVGLength& y2() const { return m_y2->y2(); }

    bool hasX1() const { return m_x1; }
    bool hasY1() const { return m_y1; }
    bool hasX2() const { return m_x2; }
    bool hasY2() const { return m_y2; }

    void setX1(const SVGLinearGradientElement* value) { m_x1 = value; }
    void setY1(const SVGLinearGradientElement* value) { m_y1 = value; }
    void setX2(const SVGLinearGradientElement* value) { m_x2 = value; }
    void setY2(const SVGLinearGradientElement* value) { m_y2 = value; }

    void setDefaultValues(const SVGLinearGradientElement* element) {
        SVGGradientAttributes::setDefaultValues(element);
        if(!m_x1) { m_x1 = element; }
        if(!m_y1) { m_y1 = element; }
        if(!m_x2) { m_x2 = element; }
        if(!m_y2) { m_y2 = element; }
    }

private:
    const SVGLinearGradientElement* m_x1{nullptr};
    const SVGLinearGradientElement* m_y1{nullptr};
    const SVGLinearGradientElement* m_x2{nullptr};
    const SVGLinearGradientElement* m_y2{nullptr};
};

class SVGRadialGradientAttributes;

class SVGRadialGradientElement final : public SVGGradientElement {
public:
    SVGRadialGradientElement(Document* document);

    const SVGLength& cx() const { return m_cx; }
    const SVGLength& cy() const { return m_cy; }
    const SVGLength& r() const { return m_r; }
    const SVGLength& fx() const { return m_fx; }
    const SVGLength& fy() const { return m_fy; }
    SVGRadialGradientAttributes collectGradientAttributes() const;
    Box* createBox(const RefPtr<BoxStyle>& style) final;

private:
    SVGLength m_cx;
    SVGLength m_cy;
    SVGLength m_r;
    SVGLength m_fx;
    SVGLength m_fy;
};

class SVGRadialGradientAttributes : public SVGGradientAttributes {
public:
    SVGRadialGradientAttributes() = default;

    const SVGLength& cx() const { return m_cx->cx(); }
    const SVGLength& cy() const { return m_cy->cy(); }
    const SVGLength& r() const { return m_r->r(); }
    const SVGLength& fx() const { return m_fx ? m_fx->fx() : m_cx->cx(); }
    const SVGLength& fy() const { return m_fy ? m_fy->fy() : m_cy->cy(); }

    bool hasCx() const { return m_cx; }
    bool hasCy() const { return m_cy; }
    bool hasR() const { return m_r; }
    bool hasFx() const { return m_fx; }
    bool hasFy() const { return m_fy; }

    void setCx(const SVGRadialGradientElement* value) { m_cx = value; }
    void setCy(const SVGRadialGradientElement* value) { m_cy = value; }
    void setR(const SVGRadialGradientElement* value) { m_r = value; }
    void setFx(const SVGRadialGradientElement* value) { m_fx = value; }
    void setFy(const SVGRadialGradientElement* value) { m_fy = value; }

    void setDefaultValues(const SVGRadialGradientElement* element) {
        SVGGradientAttributes::setDefaultValues(element);
        if(!m_cx) { m_cx = element; }
        if(!m_cy) { m_cy = element; }
        if(!m_r) { m_r = element; }
    }

private:
    const SVGRadialGradientElement* m_cx{nullptr};
    const SVGRadialGradientElement* m_cy{nullptr};
    const SVGRadialGradientElement* m_r{nullptr};
    const SVGRadialGradientElement* m_fx{nullptr};
    const SVGRadialGradientElement* m_fy{nullptr};
};

class SVGStyleElement final : public SVGElement {
public:
    SVGStyleElement(Document* document);

    const HeapString& type() const;
    const HeapString& media() const;

    void finishParsingDocument() final;
};

class SVGDocument final : public XMLDocument {
public:
    static std::unique_ptr<SVGDocument> create(Book* book, Heap* heap, ResourceFetcher* fetcher, Url baseUrl);

    bool isSVGDocument() const final { return true; }

private:
    SVGDocument(Book* book, Heap* heap, ResourceFetcher* fetcher, Url baseUrl);
};

template<>
struct is_a<SVGDocument> {
    static bool check(const Node& value) { return value.isSVGDocument(); }
};

} // namespace plutobook

#endif // PLUTOBOOK_SVGDOCUMENT_H
