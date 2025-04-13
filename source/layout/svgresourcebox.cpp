#include "svgresourcebox.h"
#include "svggeometrybox.h"

#include <cairo.h>

namespace plutobook {

SVGResourceMarkerBox::SVGResourceMarkerBox(SVGMarkerElement* element, const RefPtr<BoxStyle>& style)
    : SVGResourceContainerBox(element, style)
{
    setOverflowHidden(style->isOverflowHidden());
}

Point SVGResourceMarkerBox::refPoint() const
{
    SVGLengthContext lengthContext(element());
    const Point refPoint = {
        lengthContext.valueForLength(element()->refX()),
        lengthContext.valueForLength(element()->refY())
    };

    return refPoint;
}

Size SVGResourceMarkerBox::markerSize() const
{
    SVGLengthContext lengthContext(element());
    const Size markerSize = {
        lengthContext.valueForLength(element()->markerWidth()),
        lengthContext.valueForLength(element()->markerHeight())
    };

    return markerSize;
}

Transform SVGResourceMarkerBox::markerTransform(const Point& origin, float angle, float strokeWidth) const
{
    auto& orient = element()->orient();
    auto transform = Transform::translated(origin.x, origin.y);
    if(orient.orientType() == SVGAngle::OrientType::Angle) {
        transform.rotate(orient.value());
    } else {
        transform.rotate(angle);
    }

    auto reference = m_localTransform.mapPoint(refPoint());
    if(element()->markerUnits() == SVGMarkerUnitsTypeStrokeWidth)
        transform.scale(strokeWidth, strokeWidth);
    transform.translate(-reference.x, -reference.y);
    return transform * m_localTransform;
}

Rect SVGResourceMarkerBox::markerBoundingBox(const Point& origin, float angle, float strokeWidth) const
{
    return markerTransform(origin, angle, strokeWidth).mapRect(paintBoundingBox());
}

void SVGResourceMarkerBox::renderMarker(const SVGRenderState& state, const Point& origin, float angle, float strokeWidth) const
{
    if(state.hasCycleReference(this))
        return;
    SVGBlendInfo blendInfo(m_clipper, m_masker, style());
    SVGRenderState newState(blendInfo, this, state, markerTransform(origin, angle, strokeWidth));
    if(isOverflowHidden())
        newState->clipRect(element()->getClipRect(markerSize()));
    renderChildren(newState);
}

void SVGResourceMarkerBox::layout()
{
    m_localTransform = element()->viewBoxToViewTransform(markerSize());
    SVGResourceContainerBox::layout();
}

SVGResourceClipperBox::SVGResourceClipperBox(SVGClipPathElement* element, const RefPtr<BoxStyle>& style)
    : SVGResourceContainerBox(element, style)
{
}

bool SVGResourceClipperBox::requiresMasking() const
{
    if(m_clipper != nullptr)
        return true;
    const SVGGeometryBox* prevClipShape = nullptr;
    for(auto child = firstChild(); child; child = child->nextSibling()) {
        if(child->style()->visibility() != Visibility::Visible)
            continue;
        const SVGGeometryBox* clipShape = nullptr;
        if(auto container = to<SVGTransformableContainerBox>(child)) {
            if(container->element()->tagName() != useTag)
                continue;
            if(container->clipper())
                return true;
            clipShape = to<SVGGeometryBox>(container->firstChild());
        } else {
            if(child->isSVGTextBox())
                return true;
            clipShape = to<SVGGeometryBox>(child);
        }

        if(clipShape == nullptr)
            continue;
        if(prevClipShape || clipShape->clipper())
            return true;
        prevClipShape = clipShape;
    }

    return false;
}

Rect SVGResourceClipperBox::clipBoundingBox(const Box* box) const
{
    auto clipBoundingBox = paintBoundingBox();
    if(element()->clipPathUnits() == SVGUnitsTypeObjectBoundingBox) {
        auto& bbox = box->fillBoundingBox();
        clipBoundingBox.x = clipBoundingBox.x * bbox.w + bbox.x;
        clipBoundingBox.y = clipBoundingBox.y * bbox.h + bbox.y;
        clipBoundingBox.w = clipBoundingBox.w * bbox.w;
        clipBoundingBox.h = clipBoundingBox.h * bbox.h;
    }

    return element()->transform().mapRect(clipBoundingBox);
}

void SVGResourceClipperBox::applyClipPath(const SVGRenderState& state) const
{
    auto transform = element()->transform();
    if(element()->clipPathUnits() == SVGUnitsTypeObjectBoundingBox) {
        auto& bbox = state.fillBoundingBox();
        transform.translate(bbox.x, bbox.y);
        transform.scale(bbox.w, bbox.h);
    }

    for(auto child = firstChild(); child; child = child->nextSibling()) {
        if(child->style()->visibility() != Visibility::Visible)
            continue;
        Transform clipTransform(transform);
        const SVGGeometryBox* clipShape = nullptr;
        if(auto container = to<SVGTransformableContainerBox>(child)) {
            if(container->element()->tagName() != useTag)
                continue;
            clipTransform.multiply(container->localTransform());
            clipShape = to<SVGGeometryBox>(container->firstChild());
        } else {
            clipShape = to<SVGGeometryBox>(child);
        }

        if(clipShape == nullptr)
            continue;
        auto path = clipShape->path().transformed(clipTransform * clipShape->localTransform());
        state->clipPath(path, clipShape->style()->clipRule());
        return;
    }

    state->clipRect(Rect(0, 0, 0, 0));
}

void SVGResourceClipperBox::applyClipMask(const SVGRenderState& state) const
{
    if(state.hasCycleReference(this))
        return;
    auto maskImage = ImageBuffer::create(state.currentTransform().mapRect(state.paintBoundingBox()));
    GraphicsContext context(maskImage->canvas());
    context.addTransform(state.currentTransform());
    context.addTransform(element()->transform());
    if(element()->clipPathUnits() == SVGUnitsTypeObjectBoundingBox) {
        auto& bbox = state.fillBoundingBox();
        context.translate(bbox.x, bbox.y);
        context.scale(bbox.w, bbox.h);
    }
    {
        SVGBlendInfo blendInfo(m_clipper, nullptr, 1.f, BlendMode::Normal);
        SVGRenderState newState(blendInfo, this, &state, SVGRenderMode::Clipping, context, context.getTransform());
        renderChildren(newState);
    }

    state->applyMask(*maskImage);
}

SVGResourceMaskerBox::SVGResourceMaskerBox(SVGMaskElement* element, const RefPtr<BoxStyle>& style)
    : SVGResourceContainerBox(element, style)
{
}

Rect SVGResourceMaskerBox::maskBoundingBox(const Box* box) const
{
    auto maskBoundingBox = paintBoundingBox();
    if(element()->maskContentUnits() == SVGUnitsTypeObjectBoundingBox) {
        auto& bbox = box->fillBoundingBox();
        maskBoundingBox.x = maskBoundingBox.x * bbox.w + bbox.x;
        maskBoundingBox.y = maskBoundingBox.y * bbox.h + bbox.y;
        maskBoundingBox.w = maskBoundingBox.w * bbox.w;
        maskBoundingBox.h = maskBoundingBox.h * bbox.h;
    }

    SVGLengthContext lengthContext(element(), element()->maskUnits());
    Rect maskRect = {
        lengthContext.valueForLength(element()->x()),
        lengthContext.valueForLength(element()->y()),
        lengthContext.valueForLength(element()->width()),
        lengthContext.valueForLength(element()->height())
    };

    if(element()->maskUnits() == SVGUnitsTypeObjectBoundingBox) {
        auto& bbox = box->fillBoundingBox();
        maskRect.x = maskRect.x * bbox.w + bbox.x;
        maskRect.y = maskRect.y * bbox.h + bbox.y;
        maskRect.w = maskRect.w * bbox.w;
        maskRect.h = maskRect.h * bbox.h;
    }

    return maskBoundingBox.intersected(maskRect);
}

void SVGResourceMaskerBox::applyMask(const SVGRenderState& state) const
{
    if(state.hasCycleReference(this))
        return;
    SVGLengthContext lengthContext(element(), element()->maskUnits());
    Rect maskRect = {
        lengthContext.valueForLength(element()->x()),
        lengthContext.valueForLength(element()->y()),
        lengthContext.valueForLength(element()->width()),
        lengthContext.valueForLength(element()->height())
    };

    if(element()->maskUnits() == SVGUnitsTypeObjectBoundingBox) {
        auto& bbox = state.fillBoundingBox();
        maskRect.x = maskRect.x * bbox.w + bbox.x;
        maskRect.y = maskRect.y * bbox.h + bbox.y;
        maskRect.w = maskRect.w * bbox.w;
        maskRect.h = maskRect.h * bbox.h;
    }

    auto maskImage = ImageBuffer::create(state.currentTransform().mapRect(state.paintBoundingBox()));
    GraphicsContext context(maskImage->canvas());
    context.addTransform(state.currentTransform());
    context.clipRect(maskRect);
    if(element()->maskContentUnits() == SVGUnitsTypeObjectBoundingBox) {
        auto& bbox = state.fillBoundingBox();
        context.translate(bbox.x, bbox.y);
        context.scale(bbox.w, bbox.h);
    }
    {
        SVGBlendInfo blendInfo(m_clipper, m_masker, 1.f, BlendMode::Normal);
        SVGRenderState newState(blendInfo, this, &state, state.mode(), context, context.getTransform());
        renderChildren(newState);
    }

    if(style()->maskType() == MaskType::Luminance)
        maskImage->convertToLuminanceMask();
    state->applyMask(*maskImage);
}

SVGResourcePaintServerBox::SVGResourcePaintServerBox(SVGElement* element, const RefPtr<BoxStyle>& style)
    : SVGResourceContainerBox(element, style)
{
}

SVGResourcePatternBox::SVGResourcePatternBox(SVGPatternElement* element, const RefPtr<BoxStyle>& style)
    : SVGResourcePaintServerBox(element, style)
{
}

void SVGResourcePatternBox::build()
{
    m_attributes = element()->collectPatternAttributes();
    SVGResourcePaintServerBox::build();
}

void SVGResourcePatternBox::applyPaint(const SVGRenderState& state, float opacity) const
{
    if(state.hasCycleReference(this))
        return;
    auto patternContentBox = to<SVGResourcePatternBox>(m_attributes.patternContentElement()->box());
    if(patternContentBox == nullptr)
        return;
    SVGLengthContext lengthContext(element(), m_attributes.patternUnits());
    Rect patternRect = {
        lengthContext.valueForLength(m_attributes.x()),
        lengthContext.valueForLength(m_attributes.y()),
        lengthContext.valueForLength(m_attributes.width()),
        lengthContext.valueForLength(m_attributes.height())
    };

    if(m_attributes.patternUnits() == SVGUnitsTypeObjectBoundingBox) {
        auto& bbox = state.fillBoundingBox();
        patternRect.x = patternRect.x * bbox.w + bbox.x;
        patternRect.y = patternRect.y * bbox.h + bbox.y;
        patternRect.w = patternRect.w * bbox.w;
        patternRect.h = patternRect.h * bbox.h;
    }

    auto currentTransform = m_attributes.patternTransform() * state.currentTransform();
    auto xScale = currentTransform.xScale();
    auto yScale = currentTransform.yScale();

    cairo_rectangle_t rectangle = {0, 0, patternRect.w * xScale, patternRect.h * yScale};
    auto surface = cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA, &rectangle);
    auto canvas = cairo_create(surface);

