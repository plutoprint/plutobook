#ifndef PLUTOBOOK_URL_H
#define PLUTOBOOK_URL_H

#include <string>
#include <ostream>

namespace plutobook {

class Url {
public:
    Url() = default;
    explicit Url(const std::string_view& input);

    Url complete(std::string_view input) const;
    bool protocolIs(const std::string_view& protocol) const;
    bool isHierarchical() const { return m_schemeEnd < m_userBegin && m_value[m_schemeEnd + 1] == '/'; }
    bool isEmpty() const { return m_value.empty(); }
    const std::string& value() const { return m_value; }

    std::string_view path() const { return componentString(m_portEnd, m_pathEnd); }
    std::string_view query() const { return componentString(m_pathEnd, m_queryEnd); }
    std::string_view fragment() const { return componentString(m_queryEnd, m_fragmentEnd); }
    std::string_view base() const;

private:
    std::string_view componentString(size_t begin, size_t end) const;
    std::string m_value;
    unsigned m_schemeEnd{0};
    unsigned m_userBegin{0};
    unsigned m_userEnd{0};
    unsigned m_passwordEnd{0};
    unsigned m_hostEnd{0};
    unsigned m_portEnd{0};
    unsigned m_pathEnd{0};
    unsigned m_queryEnd{0};
    unsigned m_fragmentEnd{0};
};

inline std::ostream& operator<<(std::ostream& o, const Url& in) { return o << in.value(); }

inline bool operator==(const Url& a, const Url& b) { return a.value() == b.value(); }
inline bool operator!=(const Url& a, const Url& b) { return a.value() != b.value(); }

inline bool operator==(const Url& a, const std::string_view& b) { return a.value() == b; }
inline bool operator!=(const Url& a, const std::string_view& b) { return a.value() != b; }

inline bool operator==(const std::string_view& a, const Url& b) { return a == b.value(); }
inline bool operator!=(const std::string_view& a, const Url& b) { return a != b.value(); }

inline bool operator<(const Url& a, const Url& b) { return a.value() < b.value(); }
inline bool operator>(const Url& a, const Url& b) { return a.value() > b.value(); }

inline bool operator<(const Url& a, const std::string_view& b) { return a.value() < b; }
inline bool operator>(const Url& a, const std::string_view& b) { return a.value() > b; }

inline bool operator<(const std::string_view& a, const Url& b) { return a < b.value(); }
inline bool operator>(const std::string_view& a, const Url& b) { return a > b.value(); }

} // namespace plutobook

#endif // PLUTOBOOK_URL_H
