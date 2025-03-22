#include "box.h"
#include "boxlayer.h"
#include "flexiblebox.h"
#include "listitembox.h"
#include "tablebox.h"
#include "multicolumnbox.h"
#include "linebox.h"
#include "borderpainter.h"
#include "graphicscontext.h"
#include "imageresource.h"
#include "document.h"

#include <cmath>

namespace plutobook {

Box::Box(Node* node, const RefPtr<BoxStyle>& style)
    : m_node(node), m_style(style)
{
    if(node) {
        node->setBox(this);
    }
}

Box::~Box()
{
    auto child = m_firstChild;
    while(child) {
        auto nextChild = child->nextSibling();
        child->setParentBox(nullptr);
        child->setNextSibling(nullptr);
        child->setPrevSibling(nullptr);
        delete child;
        child = nextChild;
    }

    if(m_parentBox)
        m_parentBox->removeChild(this);
    if(m_node) {
        m_node->setBox(nullptr);
    }
}

void Box::addChild(Box* newChild)
{
    appendChild(newChild);
}

void Box::insertChild(Box* newChild, Box* nextChild)
{
    if(nextChild == nullptr) {
        appendChild(newChild);
        return;
    }

    assert(nextChild->parentBox() == this);
    assert(newChild->parentBox() == nullptr);
    assert(newChild->nextSibling() == nullptr);
    assert(newChild->prevSibling() == nullptr);

    auto prevChild = nextChild->prevSibling();
    nextChild->setPrevSibling(newChild);
    assert(m_lastChild != prevChild);
    if(prevChild == nullptr) {
        assert(m_firstChild == nextChild);
        m_firstChild = newChild;
    } else {
        assert(m_firstChild != nextChild);
        prevChild->setNextSibling(newChild);
    }

    newChild->setParentBox(this);
    newChild->setNextSibling(nextChild);
    newChild->setPrevSibling(prevChild);
}

void Box::appendChild(Box* newChild)
{
    assert(newChild->parentBox() == nullptr);
    assert(newChild->nextSibling() == nullptr);
    assert(newChild->prevSibling() == nullptr);
    newChild->setParentBox(this);
    if(m_lastChild == nullptr) {
        assert(m_firstChild == nullptr);
        m_firstChild = m_lastChild = newChild;
        return;
    }

    newChild->setPrevSibling(m_lastChild);
    m_lastChild->setNextSibling(newChild);
    m_lastChild = newChild;
}

void Box::removeChild(Box* child)
{
    assert(child->parentBox() == this);
    auto nextChild = child->nextSibling();
    auto prevChild = child->prevSibling();
    if(nextChild)
        nextChild->setPrevSibling(prevChild);
    if(prevChild) {
        prevChild->setNextSibling(nextChild);
    }

    if(m_firstChild == child)
        m_firstChild = nextChild;
    if(m_lastChild == child) {
        m_lastChild = prevChild;
    }

    child->setParentBox(nullptr);
    child->setPrevSibling(nullptr);
    child->setNextSibling(nullptr);
}

void Box::moveChildrenTo(Box* to, Box* begin, Box* end)
{
    auto child = begin;
    while(child && child != end) {
        auto nextChild = child->nextSibling();
        removeChild(child);
        to->appendChild(child);
        child = nextChild;
    }
}

void Box::moveChildrenTo(Box* to, Box* begin)
{
    moveChildrenTo(to, begin, nullptr);
}

void Box::moveChildrenTo(Box* to)
{
    moveChildrenTo(to, m_firstChild, nullptr);
}

Box* Box::create(Node* node, const RefPtr<BoxStyle>& style)
{
    if(style->pseudoType() == PseudoType::Marker) {
        if(style->listStylePosition() == ListStylePosition::Inside)
            return new (style->heap()) InsideListMarkerBox(style);
        return new (style->heap()) OutsideListMarkerBox(style);
    }

    switch(style->display()) {
    case Display::Inline:
        return new (style->heap()) InlineBox(node, style);
    case Display::Block:
    case Display::InlineBlock:
        return new (style->heap()) BlockFlowBox(node, style);
    case Display::Flex:
    case Display::InlineFlex:
        return new (style->heap()) FlexibleBox(node, style);
    case Display::Table:
    case Display::InlineTable:
        return new (style->heap()) TableBox(node, style);
    case Display::ListItem:
        return new (style->heap()) ListItemBox(node, style);
    case Display::TableCell:
        return new (style->heap()) TableCellBox(node, style);
    case Display::TableRow:
        return new (style->heap()) TableRowBox(node, style);
    case Display::TableCaption:
        return new (style->heap()) TableCaptionBox(node, style);
    case Display::TableColumn:
    case Display::TableColumnGroup:
        return new (style->heap()) TableColumnBox(node, style);
    case Display::TableRowGroup:
    case Display::TableHeaderGroup:
    case Display::TableFooterGroup:
        return new (style->heap()) TableSectionBox(node, style);
    default:
        assert(false);
    }

    return nullptr;
}

Box* Box::createAnonymous(Display display, const BoxStyle* parentStyle)
{
    auto newBox = create(nullptr, BoxStyle::create(*parentStyle, display));
    newBox->setAnonymous(true);
    return newBox;
}

BlockFlowBox* Box::createAnonymousBlock(const BoxStyle* parentStyle)
{
    auto newStyle = BoxStyle::create(*parentStyle, Display::Block);
    auto newBlock = new (newStyle->heap()) BlockFlowBox(nullptr, newStyle);
    newBlock->setAnonymous(true);
    return newBlock;
}

BlockBox* Box::containingBlock() const
{
    if(hasColumnSpanBox())
        return to<BoxFrame>(*this).columnSpanBox()->containingBlock();
    auto parent = parentBox();
    if(style()->position() == Position::Static || style()->position() == Position::Relative || isTextBox()) {
        while(parent && !parent->isBlockBox())
            parent = parent->parentBox();
        return to<BlockBox>(parent);
    }

    if(style()->position() == Position::Fixed) {
        while(parent && !parent->isBoxView()) {
            if(parent->hasTransform() && parent->isBlockBox())
                break;
            parent = parent->parentBox();
        }

        return to<BlockBox>(parent);
    }

    while(parent && parent->style()->position() == Position::Static) {
        if(parent->hasTransform() && parent->isBlockBox())
            break;
        parent = parent->parentBox();
    }

    if(parent && !parent->isBlockBox())
        parent = parent->containingBlock();
    while(parent && parent->isAnonymous())
        parent = parent->containingBlock();
    return to<BlockBox>(parent);
}

BoxModel* Box::containingBox() const
{
    if(hasColumnSpanBox())
        return to<BoxFrame>(*this).columnSpanBox()->containingBox();
    auto parent = parentBox();
    if(!isTextBox()) {
        if(style()->position() == Position::Fixed) {
            while(parent && !parent->isBoxView()) {
                if(parent->hasTransform() && parent->isBlockBox())
                    break;
                parent = parent->parentBox();
            }
        } else if(style()->position() == Position::Absolute) {
            while(parent && parent->style()->position() == Position::Static) {
                if(parent->hasTransform() && parent->isBlockBox())
                    break;
                parent = parent->parentBox();
            }
        }
    }

    return to<BoxModel>(parent);
}

BoxLayer* Box::enclosingLayer() const
{
    if(hasColumnSpanBox()) {
        auto container = containingBlock();
        assert(container->hasLayer());
        return container->layer();
    }

    for(auto current = this; current; current = current->parentBox()) {
        if(current->hasLayer()) {
            return to<BoxModel>(*current).layer();
        }
    }

    return nullptr;
}

BoxView* Box::view() const
{
    return document()->box();
}

bool Box::isBodyBox() const
{
    return m_node && m_node->isOfType(xhtmlNs, bodyTag);
}

bool Box::isRootBox() const
{
    return m_node && m_node->isRootNode();
}

bool Box::isFlexItem() const
{
    return m_parentBox && m_parentBox->isFlexibleBox();
}

void Box::layout(FragmentBuilder* fragmentainer)
{
    assert(false);
}

void Box::build()
{
    auto child = m_firstChild;
    while(child) {
        if(!child->isTargetCounterBox())
            child->build();
        child = child->nextSibling();
    }
}

BoxModel::BoxModel(Node* node, const RefPtr<BoxStyle>& style)
    : Box(node, style)
{
    setInline(style->isDisplayInlineType());
}

BoxModel::~BoxModel() = default;

void BoxModel::addChild(Box* newChild)
{
    if(!newChild->isTableCellBox() && !newChild->isTableRowBox()
        && !newChild->isTableCaptionBox() && !newChild->isTableColumnBox()
        && !newChild->isTableSectionBox()) {
        appendChild(newChild);
        return;
    }

    auto lastTable = lastChild();
    if(lastTable && lastTable->isAnonymous() && lastTable->isTableBox()) {
        lastTable->addChild(newChild);
        return;
    }

    auto newTable = createAnonymous(Display::Table, style());
    appendChild(newTable);
    newTable->addChild(newChild);
}

void BoxModel::paintBackground(const PaintInfo& info, const Rect& borderRect, const BoxStyle& backgroundStyle, bool includeLeftEdge, bool includeRightEdge) const
{
    auto color = backgroundStyle.backgroundColor();
    auto image = backgroundStyle.backgroundImage();
    if(image == nullptr && !color.alpha())
        return;
    auto clip = backgroundStyle.backgroundClip();
    auto clipRect = style()->getBorderRoundedRect(borderRect, includeLeftEdge, includeRightEdge);
    if(clip == BackgroundBox::PaddingBox || clip == BackgroundBox::ContentBox) {
        auto topWidth = borderTop();
        auto bottomWidth = borderBottom();
        auto leftWidth = borderLeft();
        auto rightWidth = borderRight();
        if(clip == BackgroundBox::ContentBox) {
            topWidth += paddingTop();
            bottomWidth += paddingBottom();
            leftWidth += paddingLeft();
            rightWidth += paddingRight();
        }

        if(!includeLeftEdge)
            leftWidth = 0.f;
        if(!includeRightEdge)
            rightWidth = 0.f;
        clipRect.shrink(topWidth, bottomWidth, leftWidth, rightWidth);
    }

    if(!clipRect.rect().intersects(info.rect()))
        return;
    auto clipping = clip == BackgroundBox::PaddingBox || clip == BackgroundBox::ContentBox || clipRect.isRounded();
    if(clipping) {
        info->save();
        info->clipRoundedRect(clipRect);
    }

    info->setColor(color);
    info->fillRect(borderRect);
    if(image && image->width() && image->height()) {
        auto origin = backgroundStyle.backgroundOrigin();
        Rect positioningArea(0, 0, borderRect.w, borderRect.h);
        if(origin == BackgroundBox::PaddingBox || origin == BackgroundBox::ContentBox) {
            auto topWidth = borderTop();
            auto bottomWidth = borderBottom();
            auto leftWidth = borderLeft();
            auto rightWidth = borderRight();
            if(origin == BackgroundBox::ContentBox) {
                topWidth += paddingTop();
                bottomWidth += paddingBottom();
                leftWidth += paddingLeft();
                rightWidth += paddingRight();
            }

            positioningArea.shrink(topWidth, bottomWidth, leftWidth, rightWidth);
        }

        Rect tileRect;
        auto size = backgroundStyle.backgroundSize();
        switch(size.type()) {
        case BackgroundSize::Type::Contain:
        case BackgroundSize::Type::Cover: {
            auto xScale = positioningArea.w / image->width();
            auto yScale = positioningArea.h / image->height();
            auto scale = size.type() == BackgroundSize::Type::Contain ? std::min(xScale, yScale) : std::max(xScale, yScale);
            tileRect.w = image->width() * scale;
            tileRect.h = image->height() * scale;
            break;
        }

        case BackgroundSize::Type::Length:
            auto& widthLength = size.width();
            auto& heightLength = size.height();
            if(widthLength.isFixed())
                tileRect.w = widthLength.value();
            else if(widthLength.isPercent())
                tileRect.w = widthLength.calc(positioningArea.w);
            else {
                tileRect.w = positioningArea.w;
            }

            if(heightLength.isFixed())
                tileRect.h = heightLength.value();
            else if(heightLength.isPercent())
                tileRect.h = heightLength.calc(positioningArea.h);
            else {
                tileRect.h = positioningArea.h;
            }

            if(widthLength.isAuto() && !heightLength.isAuto()) {
                tileRect.w = image->width() * tileRect.h / image->height();
            } else if (!widthLength.isAuto() && heightLength.isAuto()) {
                tileRect.h = image->height() * tileRect.w / image->width();
            } else if(widthLength.isAuto() && heightLength.isAuto()) {
                tileRect.w = image->width();
                tileRect.h = image->height();
            }
        }

        auto position = backgroundStyle.backgroundPosition();
        tileRect.x = position.left().calcMin(positioningArea.w - tileRect.w);
        tileRect.y = position.top().calcMin(positioningArea.h - tileRect.h);

        Rect destRect(borderRect);
        auto repeat = backgroundStyle.backgroundRepeat();
        if(repeat == BackgroundRepeat::Repeat || repeat == BackgroundRepeat::RepeatX) {
            tileRect.x = tileRect.w - std::fmod(tileRect.x + positioningArea.x, tileRect.w);
        } else {
            destRect.x += std::max(0.f, tileRect.x + positioningArea.x);
            tileRect.x = -std::min(0.f, tileRect.x + positioningArea.x);
            destRect.w = tileRect.w - tileRect.x;
        }

        if(repeat == BackgroundRepeat::Repeat || repeat == BackgroundRepeat::RepeatY) {
            tileRect.y = tileRect.h - std::fmod(tileRect.y + positioningArea.y, tileRect.h);
        } else {
            destRect.y += std::max(0.f, tileRect.y + positioningArea.y);
            tileRect.y = -std::min(0.f, tileRect.y + positioningArea.y);
            destRect.h = tileRect.h - tileRect.y;
        }

        destRect.intersect(borderRect);
        if(destRect.intersects(info.rect())) {
            image->drawTiled(*info, destRect, tileRect);
        }
    }

    if(clipping) {
        info->restore();
    }
}

void BoxModel::paintBackground(const PaintInfo& info, const Rect& borderRect) const
{
    if(style() == document()->backgroundStyle())
        return;
    paintBackground(info, borderRect, *style());
}

void BoxModel::paintRootBackground(const PaintInfo& info) const
{
    paintBackground(info, document()->backgroundRect(), *document()->backgroundStyle());
}

void BoxModel::paintBorder(const PaintInfo& info, const Rect& borderRect, bool includeLeftEdge, bool includeRightEdge) const
{
    BorderPainter painter(BorderPainterType::Border, borderRect, *style(), includeLeftEdge, includeRightEdge);
    painter.paint(*info, info.rect());
}

void BoxModel::paintOutline(const PaintInfo& info, const Rect& borderRect) const
{
    BorderPainter painter(BorderPainterType::Outline, borderRect, *style(), true, true);
    painter.paint(*info, info.rect());
}

void BoxModel::paint(const PaintInfo& info, const Point& offset, PaintPhase phase)
{
    assert(false);
}

Point BoxModel::relativePositionOffset() const
{
    auto container = containingBlock();
    auto leftLength = style()->left();
    auto rightLength = style()->right();
    auto topLength = style()->top();
    auto bottomLength = style()->bottom();

    Point offset;
    if(!leftLength.isAuto()) {
        offset.x = leftLength.calc(container->availableWidth());
    } else if(!rightLength.isAuto()) {
        offset.x = -rightLength.calc(container->availableWidth());
    }

    auto availableHeight = container->availableHeight();
    if(!topLength.isAuto() && (availableHeight || !topLength.isPercent())) {
        offset.y = topLength.calc(availableHeight.value_or(0.f));
    } else if(!bottomLength.isAuto() && (availableHeight || !bottomLength.isPercent())) {
        offset.y = -bottomLength.calc(availableHeight.value_or(0.f));
    }

    return offset;
}

float BoxModel::availableWidthForPositioned() const
{
    if(isBoxView())
        return document()->availableWidth();
    auto rect = borderBoundingBox();
    if(rect.w > 0.f)
        return rect.w - borderLeft() - borderRight();
    return 0.f;
}

float BoxModel::availableHeightForPositioned() const
{
    if(isBoxView())
        return document()->availableHeight();
    auto rect = borderBoundingBox();
    if(rect.h > 0.f)
        return rect.h - borderTop() - borderBottom();
    return 0.f;
}

void BoxModel::updateMarginWidths()
{
    auto calc = [this](const Length& margin) {
        float containerWidth = 0;
        if(margin.isPercent())
            containerWidth = containingBlock()->availableWidth();
        return margin.calcMin(containerWidth);
    };

    m_marginTop = calc(style()->marginTop());
    m_marginBottom = calc(style()->marginBottom());
    m_marginLeft = calc(style()->marginLeft());
    m_marginRight = calc(style()->marginRight());
}

void BoxModel::updateBorderWidths() const
{
    auto calc = [](LineStyle style, float width) {
        if(style > LineStyle::Hidden)
            return width;
        return 0.f;
    };

    m_borderTop = calc(style()->borderTopStyle(), style()->borderTopWidth());
    m_borderBottom = calc(style()->borderBottomStyle(), style()->borderBottomWidth());
    m_borderLeft = calc(style()->borderLeftStyle(), style()->borderLeftWidth());
    m_borderRight = calc(style()->borderRightStyle(), style()->borderRightWidth());
}

void BoxModel::updatePaddingWidths() const
{
    auto calc = [this](const Length& padding) {
        float containerWidth = 0;
        if(padding.isPercent())
            containerWidth = containingBlock()->availableWidth();
        return padding.calcMin(containerWidth);
    };

    m_paddingTop = calc(style()->paddingTop());
    m_paddingBottom = calc(style()->paddingBottom());
    m_paddingLeft = calc(style()->paddingLeft());
    m_paddingRight = calc(style()->paddingRight());
}

float BoxModel::borderTop() const
{
    if(m_borderTop < 0)
        updateBorderWidths();
    return m_borderTop;
}

float BoxModel::borderBottom() const
{
    if(m_borderBottom < 0)
        updateBorderWidths();
    return m_borderBottom;
}

float BoxModel::borderLeft() const
{
    if(m_borderLeft < 0)
        updateBorderWidths();
    return m_borderLeft;
}

float BoxModel::borderRight() const
{
    if(m_borderRight < 0)
        updateBorderWidths();
    return m_borderRight;
}

float BoxModel::paddingTop() const
{
    if(m_paddingTop < 0)
        updatePaddingWidths();
    return m_paddingTop;
}

float BoxModel::paddingBottom() const
{
    if(m_paddingBottom < 0)
        updatePaddingWidths();
    return m_paddingBottom;
}

float BoxModel::paddingLeft() const
{
    if(m_paddingLeft < 0)
        updatePaddingWidths();
    return m_paddingLeft;
}

float BoxModel::paddingRight() const
{
    if(m_paddingRight < 0)
        updatePaddingWidths();
    return m_paddingRight;
}

void BoxModel::build()
{
    if(requiresLayer()) {
        assert(!hasLayer());
        m_layer = BoxLayer::create(this, enclosingLayer());
        setHasLayer(true);
    }

    Box::build();
}

BoxFrame::BoxFrame(Node* node, const RefPtr<BoxStyle>& style)
    : BoxModel(node, style)
{
    setHasTransform(style->hasTransform());
    switch(style->position()) {
    case Position::Static:
    case Position::Relative:
        setPositioned(false);
        break;
    default:
        setPositioned(true);
        break;
    }

    setOverflowHidden(style->isOverflowHidden());
    switch(style->floating()) {
    case Float::None:
        setFloating(false);
        break;
    default:
        setFloating(true);
        break;
    }
}

bool BoxFrame::requiresLayer() const
{
    return isPositioned() || isRelPositioned() || isOverflowHidden() || hasTransform() || hasColumnFlowBox()
        || style()->hasOpacity() || style()->hasBlendMode() || style()->zIndex();
}

BoxFrame::~BoxFrame() = default;

void BoxFrame::setLine(std::unique_ptr<ReplacedLineBox> line)
{
    m_line = std::move(line);
}

void BoxFrame::computePreferredWidths(float& minPreferredWidth, float& maxPreferredWidth) const
{
    minPreferredWidth = 0;
    maxPreferredWidth = 0;
}

float BoxFrame::minPreferredWidth() const
{
    if(m_minPreferredWidth < 0)
        computePreferredWidths(m_minPreferredWidth, m_maxPreferredWidth);
    return m_minPreferredWidth;
}

float BoxFrame::maxPreferredWidth() const
{
    if(m_maxPreferredWidth < 0)
        computePreferredWidths(m_minPreferredWidth, m_maxPreferredWidth);
    return m_maxPreferredWidth;
}

float BoxFrame::adjustBorderBoxWidth(float width) const
{
    if(style()->boxSizing() == BoxSizing::ContentBox)
        return width + borderAndPaddingWidth();
    return std::max(width, borderAndPaddingWidth());
}

float BoxFrame::adjustBorderBoxHeight(float height) const
{
    if(style()->boxSizing() == BoxSizing::ContentBox)
        return height + borderAndPaddingHeight();
    return std::max(height, borderAndPaddingHeight());
}

float BoxFrame::adjustContentBoxWidth(float width) const
{
    if(style()->boxSizing() == BoxSizing::BorderBox)
        width -= borderAndPaddingWidth();
    return std::max(0.f, width);
}

float BoxFrame::adjustContentBoxHeight(float height) const
{
    if(style()->boxSizing() == BoxSizing::BorderBox)
        height -= borderAndPaddingHeight();
    return std::max(0.f, height);
}

void BoxFrame::computeHorizontalStaticDistance(Length& leftLength, Length& rightLength, const BoxModel* container, float containerWidth) const
{
    if(!leftLength.isAuto() || !rightLength.isAuto())
        return;
    auto parent = parentBox();
    if(parent->style()->direction() == Direction::Ltr) {
        auto staticPosition = layer()->staticLeft() - container->borderLeft();
        for(; parent && parent != container; parent = parent->containingBox()) {
            if(auto box = to<BoxFrame>(parent)) {
                staticPosition += box->x();
            }
        }

        leftLength = Length(Length::Type::Fixed, staticPosition);
    } else {
        auto staticPosition = layer()->staticLeft() + containerWidth + container->borderRight();
        while(parent && !parent->isBoxFrame())
            parent = parent->parentBox();
        if(auto box = to<BoxFrame>(parent))
            staticPosition -= box->width();
        for(; parent && parent != container; parent = parent->containingBox()) {
            if(auto box = to<BoxFrame>(parent)) {
                staticPosition -= box->x();
            }
        }

        rightLength = Length(Length::Type::Fixed, staticPosition);
    }
}

void BoxFrame::computeVerticalStaticDistance(Length& topLength, Length& bottomLength, const BoxModel* container) const
{
    if(!topLength.isAuto() || !bottomLength.isAuto())
        return;
    auto staticTop = layer()->staticTop() - container->borderTop();
    for(auto parent = parentBox(); parent && parent != container; parent = parent->containingBox()) {
        if(auto box = to<BoxFrame>(parent)) {
            if(!box->isTableRowBox()) {
                staticTop += box->y();
            }
        }
    }

    topLength = Length(Length::Type::Fixed, staticTop);
}

void BoxFrame::computeHorizontalMargins(float& marginLeft, float& marginRight, float childWidth, const BlockBox* container, float containerWidth) const
{
    auto marginLeftLength = style()->marginLeft();
    auto marginRightLength = style()->marginRight();
    if(isInline() || isFloating() || container->isFlexibleBox()) {
        marginLeft = marginLeftLength.calcMin(containerWidth);
        marginRight = marginRightLength.calcMin(containerWidth);
        return;
    }

    auto containerBlock = to<BlockFlowBox>(container);
    if(containerBlock && containerBlock->containsFloats() && avoidsFloats())
        containerWidth = containerBlock->availableWidthForLine(y(), false);
    if(childWidth < containerWidth) {
        if(marginLeftLength.isAuto() && marginRightLength.isAuto()) {
            marginLeft = std::max(0.f, (containerWidth - childWidth) / 2.f);
            marginRight = containerWidth - childWidth - marginLeft;
            return;
        }

        if(marginRightLength.isAuto()) {
            marginLeft = marginLeftLength.calc(containerWidth);
            marginRight = containerWidth - childWidth - marginLeft;
            return;
        }

        if(marginLeftLength.isAuto()) {
            marginRight = marginRightLength.calc(containerWidth);
            marginLeft = containerWidth - childWidth - marginRight;
            return;
        }
    }

    marginLeft = marginLeftLength.calcMin(containerWidth);
    marginRight = marginRightLength.calcMin(containerWidth);
}

void BoxFrame::computeVerticalMargins(float& marginTop, float& marginBottom) const
{
    if(isTableCellBox()) {
        marginTop = 0;
        marginBottom = 0;
        return;
    }

    auto containerWidth = containingBlock()->availableWidth();
    marginTop = style()->marginTop().calcMin(containerWidth);
    marginBottom = style()->marginBottom().calcMin(containerWidth);
}

float BoxFrame::computeIntrinsicWidthUsing(const Length& widthLength, float containerWidth) const
{
    if(widthLength.isMinContent())
        return minPreferredWidth();
    if(widthLength.isMaxContent()) {
        return maxPreferredWidth();
    }

    assert(widthLength.isFitContent());
    auto marginLeft = style()->marginLeft().calcMin(containerWidth);
    auto marginRight = style()->marginRight().calcMin(containerWidth);
    auto width = containerWidth - marginLeft - marginRight;
    return std::max(minPreferredWidth(), std::min(width, maxPreferredWidth()));
}

void BoxFrame::computeWidth(float& x, float& width, float& marginLeft, float& marginRight) const
{
    assert(false);
}

void BoxFrame::computeHeight(float& y, float& height, float& marginTop, float& marginBottom) const
{
    assert(false);
}

void BoxFrame::updateWidth()
{
    computeWidth(m_x, m_width, m_marginLeft, m_marginRight);
}

void BoxFrame::updateHeight()
{
    computeHeight(m_y, m_height, m_marginTop, m_marginBottom);
}

void BoxFrame::updateVerticalMargins()
{
    computeVerticalMargins(m_marginTop, m_marginBottom);
}

float BoxFrame::maxMarginTop(bool positive) const
{
    return positive ? std::max(0.f, m_marginTop) : -std::min(0.f, m_marginTop);
}

float BoxFrame::maxMarginBottom(bool positive) const
{
    return positive ? std::max(0.f, m_marginBottom) : -std::min(0.f, m_marginBottom);
}

float BoxFrame::collapsedMarginTop() const
{
    return maxMarginTop(true) - maxMarginTop(false);
}

float BoxFrame::collapsedMarginBottom() const
{
    return maxMarginBottom(true) - maxMarginBottom(false);
}

void BoxFrame::updateOverflowRect()
{
    Rect borderRect(0, 0, m_width, m_height);
    auto outlineEdge = style()->getOutlineEdge();
    if(outlineEdge.isRenderable()) {
        borderRect.inflate(outlineEdge.width() + style()->outlineOffset());
    }

    m_overflowTop = std::min(0.f, borderRect.y);
    m_overflowBottom = std::max(m_height, borderRect.bottom());
    m_overflowLeft = std::min(0.f, borderRect.x);
    m_overflowRight = std::max(m_width, borderRect.right());
}

void BoxFrame::addOverflowRect(const BoxFrame* child, float dx, float dy)
{
    if(child->hasLayer())
        return;
    auto overflowRect = child->visualOverflowRect();
    overflowRect.move(dx, dy);
    addOverflowRect(overflowRect);
}

void BoxFrame::addOverflowRect(float top, float bottom, float left, float right)
{
    m_overflowTop = std::min(top, m_overflowTop);
    m_overflowBottom = std::max(bottom, m_overflowBottom);
    m_overflowLeft = std::min(left, m_overflowLeft);
    m_overflowRight = std::max(right, m_overflowRight);
}

void BoxFrame::addOverflowRect(const Rect& overflowRect)
{
    addOverflowRect(overflowRect.y, overflowRect.bottom(), overflowRect.x, overflowRect.right());
}

void BoxFrame::paintOutlines(const PaintInfo& info, const Point& offset)
{
    Rect borderRect(offset, size());
    paintOutline(info, borderRect);
}

void BoxFrame::paintDecorations(const PaintInfo& info, const Point& offset)
{
    Rect borderRect(offset, size());
    paintBackground(info, borderRect);
    paintBorder(info, borderRect);
}

} // namespace plutobook
