#include "counters.h"
#include "htmldocument.h"
#include "box.h"
#include "cssrule.h"

namespace plutobook {

Counters::Counters(Document* document)
    : m_document(document)
{
}

void Counters::push()
{
    m_scopes.push_back({});
}

void Counters::pop()
{
    for(auto name : m_scopes.back()) {
        m_values[name].pop_back();
    }

    m_scopes.pop_back();
}

void Counters::reset(const GlobalString& name, int value)
{
    auto& scopes = m_scopes.back();
    auto& values = m_values[name];
    if(!scopes.contains(name)) {
        scopes.insert(name);
        values.push_back(value);
    } else {
        values.back() = value;
    }
}

void Counters::increment(const GlobalString& name, int value)
{
    auto& scopes = m_scopes.back();
    auto& values = m_values[name];
    if(values.empty()) {
        scopes.insert(name);
        values.push_back(value);
    } else {
        values.back() += value;
    }
}

void Counters::set(const GlobalString& name, int value)
{
    auto& scopes = m_scopes.back();
    auto& values = m_values[name];
    if(values.empty()) {
        scopes.insert(name);
        values.push_back(value);
    } else {
        values.back() = value;
    }
}

void Counters::update(const Box* box)
{
    auto hasListItem = false;
    static const GlobalString listItem("list-item");
    for(auto id : { CSSPropertyID::CounterReset, CSSPropertyID::CounterIncrement, CSSPropertyID::CounterSet }) {
        auto value = box->style()->get(id);
        if(value == nullptr || value->id() == CSSValueID::None)
            continue;
        for(auto& counter : to<CSSListValue>(*value)) {
            auto& pair = to<CSSPairValue>(*counter);
            auto& name = to<CSSCustomIdentValue>(*pair.first());
            auto& value = to<CSSIntegerValue>(*pair.second());
            hasListItem |= listItem == name.value();
            switch(id) {
            case CSSPropertyID::CounterReset:
                reset(name.value(), value.value());
                break;
            case CSSPropertyID::CounterIncrement:
                increment(name.value(), value.value());
                break;
            case CSSPropertyID::CounterSet:
                set(name.value(), value.value());
                break;
            default:
                assert(false);
            }
        }
    }

    if(hasListItem)
        return;
    auto element = to<HTMLElement>(box->node());
    if(box->isListItemBox()) {
        if(element && element->tagName() == liTag) {
            auto liElement = static_cast<HTMLLIElement*>(element);
            if(auto value = liElement->value()) {
                reset(listItem, *value);
                return;
            }
        }

        increment(listItem, 1);
        return;
    }

    if(element == nullptr)
        return;
    if(element->tagName() == olTag) {
        auto olElement = static_cast<HTMLOLElement*>(element);
        reset(listItem, olElement->start() - 1);
        return;
    }

    if(element->tagName() == ulTag
        || element->tagName() == dirTag
        || element->tagName() == menuTag) {
        reset(listItem, 0);
    }
}

HeapString Counters::counterText(const GlobalString& name, const GlobalString& listStyle, const HeapString& separator) const
{
    auto heap = m_document->heap();
    auto it = m_values.find(name);
    if(it == m_values.end())
        return heap->createString(m_document->getCounterText(0, listStyle));
    if(separator.empty()) {
        int value = 0;
        if(!it->second.empty())
            value = it->second.back();
        return heap->createString(m_document->getCounterText(value, listStyle));
    }

    std::string text;
    for(auto value : it->second) {
        if(!text.empty())
            text += separator.value();
        text += m_document->getCounterText(value, listStyle);
    }

    return heap->createString(text);
}

HeapString Counters::markerText(const GlobalString& listStyle) const
{
    static const GlobalString name("list-item");
    auto heap = m_document->heap();
    auto it = m_values.find(name);
    if(it == m_values.end())
        return heap->createString(m_document->getMarkerText(0, listStyle));
    int value = 0;
    if(!it->second.empty())
        value = it->second.back();
    return heap->createString(m_document->getMarkerText(value, listStyle));
}

} // namespace plutobook
