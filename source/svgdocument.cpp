#include "svgdocument.h"
#include "svgreplacedbox.h"
#include "svgresourcebox.h"
#include "svgshapebox.h"
#include "svgtextbox.h"
#include "imageresource.h"

#include <set>
#include <numbers>
#include <cmath>

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
        alignment_baselineAttr,
        baseline_shiftAttr,
        clipAttr,
        clip_pathAttr,
        clip_ruleAttr,
        colorAttr,
        directionAttr,
        displayAttr,
        dominant_baselineAttr,
        fillAttr,
        fill_opacityAttr,
        fill_ruleAttr,
        font_familyAttr,
        font_sizeAttr,
        font_stretchAttr,
        font_styleAttr,
        font_variantAttr,
        font_weightAttr,
        letter_spacingAttr,
        marker_endAttr,
        marker_midAttr,
        marker_startAttr,
        maskAttr,
        mask_typeAttr,
        opacityAttr,
        overflowAttr,
        paint_orderAttr,
        stop_colorAttr,
        stop_opacityAttr,
        strokeAttr,
        stroke_dasharrayAttr,
        stroke_dashoffsetAttr,
        stroke_linecapAttr,
        stroke_linejoinAttr,
        stroke_miterlimitAttr,
        stroke_opacityAttr,
        stroke_widthAttr,
        text_anchorAttr,
        text_decorationAttr,
        transform_originAttr,
        unicode_bidiAttr,
        vector_effectAttr,
        visibilityAttr,
        word_spacingAttr,
        writing_modeAttr
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
        auto element = static_cast<const SVGSVGElement*>(this);
        auto& viewBoxRect = element->viewBox();
        if(viewBoxRect.isValid())
            return viewBoxRect.size();
        return Size(300, 150);
    }

    if(parent->tagName() == svgTag) {
        auto element = static_cast<const SVGSVGElement*>(parent);
        auto& viewBoxRect = element->viewBox();
        if(viewBoxRect.isValid())
            return viewBoxRect.size();
        SVGLengthContext lengthContext(element);
        auto width = lengthContext.valueForLength(element->width());
        auto height = lengthContext.valueForLength(element->height());
        return Size(width, height);
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
    auto painter = getPainter(paint.uri());
    auto& color = paint.color();
    return SVGPaintServer(painter, color, opacity);
}

StrokeData SVGGraphicsElement::getStrokeData(const BoxStyle* style) const
{
    if(!style->hasStroke())
        return StrokeData();
    SVGLengthContext lengthContext(this);
    StrokeData strokeData(lengthContext.valueForLength(style->strokeWidth()));
    strokeData.setMiterLimit(style->strokeMiterlimit());
    strokeData.setLineCap(style->strokeLinecap());
    strokeData.setLineJoin(style->strokeLinejoin());
    strokeData.setDashOffset(lengthContext.valueForLength(style->strokeDashoffset()));

    DashArray dashArray;
    for(auto& dash : style->strokeDasharray())
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
    auto& viewBoxRect = m_viewBox.value();
    if(viewBoxRect.isEmpty() || viewportSize.isEmpty())
        return Transform::Identity;
    return m_preserveAspectRatio.getTransform(viewBoxRect, viewportSize);
}

