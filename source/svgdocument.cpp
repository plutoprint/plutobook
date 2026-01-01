/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "svgdocument.h"
#include "svgreplacedbox.h"
#include "svgresourcebox.h"
#include "svggeometrybox.h"
#include "svgtextbox.h"
#include "imageresource.h"
#include "stringutils.h"

#include <set>

namespace plutobook {

SVGElement::SVGElement(Document* document, const GlobalString& tagName)
    : Element(document, svgNs, tagName)
    , m_properties(document->heap())
{
}

void SVGElement::parseAttribute(const GlobalString& name, const HeapString& value)
{
    if(auto property = getProperty(name)) {
        property->parse(value);
    } else {
        Element::parseAttribute(name, value);
    }
}

static void addSVGAttributeStyle(std::string& output, const std::string_view& name, const std::string_view& value)
{
    if(value.empty())
        return;
    output += name;
    output += ':';
    output += value;
    output += ';';
}

void SVGElement::collectAttributeStyle(std::string& output, const GlobalString& name, const HeapString& value) const
{
    static const std::set<GlobalString> presentationAttrs = {
        "alignment-baseline"_glo,
        "baseline-shift"_glo,
        "clip"_glo,
        "clip-path"_glo,
        "clip-rule"_glo,
        "color"_glo,
        "direction"_glo,
        "display"_glo,
        "dominant-baseline"_glo,
        "fill"_glo,
        "fill-opacity"_glo,
        "fill-rule"_glo,
        "font-family"_glo,
        "font-size"_glo,
        "font-stretch"_glo,
        "font-style"_glo,
        "font-variant"_glo,
        "font-weight"_glo,
        "letter-spacing"_glo,
        "marker-end"_glo,
        "marker-mid"_glo,
        "marker-start"_glo,
        "mask"_glo,
        "mask-type"_glo,
        "opacity"_glo,
        "overflow"_glo,
        "paint-order"_glo,
        "stop-color"_glo,
        "stop-opacity"_glo,
        "stroke"_glo,
        "stroke-dasharray"_glo,
        "stroke-dashoffset"_glo,
        "stroke-linecap"_glo,
        "stroke-linejoin"_glo,
        "stroke-miterlimit"_glo,
        "stroke-opacity"_glo,
        "stroke-width"_glo,
        "text-anchor"_glo,
        "text-decoration"_glo,
        "text-orientation"_glo,
        "transform-origin"_glo,
        "unicode-bidi"_glo,
        "vector-effect"_glo,
        "visibility"_glo,
        "word-spacing"_glo,
        "writing-mode"_glo
    };

    if(presentationAttrs.contains(name)) {
        addSVGAttributeStyle(output, name, value);
    } else {
        Element::collectAttributeStyle(output, name, value);
    }
}

void SVGElement::addProperty(const GlobalString& name, SVGProperty& value)
{
    m_properties.emplace(name, &value);
}

SVGProperty* SVGElement::getProperty(const GlobalString& name) const
{
    auto it = m_properties.find(name);
    if(it == m_properties.end())
        return nullptr;
    return it->second;
}

Size SVGElement::currentViewportSize() const
{
    auto parent = to<SVGElement>(parentNode());
    if(parent == nullptr) {
        return to<SVGRootBox>(box())->contentBoxSize();
    }

    if(parent->tagName() == svgTag) {
        auto element = static_cast<const SVGSVGElement*>(parent);
        const auto& viewBoxRect = element->viewBox();
        if(viewBoxRect.isValid())
            return viewBoxRect.size();
        if(auto rootBox = to<SVGRootBox>(element->box()))
            return rootBox->contentBoxSize();
        SVGLengthContext lengthContext(element);
        const Size viewportSize = {
            lengthContext.valueForLength(element->width()),
            lengthContext.valueForLength(element->height())
        };

        return viewportSize;
    }

    return parent->currentViewportSize();
}

SVGResourceContainerBox* SVGElement::getResourceById(const std::string_view& id) const
{
    if(id.empty() || id.front() != '#')
        return nullptr;
    auto element = document()->getElementById(id.substr(1));
    if(element == nullptr)
        return nullptr;
    return to<SVGResourceContainerBox>(element->box());
}

SVGResourceClipperBox* SVGElement::getClipper(const std::string_view& id) const
{
    return to<SVGResourceClipperBox>(getResourceById(id));
}

SVGResourceMaskerBox* SVGElement::getMasker(const std::string_view& id) const
{
    return to<SVGResourceMaskerBox>(getResourceById(id));
}

SVGGraphicsElement::SVGGraphicsElement(Document* document, const GlobalString& tagName)
    : SVGElement(document, tagName)
{
    addProperty(transformAttr, m_transform);
}

SVGResourcePaintServerBox* SVGGraphicsElement::getPainter(const std::string_view& id) const
{
    return to<SVGResourcePaintServerBox>(getResourceById(id));
}

SVGPaintServer SVGGraphicsElement::getPaintServer(const Paint& paint, float opacity) const
{
    return SVGPaintServer(getPainter(paint.uri()), paint.color(), opacity);
}

StrokeData SVGGraphicsElement::getStrokeData(const BoxStyle* style) const
{
    SVGLengthContext lengthContext(this);
    StrokeData strokeData(lengthContext.valueForLength(style->strokeWidth()));
    strokeData.setMiterLimit(style->strokeMiterlimit());
    strokeData.setLineCap(style->strokeLinecap());
    strokeData.setLineJoin(style->strokeLinejoin());
    strokeData.setDashOffset(lengthContext.valueForLength(style->strokeDashoffset()));

    DashArray dashArray;
    for(const auto& dash : style->strokeDasharray())
        dashArray.push_back(lengthContext.valueForLength(dash));
    strokeData.setDashArray(std::move(dashArray));
    return strokeData;
}

SVGFitToViewBox::SVGFitToViewBox(SVGElement* element)
{
    element->addProperty(viewBoxAttr, m_viewBox);
    element->addProperty(preserveAspectRatioAttr, m_preserveAspectRatio);
}

Transform SVGFitToViewBox::viewBoxToViewTransform(const Size& viewportSize) const
{
    const auto& viewBoxRect = m_viewBox.value();
    if(viewBoxRect.isEmpty() || viewportSize.isEmpty())
        return Transform::Identity;
    return m_preserveAspectRatio.getTransform(viewBoxRect, viewportSize);
}

Rect SVGFitToViewBox::getClipRect(const Size& viewportSize) const
{
    const auto& viewBoxRect = m_viewBox.value();
    if(viewBoxRect.isEmpty() || viewportSize.isEmpty())
        return Rect(0, 0, viewportSize.w, viewportSize.h);
    return m_preserveAspectRatio.getClipRect(viewBoxRect, viewportSize);
}

SVGURIReference::SVGURIReference(SVGElement* element)
{
    element->addProperty(hrefAttr, m_href);
}

SVGElement* SVGURIReference::getTargetElement(const Document* document) const
{
    std::string_view value(m_href.value());
    if(value.empty() || value.front() != '#')
        return nullptr;
    return to<SVGElement>(document->getElementById(value.substr(1)));
}

SVGSVGElement::SVGSVGElement(Document* document)
    : SVGGraphicsElement(document, svgTag)
    , SVGFitToViewBox(this)
    , m_x(0.f, SVGLengthType::Number, SVGLengthDirection::Horizontal, SVGLengthNegativeValuesMode::Allow)
    , m_y(0.f, SVGLengthType::Number, SVGLengthDirection::Vertical, SVGLengthNegativeValuesMode::Allow)
    , m_width(100.f, SVGLengthType::Percentage, SVGLengthDirection::Horizontal, SVGLengthNegativeValuesMode::Forbid)
    , m_height(100.f, SVGLengthType::Percentage, SVGLengthDirection::Vertical, SVGLengthNegativeValuesMode::Forbid)
{
    addProperty(xAttr, m_x);
    addProperty(yAttr, m_y);
    addProperty(widthAttr, m_width);
    addProperty(heightAttr, m_height);
}

void SVGSVGElement::computeIntrinsicDimensions(float& intrinsicWidth, float& intrinsicHeight, double& intrinsicRatio)
{
    SVGLengthContext lengthContext(this);
    if(m_width.type() != SVGLengthType::Percentage) {
        intrinsicWidth = lengthContext.valueForLength(m_width);
    } else {
        intrinsicWidth = 0.f;
    }

    if(m_height.type() != SVGLengthType::Percentage) {
        intrinsicHeight = lengthContext.valueForLength(m_height);
    } else {
        intrinsicHeight = 0.f;
    }

    const auto& viewBoxRect = viewBox();
    if(intrinsicWidth > 0.f && intrinsicHeight > 0.f) {
        intrinsicRatio = intrinsicWidth / intrinsicHeight;
    } else if(!viewBoxRect.isEmpty()) {
        intrinsicRatio = viewBoxRect.w / viewBoxRect.h;
    } else {
        intrinsicRatio = 0.0;
    }
}

static void addSVGTransformAttributeStyle(std::string& output, const Transform& matrix)
{
    output += "transform:matrix(";
    output += toString(matrix.a);
    output += ',';
    output += toString(matrix.b);
    output += ',';
    output += toString(matrix.c);
    output += ',';
    output += toString(matrix.d);
    output += ',';
    output += toString(matrix.e);
    output += ',';
    output += toString(matrix.f);
    output += ");";
}

void SVGSVGElement::collectAttributeStyle(std::string& output, const GlobalString& name, const HeapString& value) const
{
    if(name == transformAttr && isSVGRootNode()) {
        addSVGTransformAttributeStyle(output, transform());
    } else if(isSVGRootNode() && (name == widthAttr || name == heightAttr)) {
        addSVGAttributeStyle(output, name, value);
    } else {
        SVGElement::collectAttributeStyle(output, name, value);
    }
}

Box* SVGSVGElement::createBox(const RefPtr<BoxStyle>& style)
{
    if(isSVGRootNode())
        return new (heap()) SVGRootBox(this, style);
    return new (heap()) SVGViewportContainerBox(this, style);
}

SVGUseElement::SVGUseElement(Document* document)
    : SVGGraphicsElement(document, useTag)
    , SVGURIReference(this)
    , m_x(0.f, SVGLengthType::Number, SVGLengthDirection::Horizontal, SVGLengthNegativeValuesMode::Allow)
    , m_y(0.f, SVGLengthType::Number, SVGLengthDirection::Vertical, SVGLengthNegativeValuesMode::Allow)
    , m_width(100.f, SVGLengthType::Percentage, SVGLengthDirection::Horizontal, SVGLengthNegativeValuesMode::Forbid)
    , m_height(100.f, SVGLengthType::Percentage, SVGLengthDirection::Vertical, SVGLengthNegativeValuesMode::Forbid)
{
    addProperty(xAttr, m_x);
    addProperty(yAttr, m_y);
    addProperty(widthAttr, m_width);
    addProperty(heightAttr, m_height);
}

Box* SVGUseElement::createBox(const RefPtr<BoxStyle>& style)
{
    return new (heap()) SVGTransformableContainerBox(this, style);
}

void SVGUseElement::finishParsingDocument()
{
    if(auto targetElement = getTargetElement(document())) {
        if(auto newElement = cloneTargetElement(targetElement)) {
            appendChild(newElement);
        }
    }

    SVGElement::finishParsingDocument();
}

static bool isDisallowedElement(const SVGElement* element)
{
    static const std::set<GlobalString> allowedElementTags = {
        aTag,
        circleTag,
        descTag,
        ellipseTag,
        gTag,
        imageTag,
        lineTag,
        metadataTag,
        pathTag,
        polygonTag,
        polylineTag,
        rectTag,
        svgTag,
        switchTag,
        symbolTag,
        textTag,
        textPathTag,
        titleTag,
        tspanTag,
        useTag
    };

    return !allowedElementTags.contains(element->tagName());
}

Element* SVGUseElement::cloneTargetElement(SVGElement* targetElement)
{
    if(targetElement == this || isDisallowedElement(targetElement))
        return nullptr;
    auto parent = parentNode();
    const auto& id = targetElement->id();
    while(parent && parent->isSVGElement()) {
        const auto& element = to<SVGElement>(*parent);
        if(!id.empty() && id == element.id())
            return nullptr;
        parent = parent->parentNode();
    }

    auto tagName = targetElement->tagName();
    if(tagName == symbolTag) {
        tagName = svgTag;
    }

    auto newElement = document()->createElement(svgNs, tagName);
    newElement->setAttributes(targetElement->attributes());
    if(newElement->tagName() == svgTag) {
        for(const auto& attribute : attributes()) {
            if(attribute.name() == widthAttr || attribute.name() == heightAttr) {
                newElement->setAttribute(attribute);
            }
        }
    }

    if(newElement->tagName() != useTag)
        targetElement->cloneChildren(newElement);
    return newElement;
}

SVGImageElement::SVGImageElement(Document* document)
    : SVGGraphicsElement(document, imageTag)
    , SVGURIReference(this)
    , m_x(0.f, SVGLengthType::Number, SVGLengthDirection::Horizontal, SVGLengthNegativeValuesMode::Allow)
    , m_y(0.f, SVGLengthType::Number, SVGLengthDirection::Vertical, SVGLengthNegativeValuesMode::Allow)
    , m_width(100.f, SVGLengthType::Percentage, SVGLengthDirection::Horizontal, SVGLengthNegativeValuesMode::Forbid)
    , m_height(100.f, SVGLengthType::Percentage, SVGLengthDirection::Vertical, SVGLengthNegativeValuesMode::Forbid)
{
    addProperty(xAttr, m_x);
    addProperty(yAttr, m_y);
    addProperty(widthAttr, m_width);
    addProperty(heightAttr, m_height);
    addProperty(preserveAspectRatioAttr, m_preserveAspectRatio);
}

Box* SVGImageElement::createBox(const RefPtr<BoxStyle>& style)
{
    return new (heap()) SVGImageBox(this, style);
}

RefPtr<Image> SVGImageElement::image() const
{
    auto url = document()->completeUrl(href());
    if(auto resource = document()->fetchImageResource(url))
        return resource->image();
    return nullptr;
}

SVGSymbolElement::SVGSymbolElement(Document* document)
    : SVGGraphicsElement(document, symbolTag)
    , SVGFitToViewBox(this)
{
}

Box* SVGSymbolElement::createBox(const RefPtr<BoxStyle>& style)
{
    return new (heap()) SVGHiddenContainerBox(this, style);
}

SVGAElement::SVGAElement(Document* document)
    : SVGGraphicsElement(document, aTag)
    , SVGURIReference(this)
{
}

Box* SVGAElement::createBox(const RefPtr<BoxStyle>& style)
{
    return new (heap()) SVGTransformableContainerBox(this, style);
}

SVGGElement::SVGGElement(Document* document)
    : SVGGraphicsElement(document, gTag)
{
}

Box* SVGGElement::createBox(const RefPtr<BoxStyle>& style)
{
    return new (heap()) SVGTransformableContainerBox(this, style);
}

SVGDefsElement::SVGDefsElement(Document* document)
    : SVGGraphicsElement(document, defsTag)
{
}

Box* SVGDefsElement::createBox(const RefPtr<BoxStyle>& style)
{
    return new (heap()) SVGHiddenContainerBox(this, style);
}

SVGGeometryElement::SVGGeometryElement(Document* document, const GlobalString& tagName)
    : SVGGraphicsElement(document, tagName)
{
}

SVGResourceMarkerBox* SVGGeometryElement::getMarker(const std::string_view& id) const
{
    return to<SVGResourceMarkerBox>(getResourceById(id));
}

SVGPathElement::SVGPathElement(Document* document)
    : SVGGeometryElement(document, pathTag)
{
    addProperty(dAttr, m_d);
}

Box* SVGPathElement::createBox(const RefPtr<BoxStyle>& style)
{
    return new (heap()) SVGPathBox(this, style);
}

SVGShapeElement::SVGShapeElement(Document* document, const GlobalString& tagName)
    : SVGGeometryElement(document, tagName)
{
}

Box* SVGShapeElement::createBox(const RefPtr<BoxStyle> &style)
{
    return new (heap()) SVGShapeBox(this, style);
}

SVGLineElement::SVGLineElement(Document* document)
    : SVGShapeElement(document, lineTag)
    , m_x1(SVGLengthDirection::Horizontal, SVGLengthNegativeValuesMode::Allow)
    , m_y1(SVGLengthDirection::Vertical, SVGLengthNegativeValuesMode::Allow)
    , m_x2(SVGLengthDirection::Horizontal, SVGLengthNegativeValuesMode::Allow)
    , m_y2(SVGLengthDirection::Vertical, SVGLengthNegativeValuesMode::Allow)
{
    addProperty(x1Attr, m_x1);
    addProperty(y1Attr, m_y1);
    addProperty(x2Attr, m_x2);
    addProperty(y2Attr, m_y2);
}

Rect SVGLineElement::getPath(Path& path) const
{
    SVGLengthContext lengthContext(this);
    auto x1 = lengthContext.valueForLength(m_x1);
    auto y1 = lengthContext.valueForLength(m_y1);
    auto x2 = lengthContext.valueForLength(m_x2);
    auto y2 = lengthContext.valueForLength(m_y2);

    path.moveTo(x1, y1);
    path.lineTo(x2, y2);
    return Rect(x1, y1, x2 - x1, y2 - y1);
}

SVGRectElement::SVGRectElement(Document* document)
    : SVGShapeElement(document, rectTag)
    , m_x(SVGLengthDirection::Horizontal, SVGLengthNegativeValuesMode::Allow)
    , m_y(SVGLengthDirection::Vertical, SVGLengthNegativeValuesMode::Allow)
    , m_width(SVGLengthDirection::Horizontal, SVGLengthNegativeValuesMode::Forbid)
    , m_height(SVGLengthDirection::Vertical, SVGLengthNegativeValuesMode::Forbid)
    , m_rx(SVGLengthDirection::Horizontal, SVGLengthNegativeValuesMode::Forbid)
    , m_ry(SVGLengthDirection::Vertical, SVGLengthNegativeValuesMode::Forbid)
{
    addProperty(xAttr, m_x);
    addProperty(yAttr, m_y);
    addProperty(widthAttr, m_width);
    addProperty(heightAttr, m_height);
    addProperty(rxAttr, m_rx);
    addProperty(ryAttr, m_ry);
}

Rect SVGRectElement::getPath(Path& path) const
{
    SVGLengthContext lengthContext(this);
    auto width = lengthContext.valueForLength(m_width);
    auto height = lengthContext.valueForLength(m_height);
    if(width <= 0.f || height <= 0.f) {
        return Rect::Empty;
    }

    auto x = lengthContext.valueForLength(m_x);
    auto y = lengthContext.valueForLength(m_y);

    auto rx = lengthContext.valueForLength(m_rx);
    auto ry = lengthContext.valueForLength(m_ry);

    if(rx <= 0.f) rx = ry;
    if(ry <= 0.f) ry = rx;

    rx = std::min(rx, width / 2.f);
    ry = std::min(ry, height / 2.f);

    path.addRoundedRect(Rect(x, y, width, height), RectRadii(rx, ry));
    return Rect(x, y, width, height);
}

SVGCircleElement::SVGCircleElement(Document* document)
    : SVGShapeElement(document, circleTag)
    , m_cx(SVGLengthDirection::Horizontal, SVGLengthNegativeValuesMode::Allow)
    , m_cy(SVGLengthDirection::Vertical, SVGLengthNegativeValuesMode::Allow)
    , m_r(SVGLengthDirection::Diagonal, SVGLengthNegativeValuesMode::Forbid)
{
    addProperty(cxAttr, m_cx);
    addProperty(cyAttr, m_cy);
    addProperty(rAttr, m_r);
}

Rect SVGCircleElement::getPath(Path& path) const
{
    SVGLengthContext lengthContext(this);
    auto r = lengthContext.valueForLength(m_r);
    if(r <= 0.f) {
        return Rect::Empty;
    }

    auto cx = lengthContext.valueForLength(m_cx);
    auto cy = lengthContext.valueForLength(m_cy);
    path.addEllipse(cx, cy, r, r);
    return Rect(cx - r, cy - r, r + r, r + r);
}

SVGEllipseElement::SVGEllipseElement(Document* document)
    : SVGShapeElement(document, ellipseTag)
    , m_cx(SVGLengthDirection::Horizontal, SVGLengthNegativeValuesMode::Allow)
    , m_cy(SVGLengthDirection::Vertical, SVGLengthNegativeValuesMode::Allow)
    , m_rx(SVGLengthDirection::Horizontal, SVGLengthNegativeValuesMode::Forbid)
    , m_ry(SVGLengthDirection::Vertical, SVGLengthNegativeValuesMode::Forbid)
{
    addProperty(cxAttr, m_cx);
    addProperty(cyAttr, m_cy);
    addProperty(rxAttr, m_rx);
    addProperty(ryAttr, m_ry);
}

Rect SVGEllipseElement::getPath(Path& path) const
{
    SVGLengthContext lengthContext(this);
    auto rx = lengthContext.valueForLength(m_rx);
    auto ry = lengthContext.valueForLength(m_ry);
    if(rx <= 0.f || ry <= 0.f) {
        return Rect::Empty;
    }

    auto cx = lengthContext.valueForLength(m_cx);
    auto cy = lengthContext.valueForLength(m_cy);
    path.addEllipse(cx, cy, rx, ry);
    return Rect(cx - rx, cy - ry, rx + rx, ry + ry);
}

SVGPolyElement::SVGPolyElement(Document* document, const GlobalString& tagName)
    : SVGShapeElement(document, tagName)
{
    addProperty(pointsAttr, m_points);
}

Rect SVGPolyElement::getPath(Path& path) const
{
    const auto& points = m_points.values();
    if(points.empty()) {
        return Rect::Empty;
    }

    path.moveTo(points[0].x, points[0].y);
    for(size_t i = 1; i < points.size(); i++) {
        path.lineTo(points[i].x, points[i].y);
    }

    if(tagName() == polygonTag)
        path.close();
    return path.boundingRect();
}

SVGTextPositioningElement::SVGTextPositioningElement(Document* document, const GlobalString& tagName)
    : SVGGraphicsElement(document, tagName)
    , m_x(SVGLengthDirection::Horizontal, SVGLengthNegativeValuesMode::Allow)
    , m_y(SVGLengthDirection::Vertical, SVGLengthNegativeValuesMode::Allow)
    , m_dx(SVGLengthDirection::Horizontal, SVGLengthNegativeValuesMode::Allow)
    , m_dy(SVGLengthDirection::Vertical, SVGLengthNegativeValuesMode::Allow)
{
    addProperty(xAttr, m_x);
    addProperty(yAttr, m_y);
    addProperty(dxAttr, m_dx);
    addProperty(dyAttr, m_dy);
    addProperty(rotateAttr, m_rotate);
}

SVGTSpanElement::SVGTSpanElement(Document* document)
    : SVGTextPositioningElement(document, tspanTag)
{
}

Box* SVGTSpanElement::createBox(const RefPtr<BoxStyle>& style)
{
    return new (heap()) SVGTSpanBox(this, style);
}

SVGTextElement::SVGTextElement(Document* document)
    : SVGTextPositioningElement(document, textTag)
{
}

Box* SVGTextElement::createBox(const RefPtr<BoxStyle>& style)
{
    return new (heap()) SVGTextBox(this, style);
}

SVGMarkerElement::SVGMarkerElement(Document* document)
    : SVGElement(document, markerTag)
    , SVGFitToViewBox(this)
    , m_refX(0.f, SVGLengthType::Number, SVGLengthDirection::Horizontal, SVGLengthNegativeValuesMode::Allow)
    , m_refY(0.f, SVGLengthType::Number, SVGLengthDirection::Vertical, SVGLengthNegativeValuesMode::Allow)
    , m_markerWidth(3.f, SVGLengthType::Number, SVGLengthDirection::Horizontal, SVGLengthNegativeValuesMode::Forbid)
    , m_markerHeight(3.f, SVGLengthType::Number, SVGLengthDirection::Vertical, SVGLengthNegativeValuesMode::Forbid)
    , m_markerUnits(SVGMarkerUnitsTypeStrokeWidth)
{
    addProperty(refXAttr, m_refX);
    addProperty(refYAttr, m_refY);
    addProperty(markerWidthAttr, m_markerWidth);
    addProperty(markerHeightAttr, m_markerHeight);
    addProperty(markerUnitsAttr, m_markerUnits);
    addProperty(orientAttr, m_orient);
}

Box* SVGMarkerElement::createBox(const RefPtr<BoxStyle>& style)
{
    return new (heap()) SVGResourceMarkerBox(this, style);
}

SVGClipPathElement::SVGClipPathElement(Document* document)
    : SVGGraphicsElement(document, clipPathTag)
    , m_clipPathUnits(SVGUnitsTypeUserSpaceOnUse)
{
    addProperty(clipPathUnitsAttr, m_clipPathUnits);
}

Box* SVGClipPathElement::createBox(const RefPtr<BoxStyle>& style)
{
    return new (heap()) SVGResourceClipperBox(this, style);
}

SVGMaskElement::SVGMaskElement(Document* document)
    : SVGElement(document, maskTag)
    , m_x(-10.f, SVGLengthType::Percentage, SVGLengthDirection::Horizontal, SVGLengthNegativeValuesMode::Allow)
    , m_y(-10.f, SVGLengthType::Percentage, SVGLengthDirection::Vertical, SVGLengthNegativeValuesMode::Allow)
    , m_width(120.f, SVGLengthType::Percentage, SVGLengthDirection::Horizontal, SVGLengthNegativeValuesMode::Forbid)
    , m_height(120.f, SVGLengthType::Percentage, SVGLengthDirection::Vertical, SVGLengthNegativeValuesMode::Forbid)
    , m_maskUnits(SVGUnitsTypeObjectBoundingBox)
    , m_maskContentUnits(SVGUnitsTypeUserSpaceOnUse)
{
    addProperty(xAttr, m_x);
    addProperty(yAttr, m_y);
    addProperty(widthAttr, m_width);
    addProperty(heightAttr, m_height);
    addProperty(maskUnitsAttr, m_maskUnits);
    addProperty(maskContentUnitsAttr, m_maskContentUnits);
}

Box* SVGMaskElement::createBox(const RefPtr<BoxStyle>& style)
{
    return new (heap()) SVGResourceMaskerBox(this, style);
}

SVGPatternElement::SVGPatternElement(Document* document)
    : SVGElement(document, patternTag)
    , SVGURIReference(this)
    , SVGFitToViewBox(this)
    , m_x(SVGLengthDirection::Horizontal, SVGLengthNegativeValuesMode::Allow)
    , m_y(SVGLengthDirection::Vertical, SVGLengthNegativeValuesMode::Allow)
    , m_width(SVGLengthDirection::Horizontal, SVGLengthNegativeValuesMode::Forbid)
    , m_height(SVGLengthDirection::Vertical, SVGLengthNegativeValuesMode::Forbid)
    , m_patternUnits(SVGUnitsTypeObjectBoundingBox)
    , m_patternContentUnits(SVGUnitsTypeUserSpaceOnUse)
{
    addProperty(xAttr, m_x);
    addProperty(yAttr, m_y);
    addProperty(widthAttr, m_width);
    addProperty(heightAttr, m_height);
    addProperty(patternTransformAttr, m_patternTransform);
    addProperty(patternUnitsAttr, m_patternUnits);
    addProperty(patternContentUnitsAttr, m_patternContentUnits);
}

SVGPatternAttributes SVGPatternElement::collectPatternAttributes() const
{
    SVGPatternAttributes attributes;
    std::set<const SVGPatternElement*> processedPatterns;
    const SVGPatternElement* current = this;
    while(true) {
        if(!attributes.hasX() && current->hasAttribute(xAttr))
            attributes.setX(current);
        if(!attributes.hasY() && current->hasAttribute(yAttr))
            attributes.setY(current);
        if(!attributes.hasWidth() && current->hasAttribute(widthAttr))
            attributes.setWidth(current);
        if(!attributes.hasHeight() && current->hasAttribute(heightAttr))
            attributes.setHeight(current);
        if(!attributes.hasPatternTransform() && current->hasAttribute(patternTransformAttr))
            attributes.setPatternTransform(current);
        if(!attributes.hasPatternUnits() && current->hasAttribute(patternUnitsAttr))
            attributes.setPatternUnits(current);
        if(!attributes.hasPatternContentUnits() && current->hasAttribute(patternContentUnitsAttr))
            attributes.setPatternContentUnits(current);
        if(!attributes.hasViewBox() && current->hasAttribute(viewBoxAttr))
            attributes.setViewBox(current);
        if(!attributes.hasPreserveAspectRatio() && current->hasAttribute(preserveAspectRatioAttr))
            attributes.setPreserveAspectRatio(current);
        if(!attributes.hasPatternContentElement() && current->box()) {
            for(auto child = current->firstChild(); child; child = child->nextSibling()) {
                if(child->isSVGElement()) {
                    attributes.setPatternContentElement(current);
                    break;
                }
            }
        }

        auto targetElement = current->getTargetElement(document());
        if(!targetElement || targetElement->tagName() != patternTag)
            break;
        processedPatterns.insert(current);
        current = static_cast<const SVGPatternElement*>(targetElement);
        if(processedPatterns.contains(current)) {
            break;
        }
    }

    attributes.setDefaultValues(this);
    return attributes;
}

Box* SVGPatternElement::createBox(const RefPtr<BoxStyle>& style)
{
    return new (heap()) SVGResourcePatternBox(this, style);
}

SVGStopElement::SVGStopElement(Document* document)
    : SVGElement(document, stopTag)
{
    addProperty(offsetAttr, m_offset);
}

Color SVGStopElement::stopColorIncludingOpacity() const
{
    if(const auto* stopStyle = style())
        return stopStyle->stopColor().colorWithAlpha(stopStyle->stopOpacity());
    return Color::Transparent;
}

Box* SVGStopElement::createBox(const RefPtr<BoxStyle>& style)
{
    return new (heap()) SVGGradientStopBox(this, style);
}

SVGGradientElement::SVGGradientElement(Document* document, const GlobalString& tagName)
    : SVGElement(document, tagName)
    , SVGURIReference(this)
    , m_gradientUnits(SVGUnitsTypeObjectBoundingBox)
    , m_spreadMethod(SVGSpreadMethodTypePad)
{
    addProperty(gradientTransformAttr, m_gradientTransform);
    addProperty(gradientUnitsAttr, m_gradientUnits);
    addProperty(spreadMethodAttr, m_spreadMethod);
}

void SVGGradientElement::collectGradientAttributes(SVGGradientAttributes& attributes) const
{
    if(!attributes.hasGradientTransform() && hasAttribute(gradientTransformAttr))
        attributes.setGradientTransform(this);
    if(!attributes.hasSpreadMethod() && hasAttribute(spreadMethodAttr))
        attributes.setSpreadMethod(this);
    if(!attributes.hasGradientUnits() && hasAttribute(gradientUnitsAttr))
        attributes.setGradientUnits(this);
    if(!attributes.hasGradientContentElement()) {
        for(auto child = firstChild(); child; child = child->nextSibling()) {
            if(child->isOfType(svgNs, stopTag)) {
                attributes.setGradientContentElement(this);
                break;
            }
        }
    }
}

SVGLinearGradientElement::SVGLinearGradientElement(Document* document)
    : SVGGradientElement(document, linearGradientTag)
    , m_x1(0.f, SVGLengthType::Percentage, SVGLengthDirection::Horizontal, SVGLengthNegativeValuesMode::Allow)
    , m_y1(0.f, SVGLengthType::Percentage, SVGLengthDirection::Vertical, SVGLengthNegativeValuesMode::Allow)
    , m_x2(100.f, SVGLengthType::Percentage, SVGLengthDirection::Horizontal, SVGLengthNegativeValuesMode::Allow)
    , m_y2(0.f, SVGLengthType::Percentage, SVGLengthDirection::Vertical, SVGLengthNegativeValuesMode::Allow)
{
    addProperty(x1Attr, m_x1);
    addProperty(y1Attr, m_y1);
    addProperty(x2Attr, m_x2);
    addProperty(y2Attr, m_y2);
}

SVGLinearGradientAttributes SVGLinearGradientElement::collectGradientAttributes() const
{
    SVGLinearGradientAttributes attributes;
    std::set<const SVGGradientElement*> processedGradients;
    const SVGGradientElement* current = this;
    while(true) {
        current->collectGradientAttributes(attributes);
        if(current->tagName() == linearGradientTag) {
            auto element = static_cast<const SVGLinearGradientElement*>(current);
            if(!attributes.hasX1() && element->hasAttribute(x1Attr))
                attributes.setX1(element);
            if(!attributes.hasY1() && element->hasAttribute(y1Attr))
                attributes.setY1(element);
            if(!attributes.hasX2() && element->hasAttribute(x2Attr))
                attributes.setX2(element);
            if(!attributes.hasY2() && element->hasAttribute(y2Attr)) {
                attributes.setY2(element);
            }
        }

        auto targetElement = current->getTargetElement(document());
        if(!targetElement || !(targetElement->tagName() == linearGradientTag || targetElement->tagName() == radialGradientTag))
            break;
        processedGradients.insert(current);
        current = static_cast<const SVGGradientElement*>(targetElement);
        if(processedGradients.contains(current)) {
            break;
        }
    }

    attributes.setDefaultValues(this);
    return attributes;
}

Box* SVGLinearGradientElement::createBox(const RefPtr<BoxStyle>& style)
{
    return new (heap()) SVGResourceLinearGradientBox(this, style);
}

SVGRadialGradientElement::SVGRadialGradientElement(Document* document)
    : SVGGradientElement(document, radialGradientTag)
    , m_cx(50.f, SVGLengthType::Percentage, SVGLengthDirection::Horizontal, SVGLengthNegativeValuesMode::Allow)
    , m_cy(50.f, SVGLengthType::Percentage, SVGLengthDirection::Vertical, SVGLengthNegativeValuesMode::Allow)
    , m_r(50.f, SVGLengthType::Percentage, SVGLengthDirection::Diagonal, SVGLengthNegativeValuesMode::Forbid)
    , m_fx(0.f, SVGLengthType::Number, SVGLengthDirection::Horizontal, SVGLengthNegativeValuesMode::Allow)
    , m_fy(0.f, SVGLengthType::Number, SVGLengthDirection::Vertical, SVGLengthNegativeValuesMode::Allow)
{
    addProperty(cxAttr, m_cx);
    addProperty(cyAttr, m_cy);
    addProperty(rAttr, m_r);
    addProperty(fxAttr, m_fx);
    addProperty(fyAttr, m_fy);
}

SVGRadialGradientAttributes SVGRadialGradientElement::collectGradientAttributes() const
{
    SVGRadialGradientAttributes attributes;
    std::set<const SVGGradientElement*> processedGradients;
    const SVGGradientElement* current = this;
    while(true) {
        current->collectGradientAttributes(attributes);
        if(current->tagName() == radialGradientTag) {
            auto element = static_cast<const SVGRadialGradientElement*>(current);
            if(!attributes.hasCx() && element->hasAttribute(cxAttr))
                attributes.setCx(element);
            if(!attributes.hasCy() && element->hasAttribute(cyAttr))
                attributes.setCy(element);
            if(!attributes.hasR() && element->hasAttribute(rAttr))
                attributes.setR(element);
            if(!attributes.hasFx() && element->hasAttribute(fxAttr))
                attributes.setFx(element);
            if(!attributes.hasFy() && element->hasAttribute(fyAttr)) {
                attributes.setFy(element);
            }
        }

        auto targetElement = current->getTargetElement(document());
        if(!targetElement || !(targetElement->tagName() == linearGradientTag || targetElement->tagName() == radialGradientTag))
            break;
        processedGradients.insert(current);
        current = static_cast<const SVGGradientElement*>(targetElement);
        if(processedGradients.contains(current)) {
            break;
        }
    }

    attributes.setDefaultValues(this);
    return attributes;
}

Box* SVGRadialGradientElement::createBox(const RefPtr<BoxStyle>& style)
{
    return new (heap()) SVGResourceRadialGradientBox(this, style);
}

SVGStyleElement::SVGStyleElement(Document* document)
    : SVGElement(document, styleTag)
{
}

const HeapString& SVGStyleElement::type() const
{
    return getAttribute(typeAttr);
}

const HeapString& SVGStyleElement::media() const
{
    return getAttribute(mediaAttr);
}

void SVGStyleElement::finishParsingDocument()
{
    if(document()->supportsMedia(type(), media()))
        document()->addAuthorStyleSheet(textFromChildren(), document()->baseUrl());
    SVGElement::finishParsingDocument();
}

std::unique_ptr<SVGDocument> SVGDocument::create(Book* book, Heap* heap, ResourceFetcher* fetcher, Url baseUrl)
{
    return std::unique_ptr<SVGDocument>(new (heap) SVGDocument(book, heap, fetcher, std::move(baseUrl)));
}

SVGDocument::SVGDocument(Book* book, Heap* heap, ResourceFetcher* fetcher, Url baseUrl)
    : XMLDocument(book, heap, fetcher, std::move(baseUrl))
{
}

} // namespace plutobook