    GraphicsContext context(canvas);
    context.scale(xScale, yScale);
    if(m_attributes.viewBox().isValid()) {
        context.addTransform(m_attributes.preserveAspectRatio().getTransform(m_attributes.viewBox(), patternRect.size()));
    } else if(m_attributes.patternContentUnits() == SVGUnitsTypeObjectBoundingBox) {
        auto& bbox = state.fillBoundingBox();
        context.scale(bbox.w, bbox.h);
    }
    {
        SVGBlendInfo blendInfo(m_clipper, m_masker, opacity, BlendMode::Normal);
        SVGRenderState newState(blendInfo, this, &state, SVGRenderMode::Painting, context, context.getTransform());
        patternContentBox->renderChildren(newState);
    }

    Transform patternTransform(m_attributes.patternTransform());
    patternTransform.translate(patternRect.x, patternRect.y);
    patternTransform.scale(1.0 / xScale, 1.0 / yScale);
    state->setPattern(surface, patternTransform);

    cairo_destroy(canvas);
    cairo_surface_destroy(surface);
}

SVGGradientStopBox::SVGGradientStopBox(SVGStopElement* element, const RefPtr<BoxStyle>& style)
    : Box(element, style)
{
}

SVGResourceGradientBox::SVGResourceGradientBox(SVGGradientElement* element, const RefPtr<BoxStyle>& style)
    : SVGResourcePaintServerBox(element, style)
{
}

