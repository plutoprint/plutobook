/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_URL_H
#define PLUTOBOOK_URL_H

#include "ada.h"

namespace plutobook {

class Url {
public:
    Url() = default;
    explicit Url(std::string_view input);

    Url complete(std::string_view input) const;

    bool protocolIs(std::string_view protocol) const;
    bool isHierarchical() const { return !m_aggregator.has_opaque_path; }
    bool isValid() const { return m_aggregator.is_valid; }

    const std::string& value() const { return m_aggregator.get_buffer(); }

    std::string_view path() const { return m_aggregator.get_pathname(); }
    std::string_view query() const { return m_aggregator.get_search(); }
    std::string_view fragment() const { return m_aggregator.get_hash(); }

    std::string_view base() const;

private:
    explicit Url(ada::url_aggregator aggregator);
    ada::url_aggregator m_aggregator;
};

inline Url::Url(std::string_view input)
    : Url(ada::parser::parse_url(input))
{
}

inline Url::Url(ada::url_aggregator aggregator)
    : m_aggregator(std::move(aggregator))
{
}

inline Url Url::complete(std::string_view input) const
{
    return Url(ada::parser::parse_url(input, &m_aggregator));
}

inline bool Url::protocolIs(std::string_view protocol) const
{
    auto value = m_aggregator.get_protocol();
    if(!value.empty() && value.back() == ':')
        value.remove_suffix(1);
    return value == protocol;
}

inline std::string_view Url::base() const
{
    const auto& components = m_aggregator.get_components();

    auto end = m_aggregator.get_href_size();
    if(m_aggregator.has_opaque_path) {
        end = components.protocol_end;
    } else if(components.search_start != ada::url_components::omitted) {
        end = components.search_start;
    } else if(components.hash_start != ada::url_components::omitted) {
        end = components.hash_start;
    }

    return m_aggregator.get_href().substr(0, end);
}

inline std::ostream& operator<<(std::ostream& o, const Url& in) { return o << in.value(); }

inline bool operator==(const Url& a, const Url& b) { return a.value() == b.value(); }
inline bool operator==(const Url& a, std::string_view b) { return a.value() == b; }
inline bool operator==(std::string_view a, const Url& b) { return a == b.value(); }

inline bool operator<(const Url& a, const Url& b) { return a.value() < b.value(); }
inline bool operator<(const Url& a, std::string_view b) { return a.value() < b; }
inline bool operator<(std::string_view a, const Url& b) { return a < b.value(); }

} // namespace plutobook

#endif // PLUTOBOOK_URL_H
