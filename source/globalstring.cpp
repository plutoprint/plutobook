/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "globalstring.h"
#include "stringutils.h"

#include <mutex>
#include <set>

namespace plutobook {

class GlobalStringTable {
public:
    GlobalStringTable();

    const HeapString* add(std::string_view value);

private:
    using StringSet = std::pmr::set<HeapString, std::less<>>;
    Heap m_heap;
    StringSet m_table;
    std::mutex m_mutex;
};

GlobalStringTable::GlobalStringTable()
    : m_heap(1024 * 24)
    , m_table(&m_heap)
{
}

const HeapString* GlobalStringTable::add(std::string_view value)
{
    std::lock_guard guard(m_mutex);
    auto lb = m_table.lower_bound(value);
    if(lb != m_table.end() && *lb == value)
        return &*lb;
    return &*m_table.emplace_hint(lb, m_heap.createString(value));
}

GlobalStringTable* globalStringTable()
{
    static GlobalStringTable table;
    return &table;
}

GlobalString::GlobalString(std::string_view value)
    : m_entry(globalStringTable()->add(value))
{
}

GlobalString GlobalString::foldCase() const
{
    if(m_entry == nullptr)
        return nullGlo;
    auto size = m_entry->size();
    auto data = m_entry->data();

    size_t index = 0;
    while(index < size && !isUpper(data[index]))
        ++index;
    if(index == size) {
        return *this;
    }

    constexpr auto kBufferSize = 128;
    if(size <= kBufferSize) {
        char buffer[kBufferSize];
        for(size_t i = 0; i < index; i++)
            buffer[i] = data[i];
        for(size_t i = index; i < size; i++) {
            buffer[i] = toLower(data[i]);
        }

        return GlobalString({buffer, size});
    }

    std::string value(data, size);
    for(size_t i = index; i < size; i++) {
        value[i] = toLower(data[i]);
    }

    return GlobalString(value);
}

} // namespace plutobook