SVGResourceLinearGradientBox::SVGResourceLinearGradientBox(SVGLinearGradientElement* element, const RefPtr<BoxStyle>& style)
    : SVGResourceGradientBox(element, style)
{
}

static GradientStops buildGradientStops(const SVGGradientElement* element)
{
    GradientStops gradientStops;
    float previousOffset = 0.f;
    for(auto child = element->firstChild(); child; child = child->nextSibling()) {
        if(child->isOfType(svgNs, stopTag)) {
            auto stopElement = static_cast<const SVGStopElement*>(child);
            auto offset = std::max(previousOffset, stopElement->offset());
            gradientStops.emplace_back(offset, stopElement->stopColorIncludingOpacity());
            previousOffset = offset;
        }
    }

    return gradientStops;
}

constexpr SpreadMethod toSpreadMethod(SVGSpreadMethodType spreadMethodType)
{
    switch(spreadMethodType) {
    case SVGSpreadMethodTypePad:
        return SpreadMethod::Pad;
    case SVGSpreadMethodTypeReflect:
        return SpreadMethod::Reflect;
    case SVGSpreadMethodTypeRepeat:
        return SpreadMethod::Repeat;
    default:
        assert(false);
    }

    return SpreadMethod::Pad;
}

void SVGResourceLinearGradientBox::build()
{
    m_attributes = element()->collectGradientAttributes();
    SVGResourceGradientBox::build();
}

