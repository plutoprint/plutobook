/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "htmldocument.h"
#include "htmlparser.h"
#include "cssrule.h"
#include "counters.h"
#include "textbox.h"
#include "contentbox.h"
#include "replacedbox.h"
#include "tablebox.h"
#include "formcontrolbox.h"
#include "textresource.h"
#include "imageresource.h"
#include "stringutils.h"

#include "plutobook.hpp"

#include <unicode/uchar.h>
#include <unicode/uiter.h>

namespace plutobook {

HTMLElement::HTMLElement(Document* document, const GlobalString& tagName)
    : Element(document, xhtmlNs, tagName)
{
}

inline bool isFirstLetterPunctuation(UChar32 cc)
{
    switch(u_charType(cc)) {
    case U_START_PUNCTUATION:
    case U_END_PUNCTUATION:
    case U_INITIAL_PUNCTUATION:
    case U_FINAL_PUNCTUATION:
    case U_OTHER_PUNCTUATION:
        return true;
    default:
        return false;
    }
}

static size_t firstLetterTextLength(const HeapString& text)
{
    UCharIterator it;
    uiter_setUTF8(&it, text.data(), text.size());

    auto hasLetter = false;
    auto hasPunctuation = false;
    size_t textLength = 0;
    while(it.hasNext(&it)) {
        auto cc = uiter_next32(&it);
        if(!isSpace(cc)) {
            if(!isFirstLetterPunctuation(cc)) {
                if(hasLetter)
                    break;
                hasLetter = true;
            } else {
                hasPunctuation = true;
            }
        }

        textLength += U8_LENGTH(cc);
    }

    if(!hasLetter && !hasPunctuation)
        return 0;
    return textLength;
}

void HTMLElement::buildFirstLetterPseudoBox(Box* parent)
{
    if(!parent->isBlockFlowBox())
        return;
    auto style = document()->pseudoStyleForElement(this, PseudoType::FirstLetter, parent->style());
    if(style == nullptr || style->display() == Display::None)
        return;
    auto child = parent->firstChild();
    while(child) {
        if(child->style()->pseudoType() == PseudoType::FirstLetter || child->isReplaced()
            || child->isLineBreakBox() || child->isWordBreakBox()) {
            return;
        }

        if(auto textBox = to<TextBox>(child)) {
            const auto& text = textBox->text();
            if(auto length = firstLetterTextLength(text)) {
                auto newTextBox = new (heap()) TextBox(nullptr, style);
                newTextBox->setText(text.substring(0, length));
                textBox->setText(text.substring(length));

                auto letterBox = Box::create(nullptr, style);
                letterBox->addChild(newTextBox);
                textBox->parentBox()->insertChild(letterBox, textBox);
                break;
            }
        }

        if(!child->isFloatingOrPositioned() && !child->isListMarkerBox()
            && !child->isTableBox() && !child->isFlexibleBox()) {
            if(child->firstChild()) {
                child = child->firstChild();
                continue;
            }
        }

        while(true) {
            if(child->nextSibling()) {
                child = child->nextSibling();
                break;
            }

            child = child->parentBox();
            if(child == parent) {
                return;
            }
        }
    }
}

void HTMLElement::buildPseudoBox(Counters& counters, Box* parent, PseudoType pseudoType)
{
    if(pseudoType == PseudoType::Marker && !parent->isListItemBox())
        return;
    auto style = document()->pseudoStyleForElement(this, pseudoType, parent->style());
    if(style == nullptr || style->display() == Display::None) {
        return;
    }

    auto content = style->get(CSSPropertyID::Content);
    if(content == nullptr || content->id() == CSSValueID::None)
        return;
    if(pseudoType != PseudoType::Marker && content->id() == CSSValueID::Normal) {
        return;
    }

    auto box = Box::create(nullptr, style);
    parent->addChild(box);
    if(pseudoType == PseudoType::Before || pseudoType == PseudoType::After) {
        counters.update(box);
        buildPseudoBox(counters, box, PseudoType::Marker);
    }

    ContentBoxBuilder(counters, this, box).build(*content);
}

void HTMLElement::buildElementBox(Counters& counters, Box* box)
{
    counters.update(box);
    counters.push();
    buildPseudoBox(counters, box, PseudoType::Marker);
    buildPseudoBox(counters, box, PseudoType::Before);
    buildChildrenBox(counters, box);
    buildPseudoBox(counters, box, PseudoType::After);
    buildFirstLetterPseudoBox(box);
    counters.pop();
}

void HTMLElement::buildBox(Counters& counters, Box* parent)
{
    auto style = document()->styleForElement(this, parent->style());
    if(style == nullptr || style->display() == Display::None)
        return;
    if(style->position() == Position::Running) {
        const auto* value = style->get(CSSPropertyID::Position);
        const auto& position = to<CSSUnaryFunctionValue>(*value);
        assert(position.id() == CSSFunctionID::Running);
        const auto& name = to<CSSCustomIdentValue>(*position.value());
        document()->addRunningStyle(name.value(), std::move(style));
        return;
    }

    auto box = createBox(style);
    if(box == nullptr)
        return;
    parent->addChild(box);
    buildElementBox(counters, box);
}

static void addHTMLAttributeStyle(std::string& output, const std::string_view& name, const std::string_view& value)
{
    if(value.empty())
        return;
    output += name;
    output += ':';
    output += value;
    output += ';';
}

void HTMLElement::collectAttributeStyle(std::string& output, const GlobalString& name, const HeapString& value) const
{
    if(name == hiddenAttr) {
        addHTMLAttributeStyle(output, "display", "none");
    } else if(name == alignAttr) {
        addHTMLAttributeStyle(output, "text-align", value);
    } else {
        Element::collectAttributeStyle(output, name, value);
    }
}

template<typename T = int>
static std::optional<T> parseHTMLInteger(std::string_view input)
{
    constexpr auto isSigned = std::numeric_limits<T>::is_signed;
    stripLeadingAndTrailingSpaces(input);
    bool isNegative = false;
    if(!input.empty() && input.front() == '+')
        input.remove_prefix(1);
    else if(!input.empty() && isSigned && input.front() == '-') {
        input.remove_prefix(1);
        isNegative = true;
    }

    if(input.empty() || !isDigit(input.front()))
        return std::nullopt;
    T output = 0;
    do {
        output = output * 10 + input.front() - '0';
        input.remove_prefix(1);
    } while(!input.empty() && isDigit(input.front()));

    using SignedType = typename std::make_signed<T>::type;
    if(isNegative)
        output = -static_cast<SignedType>(output);
    return output;
}

static std::optional<unsigned> parseHTMLNonNegativeInteger(std::string_view input)
{
    return parseHTMLInteger<unsigned>(input);
}

template<typename T>
std::optional<T> HTMLElement::parseIntegerAttribute(const GlobalString& name) const
{
    const auto& value = getAttribute(name);
    if(!value.empty())
        return parseHTMLInteger<T>(value);
    return std::nullopt;
}

std::optional<unsigned> HTMLElement::parseNonNegativeIntegerAttribute(const GlobalString& name) const
{
    return parseIntegerAttribute<unsigned>(name);
}

static void addHTMLLengthAttributeStyle(std::string& output, const std::string_view& name, const std::string_view& value)
{
    if(value.empty())
        return;
    size_t index = 0;
    while(index < value.length() && isSpace(value[index]))
        ++index;
    size_t begin = index;
    while(index < value.length() && isDigit(value[index])) {
        ++index;
    }

    if(index == begin)
        return;
    if(index < value.length() && value[index] == '.') {
        ++index;
        while(index < value.length() && isDigit(value[index])) {
            ++index;
        }
    }

    output += name;
    output += ':';
    output += value.substr(begin, index - begin);
    if(index < value.length() && value[index] == '%') {
        output += "%;";
    } else {
        output += "px;";
    }
}

static void addHTMLLengthAttributeStyle(std::string& output, const std::string_view& name, int value)
{
    output += name;
    output += ':';
    output += toString(value);
    if(value) {
        output += "px;";
    } else {
        output += ';';
    }
}

static void addHTMLUrlAttributeStyle(std::string& output, const std::string_view& name, const std::string_view& value)
{
    if(value.empty())
        return;
    output += name;
    output += ':';
    output += "url(";
    output += value;
    output += ");";
}

HTMLBodyElement::HTMLBodyElement(Document* document)
    : HTMLElement(document, bodyTag)
{
}

void HTMLBodyElement::collectAttributeStyle(std::string& output, const GlobalString& name, const HeapString& value) const
{
    if(name == textAttr) {
        addHTMLAttributeStyle(output, "color", value);
    } else if(name == bgcolorAttr) {
        addHTMLAttributeStyle(output, "background-color", value);
    } else if(name == backgroundAttr) {
        addHTMLUrlAttributeStyle(output, "background-image", value);
    } else {
        HTMLElement::collectAttributeStyle(output, name, value);
    }
}

HTMLFontElement::HTMLFontElement(Document* document)
    : HTMLElement(document, fontTag)
{
}

static void addHTMLFontSizeAttributeStyle(std::string& output, std::string_view input)
{
    bool hasPlusSign = false;
    bool hasMinusSign = false;
    stripLeadingAndTrailingSpaces(input);
    if(!input.empty() && input.front() == '+') {
        input.remove_prefix(1);
        hasPlusSign = true;
    } else if(!input.empty() && input.front() == '-') {
        input.remove_prefix(1);
        hasMinusSign = true;
    }

    if(input.empty() || !isDigit(input.front()))
        return;
    int value = 0;
    do {
        value = value * 10 + input.front() - '0';
        input.remove_prefix(1);
    } while(!input.empty() && isDigit(input.front()));

    if(hasPlusSign)
        value += 3;
    else if(hasMinusSign) {
        value = 3 - value;
    }

    if(value > 7)
        value = 7;
    if(value < 1) {
        value = 1;
    }

    output += "font-size:";
    switch(value) {
    case 1:
        output += "x-small;";
        break;
    case 2:
        output += "small;";
        break;
    case 3:
        output += "medium;";
        break;
    case 4:
        output += "large;";
        break;
    case 5:
        output += "x-large;";
        break;
    case 6:
        output += "xx-large;";
        break;
    case 7:
        output += "xxx-large;";
        break;
    default:
        assert(false);
    }
}

void HTMLFontElement::collectAttributeStyle(std::string& output, const GlobalString& name, const HeapString& value) const
{
    if(name == sizeAttr) {
        addHTMLFontSizeAttributeStyle(output, value);
    } else if(name == faceAttr) {
        addHTMLAttributeStyle(output, "font-family", value);
    } else if(name == colorAttr) {
        addHTMLAttributeStyle(output, "color", value);
    } else {
        HTMLElement::collectAttributeStyle(output, name, value);
    }
}

HTMLImageElement::HTMLImageElement(Document* document)
    : HTMLElement(document, imgTag)
{
}

void HTMLImageElement::collectAttributeStyle(std::string& output, const GlobalString& name, const HeapString& value) const
{
    if(name == widthAttr) {
        addHTMLLengthAttributeStyle(output, "width", value);
    } else if(name == heightAttr) {
        addHTMLLengthAttributeStyle(output, "height", value);
    } else if(name == hspaceAttr) {
        addHTMLLengthAttributeStyle(output, "margin-left", value);
        addHTMLLengthAttributeStyle(output, "margin-right", value);
    } else if(name == vspaceAttr) {
        addHTMLLengthAttributeStyle(output, "margin-top", value);
        addHTMLLengthAttributeStyle(output, "margin-bottom", value);
    } else if(name == borderAttr) {
        addHTMLLengthAttributeStyle(output, "border-width", value);
        addHTMLAttributeStyle(output, "border-style", "solid");
    } else if(name == valignAttr) {
        addHTMLAttributeStyle(output, "vertical-align", value);
    } else {
        HTMLElement::collectAttributeStyle(output, name, value);
    }
}

const HeapString& HTMLImageElement::altText() const
{
    return getAttribute(altAttr);
}

RefPtr<Image> HTMLImageElement::srcImage() const
{
    auto url = getUrlAttribute(srcAttr);
    if(auto resource = document()->fetchImageResource(url))
        return resource->image();
    return nullptr;
}

Box* HTMLImageElement::createBox(const RefPtr<BoxStyle>& style)
{
    auto image = srcImage();
    auto text = altText();
    if(image == nullptr && text.empty())
        return new (heap()) ImageBox(this, style);
    if(image == nullptr) {
        auto container = Box::create(this, style);
        auto box = new (heap()) TextBox(nullptr, style);
        box->setText(text);
        container->addChild(box);
        return container;
    }

    auto box = new (heap()) ImageBox(this, style);
    box->setImage(std::move(image));
    return box;
}

HTMLHRElement::HTMLHRElement(Document* document)
    : HTMLElement(document, hrTag)
{
}

void HTMLHRElement::collectAttributeStyle(std::string& output, const GlobalString& name, const HeapString& value) const
{
    if(name == widthAttr) {
        addHTMLLengthAttributeStyle(output, "width", value);
    } else if(name == sizeAttr) {
        auto size = parseHTMLInteger(value);
        if(size && size.value() > 1) {
            addHTMLLengthAttributeStyle(output, "height", size.value() - 2);
        } else {
            addHTMLLengthAttributeStyle(output, "border-bottom-width", 0);
        }
    } else if(name == alignAttr) {
        if(equalsIgnoringCase(value, "left")) {
            addHTMLLengthAttributeStyle(output, "margin-left", 0);
            addHTMLAttributeStyle(output, "margin-right", "auto");
        } else if(equalsIgnoringCase(value, "right")) {
            addHTMLAttributeStyle(output, "margin-left", "auto");
            addHTMLLengthAttributeStyle(output, "margin-right", 0);
        } else {
            addHTMLAttributeStyle(output, "margin-left", "auto");
            addHTMLAttributeStyle(output, "margin-right", "auto");
        }
    } else if(name == colorAttr) {
        addHTMLAttributeStyle(output, "border-style", "solid");
        addHTMLAttributeStyle(output, "border-color", value);
        addHTMLAttributeStyle(output, "background-color", value);
    } else if(name == noshadeAttr) {
        addHTMLAttributeStyle(output, "border-style", "solid");
        addHTMLAttributeStyle(output, "border-color", "darkgray");
        addHTMLAttributeStyle(output, "background-color", "darkgray");
    } else {
        HTMLElement::collectAttributeStyle(output, name, value);
    }
}

HTMLBRElement::HTMLBRElement(Document* document)
    : HTMLElement(document, brTag)
{
}

Box* HTMLBRElement::createBox(const RefPtr<BoxStyle>& style)
{
    return new (heap()) LineBreakBox(this, style);
}

HTMLWBRElement::HTMLWBRElement(Document* document)
    : HTMLElement(document, wbrTag)
{
}

Box* HTMLWBRElement::createBox(const RefPtr<BoxStyle>& style)
{
    return new (heap()) WordBreakBox(this, style);
}

HTMLLIElement::HTMLLIElement(Document* document)
    : HTMLElement(document, liTag)
{
}

std::optional<int> HTMLLIElement::value() const
{
    return parseIntegerAttribute(valueAttr);
}

std::string_view listTypeAttributeToStyleName(const std::string_view& value)
{
    if(value == "a")
        return "lower-alpha";
    if(value == "A")
        return "upper-alpha";
    if(value == "i")
        return "lower-roman";
    if(value == "I")
        return "upper-roman";
    if(value == "1") {
        return "decimal";
    }

    return value;
}

void HTMLLIElement::collectAttributeStyle(std::string& output, const GlobalString& name, const HeapString& value) const
{
    if(name == typeAttr) {
        addHTMLAttributeStyle(output, "list-style-type", listTypeAttributeToStyleName(value));
    } else {
        HTMLElement::collectAttributeStyle(output, name, value);
    }
}

HTMLOLElement::HTMLOLElement(Document* document)
    : HTMLElement(document, olTag)
{
}

int HTMLOLElement::start() const
{
    return parseIntegerAttribute(startAttr).value_or(1);
}

void HTMLOLElement::collectAttributeStyle(std::string& output, const GlobalString& name, const HeapString& value) const
{
    if(name == typeAttr) {
        addHTMLAttributeStyle(output, "list-style-type", listTypeAttributeToStyleName(value));
    } else {
        HTMLElement::collectAttributeStyle(output, name, value);
    }
}

HTMLTableElement::HTMLTableElement(Document* document)
    : HTMLElement(document, tableTag)
{
}

void HTMLTableElement::parseAttribute(const GlobalString& name, const HeapString& value)
{
    if(name == cellpaddingAttr) {
        m_padding = parseHTMLNonNegativeInteger(value).value_or(0);
    } else if(name == borderAttr) {
        m_border = parseHTMLNonNegativeInteger(value).value_or(1);
    } else if(name == rulesAttr) {
        m_rules = parseRulesAttribute(value);
    } else if(name == frameAttr) {
        m_frame = parseFrameAttribute(value);
    } else {
        HTMLElement::parseAttribute(name, value);
    }
}

void HTMLTableElement::collectAdditionalCellAttributeStyle(std::string& output) const
{
    if(m_padding > 0) {
        addHTMLLengthAttributeStyle(output, "padding", m_padding);
    }

    if(m_border > 0 && m_rules == Rules::Unset) {
        addHTMLAttributeStyle(output, "border-width", "inset");
        addHTMLAttributeStyle(output, "border-style", "solid");
        addHTMLAttributeStyle(output, "border-color", "inherit");
    } else {
        switch(m_rules) {
        case Rules::Rows:
            addHTMLAttributeStyle(output, "border-top-width", "thin");
            addHTMLAttributeStyle(output, "border-bottom-width", "thin");
            addHTMLAttributeStyle(output, "border-top-style", "solid");
            addHTMLAttributeStyle(output, "border-bottom-style", "solid");
            addHTMLAttributeStyle(output, "border-color", "inherit");
        case Rules::Cols:
            addHTMLAttributeStyle(output, "border-left-width", "thin");
            addHTMLAttributeStyle(output, "border-right-width", "thin");
            addHTMLAttributeStyle(output, "border-left-style", "solid");
            addHTMLAttributeStyle(output, "border-right-style", "solid");
            addHTMLAttributeStyle(output, "border-color", "inherit");
            break;
        case Rules::All:
            addHTMLAttributeStyle(output, "border-width", "thin");
            addHTMLAttributeStyle(output, "border-style", "solid");
            addHTMLAttributeStyle(output, "border-color", "inherit");
            break;
        default:
            break;
        }
    }
}

void HTMLTableElement::collectAdditionalRowGroupAttributeStyle(std::string& output) const
{
    if(m_rules == Rules::Groups) {
        addHTMLAttributeStyle(output, "border-top-width", "thin");
        addHTMLAttributeStyle(output, "border-bottom-width", "thin");
        addHTMLAttributeStyle(output, "border-top-style", "solid");
        addHTMLAttributeStyle(output, "border-bottom-style", "solid");
    }
}

void HTMLTableElement::collectAdditionalColGroupAttributeStyle(std::string& output) const
{
    if(m_rules == Rules::Groups) {
        addHTMLAttributeStyle(output, "border-left-width", "thin");
        addHTMLAttributeStyle(output, "border-right-width", "thin");
        addHTMLAttributeStyle(output, "border-left-style", "solid");
        addHTMLAttributeStyle(output, "border-right-style", "solid");
    }
}

void HTMLTableElement::collectAdditionalAttributeStyle(std::string& output) const
{
    HTMLElement::collectAdditionalAttributeStyle(output);
    if(m_rules > Rules::Unset) {
        addHTMLAttributeStyle(output, "border-collapse", "collapse");
    }

    if(m_frame > Frame::Unset) {
        auto topStyle = "hidden";
        auto bottomStyle = "hidden";
        auto leftStyle = "hidden";
        auto rightStyle = "hidden";
        switch(m_frame) {
        case Frame::Above:
            topStyle = "solid";
            break;
        case Frame::Below:
            bottomStyle = "solid";
            break;
        case Frame::Hsides:
            topStyle = bottomStyle = "solid";
            break;
        case Frame::Lhs:
            leftStyle = "solid";
            break;
        case Frame::Rhs:
            rightStyle = "solid";
            break;
        case Frame::Vsides:
            leftStyle = rightStyle = "solid";
            break;
        case Frame::Box:
        case Frame::Border:
            topStyle = bottomStyle = "solid";
            leftStyle = rightStyle = "solid";
            break;
        default:
            break;
        }

        addHTMLAttributeStyle(output, "border-width", "thin");
        addHTMLAttributeStyle(output, "border-top-style", topStyle);
        addHTMLAttributeStyle(output, "border-bottom-style", bottomStyle);
        addHTMLAttributeStyle(output, "border-left-style", leftStyle);
        addHTMLAttributeStyle(output, "border-right-style", rightStyle);
    } else {
        if(m_border > 0) {
            addHTMLLengthAttributeStyle(output, "border-width", m_border);
            addHTMLAttributeStyle(output, "border-style", "outset");
        } else if(m_rules > Rules::Unset) {
            addHTMLAttributeStyle(output, "border-style", "hidden");
        }
    }
}

void HTMLTableElement::collectAttributeStyle(std::string& output, const GlobalString& name, const HeapString& value) const
{
    if(name == widthAttr) {
        addHTMLLengthAttributeStyle(output, "width", value);
    } else if(name == heightAttr) {
        addHTMLLengthAttributeStyle(output, "height", value);
    } else if(name == valignAttr) {
        addHTMLAttributeStyle(output, "vertical-align", value);
    } else if(name == cellspacingAttr) {
        addHTMLLengthAttributeStyle(output, "border-spacing", value);
    } else if(name == bordercolorAttr) {
        addHTMLAttributeStyle(output, "border-color", value);
    } else if(name == bgcolorAttr) {
        addHTMLAttributeStyle(output, "background-color", value);
    } else if(name == backgroundAttr) {
        addHTMLUrlAttributeStyle(output, "background-image", value);
    } else {
        HTMLElement::collectAttributeStyle(output, name, value);
    }
}

HTMLTableElement::Rules HTMLTableElement::parseRulesAttribute(std::string_view value)
{
    if(equalsIgnoringCase(value, "none"))
        return Rules::None;
    if(equalsIgnoringCase(value, "groups"))
        return Rules::Groups;
    if(equalsIgnoringCase(value, "rows"))
        return Rules::Rows;
    if(equalsIgnoringCase(value, "cols"))
        return Rules::Cols;
    if(equalsIgnoringCase(value, "all"))
        return Rules::All;
    return Rules::Unset;
}

HTMLTableElement::Frame HTMLTableElement::parseFrameAttribute(std::string_view value)
{
    if(equalsIgnoringCase(value, "void"))
        return Frame::Void;
    if(equalsIgnoringCase(value, "above"))
        return Frame::Above;
    if(equalsIgnoringCase(value, "below"))
        return Frame::Below;
    if(equalsIgnoringCase(value, "hsides"))
        return Frame::Hsides;
    if(equalsIgnoringCase(value, "lhs"))
        return Frame::Lhs;
    if(equalsIgnoringCase(value, "rhs"))
        return Frame::Rhs;
    if(equalsIgnoringCase(value, "vsides"))
        return Frame::Vsides;
    if(equalsIgnoringCase(value, "box"))
        return Frame::Box;
    if(equalsIgnoringCase(value, "border"))
        return Frame::Border;
    return Frame::Unset;
}

HTMLTablePartElement::HTMLTablePartElement(Document* document, const GlobalString& tagName)
    : HTMLElement(document, tagName)
{
}

HTMLTableElement* HTMLTablePartElement::findParentTable() const
{
    auto parent = parentElement();
    while(parent && !parent->isOfType(xhtmlNs, tableTag))
        parent = parent->parentElement();
    return static_cast<HTMLTableElement*>(parent);
}

void HTMLTablePartElement::collectAttributeStyle(std::string& output, const GlobalString& name, const HeapString& value) const
{
    if(name == heightAttr) {
        addHTMLLengthAttributeStyle(output, "height", value);
    } else if(name == valignAttr) {
        addHTMLAttributeStyle(output, "vertical-align", value);
    } else if(name == bgcolorAttr) {
        addHTMLAttributeStyle(output, "background-color", value);
    } else if(name == backgroundAttr) {
        addHTMLUrlAttributeStyle(output, "background-image", value);
    } else {
        HTMLElement::collectAttributeStyle(output, name, value);
    }
}

HTMLTableSectionElement::HTMLTableSectionElement(Document* document, const GlobalString& tagName)
    : HTMLTablePartElement(document, tagName)
{
}

void HTMLTableSectionElement::collectAdditionalAttributeStyle(std::string& output) const
{
    HTMLTablePartElement::collectAdditionalAttributeStyle(output);
    if(auto table = findParentTable()) {
        table->collectAdditionalRowGroupAttributeStyle(output);
    }
}

HTMLTableRowElement::HTMLTableRowElement(Document* document)
    : HTMLTablePartElement(document, trTag)
{
}

HTMLTableColElement::HTMLTableColElement(Document* document, const GlobalString& tagName)
    : HTMLTablePartElement(document, tagName)
{
}

unsigned HTMLTableColElement::span() const
{
    return parseNonNegativeIntegerAttribute(spanAttr).value_or(1);
}

void HTMLTableColElement::collectAttributeStyle(std::string& output, const GlobalString& name, const HeapString& value) const
{
    if(name == widthAttr) {
        addHTMLLengthAttributeStyle(output, "width", value);
    } else {
        HTMLTablePartElement::collectAttributeStyle(output, name, value);
    }
}

void HTMLTableColElement::collectAdditionalAttributeStyle(std::string& output) const
{
    HTMLTablePartElement::collectAdditionalAttributeStyle(output);
    if(tagName() == colgroupTag) {
        if(auto table = findParentTable()) {
            table->collectAdditionalColGroupAttributeStyle(output);
        }
    }
}

Box* HTMLTableColElement::createBox(const RefPtr<BoxStyle>& style)
{
    auto box = HTMLElement::createBox(style);
    if(auto column = to<TableColumnBox>(box))
        column->setSpan(span());
    return box;
}

HTMLTableCellElement::HTMLTableCellElement(Document* document, const GlobalString& tagName)
    : HTMLTablePartElement(document, tagName)
{
}

unsigned HTMLTableCellElement::colSpan() const
{
    return std::max(1u, parseNonNegativeIntegerAttribute(colspanAttr).value_or(1));
}

unsigned HTMLTableCellElement::rowSpan() const
{
    return parseNonNegativeIntegerAttribute(rowspanAttr).value_or(1);
}

void HTMLTableCellElement::collectAttributeStyle(std::string& output, const GlobalString& name, const HeapString& value) const
{
    if(name == widthAttr) {
        addHTMLLengthAttributeStyle(output, "width", value);
    } else {
        HTMLTablePartElement::collectAttributeStyle(output, name, value);
    }
}

void HTMLTableCellElement::collectAdditionalAttributeStyle(std::string& output) const
{
    HTMLTablePartElement::collectAdditionalAttributeStyle(output);
    if(auto table = findParentTable()) {
        table->collectAdditionalCellAttributeStyle(output);
    }
}

Box* HTMLTableCellElement::createBox(const RefPtr<BoxStyle>& style)
{
    auto box = HTMLElement::createBox(style);
    if(auto cell = to<TableCellBox>(box)) {
        cell->setColSpan(colSpan());
        cell->setRowSpan(rowSpan());
    }

    return box;
}

HTMLInputElement::HTMLInputElement(Document* document)
    : HTMLElement(document, inputTag)
{
}

unsigned HTMLInputElement::size() const
{
    return std::max(1u, parseNonNegativeIntegerAttribute(sizeAttr).value_or(20));
}

Box* HTMLInputElement::createBox(const RefPtr<BoxStyle>& style)
{
    const auto& type = getAttribute(typeAttr);
    if(!type.empty() && !equals(type, "text", false)
        && !equals(type, "search", false)
        && !equals(type, "url", false)
        && !equals(type, "tel", false)
        && !equals(type, "email", false)
        && !equals(type, "password", false)) {
        return HTMLElement::createBox(style);
    }

    auto box = new (heap()) TextInputBox(this, style);
    box->setCols(size());
    return box;
}

HTMLTextAreaElement::HTMLTextAreaElement(Document* document)
    : HTMLElement(document, textareaTag)
{
}

unsigned HTMLTextAreaElement::rows() const
{
    return std::max(1u, parseNonNegativeIntegerAttribute(rowsAttr).value_or(2));
}

unsigned HTMLTextAreaElement::cols() const
{
    return std::max(1u, parseNonNegativeIntegerAttribute(colsAttr).value_or(20));
}

Box* HTMLTextAreaElement::createBox(const RefPtr<BoxStyle>& style)
{
    auto box = new (heap()) TextInputBox(this, style);
    box->setRows(rows());
    box->setCols(cols());
    return box;
}

HTMLSelectElement::HTMLSelectElement(Document* document)
    : HTMLElement(document, selectTag)
{
}

unsigned HTMLSelectElement::size() const
{
    if(auto size = parseNonNegativeIntegerAttribute(sizeAttr))
        return std::max(1u, size.value());
    return hasAttribute(multipleAttr) ? 4 : 1;
}

Box* HTMLSelectElement::createBox(const RefPtr<BoxStyle>& style)
{
    return new (heap()) SelectBox(this, style);
}

HTMLStyleElement::HTMLStyleElement(Document* document)
    : HTMLElement(document, styleTag)
{
}

const HeapString& HTMLStyleElement::type() const
{
    return getAttribute(typeAttr);
}

const HeapString& HTMLStyleElement::media() const
{
    return getAttribute(mediaAttr);
}

void HTMLStyleElement::finishParsingDocument()
{
    if(document()->supportsMedia(type(), media()))
        document()->addAuthorStyleSheet(textFromChildren(), document()->baseUrl());
    HTMLElement::finishParsingDocument();
}

HTMLLinkElement::HTMLLinkElement(Document* document)
    : HTMLElement(document, linkTag)
{
}

const HeapString& HTMLLinkElement::rel() const
{
    return getAttribute(relAttr);
}

const HeapString& HTMLLinkElement::type() const
{
    return getAttribute(typeAttr);
}

const HeapString& HTMLLinkElement::media() const
{
    return getAttribute(mediaAttr);
}

void HTMLLinkElement::finishParsingDocument()
{
    if(equals(rel(), "stylesheet", false) && document()->supportsMedia(type(), media())) {
        auto url = getUrlAttribute(hrefAttr);
        if(auto resource = document()->fetchTextResource(url)) {
            document()->addAuthorStyleSheet(resource->text(), std::move(url));
        }
    }

    HTMLElement::finishParsingDocument();
}

HTMLTitleElement::HTMLTitleElement(Document* document)
    : HTMLElement(document, titleTag)
{
}

void HTMLTitleElement::finishParsingDocument()
{
    auto book = document()->book();
    if(book && book->title().empty())
        book->setTitle(textFromChildren());
    HTMLElement::finishParsingDocument();
}

HTMLBaseElement::HTMLBaseElement(Document* document)
    : HTMLElement(document, baseTag)
{
}

void HTMLBaseElement::finishParsingDocument()
{
    Url baseUrl(getAttribute(hrefAttr));
    if(!baseUrl.isEmpty())
        document()->setBaseUrl(std::move(baseUrl));
    HTMLElement::finishParsingDocument();
}

std::unique_ptr<HTMLDocument> HTMLDocument::create(Book* book, Heap* heap, ResourceFetcher* fetcher, Url baseUrl)
{
    return std::unique_ptr<HTMLDocument>(new (heap) HTMLDocument(book, heap, fetcher, std::move(baseUrl)));
}

bool HTMLDocument::parse(const std::string_view& content)
{
    return HTMLParser(this, content).parse();
}

HTMLDocument::HTMLDocument(Book* book, Heap* heap, ResourceFetcher* fetcher, Url baseUrl)
    : Document(book, heap, fetcher, std::move(baseUrl))
{
}

} // namespace plutobook
