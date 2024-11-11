#include "svgresourcebox.h"
#include "svgshapebox.h"

#include <cairo.h>

namespace plutobook {

SVGResourceMarkerBox::SVGResourceMarkerBox(SVGMarkerElement* element, const RefPtr<BoxStyle>& style)
    : SVGResourceContainerBox(element, style)
{
    setOverflowHidden(style->isOverflowHidden());
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

    if(element()->markerUnits() == SVGMarkerUnitsTypeStrokeWidth)
        transform.scale(strokeWidth, strokeWidth);
    transform.translate(-m_refPoint.x, -m_refPoint.y);
    return transform * m_localTransform;
}

Rect SVGResourceMarkerBox::markerBoundingBox(const Point& origin, float angle, float strokeWidth) const
{
    return markerTransform(origin, angle, strokeWidth).mapRect(paintBoundingBox());
}

void SVGResourceMarkerBox::renderMarker(const SVGRenderState& state, const Point& origin, float angle, float strokeWidth) const
{
    if(m_clipRect.isEmpty() || state.hasCycleReference(this))
        return;
    SVGBlendInfo blendInfo(m_clipper, m_masker, style());
    SVGRenderState newState(blendInfo, this, state, markerTransform(origin, angle, strokeWidth));
    if(isOverflowHidden())
        newState->clipRect(m_clipRect);
    renderChildren(newState);
}

void SVGResourceMarkerBox::build()
{
    SVGLengthContext lengthContext(element());
    const Point refPoint = {
        lengthContext.valueForLength(element()->refX()),
        lengthContext.valueForLength(element()->refY())
    };

    const Size markerSize = {
        lengthContext.valueForLength(element()->markerWidth()),
        lengthContext.valueForLength(element()->markerHeight())
    };

    m_clipRect = element()->getClipRect(markerSize);
    m_localTransform = element()->viewBoxToViewTransform(markerSize);
    m_refPoint = m_localTransform.mapPoint(refPoint);
    SVGResourceContainerBox::build();
}

SVGResourceClipperBox::SVGResourceClipperBox(SVGClipPathElement* element, const RefPtr<BoxStyle>& style)
    : SVGResourceContainerBox(element, style)
{
}

bool SVGResourceClipperBox::requiresMasking() const
{
    if(m_clipper != nullptr)
        return true;
    const SVGShapeBox* prevClipShape = nullptr;
    for(auto child = firstChild(); child; child = child->nextSibling()) {
        if(child->style()->visibility() != Visibility::Visible)
            continue;
        const SVGShapeBox* clipShape = nullptr;
        if(auto container = to<SVGTransformableContainerBox>(child)) {
            if(container->element()->tagName() != useTag)
                continue;
            if(container->clipper())
                return true;
            clipShape = to<SVGShapeBox>(container->firstChild());
        } else {
            if(child->isSVGTextBox())
                return true;
            clipShape = to<SVGShapeBox>(child);
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
        const SVGShapeBox* clipShape = nullptr;
        if(auto container = to<SVGTransformableContainerBox>(child)) {
            if(container->element()->tagName() != useTag)
                continue;
            clipTransform.multiply(container->localTransform());
            clipShape = to<SVGShapeBox>(container->firstChild());
        } else {
            clipShape = to<SVGShapeBox>(child);
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

    Rect maskRect(m_maskRect);
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
    Rect maskRect(m_maskRect);
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

void SVGResourceMaskerBox::build()
{
    SVGLengthContext lengthContext(element(), element()->maskUnits());
    m_maskRect.x = lengthContext.valueForLength(element()->x());
    m_maskRect.y = lengthContext.valueForLength(element()->y());
    m_maskRect.w = lengthContext.valueForLength(element()->width());
    m_maskRect.h = lengthContext.valueForLength(element()->height());
    SVGResourceContainerBox::build();
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
    auto attributes = element()->collectPatternAttributes();
    auto patternContentBox = attributes.patternContentElement()->box();
    assert(patternContentBox && patternContentBox->isSVGResourcePatternBox());
    m_patternContentBox = to<SVGResourcePatternBox>(patternContentBox);
    m_patternTransform = attributes.patternTransform();
    m_patternUnits = attributes.patternUnits();
    m_patternContentUnits = attributes.patternContentUnits();
    m_preserveAspectRatio = attributes.preserveAspectRatio();
    m_viewBox = attributes.viewBox();

    SVGLengthContext lengthContext(element(), attributes.patternUnits());
    m_patternRect.x = lengthContext.valueForLength(attributes.x());
    m_patternRect.y = lengthContext.valueForLength(attributes.y());
    m_patternRect.w = lengthContext.valueForLength(attributes.width());
    m_patternRect.h = lengthContext.valueForLength(attributes.height());
    SVGResourcePaintServerBox::build();
}

void SVGResourcePatternBox::applyPaint(const SVGRenderState& state, float opacity) const
{
    if(state.hasCycleReference(this))
        return;
    Rect patternRect(m_patternRect);
    if(m_patternUnits == SVGUnitsTypeObjectBoundingBox) {
        auto& bbox = state.fillBoundingBox();
        patternRect.x = patternRect.x * bbox.w + bbox.x;
        patternRect.y = patternRect.y * bbox.h + bbox.y;
        patternRect.w = patternRect.w * bbox.w;
        patternRect.h = patternRect.h * bbox.h;
    }

    auto currentTransform = m_patternTransform * state.currentTransform();
    auto xScale = currentTransform.xScale();
    auto yScale = currentTransform.yScale();

    cairo_rectangle_t rectangle = {0, 0, patternRect.w * xScale, patternRect.h * yScale};
    auto surface = cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA, &rectangle);
    auto canvas = cairo_create(surface);

    GraphicsContext context(canvas);
    context.scale(xScale, yScale);
    if(m_viewBox.isValid()) {
        context.addTransform(m_preserveAspectRatio.getTransform(m_viewBox, patternRect.size()));
    } else if(m_patternContentUnits == SVGUnitsTypeObjectBoundingBox) {
        auto& bbox = state.fillBoundingBox();
        context.scale(bbox.w, bbox.h);
    }
    {
        SVGBlendInfo blendInfo(m_clipper, m_masker, opacity, BlendMode::Normal);
        SVGRenderState newState(blendInfo, this, &state, SVGRenderMode::Painting, context, context.getTransform());
        m_patternContentBox->renderChildren(newState);
    }

    Transform patternTransform(m_patternTransform);
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
    auto attributes = element()->collectGradientAttributes();
    m_gradientTransform = attributes.gradientTransform();
    m_gradientStops = buildGradientStops(attributes.gradientContentElement());
    m_gradientUnits = attributes.gradientUnits();
    m_spreadMethod = toSpreadMethod(attributes.spreadMethod());

    SVGLengthContext lengthContext(element(), attributes.gradientUnits());
    m_values.x0 = lengthContext.valueForLength(attributes.x1());
    m_values.y0 = lengthContext.valueForLength(attributes.y1());
    m_values.x1 = lengthContext.valueForLength(attributes.x2());
    m_values.y1 = lengthContext.valueForLength(attributes.y2());
    SVGResourceGradientBox::build();
}

void SVGResourceLinearGradientBox::applyPaint(const SVGRenderState& state, float opacity) const
{
    if(m_gradientStops.empty()) {
        state->setColor(Color::Transparent);
        return;
    }

    if((m_gradientStops.size() == 1 || (m_values.x0 == m_values.x1 && m_values.y0 == m_values.y1))) {
        auto& lastStop = m_gradientStops.back();
        state->setColor(lastStop.second.colorWithAlpha(opacity));
        return;
    }

    Transform gradientTransform(m_gradientTransform);
    if(m_gradientUnits == SVGUnitsTypeObjectBoundingBox) {
        auto& bbox = state.fillBoundingBox();
        gradientTransform.postMultiply(Transform(bbox.w, 0, 0, bbox.h, bbox.x, bbox.y));
    }

    state->setLinearGradient(m_values, m_gradientStops, gradientTransform, m_spreadMethod, opacity);
}

SVGResourceRadialGradientBox::SVGResourceRadialGradientBox(SVGRadialGradientElement* element, const RefPtr<BoxStyle>& style)
    : SVGResourceGradientBox(element, style)
{
}

void SVGResourceRadialGradientBox::build()
{
    auto attributes = element()->collectGradientAttributes();
    m_gradientTransform = attributes.gradientTransform();
    m_gradientStops = buildGradientStops(attributes.gradientContentElement());
    m_gradientUnits = attributes.gradientUnits();
    m_spreadMethod = toSpreadMethod(attributes.spreadMethod());

    SVGLengthContext lengthContext(element(), attributes.gradientUnits());
    m_values.x0 = lengthContext.valueForLength(attributes.fx());
    m_values.y0 = lengthContext.valueForLength(attributes.fy());
    m_values.x1 = lengthContext.valueForLength(attributes.cx());
    m_values.y1 = lengthContext.valueForLength(attributes.cy());
    m_values.r1 = lengthContext.valueForLength(attributes.r());
    SVGResourceGradientBox::build();
}

void SVGResourceRadialGradientBox::applyPaint(const SVGRenderState& state, float opacity) const
{
    if(m_gradientStops.empty()) {
        state->setColor(Color::Transparent);
        return;
    }

    if(m_values.r1 == 0.f || m_gradientStops.size() == 1) {
        auto& lastStop = m_gradientStops.back();
        state->setColor(lastStop.second.colorWithAlpha(opacity));
        return;
    }

    Transform gradientTransform(m_gradientTransform);
    if(m_gradientUnits == SVGUnitsTypeObjectBoundingBox) {
        auto& bbox = state.fillBoundingBox();
        gradientTransform.postMultiply(Transform(bbox.w, 0, 0, bbox.h, bbox.x, bbox.y));
    }

    state->setRadialGradient(m_values, m_gradientStops, gradientTransform, m_spreadMethod, opacity);
}

} // namespace plutobook
