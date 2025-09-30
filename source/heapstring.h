#ifndef PLUTOBOOK_HEAPSTRING_H
#define PLUTOBOOK_HEAPSTRING_H

#include <string_view>
#include <memory_resource>
#include <ostream>
#include <cstring>

namespace plutobook {

class HeapString {
public:
    HeapString() = default;

    const char* data() const { return m_value.data(); }
    size_t size() const { return m_value.size(); }

    const char* begin() const { return m_value.data(); }
    const char* end() const { return m_value.data() + m_value.size(); }

    const char& at(size_t index) const { return m_value.at(index); }
    const char& operator[](size_t index) const { return m_value.operator[](index); }

    const char& front() const { return m_value.front(); }
    const char& back() const { return m_value.back(); }

    bool empty() const { return m_value.empty(); }

    HeapString substring(size_t offset) const { return m_value.substr(offset); }
    HeapString substring(size_t offset, size_t count) const { return m_value.substr(offset, count); }

    const std::string_view& value() const { return m_value; }
    operator const std::string_view&() const { return m_value; }

private:
    HeapString(const std::string_view& value) : m_value(value) {}
    std::string_view m_value;
    friend class Heap;
};

inline std::ostream& operator<<(std::ostream& o, const HeapString& in) { return o << in.value(); }

inline bool operator==(const HeapString& a, const HeapString& b) { return a.value() == b.value(); }
inline bool operator!=(const HeapString& a, const HeapString& b) { return a.value() != b.value(); }

inline bool operator<(const HeapString& a, const HeapString& b) { return a.value() < b.value(); }
inline bool operator>(const HeapString& a, const HeapString& b) { return a.value() > b.value(); }

inline bool operator==(const HeapString& a, const std::string_view& b) { return a.value() == b; }
inline bool operator!=(const HeapString& a, const std::string_view& b) { return a.value() != b; }

inline bool operator==(const std::string_view& a, const HeapString& b) { return a == b.value(); }
inline bool operator!=(const std::string_view& a, const HeapString& b) { return a != b.value(); }

inline bool operator<(const HeapString& a, const std::string_view& b) { return a.value() < b; }
inline bool operator>(const HeapString& a, const std::string_view& b) { return a.value() > b; }

inline bool operator<(const std::string_view& a, const HeapString& b) { return a < b.value(); }
inline bool operator>(const std::string_view& a, const HeapString& b) { return a > b.value(); }

using HeapBase = std::pmr::monotonic_buffer_resource;

class Heap : public HeapBase {
public:
    explicit Heap(size_t capacity) : HeapBase(capacity) {}

    HeapString createString(const std::string_view& value);
    HeapString concatenateString(const std::string_view& a, const std::string_view& b);
};

inline HeapString Heap::createString(const std::string_view& value)
{
    auto content = static_cast<char*>(allocate(value.size(), alignof(char)));
    std::memcpy(content, value.data(), value.size());
    return HeapString({content, value.size()});
}

inline HeapString Heap::concatenateString(const std::string_view& a, const std::string_view& b)
{
    auto content = static_cast<char*>(allocate(a.size() + b.size(), alignof(char)));
    std::memcpy(content, a.data(), a.size());
    std::memcpy(content + a.size(), b.data(), b.size());
    return HeapString({content, a.size() + b.size()});
}

class HeapMember {
public:
    HeapMember() = default;

    static void* operator new(size_t size, Heap* heap) { return heap->allocate(size); }
    static void* operator new[](size_t size, Heap* heap) { return heap->allocate(size); }

    static void operator delete(void* data, Heap* heap) {}
    static void operator delete[](void* data, Heap* heap) {}

    static void operator delete(void* data) {}
    static void operator delete[](void* data) {}

private:
    HeapMember(const HeapMember&) = delete;
    HeapMember& operator=(const HeapMember&) = delete;
};

} // namespace plutobook

#endif // PLUTOBOOK_HEAPSTRING_H
