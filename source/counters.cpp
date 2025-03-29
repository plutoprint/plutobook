#include "counters.h"
#include "htmldocument.h"
#include "box.h"
#include "cssrule.h"

namespace plutobook {

static const GlobalString pageGlo("page");
static const GlobalString pagesGlo("pages");
static const GlobalString listItemGlo("list-item");

Counters::Counters(Document* document, uint32_t pageCount)
    : m_document(document)
    , m_pageCount(pageCount)
{
    if(m_pageCount) {
        m_scopes.push_back({pagesGlo});
        m_values[pagesGlo].push_back(pageCount);
    }
}

void Counters::push()
{
    m_scopes.push_back({});
}

void Counters::pop()
{
    for(auto& name : m_scopes.back()) {
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
    auto hasListItemCounter = false;
    auto hasPageCounter = false;
    for(auto id : { CSSPropertyID::CounterReset, CSSPropertyID::CounterIncrement, CSSPropertyID::CounterSet }) {
        auto counters = box->style()->get(id);
        if(counters == nullptr || counters->id() == CSSValueID::None)
            continue;
        for(auto& counter : to<CSSListValue>(*counters)) {
            auto& pair = to<CSSPairValue>(*counter);
            auto& name = to<CSSCustomIdentValue>(*pair.first());
            auto& value = to<CSSIntegerValue>(*pair.second());
            hasListItemCounter |= listItemGlo == name.value();
            hasPageCounter |= pageGlo == name.value();
            if(m_pageCount && pagesGlo == name.value())
                continue;
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

    auto element = to<HTMLElement>(box->node());
    if(element && !hasListItemCounter) {
        if(element->tagName() == olTag) {
            auto olElement = static_cast<HTMLOLElement*>(element);
            reset(listItemGlo, olElement->start() - 1);
            hasListItemCounter = true;
        } else if(element->tagName() == ulTag
            || element->tagName() == dirTag
            || element->tagName() == menuTag) {
            reset(listItemGlo, 0);
            hasListItemCounter = true;
        } else if(element->tagName() == liTag) {
            auto liElement = static_cast<HTMLLIElement*>(element);
            if(auto value = liElement->value()) {
                reset(listItemGlo, *value);
                hasListItemCounter = true;
            }
        }
    }

    if(!hasListItemCounter && box->isListItemBox())
        increment(listItemGlo, 1);
    if(!hasPageCounter && box->isPageBox())
        increment(pageGlo, 1);
    if(element && !m_values.empty() && !element->id().empty()) {
        m_document->addTargetCounters(element->id(), m_values);
    }
}

HeapString Counters::counterText(const GlobalString& name, const GlobalString& listStyle, const HeapString& separator) const
{
    return m_document->getCountersText(m_values, name, listStyle, separator);
}

HeapString Counters::markerText(const GlobalString& listStyle) const
{
    int value = 0;
    auto it = m_values.find(listItemGlo);
    if(it != m_values.end() && !it->second.empty())
        value = it->second.back();
    return m_document->heap()->createString(m_document->getMarkerText(value, listStyle));
}

} // namespace plutobook
