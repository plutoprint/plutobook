/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "url.h"
#include "stringutils.h"

#include <cassert>

namespace plutobook {

enum URLCharacterClasses {
    SchemeFirstChar = 1 << 0,
    SchemeChar = 1 << 1,
    UserInfoChar = 1 << 2,
    HostnameChar = 1 << 3,
    IPv6Char = 1 << 4,
    PathSegmentEndChar = 1 << 5,
    BadChar = 1 << 6
};

constexpr uint8_t characterClassTable[256] = {
    PathSegmentEndChar, /* 0 nul */
    BadChar, /* 1 soh */
    BadChar, /* 2 stx */
    BadChar, /* 3 etx */
    BadChar, /* 4 eot */
    BadChar, /* 5 enq */
    BadChar, /* 6 ack */
    BadChar, /* 7 bel */
    BadChar, /* 8 bs */
    BadChar, /* 9 ht */
    BadChar, /* 10 nl */
    BadChar, /* 11 vt */
    BadChar, /* 12 np */
    BadChar, /* 13 cr */
    BadChar, /* 14 so */
    BadChar, /* 15 si */
    BadChar, /* 16 dle */
    BadChar, /* 17 dc1 */
    BadChar, /* 18 dc2 */
    BadChar, /* 19 dc3 */
    BadChar, /* 20 dc4 */
    BadChar, /* 21 nak */
    BadChar, /* 22 syn */
    BadChar, /* 23 etb */
    BadChar, /* 24 can */
    BadChar, /* 25 em */
    BadChar, /* 26 sub */
    BadChar, /* 27 esc */
    BadChar, /* 28 fs */
    BadChar, /* 29 gs */
    BadChar, /* 30 rs */
    BadChar, /* 31 us */
    BadChar, /* 32 sp */
    UserInfoChar, /* 33  ! */
    BadChar, /* 34  " */
    PathSegmentEndChar | BadChar, /* 35  # */
    UserInfoChar, /* 36  $ */
    UserInfoChar | HostnameChar | IPv6Char | BadChar, /* 37  % */
    UserInfoChar, /* 38  & */
    UserInfoChar, /* 39  ' */
    UserInfoChar, /* 40  ( */
    UserInfoChar, /* 41  ) */
    UserInfoChar, /* 42  * */
    SchemeChar | UserInfoChar, /* 43  + */
    UserInfoChar, /* 44  , */
    SchemeChar | UserInfoChar | HostnameChar, /* 45  - */
    SchemeChar | UserInfoChar | HostnameChar | IPv6Char, /* 46  . */
    PathSegmentEndChar, /* 47  / */
    SchemeChar | UserInfoChar | HostnameChar | IPv6Char, /* 48  0 */
    SchemeChar | UserInfoChar | HostnameChar | IPv6Char, /* 49  1 */
    SchemeChar | UserInfoChar | HostnameChar | IPv6Char, /* 50  2 */
    SchemeChar | UserInfoChar | HostnameChar | IPv6Char, /* 51  3 */
    SchemeChar | UserInfoChar | HostnameChar | IPv6Char, /* 52  4 */
    SchemeChar | UserInfoChar | HostnameChar | IPv6Char, /* 53  5 */
    SchemeChar | UserInfoChar | HostnameChar | IPv6Char, /* 54  6 */
    SchemeChar | UserInfoChar | HostnameChar | IPv6Char, /* 55  7 */
    SchemeChar | UserInfoChar | HostnameChar | IPv6Char, /* 56  8 */
    SchemeChar | UserInfoChar | HostnameChar | IPv6Char, /* 57  9 */
    UserInfoChar | IPv6Char, /* 58  : */
    UserInfoChar, /* 59  ; */
    BadChar, /* 60  < */
    UserInfoChar, /* 61  = */
    BadChar, /* 62  > */
    PathSegmentEndChar | BadChar, /* 63  ? */
    0, /* 64  @ */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar | IPv6Char, /* 65  A */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar | IPv6Char, /* 66  B */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar | IPv6Char, /* 67  C */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar | IPv6Char, /* 68  D */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar | IPv6Char, /* 69  E */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar | IPv6Char, /* 70  F */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar, /* 71  G */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar, /* 72  H */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar, /* 73  I */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar, /* 74  J */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar, /* 75  K */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar, /* 76  L */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar, /* 77  M */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar, /* 78  N */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar, /* 79  O */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar, /* 80  P */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar, /* 81  Q */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar, /* 82  R */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar, /* 83  S */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar, /* 84  T */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar, /* 85  U */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar, /* 86  V */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar, /* 87  W */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar, /* 88  X */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar, /* 89  Y */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar, /* 90  Z */
    0, /* 91  [ */
    0, /* 92  \ */
    0, /* 93  ] */
    0, /* 94  ^ */
    UserInfoChar | HostnameChar, /* 95  _ */
    0, /* 96  ` */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar | IPv6Char, /* 97  a */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar | IPv6Char, /* 98  b */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar | IPv6Char, /* 99  c */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar | IPv6Char, /* 100  d */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar | IPv6Char, /* 101  e */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar | IPv6Char, /* 102  f */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar, /* 103  g */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar, /* 104  h */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar, /* 105  i */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar, /* 106  j */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar, /* 107  k */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar, /* 108  l */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar, /* 109  m */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar, /* 110  n */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar, /* 111  o */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar, /* 112  p */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar, /* 113  q */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar, /* 114  r */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar, /* 115  s */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar, /* 116  t */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar, /* 117  u */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar, /* 118  v */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar, /* 119  w */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar, /* 120  x */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar, /* 121  y */
    SchemeFirstChar | SchemeChar | UserInfoChar | HostnameChar, /* 122  z */
    0, /* 123  { */
    0, /* 124  | */
    0, /* 125  } */
    UserInfoChar, /* 126  ~ */
    BadChar, /* 127 del */
    BadChar, /* 128 */
    BadChar, /* 129 */
    BadChar, /* 130 */
    BadChar, /* 131 */
    BadChar, /* 132 */
    BadChar, /* 133 */
    BadChar, /* 134 */
    BadChar, /* 135 */
    BadChar, /* 136 */
    BadChar, /* 137 */
    BadChar, /* 138 */
    BadChar, /* 139 */
    BadChar, /* 140 */
    BadChar, /* 141 */
    BadChar, /* 142 */
    BadChar, /* 143 */
    BadChar, /* 144 */
    BadChar, /* 145 */
    BadChar, /* 146 */
    BadChar, /* 147 */
    BadChar, /* 148 */
    BadChar, /* 149 */
    BadChar, /* 150 */
    BadChar, /* 151 */
    BadChar, /* 152 */
    BadChar, /* 153 */
    BadChar, /* 154 */
    BadChar, /* 155 */
    BadChar, /* 156 */
    BadChar, /* 157 */
    BadChar, /* 158 */
    BadChar, /* 159 */
    BadChar, /* 160 */
    BadChar, /* 161 */
    BadChar, /* 162 */
    BadChar, /* 163 */
    BadChar, /* 164 */
    BadChar, /* 165 */
    BadChar, /* 166 */
    BadChar, /* 167 */
    BadChar, /* 168 */
    BadChar, /* 169 */
    BadChar, /* 170 */
    BadChar, /* 171 */
    BadChar, /* 172 */
    BadChar, /* 173 */
    BadChar, /* 174 */
    BadChar, /* 175 */
    BadChar, /* 176 */
    BadChar, /* 177 */
    BadChar, /* 178 */
    BadChar, /* 179 */
    BadChar, /* 180 */
    BadChar, /* 181 */
    BadChar, /* 182 */
    BadChar, /* 183 */
    BadChar, /* 184 */
    BadChar, /* 185 */
    BadChar, /* 186 */
    BadChar, /* 187 */
    BadChar, /* 188 */
    BadChar, /* 189 */
    BadChar, /* 190 */
    BadChar, /* 191 */
    BadChar, /* 192 */
    BadChar, /* 193 */
    BadChar, /* 194 */
    BadChar, /* 195 */
    BadChar, /* 196 */
    BadChar, /* 197 */
    BadChar, /* 198 */
    BadChar, /* 199 */
    BadChar, /* 200 */
    BadChar, /* 201 */
    BadChar, /* 202 */
    BadChar, /* 203 */
    BadChar, /* 204 */
    BadChar, /* 205 */
    BadChar, /* 206 */
    BadChar, /* 207 */
    BadChar, /* 208 */
    BadChar, /* 209 */
    BadChar, /* 210 */
    BadChar, /* 211 */
    BadChar, /* 212 */
    BadChar, /* 213 */
    BadChar, /* 214 */
    BadChar, /* 215 */
    BadChar, /* 216 */
    BadChar, /* 217 */
    BadChar, /* 218 */
    BadChar, /* 219 */
    BadChar, /* 220 */
    BadChar, /* 221 */
    BadChar, /* 222 */
    BadChar, /* 223 */
    BadChar, /* 224 */
    BadChar, /* 225 */
    BadChar, /* 226 */
    BadChar, /* 227 */
    BadChar, /* 228 */
    BadChar, /* 229 */
    BadChar, /* 230 */
    BadChar, /* 231 */
    BadChar, /* 232 */
    BadChar, /* 233 */
    BadChar, /* 234 */
    BadChar, /* 235 */
    BadChar, /* 236 */
    BadChar, /* 237 */
    BadChar, /* 238 */
    BadChar, /* 239 */
    BadChar, /* 240 */
    BadChar, /* 241 */
    BadChar, /* 242 */
    BadChar, /* 243 */
    BadChar, /* 244 */
    BadChar, /* 245 */
    BadChar, /* 246 */
    BadChar, /* 247 */
    BadChar, /* 248 */
    BadChar, /* 249 */
    BadChar, /* 250 */
    BadChar, /* 251 */
    BadChar, /* 252 */
    BadChar, /* 253 */
    BadChar, /* 254 */
    BadChar /* 255 */
};

constexpr bool isSchemeFirstChar(uint8_t cc) { return characterClassTable[cc] & SchemeFirstChar; }
constexpr bool isSchemeChar(uint8_t cc) { return characterClassTable[cc] & SchemeChar; }
constexpr bool isUserInfoChar(uint8_t cc) { return characterClassTable[cc] & UserInfoChar; }
constexpr bool isHostnameChar(uint8_t cc) { return characterClassTable[cc] & HostnameChar; }
constexpr bool isIPv6Char(uint8_t cc) { return characterClassTable[cc] & IPv6Char; }
constexpr bool isPathSegmentEndChar(uint8_t cc) { return characterClassTable[cc] & PathSegmentEndChar; }
constexpr bool isBadChar(uint8_t cc) { return characterClassTable[cc] & BadChar; }

Url::Url(const std::string_view& input)
{
    if(input.empty() || !isSchemeFirstChar(input.front()))
        return;
    auto inputData = input.data();
    auto inputLength = input.length();
    auto peek = [&](int index) {
        if(index < inputLength)
            return inputData[index];
        return char(0);
    };

    int schemeEnd = 1;
    while(isSchemeChar(peek(schemeEnd)))
        ++schemeEnd;
    if(peek(schemeEnd) != ':') {
        return;
    }

    bool isHttp = false;
    bool isHttps = false;
    bool isFile = false;
    if(equals(inputData, schemeEnd, "http", 4, false))
        isHttp = true;
    else if(equals(inputData, schemeEnd, "https", 5, false))
        isHttps = true;
    else if(equals(inputData, schemeEnd, "file", 4, false)) {
        isFile = true;
    }

    int userBegin = 0;
    int userEnd = 0;
    int passwordBegin = 0;
    int passwordEnd = 0;
    int hostBegin = 0;
    int hostEnd = 0;
    int portBegin = 0;
    int portEnd = 0;

    bool hierarchical = peek(schemeEnd + 1) == '/';
    if(hierarchical && peek(schemeEnd + 2) == '/') {
        userBegin = schemeEnd + 3;
        userEnd = userBegin;

        int colon = 0;
        while(isUserInfoChar(peek(userEnd))) {
            if(colon == 0 && peek(userEnd) == ':')
                colon = userEnd;
            ++userEnd;
        }

        if(peek(userEnd) == '@') {
            if(colon == 0) {
                passwordBegin = userEnd;
                passwordEnd = passwordBegin;
            } else {
                passwordBegin = colon + 1;
                passwordEnd = userEnd;
                userEnd = colon;
            }

            hostBegin = passwordEnd + 1;
        } else if(peek(userEnd) == '[' || isPathSegmentEndChar(peek(userEnd))) {
            hostBegin = userBegin;
            userEnd = hostBegin;
            passwordBegin = userEnd;
            passwordEnd = passwordBegin;
        } else {
            return;
        }

        hostEnd = hostBegin;
        if(peek(hostEnd) == '[') {
            ++hostEnd;
            while(isIPv6Char(peek(hostEnd)))
                ++hostEnd;
            if(peek(hostEnd) == ']')
                ++hostEnd;
            else {
                return;
            }
        } else {
            while(isHostnameChar(peek(hostEnd))) {
                ++hostEnd;
            }
        }

        if(peek(hostEnd) == ':') {
            portBegin = hostEnd + 1;
            portEnd = portBegin;
            while(isDigit(peek(portEnd))) {
                ++portEnd;
            }
        } else {
            portBegin = hostEnd;
            portEnd = portBegin;
        }

        if(!isPathSegmentEndChar(peek(portEnd)))
            return;
        if(userBegin == portEnd && !(isHttp || isHttps || isFile)) {
            userBegin = schemeEnd + 1;
            userEnd = userBegin;
            passwordBegin = userEnd;
            passwordEnd = passwordBegin;
            hostBegin = passwordEnd;
            hostEnd = hostBegin;
            portBegin = hostEnd;
            portEnd = portBegin;
        }
    } else {
        userBegin = schemeEnd + 1;
        userEnd = userBegin;
        passwordBegin = userEnd;
        passwordEnd = passwordBegin;
        hostBegin = passwordEnd;
        hostEnd = hostBegin;
        portBegin = hostEnd;
        portEnd = portBegin;
    }

    int pathBegin = portEnd;
    int pathEnd = pathBegin;
    while(pathEnd < inputLength && inputData[pathEnd] != '?' && inputData[pathEnd] != '#')
        ++pathEnd;
    int queryBegin = pathEnd;
    int queryEnd = queryBegin;
    if(peek(queryBegin) == '?') {
        do {
            ++queryEnd;
        } while(queryEnd < inputLength && inputData[queryEnd] != '#');
    }

    int fragmentBegin = queryEnd;
    int fragmentEnd = fragmentBegin;
    if(peek(fragmentBegin) == '#') {
        ++fragmentBegin;
        fragmentEnd = fragmentBegin;
        while(fragmentEnd < inputLength) {
            ++fragmentEnd;
        }
    }

    m_value.reserve(fragmentEnd);
    for(int i = 0; i < schemeEnd; ++i)
        m_value += toLower(inputData[i]);
    m_schemeEnd = m_value.length();
    m_value += ':';
    if(isFile ? (pathBegin != pathEnd || hostBegin != hostEnd)
        : (userBegin != userEnd || passwordBegin != passwordEnd || hostEnd != portEnd || hostBegin != hostEnd)) {
        m_value += '/';
        m_value += '/';

        m_userBegin = m_value.length();
        m_value += input.substr(userBegin, userEnd - userBegin);
        m_userEnd = m_value.length();
        if(passwordBegin != passwordEnd) {
            m_value += ':';
            m_value += input.substr(passwordBegin, passwordEnd - passwordBegin);
        }

        m_passwordEnd = m_value.length();
        if(m_userBegin != m_passwordEnd)
            m_value += '@';
        for(int i = hostBegin; i < hostEnd; ++i)
            m_value += toLower(inputData[i]);
        m_hostEnd = m_value.length();
        if(hostEnd != portBegin) {
            m_value += ':';
            m_value += input.substr(portBegin, portEnd - portBegin);
        }

        m_portEnd = m_value.length();
    } else {
        m_userBegin = m_value.length();
        m_userEnd = m_userBegin;
        m_passwordEnd = m_userEnd;
        m_hostEnd = m_passwordEnd;
        m_portEnd = m_hostEnd;
    }

    if(pathBegin == pathEnd && hierarchical && (isHttp || isHttps || isFile))
        m_value += '/';
    auto append = [&](int begin, int end) {
        constexpr char hexdigits[] = "0123456789ABCDEF";
        for(int i = begin; i < end; ++i) {
            const uint8_t cc = inputData[i];
            if(cc == '%' || cc == '?' || !isBadChar(cc)) {
                m_value.push_back(cc);
            } else {
                m_value += '%';
                m_value += hexdigits[cc >> 4];
                m_value += hexdigits[cc & 0xF];
            }
        }
    };

    if(!hierarchical) {
        append(pathBegin, pathEnd);
    } else {
        auto begin = m_value.length();
        append(pathBegin, pathEnd);
        auto end = m_value.length();

        auto in = begin;
        auto peek = [&](int index) {
            index += in;
            if(index < end)
                return m_value[index];
            return char(0);
        };

        auto out = begin;
        while(in < end) {
            if(peek(0) == '.' && peek(1) == '/')
                in += 2;
            else if(peek(0) == '.' && peek(1) == '.' && peek(2) == '/')
                in += 3;
            if(peek(0) == '/' && peek(1) == '.' && (peek(2) == '/' || peek(2) == 0)) {
                in += 2;
                if(in < end)
                    continue;
                m_value[out++] = '/';
                break;
            }

            if(peek(0) == '/' && peek(1) == '.' && peek(2) == '.' && (peek(3) == '/' || peek(3) == 0)) {
                while(out > begin && m_value[--out] != '/');
                in += 3;
                if(in < end) {
                    if(out == begin && m_value[out] != '/')
                        in += 1;
                    continue;
                }

                if(m_value[out] == '/')
                    out += 1;
                break;
            }

            do {
                m_value[out++] = m_value[in++];
            } while(in < end && m_value[in] != '/');
        }

        m_value.erase(out, end - out);
    }

    if(!hierarchical) {
        m_baseEnd = m_portEnd;
    } else {
        m_baseEnd = m_value.length();
        while(m_baseEnd > m_portEnd && m_value[m_baseEnd - 1] != '/') {
            --m_baseEnd;
        }
    }

    m_pathEnd = m_value.length();
    append(queryBegin, queryEnd);
    m_queryEnd = m_value.length();
    if(fragmentBegin != queryEnd) {
        m_value += '#';
        append(fragmentBegin, fragmentEnd);
    }

    m_fragmentEnd = m_value.length();
}

constexpr bool isAbsoluteFilename(const std::string_view& input)
{
    if(!input.empty()) {
        if(input.front() == '/' || input.front() == '\\')
            return true;
        return input.size() >= 3
            && isAlpha(input[0]) && input[1] == ':'
            && (input[2] == '\\' || input[2] == '/');
    }

    return false;
}

Url Url::complete(std::string_view input) const
{
    stripLeadingAndTrailingSpaces(input);
    if(protocolIs("file") && isAbsoluteFilename(input)) {
        std::string value("file:///");
        while(!input.empty() && (input.front() == '/' || input.front() == '\\'))
            input.remove_prefix(1);
        for(auto cc : input)
            value.push_back(cc == '\\' ? '/' : cc);
        return Url(value);
    }

    if(m_value.empty())
        return Url(input);
    assert(m_value[m_schemeEnd] == ':');
    if(!input.empty() && isSchemeFirstChar(input.front())) {
        auto it = input.begin();
        auto end = input.end();
        do {
            ++it;
        } while(it != end && isSchemeChar(*it));
        if(it != end && *it == ':') {
            auto length = it - input.begin();
            ++it;
            if(it == end || *it == '/' || !isHierarchical() || !protocolIs(input.substr(0, length)))
                return Url(input);
            input.remove_prefix(length + 1);
        }
    }

    std::string relative(input);
    if(!isHierarchical()) {
        if(!relative.empty() && relative.front() == '#')
            return Url(m_value.substr(0, m_queryEnd) + relative);
        return Url();
    }

    if(relative.empty())
        return Url(m_value.substr(0, m_queryEnd));
    if(relative.front() == '#')
        return Url(m_value.substr(0, m_queryEnd) + relative);
    if(relative.front() == '?')
        return Url(m_value.substr(0, m_pathEnd) + relative);
    if(relative.front() == '/') {
        if(relative.length() > 1 && relative.at(1) == '/')
            return Url(m_value.substr(0, m_schemeEnd + 1) + relative);
        return Url(m_value.substr(0, m_portEnd) + relative);
    }

    auto value = m_value.substr(0, m_baseEnd);
    if(m_portEnd == value.length())
        value += '/';
    return Url(value + relative);
}

bool Url::protocolIs(const std::string_view& protocol) const
{
    return equals(m_value.data(), m_schemeEnd, protocol.data(), protocol.length(), false);
}

} // namespace plutobook