Rect SVGFitToViewBox::getClipRect(const Size& viewportSize) const
{
    auto& viewBoxRect = m_viewBox.value();
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
    , m_x(0.f, SVGLength::UnitType::Number, SVGLength::Direction::Horizontal, SVGLength::NegativeMode::Allow)
    , m_y(0.f, SVGLength::UnitType::Number, SVGLength::Direction::Vertical, SVGLength::NegativeMode::Allow)
    , m_width(100.f, SVGLength::UnitType::Percent, SVGLength::Direction::Horizontal, SVGLength::NegativeMode::Forbid)
    , m_height(100.f, SVGLength::UnitType::Percent, SVGLength::Direction::Vertical, SVGLength::NegativeMode::Forbid)
{
    addProperty(xAttr, m_x);
    addProperty(yAttr, m_y);
    addProperty(widthAttr, m_width);
    addProperty(heightAttr, m_height);
}

void SVGSVGElement::computeIntrinsicRatioInformation(float& intrinsicWidth, float& intrinsicHeight, double& intrinsicRatio) const
{
    SVGLengthContext lengthContext(this);
    if(m_width.unitType() != SVGLength::UnitType::Percent) {
        intrinsicWidth = lengthContext.valueForLength(m_width);
    } else {
        intrinsicWidth = 0.f;
    }

    if(m_height.unitType() != SVGLength::UnitType::Percent) {
        intrinsicHeight = lengthContext.valueForLength(m_height);
    } else {
        intrinsicHeight = 0.f;
    }

    auto& viewBoxRect = viewBox();
    if(intrinsicWidth > 0.f && intrinsicHeight > 0.f) {
        intrinsicRatio = intrinsicWidth / intrinsicHeight;
    } else if(!viewBoxRect.isEmpty()) {
        intrinsicRatio = viewBoxRect.w / viewBoxRect.h;
    } else {
        intrinsicRatio = 0.0;
    }

    if(!document()->isSVGImageDocument())
        return;
    if(intrinsicRatio && (!intrinsicWidth || !intrinsicHeight)) {
        if(!intrinsicWidth && intrinsicHeight)
            intrinsicWidth = intrinsicHeight * intrinsicRatio;
        else if(intrinsicWidth && !intrinsicHeight) {
            intrinsicHeight = intrinsicWidth / intrinsicRatio;
        }
    }

    if(viewBoxRect.isValid() && (!intrinsicWidth || !intrinsicHeight)) {
        intrinsicWidth = viewBoxRect.w;
        intrinsicHeight = viewBoxRect.h;
    }

    auto rootBox = to<SVGRootBox>(box());
    if(rootBox && (!intrinsicWidth || !intrinsicHeight)) {
        auto& boundingRect = rootBox->paintBoundingBox();
        intrinsicWidth = boundingRect.right();
        intrinsicHeight = boundingRect.bottom();
    }
}

static void addSVGTransformAttributeStyle(std::string& output, const Transform& matrix)
{
    output += "transform:matrix(";
    output += std::to_string(matrix.m00);
    output += ',';
    output += std::to_string(matrix.m10);
    output += ',';
    output += std::to_string(matrix.m01);
    output += ',';
    output += std::to_string(matrix.m11);
    output += ',';
    output += std::to_string(matrix.m02);
    output += ',';
    output += std::to_string(matrix.m12);
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
    , m_x(0.f, SVGLength::UnitType::Number, SVGLength::Direction::Horizontal, SVGLength::NegativeMode::Allow)
    , m_y(0.f, SVGLength::UnitType::Number, SVGLength::Direction::Vertical, SVGLength::NegativeMode::Allow)
    , m_width(100.f, SVGLength::UnitType::Percent, SVGLength::Direction::Horizontal, SVGLength::NegativeMode::Forbid)
    , m_height(100.f, SVGLength::UnitType::Percent, SVGLength::Direction::Vertical, SVGLength::NegativeMode::Forbid)
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
    auto& id = targetElement->id();
    auto parent = parentNode();
    while(parent && parent->isSVGElement()) {
        auto& element = to<SVGElement>(*parent);
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
        for(auto& attribute : attributes()) {
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
    , m_x(0.f, SVGLength::UnitType::Number, SVGLength::Direction::Horizontal, SVGLength::NegativeMode::Allow)
    , m_y(0.f, SVGLength::UnitType::Number, SVGLength::Direction::Vertical, SVGLength::NegativeMode::Allow)
    , m_width(100.f, SVGLength::UnitType::Percent, SVGLength::Direction::Horizontal, SVGLength::NegativeMode::Forbid)
    , m_height(100.f, SVGLength::UnitType::Percent, SVGLength::Direction::Vertical, SVGLength::NegativeMode::Forbid)
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

SVGMarkerData SVGGeometryElement::getMarkerData(const Path& path, const BoxStyle* style) const
{
    auto markerStart = getMarker(style->markerStart());
    auto markerMid = getMarker(style->markerMid());
    auto markerEnd = getMarker(style->markerEnd());
    if(markerStart == nullptr && markerMid == nullptr && markerEnd == nullptr) {
        return SVGMarkerData();
    }

    SVGLengthContext lengthContext(this);
    float strokeWidth = lengthContext.valueForLength(style->strokeWidth());
    Point origin;
    Point startPoint;
    Point inslopePoints[2];
    Point outslopePoints[2];

    int index = 0;
    std::array<Point, 3> points;
    SVGMarkerPositionList positions;
    PathIterator it(path);
    while(!it.isDone()) {
        switch(it.currentSegment(points)) {
        case PathCommand::MoveTo:
            startPoint = points[0];
            inslopePoints[0] = origin;
            inslopePoints[1] = points[0];
            origin = points[0];
            break;
        case PathCommand::LineTo:
            inslopePoints[0] = origin;
            inslopePoints[1] = points[0];
            origin = points[0];
            break;
        case PathCommand::CubicTo:
            inslopePoints[0] = points[1];
            inslopePoints[1] = points[2];
            origin = points[2];
            break;
        case PathCommand::Close:
            inslopePoints[0] = origin;
            inslopePoints[1] = points[0];
            origin = startPoint;
            startPoint = Point();
            break;
        }

        it.next();

        if(!it.isDone() && (markerStart || markerMid)) {
            it.currentSegment(points);
            outslopePoints[0] = origin;
            outslopePoints[1] = points[0];
            if(index == 0 && markerStart) {
                auto slope = outslopePoints[1] - outslopePoints[0];
                auto angle = 180.0 * std::atan2(slope.y, slope.x) / std::numbers::pi;
                auto& orient = markerStart->element()->orient();
                if(orient.orientType() == SVGAngle::OrientType::AutoStartReverse)
                    angle -= 180.0;
                positions.emplace_back(markerStart, origin, angle);
            }

            if(index > 0 && markerMid) {
                auto inslope = inslopePoints[1] - inslopePoints[0];
                auto outslope = outslopePoints[1] - outslopePoints[0];
                auto inangle = 180.0 * std::atan2(inslope.y, inslope.x) / std::numbers::pi;
                auto outangle = 180.0 * std::atan2(outslope.y, outslope.x) / std::numbers::pi;
                if(std::abs(inangle - outangle) > 180.0)
                    inangle += 360.0;
                auto angle = (inangle + outangle) * 0.5;
                positions.emplace_back(markerMid, origin, angle);
            }
        }

        if(markerEnd && it.isDone()) {
            auto slope = inslopePoints[1] - inslopePoints[0];
            auto angle = 180.0 * std::atan2(slope.y, slope.x) / std::numbers::pi;
            positions.emplace_back(markerEnd, origin, angle);
        }

        index += 1;
    }

    return SVGMarkerData(strokeWidth, std::move(positions));
}

Box* SVGGeometryElement::createBox(const RefPtr<BoxStyle>& style)
{
    return new (heap()) SVGShapeBox(this, style);
}

SVGLineElement::SVGLineElement(Document* document)
    : SVGGeometryElement(document, lineTag)
    , m_x1(SVGLength::Direction::Horizontal, SVGLength::NegativeMode::Allow)
    , m_y1(SVGLength::Direction::Vertical, SVGLength::NegativeMode::Allow)
    , m_x2(SVGLength::Direction::Horizontal, SVGLength::NegativeMode::Allow)
    , m_y2(SVGLength::Direction::Vertical, SVGLength::NegativeMode::Allow)
{
    addProperty(x1Attr, m_x1);
    addProperty(y1Attr, m_y1);
    addProperty(x2Attr, m_x2);
    addProperty(y2Attr, m_y2);
}

Path SVGLineElement::path() const
{
    SVGLengthContext lengthContext(this);
    auto x1 = lengthContext.valueForLength(m_x1);
    auto y1 = lengthContext.valueForLength(m_y1);
    auto x2 = lengthContext.valueForLength(m_x2);
    auto y2 = lengthContext.valueForLength(m_y2);

    Path path;
    path.moveTo(x1, y1);
    path.lineTo(x2, y2);
    return path;
}

SVGRectElement::SVGRectElement(Document* document)
    : SVGGeometryElement(document, rectTag)
    , m_x(SVGLength::Direction::Horizontal, SVGLength::NegativeMode::Allow)
    , m_y(SVGLength::Direction::Vertical, SVGLength::NegativeMode::Allow)
    , m_width(SVGLength::Direction::Horizontal, SVGLength::NegativeMode::Forbid)
    , m_height(SVGLength::Direction::Vertical, SVGLength::NegativeMode::Forbid)
    , m_rx(SVGLength::Direction::Horizontal, SVGLength::NegativeMode::Forbid)
    , m_ry(SVGLength::Direction::Vertical, SVGLength::NegativeMode::Forbid)
{
    addProperty(xAttr, m_x);
    addProperty(yAttr, m_y);
    addProperty(widthAttr, m_width);
    addProperty(heightAttr, m_height);
    addProperty(rxAttr, m_rx);
    addProperty(ryAttr, m_ry);
}

Path SVGRectElement::path() const
{
    Path path;
    SVGLengthContext lengthContext(this);
    auto width = lengthContext.valueForLength(m_width);
    auto height = lengthContext.valueForLength(m_height);
    if(width > 0.f && height > 0.f) {
        auto x = lengthContext.valueForLength(m_x);
        auto y = lengthContext.valueForLength(m_y);

        auto rx = lengthContext.valueForLength(m_rx);
        auto ry = lengthContext.valueForLength(m_ry);

        if(rx == 0.f) rx = ry;
        if(ry == 0.f) ry = rx;

        rx = std::min(rx, width / 2.f);
        ry = std::min(ry, height / 2.f);

        path.addRoundedRect(Rect(x, y, width, height), RectRadii(rx, ry));
    }

    return path;
}

SVGCircleElement::SVGCircleElement(Document* document)
    : SVGGeometryElement(document, circleTag)
    , m_cx(SVGLength::Direction::Horizontal, SVGLength::NegativeMode::Allow)
    , m_cy(SVGLength::Direction::Vertical, SVGLength::NegativeMode::Allow)
    , m_r(SVGLength::Direction::Diagonal, SVGLength::NegativeMode::Forbid)
{
    addProperty(cxAttr, m_cx);
    addProperty(cyAttr, m_cy);
    addProperty(rAttr, m_r);
}

Path SVGCircleElement::path() const
{
    Path path;
    SVGLengthContext lengthContext(this);
    auto r = lengthContext.valueForLength(m_r);
    if(r > 0.f) {
        auto cx = lengthContext.valueForLength(m_cx);
        auto cy = lengthContext.valueForLength(m_cy);
        path.addEllipse(cx, cy, r, r);
    }

    return path;
}

SVGEllipseElement::SVGEllipseElement(Document* document)
    : SVGGeometryElement(document, ellipseTag)
    , m_cx(SVGLength::Direction::Horizontal, SVGLength::NegativeMode::Allow)
    , m_cy(SVGLength::Direction::Vertical, SVGLength::NegativeMode::Allow)
    , m_rx(SVGLength::Direction::Horizontal, SVGLength::NegativeMode::Forbid)
    , m_ry(SVGLength::Direction::Vertical, SVGLength::NegativeMode::Forbid)
{
    addProperty(cxAttr, m_cx);
    addProperty(cyAttr, m_cy);
    addProperty(rxAttr, m_rx);
    addProperty(ryAttr, m_ry);
}

Path SVGEllipseElement::path() const
{
    Path path;
    SVGLengthContext lengthContext(this);
    auto rx = lengthContext.valueForLength(m_rx);
    auto ry = lengthContext.valueForLength(m_ry);
    if(rx > 0.f && ry > 0.f) {
        auto cx = lengthContext.valueForLength(m_cx);
        auto cy = lengthContext.valueForLength(m_cy);
        path.addEllipse(cx, cy, rx, ry);
    }

    return path;
}

SVGPolyElement::SVGPolyElement(Document* document, const GlobalString& tagName)
    : SVGGeometryElement(document, tagName)
{
    addProperty(pointsAttr, m_points);
}

SVGPolylineElement::SVGPolylineElement(Document* document)
    : SVGPolyElement(document, polylineTag)
{
}

Path SVGPolylineElement::path() const
{
    Path path;
    auto& points = this->points();
    if(!points.empty()) {
        path.moveTo(points[0].x, points[0].y);
        for(size_t i = 1; i < points.size(); i++) {
            path.lineTo(points[i].x, points[i].y);
        }
    }

    return path;
}

SVGPolygonElement::SVGPolygonElement(Document* document)
    : SVGPolyElement(document, polygonTag)
{
}

Path SVGPolygonElement::path() const
{
    Path path;
    auto& points = this->points();
    if(!points.empty()) {
        path.moveTo(points[0].x, points[0].y);
        for(size_t i = 1; i < points.size(); i++)
            path.lineTo(points[i].x, points[i].y);
        path.close();
    }

    return path;
}

SVGPathElement::SVGPathElement(Document* document)
    : SVGGeometryElement(document, pathTag)
{
    addProperty(dAttr, m_d);
}

SVGTextPositioningElement::SVGTextPositioningElement(Document* document, const GlobalString& tagName)
    : SVGGraphicsElement(document, tagName)
    , m_x(SVGLength::Direction::Horizontal, SVGLength::NegativeMode::Allow)
    , m_y(SVGLength::Direction::Vertical, SVGLength::NegativeMode::Allow)
    , m_dx(SVGLength::Direction::Horizontal, SVGLength::NegativeMode::Allow)
    , m_dy(SVGLength::Direction::Vertical, SVGLength::NegativeMode::Allow)
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
    , m_refX(0.f, SVGLength::UnitType::Number, SVGLength::Direction::Horizontal, SVGLength::NegativeMode::Allow)
    , m_refY(0.f, SVGLength::UnitType::Number, SVGLength::Direction::Vertical, SVGLength::NegativeMode::Allow)
    , m_markerWidth(3.f, SVGLength::UnitType::Number, SVGLength::Direction::Horizontal, SVGLength::NegativeMode::Forbid)
    , m_markerHeight(3.f, SVGLength::UnitType::Number, SVGLength::Direction::Vertical, SVGLength::NegativeMode::Forbid)
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
    , m_x(-10.f, SVGLength::UnitType::Percent, SVGLength::Direction::Horizontal, SVGLength::NegativeMode::Allow)
    , m_y(-10.f, SVGLength::UnitType::Percent, SVGLength::Direction::Vertical, SVGLength::NegativeMode::Allow)
    , m_width(120.f, SVGLength::UnitType::Percent, SVGLength::Direction::Horizontal, SVGLength::NegativeMode::Forbid)
    , m_height(120.f, SVGLength::UnitType::Percent, SVGLength::Direction::Vertical, SVGLength::NegativeMode::Forbid)
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
    , m_x(SVGLength::Direction::Horizontal, SVGLength::NegativeMode::Allow)
    , m_y(SVGLength::Direction::Vertical, SVGLength::NegativeMode::Allow)
    , m_width(SVGLength::Direction::Horizontal, SVGLength::NegativeMode::Forbid)
    , m_height(SVGLength::Direction::Vertical, SVGLength::NegativeMode::Forbid)
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
    if(const auto* style = this->style())
        return style->stopColor().colorWithAlpha(style->stopOpacity());
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
    , m_x1(0.f, SVGLength::UnitType::Percent, SVGLength::Direction::Horizontal, SVGLength::NegativeMode::Allow)
    , m_y1(0.f, SVGLength::UnitType::Percent, SVGLength::Direction::Vertical, SVGLength::NegativeMode::Allow)
    , m_x2(100.f, SVGLength::UnitType::Percent, SVGLength::Direction::Horizontal, SVGLength::NegativeMode::Allow)
    , m_y2(0.f, SVGLength::UnitType::Percent, SVGLength::Direction::Vertical, SVGLength::NegativeMode::Allow)
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
    , m_cx(50.f, SVGLength::UnitType::Percent, SVGLength::Direction::Horizontal, SVGLength::NegativeMode::Allow)
    , m_cy(50.f, SVGLength::UnitType::Percent, SVGLength::Direction::Vertical, SVGLength::NegativeMode::Allow)
    , m_r(50.f, SVGLength::UnitType::Percent, SVGLength::Direction::Diagonal, SVGLength::NegativeMode::Forbid)
    , m_fx(0.f, SVGLength::UnitType::Number, SVGLength::Direction::Horizontal, SVGLength::NegativeMode::Allow)
    , m_fy(0.f, SVGLength::UnitType::Number, SVGLength::Direction::Vertical, SVGLength::NegativeMode::Allow)
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

std::unique_ptr<SVGDocument> SVGDocument::create(Book* book, Heap* heap, ResourceFetcher* fetcher, Url url)
{
    return std::unique_ptr<SVGDocument>(new (heap) SVGDocument(book, heap, fetcher, std::move(url)));
}

SVGDocument::SVGDocument(Book* book, Heap* heap, ResourceFetcher* fetcher, Url url)
    : XMLDocument(book, heap, fetcher, std::move(url))
{
}

} // namespace plutobook
