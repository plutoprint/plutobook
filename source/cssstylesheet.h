/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_CSSSTYLESHEET_H
#define PLUTOBOOK_CSSSTYLESHEET_H

#include "pointer.h"

#include <memory_resource>
#include <vector>
#include <memory>
#include <map>

namespace plutobook {

class CSSRule;
class CSSRuleData;
class CSSStyleRule;
class CSSImportRule;
class CSSPageRule;
class CSSPageRuleData;
class CSSFontFaceRule;
class CSSCounterStyleRule;
class CSSCounterStyleMap;
class CSSCounterStyle;
class CSSMediaRule;

enum class CSSStyleOrigin : uint8_t;

using CSSRuleList = std::pmr::vector<RefPtr<CSSRule>>;
using CSSRuleDataList = std::pmr::vector<CSSRuleData>;
using CSSPageRuleDataList = std::pmr::vector<CSSPageRuleData>;

class Heap;

template<typename T>
class CSSRuleDataMap {
public:
    explicit CSSRuleDataMap(Heap* heap) : m_table(heap) {}

    bool add(const T& name, CSSRuleData&& rule);
    const CSSRuleDataList* get(const T& name) const;

private:
    std::pmr::map<T, CSSRuleDataList> m_table;
};

template<typename T>
bool CSSRuleDataMap<T>::add(const T& name, CSSRuleData&& rule)
{
    auto resource = m_table.get_allocator().resource();
    auto [it, inserted] = m_table.try_emplace(name, CSSRuleDataList(resource));
    it->second.push_back(std::move(rule));
    return inserted;
}

template<typename T>
const CSSRuleDataList* CSSRuleDataMap<T>::get(const T& name) const
{
    auto it = m_table.find(name);
    if(it == m_table.end())
        return nullptr;
    return &it->second;
}

class HeapString;
class GlobalString;

class FontFace;
class FontData;
class SegmentedFontFace;

struct FontDataDescription;
struct FontSelectionDescription;

class CSSFontFaceCache {
public:
    explicit CSSFontFaceCache(Heap* heap);

    RefPtr<FontData> get(const GlobalString& family, const FontDataDescription& description) const;
    void add(const GlobalString& family, const FontSelectionDescription& description, RefPtr<FontFace> face);

private:
    std::pmr::map<GlobalString, std::map<FontSelectionDescription, RefPtr<SegmentedFontFace>>> m_table;
};

class BoxStyle;
class Document;
class Element;
class Url;

enum class PseudoType : uint8_t;
enum class PageMarginType : uint8_t;

class CSSStyleSheet {
public:
    explicit CSSStyleSheet(Document* document);

    RefPtr<BoxStyle> styleForElement(Element* element, const BoxStyle* parentStyle) const;
    RefPtr<BoxStyle> pseudoStyleForElement(Element* element, PseudoType pseudoType, const BoxStyle* parentStyle) const;
    RefPtr<BoxStyle> styleForPage(const GlobalString& pageName, uint32_t pageIndex, PseudoType pseudoType) const;
    RefPtr<BoxStyle> styleForPageMargin(const GlobalString& pageName, uint32_t pageIndex, PageMarginType marginType, const BoxStyle* pageStyle) const;
    RefPtr<FontData> getFontData(const GlobalString& family, const FontDataDescription& description) const;

    const CSSCounterStyle& getCounterStyle(const GlobalString& name);
    std::string getCounterText(int value, const GlobalString& listType);
    std::string getMarkerText(int value, const GlobalString& listType);

    void parseStyle(const std::string_view& content, CSSStyleOrigin origin, Url baseUrl);

private:
    void addRuleList(const CSSRuleList& rules);
    void addStyleRule(const RefPtr<CSSStyleRule>& rule);
    void addImportRule(const RefPtr<CSSImportRule>& rule);
    void addPageRule(const RefPtr<CSSPageRule>& rule);
    void addFontFaceRule(const RefPtr<CSSFontFaceRule>& rule);
    void addCounterStyleRule(const RefPtr<CSSCounterStyleRule>& rule);
    void addMediaRule(const RefPtr<CSSMediaRule>& rule);

    Document* m_document;
    uint32_t m_position{0};
    uint32_t m_importDepth{0};

    CSSRuleDataMap<HeapString> m_idRules;
    CSSRuleDataMap<HeapString> m_classRules;
    CSSRuleDataMap<GlobalString> m_tagRules;
    CSSRuleDataMap<GlobalString> m_attributeRules;
    CSSRuleDataMap<PseudoType> m_pseudoRules;

    CSSRuleDataList m_universalRules;
    CSSPageRuleDataList m_pageRules;
    CSSRuleList m_counterStyleRules;
    CSSFontFaceCache m_fontFaceCache;
    std::unique_ptr<CSSCounterStyleMap> m_counterStyleMap;
};

} // namespace plutobook

#endif // PLUTOBOOK_CSSSTYLESHEET_H