void SVGResourceLinearGradientBox::applyPaint(const SVGRenderState& state, float opacity) const
{
    auto gradientStops = buildGradientStops(m_attributes.gradientContentElement());
    if(gradientStops.empty()) {
        state->setColor(Color::Transparent);
        return;
    }

    SVGLengthContext lengthContext(element(), m_attributes.gradientUnits());
    LinearGradientValues values = {
        lengthContext.valueForLength(m_attributes.x1()),
        lengthContext.valueForLength(m_attributes.y1()),
        lengthContext.valueForLength(m_attributes.x2()),
        lengthContext.valueForLength(m_attributes.y2())
    };

    if((gradientStops.size() == 1 || (values.x1 == values.x2 && values.y1 == values.y2))) {
        auto& lastStop = gradientStops.back();
        state->setColor(lastStop.second.colorWithAlpha(opacity));
        return;
    }

    auto spreadMethod = toSpreadMethod(m_attributes.spreadMethod());
    auto gradientTransform = m_attributes.gradientTransform();
    if(m_attributes.gradientUnits() == SVGUnitsTypeObjectBoundingBox) {
        auto& bbox = state.fillBoundingBox();
        gradientTransform.postMultiply(Transform(bbox.w, 0, 0, bbox.h, bbox.x, bbox.y));
    }

    state->setLinearGradient(values, gradientStops, gradientTransform, spreadMethod, opacity);
}

SVGResourceRadialGradientBox::SVGResourceRadialGradientBox(SVGRadialGradientElement* element, const RefPtr<BoxStyle>& style)
    : SVGResourceGradientBox(element, style)
{
}

void SVGResourceRadialGradientBox::build()
{
    m_attributes = element()->collectGradientAttributes();
    SVGResourceGradientBox::build();
}

void SVGResourceRadialGradientBox::applyPaint(const SVGRenderState& state, float opacity) const
{
    auto gradientStops = buildGradientStops(m_attributes.gradientContentElement());
    if(gradientStops.empty()) {
        state->setColor(Color::Transparent);
        return;
    }

    SVGLengthContext lengthContext(element(), m_attributes.gradientUnits());
    RadialGradientValues values = {
        lengthContext.valueForLength(m_attributes.fx()),
        lengthContext.valueForLength(m_attributes.fy()),
        lengthContext.valueForLength(m_attributes.cx()),
        lengthContext.valueForLength(m_attributes.cy()),
        lengthContext.valueForLength(m_attributes.r())
    };

    if(values.r == 0.f || gradientStops.size() == 1) {
        auto& lastStop = gradientStops.back();
        state->setColor(lastStop.second.colorWithAlpha(opacity));
        return;
    }

    auto spreadMethod = toSpreadMethod(m_attributes.spreadMethod());
    auto gradientTransform = m_attributes.gradientTransform();
    if(m_attributes.gradientUnits() == SVGUnitsTypeObjectBoundingBox) {
        auto& bbox = state.fillBoundingBox();
        gradientTransform.postMultiply(Transform(bbox.w, 0, 0, bbox.h, bbox.x, bbox.y));
    }

    state->setRadialGradient(values, gradientStops, gradientTransform, spreadMethod, opacity);
}

} // namespace plutobook
