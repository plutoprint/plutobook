/* auto-generated on 2026-07-27 16:10:09 -0400. Do not edit! */
/* begin file include/ada.h */
/**
 * @file ada.h
 * @brief Main header for the Ada URL parser library.
 *
 * This is the primary entry point for the Ada URL parser library. Including
 * this single header provides access to the complete Ada API, including:
 *
 * - URL parsing via `ada::parse()` function
 * - Two URL representations: `ada::url` and `ada::url_aggregator`
 * - URL search parameters via `ada::url_search_params`
 * - URL pattern matching via `ada::url_pattern` (URLPattern API)
 * - IDNA (Internationalized Domain Names) support
 *
 * @example
 * ```cpp
 *
 * // Parse a URL
 * auto url = ada::parse("https://example.com/path?query=1");
 * if (url) {
 *     std::cout << url->get_hostname(); // "example.com"
 * }
 * ```
 *
 * @see https://url.spec.whatwg.org/ - WHATWG URL Standard
 * @see https://github.com/ada-url/ada - Ada URL Parser GitHub Repository
 */
#ifndef ADA_H
#define ADA_H

/* begin file include/ada/ada_idna.h */
/* auto-generated on 2026-07-12 20:34:08 -0400. Do not edit! */
/* begin file include/idna.h */
#ifndef ADA_IDNA_H
#define ADA_IDNA_H

/* begin file include/ada/idna/unicode_transcoding.h */
#ifndef ADA_IDNA_UNICODE_TRANSCODING_H
#define ADA_IDNA_UNICODE_TRANSCODING_H

#include <string>
#include <string_view>

namespace ada::idna {

size_t utf8_to_utf32(const char* buf, size_t len, char32_t* utf32_output);

size_t utf8_length_from_utf32(const char32_t* buf, size_t len);

size_t utf32_length_from_utf8(const char* buf, size_t len);

size_t utf32_to_utf8(const char32_t* buf, size_t len, char* utf8_output);

}  // namespace ada::idna

#endif  // ADA_IDNA_UNICODE_TRANSCODING_H
/* end file include/ada/idna/unicode_transcoding.h */
/* begin file include/ada/idna/mapping.h */
#ifndef ADA_IDNA_MAPPING_H
#define ADA_IDNA_MAPPING_H

#include <string>
#include <string_view>

namespace ada::idna {

// If the input is ascii, then the mapping is just -> lower case.
void ascii_map(char* input, size_t length);
// Map the characters according to IDNA, returning the empty string on error.
std::u32string map(std::u32string_view input);
// Map into an existing buffer (cleared on entry). Returns false if any code
// point is disallowed. Reusing the buffer avoids repeated heap allocations
// when called in a loop over multiple labels.
bool map(std::u32string_view input, std::u32string& out);

}  // namespace ada::idna

#endif
/* end file include/ada/idna/mapping.h */
/* begin file include/ada/idna/normalization.h */
#ifndef ADA_IDNA_NORMALIZATION_H
#define ADA_IDNA_NORMALIZATION_H

#include <string>
#include <string_view>

namespace ada::idna {

// Returns true if `input` is already in Unicode Normalization Form C.
// Requires that internal tables have been loaded (call ensure via normalize
// or map first, or this returns false if tables are unavailable).
[[nodiscard]] bool is_already_nfc(std::u32string_view input) noexcept;

// Normalize the characters according to IDNA (Unicode Normalization Form C).
// Returns false if the internal Unicode tables could not be loaded; in that
// case `input` is left unchanged. Skips work when the string is already NFC.
[[nodiscard]] bool normalize(std::u32string& input);

}  // namespace ada::idna
#endif
/* end file include/ada/idna/normalization.h */
/* begin file include/ada/idna/punycode.h */
#ifndef ADA_IDNA_PUNYCODE_H
#define ADA_IDNA_PUNYCODE_H

#include <string>
#include <string_view>

namespace ada::idna {

bool punycode_to_utf32(std::string_view input, std::u32string& out);
bool verify_punycode(std::string_view input);
bool utf32_to_punycode(std::u32string_view input, std::string& out);

}  // namespace ada::idna

#endif  // ADA_IDNA_PUNYCODE_H
/* end file include/ada/idna/punycode.h */
/* begin file include/ada/idna/validity.h */
#ifndef ADA_IDNA_VALIDITY_H
#define ADA_IDNA_VALIDITY_H

#include <string>
#include <string_view>

namespace ada::idna {

/**
 * @see https://www.unicode.org/reports/tr46/#Validity_Criteria
 */
bool is_label_valid(std::u32string_view label);

}  // namespace ada::idna

#endif  // ADA_IDNA_VALIDITY_H
/* end file include/ada/idna/validity.h */
/* begin file include/ada/idna/to_ascii.h */
#ifndef ADA_IDNA_TO_ASCII_H
#define ADA_IDNA_TO_ASCII_H

#include <string>
#include <string_view>

/* begin file include/ada/idna/limits.h */
#ifndef ADA_IDNA_LIMITS_H
#define ADA_IDNA_LIMITS_H

#include <cstddef>

namespace ada::idna {

// Maximum accepted UTF-8 domain length for to_ascii / to_unicode.
// Bounds heap growth under untrusted input (DoS resistance). DNS wire limits
// are smaller; this allows long Unicode labels used in URL tests/fixtures.
inline constexpr size_t max_domain_input_bytes = 16384;

}  // namespace ada::idna

#endif  // ADA_IDNA_LIMITS_H
/* end file include/ada/idna/limits.h */

namespace ada::idna {

// Converts a domain (e.g., www.google.com) possibly containing international
// characters to an ascii domain (with punycode). It will not do percent
// decoding: percent decoding should be done prior to calling this function. We
// do not remove tabs and spaces, they should have been removed prior to calling
// this function. We also do not trim control characters. We also assume that
// the input is not empty. We return "" on error. Inputs longer than
// max_domain_input_bytes are rejected.
//
// This function may accept or even produce invalid domains (WHATWG carve-outs).
std::string to_ascii(std::string_view ut8_string);

// Same as to_ascii, but writes into `out` and returns false on error without
// relying on empty-string ambiguity.
[[nodiscard]] bool to_ascii(std::string_view ut8_string, std::string& out);

// Returns true if the string contains a forbidden code point according to the
// WHATGL URL specification:
// https://url.spec.whatwg.org/#forbidden-domain-code-point
bool contains_forbidden_domain_code_point(std::string_view ascii_string);

bool constexpr is_ascii(std::u32string_view view);
bool constexpr is_ascii(std::string_view view);

}  // namespace ada::idna

#endif  // ADA_IDNA_TO_ASCII_H
/* end file include/ada/idna/to_ascii.h */
/* begin file include/ada/idna/to_unicode.h */
#ifndef ADA_IDNA_TO_UNICODE_H
#define ADA_IDNA_TO_UNICODE_H

#include <string>
#include <string_view>

namespace ada::idna {

// UTS #46 ToUnicode. Never fails per the standard: on step failure the original
// label is kept. Inputs longer than max_domain_input_bytes are returned
// unchanged as a safety measure under untrusted input.
std::string to_unicode(std::string_view input);

// Writes into `out`. Returns false only if the input exceeds
// max_domain_input_bytes (out is left empty). Otherwise always returns true
// (ToUnicode does not fail).
[[nodiscard]] bool to_unicode(std::string_view input, std::string& out);

}  // namespace ada::idna

#endif  // ADA_IDNA_TO_UNICODE_H
/* end file include/ada/idna/to_unicode.h */
/* begin file include/ada/idna/identifier.h */
#ifndef ADA_IDNA_IDENTIFIER_H
#define ADA_IDNA_IDENTIFIER_H

#include <string>
#include <string_view>

namespace ada::idna {

// Verify if it is valid name code point given a Unicode code point and a
// boolean first: If first is true return the result of checking if code point
// is contained in the IdentifierStart set of code points. Otherwise return the
// result of checking if code point is contained in the IdentifierPart set of
// code points. Returns false if the input is empty or the code point is not
// valid. There is minimal Unicode error handling: the input should be valid
// UTF-8. https://urlpattern.spec.whatwg.org/#is-a-valid-name-code-point
bool valid_name_code_point(char32_t code_point, bool first);

}  // namespace ada::idna

#endif
/* end file include/ada/idna/identifier.h */

#endif
/* end file include/idna.h */
/* end file include/ada/ada_idna.h */
/* begin file include/ada/character_sets.h */
/**
 * @file character_sets.h
 * @brief Declaration of the character sets used by unicode functions.
 * @author Node.js
 * @see https://github.com/nodejs/node/blob/main/src/node_url_tables.cc
 */
#ifndef ADA_CHARACTER_SETS_H
#define ADA_CHARACTER_SETS_H

/* begin file include/ada/common_defs.h */
/**
 * @file common_defs.h
 * @brief Cross-platform compiler macros and common definitions.
 *
 * This header provides compiler-specific macros for optimization hints,
 * platform detection, SIMD support detection, and development/debug utilities.
 * It ensures consistent behavior across different compilers (GCC, Clang, MSVC).
 */
#ifndef ADA_COMMON_DEFS_H
#define ADA_COMMON_DEFS_H

// https://en.cppreference.com/w/cpp/feature_test#Library_features
// detect C++20 features
#include <version>

#ifdef _MSC_VER
#define ADA_VISUAL_STUDIO 1
/**
 * We want to differentiate carefully between
 * clang under visual studio and regular visual
 * studio.
 */
#ifdef __clang__
// clang under visual studio
#define ADA_CLANG_VISUAL_STUDIO 1
#else
// just regular visual studio (best guess)
#define ADA_REGULAR_VISUAL_STUDIO 1
#endif  // __clang__
#endif  // _MSC_VER

#if defined(__GNUC__)
// Marks a block with a name so that MCA analysis can see it.
#define ADA_BEGIN_DEBUG_BLOCK(name) __asm volatile("# LLVM-MCA-BEGIN " #name);
#define ADA_END_DEBUG_BLOCK(name) __asm volatile("# LLVM-MCA-END " #name);
#define ADA_DEBUG_BLOCK(name, block) \
  BEGIN_DEBUG_BLOCK(name);           \
  block;                             \
  END_DEBUG_BLOCK(name);
#else
#define ADA_BEGIN_DEBUG_BLOCK(name)
#define ADA_END_DEBUG_BLOCK(name)
#define ADA_DEBUG_BLOCK(name, block)
#endif

// Align to N-byte boundary
#define ADA_ROUNDUP_N(a, n) (((a) + ((n) - 1)) & ~((n) - 1))
#define ADA_ROUNDDOWN_N(a, n) ((a) & ~((n) - 1))

#define ADA_ISALIGNED_N(ptr, n) (((uintptr_t)(ptr) & ((n) - 1)) == 0)

#if defined(ADA_REGULAR_VISUAL_STUDIO)

#define ada_really_inline __forceinline
#define ada_never_inline __declspec(noinline)

#define ada_unused
#define ada_warn_unused

#define ADA_PUSH_DISABLE_WARNINGS __pragma(warning(push))
#define ADA_PUSH_DISABLE_ALL_WARNINGS __pragma(warning(push, 0))
#define ADA_DISABLE_VS_WARNING(WARNING_NUMBER) \
  __pragma(warning(disable : WARNING_NUMBER))
// Get rid of Intellisense-only warnings (Code Analysis)
// Though __has_include is C++17, it is supported in Visual Studio 2017 or
// better (_MSC_VER>=1910).
#ifdef __has_include
#if __has_include(<CppCoreCheck\Warnings.h>)
#include <CppCoreCheck\Warnings.h>
#define ADA_DISABLE_UNDESIRED_WARNINGS \
  ADA_DISABLE_VS_WARNING(ALL_CPPCORECHECK_WARNINGS)
#endif
#endif

#ifndef ADA_DISABLE_UNDESIRED_WARNINGS
#define ADA_DISABLE_UNDESIRED_WARNINGS
#endif

#define ADA_DISABLE_DEPRECATED_WARNING ADA_DISABLE_VS_WARNING(4996)
#define ADA_DISABLE_STRICT_OVERFLOW_WARNING
#define ADA_POP_DISABLE_WARNINGS __pragma(warning(pop))

#else  // ADA_REGULAR_VISUAL_STUDIO

#define ada_really_inline inline __attribute__((always_inline))
#define ada_never_inline inline __attribute__((noinline))

#define ada_unused __attribute__((unused))
#define ada_warn_unused __attribute__((warn_unused_result))

#define ADA_PUSH_DISABLE_WARNINGS _Pragma("GCC diagnostic push")
// gcc doesn't seem to disable all warnings with all and extra, add warnings
// here as necessary
#define ADA_PUSH_DISABLE_ALL_WARNINGS               \
  ADA_PUSH_DISABLE_WARNINGS                         \
  ADA_DISABLE_GCC_WARNING("-Weffc++")               \
  ADA_DISABLE_GCC_WARNING("-Wall")                  \
  ADA_DISABLE_GCC_WARNING("-Wconversion")           \
  ADA_DISABLE_GCC_WARNING("-Wextra")                \
  ADA_DISABLE_GCC_WARNING("-Wattributes")           \
  ADA_DISABLE_GCC_WARNING("-Wimplicit-fallthrough") \
  ADA_DISABLE_GCC_WARNING("-Wnon-virtual-dtor")     \
  ADA_DISABLE_GCC_WARNING("-Wreturn-type")          \
  ADA_DISABLE_GCC_WARNING("-Wshadow")               \
  ADA_DISABLE_GCC_WARNING("-Wunused-parameter")     \
  ADA_DISABLE_GCC_WARNING("-Wunused-variable")      \
  ADA_DISABLE_GCC_WARNING("-Wsign-compare")
#define ADA_PRAGMA(P) _Pragma(#P)
#define ADA_DISABLE_GCC_WARNING(WARNING) \
  ADA_PRAGMA(GCC diagnostic ignored WARNING)
#if defined(ADA_CLANG_VISUAL_STUDIO)
#define ADA_DISABLE_UNDESIRED_WARNINGS \
  ADA_DISABLE_GCC_WARNING("-Wmicrosoft-include")
#else
#define ADA_DISABLE_UNDESIRED_WARNINGS
#endif
#define ADA_DISABLE_DEPRECATED_WARNING \
  ADA_DISABLE_GCC_WARNING("-Wdeprecated-declarations")
#define ADA_DISABLE_STRICT_OVERFLOW_WARNING \
  ADA_DISABLE_GCC_WARNING("-Wstrict-overflow")
#define ADA_POP_DISABLE_WARNINGS _Pragma("GCC diagnostic pop")

#endif  // MSC_VER

#if defined(ADA_VISUAL_STUDIO)
/**
 * It does not matter here whether you are using
 * the regular visual studio or clang under visual
 * studio.
 */
#if ADA_USING_LIBRARY
#define ADA_DLLIMPORTEXPORT __declspec(dllimport)
#else
#define ADA_DLLIMPORTEXPORT __declspec(dllexport)
#endif
#else
#define ADA_DLLIMPORTEXPORT
#endif

/// If EXPR is an error, returns it.
#define ADA_TRY(EXPR)   \
  {                     \
    auto _err = (EXPR); \
    if (_err) {         \
      return _err;      \
    }                   \
  }

// __has_cpp_attribute is part of C++20
#if !defined(__has_cpp_attribute)
#define __has_cpp_attribute(x) 0
#endif

#if __has_cpp_attribute(gnu::noinline)
#define ADA_ATTRIBUTE_NOINLINE [[gnu::noinline]]
#else
#define ADA_ATTRIBUTE_NOINLINE
#endif

namespace ada {
[[noreturn]] inline void unreachable() {
#ifdef __GNUC__
  __builtin_unreachable();
#elif defined(_MSC_VER)
  __assume(false);
#else
#endif
}
}  // namespace ada

#define ADA_DEVELOPMENT_CHECKS 0

// Unless the programmer has already set ADA_DEVELOPMENT_CHECKS,
// we want to set it under debug builds. We detect a debug build
// under Visual Studio when the _DEBUG macro is set. Under the other
// compilers, we use the fact that they define __OPTIMIZE__ whenever
// they allow optimizations.
// It is possible that this could miss some cases where ADA_DEVELOPMENT_CHECKS
// is helpful, but the programmer can set the macro ADA_DEVELOPMENT_CHECKS.
// It could also wrongly set ADA_DEVELOPMENT_CHECKS (e.g., if the programmer
// sets _DEBUG in a release build under Visual Studio, or if some compiler fails
// to set the __OPTIMIZE__ macro).
#if !defined(ADA_DEVELOPMENT_CHECKS) && !defined(NDEBUG)
#ifdef _MSC_VER
// Visual Studio seems to set _DEBUG for debug builds.
#ifdef _DEBUG
#define ADA_DEVELOPMENT_CHECKS 1
#endif  // _DEBUG
#else   // _MSC_VER
// All other compilers appear to set __OPTIMIZE__ to a positive integer
// when the compiler is optimizing.
#ifndef __OPTIMIZE__
#define ADA_DEVELOPMENT_CHECKS 1
#endif  // __OPTIMIZE__
#endif  // _MSC_VER
#endif  // ADA_DEVELOPMENT_CHECKS

#define ADA_STR(x) #x

#if ADA_DEVELOPMENT_CHECKS
#define ADA_REQUIRE(EXPR) \
  {                       \
    if (!(EXPR) { abort(); }) }

#define ADA_FAIL(MESSAGE)                            \
  do {                                               \
    std::cerr << "FAIL: " << (MESSAGE) << std::endl; \
    abort();                                         \
  } while (0);
#define ADA_ASSERT_EQUAL(LHS, RHS, MESSAGE)                                    \
  do {                                                                         \
    if (LHS != RHS) {                                                          \
      std::cerr << "Mismatch: '" << LHS << "' - '" << RHS << "'" << std::endl; \
      ADA_FAIL(MESSAGE);                                                       \
    }                                                                          \
  } while (0);
#define ADA_ASSERT_TRUE(COND)                                               \
  do {                                                                      \
    if (!(COND)) {                                                          \
      std::cerr << "Assert at line " << __LINE__ << " of file " << __FILE__ \
                << std::endl;                                               \
      ADA_FAIL(ADA_STR(COND));                                              \
    }                                                                       \
  } while (0);
#else
#define ADA_FAIL(MESSAGE)
#define ADA_ASSERT_EQUAL(LHS, RHS, MESSAGE)
#define ADA_ASSERT_TRUE(COND)
#endif

#ifdef ADA_VISUAL_STUDIO
#define ADA_ASSUME(COND) __assume(COND)
#else
#define ADA_ASSUME(COND)       \
  do {                         \
    if (!(COND)) {             \
      __builtin_unreachable(); \
    }                          \
  } while (0)
#endif

#if defined(__SSSE3__)
#define ADA_SSSE3 1
#endif

#if defined(__SSE2__) || defined(__x86_64__) || defined(__x86_64) || \
    (defined(_M_AMD64) || defined(_M_X64) ||                         \
     (defined(_M_IX86_FP) && _M_IX86_FP == 2))
#define ADA_SSE2 1
#endif

// AVX-512 byte/word ops + 128/256-bit vectors of AVX-512 instructions.
// Used for optional high-performance IP address parsing kernels.
#if defined(__AVX512BW__) && defined(__AVX512VL__)
#define ADA_AVX512 1
#endif

#if defined(__aarch64__) || defined(_M_ARM64)
#define ADA_NEON 1
#endif

#if defined(__loongarch_sx)
#define ADA_LSX 1
#endif

#if defined(__riscv_v) && __riscv_v_intrinsic >= 11000
// Support RVV intrinsics v0.11 and above
#define ADA_RVV 1
#endif

#ifndef __has_cpp_attribute
#define ada_lifetime_bound
#elif __has_cpp_attribute(msvc::lifetimebound)
#define ada_lifetime_bound [[msvc::lifetimebound]]
#elif __has_cpp_attribute(clang::lifetimebound)
#define ada_lifetime_bound [[clang::lifetimebound]]
#elif __has_cpp_attribute(lifetimebound)
#define ada_lifetime_bound [[lifetimebound]]
#else
#define ada_lifetime_bound
#endif

#define ADA_INCLUDE_URL_PATTERN 0

#ifndef ADA_INCLUDE_URL_PATTERN
#define ADA_INCLUDE_URL_PATTERN 1
#endif  // ADA_INCLUDE_URL_PATTERN

#endif  // ADA_COMMON_DEFS_H
/* end file include/ada/common_defs.h */
#include <cstdint>

/**
 * These functions are not part of our public API and may
 * change at any time.
 * @private
 * @namespace ada::character_sets
 * @brief Includes the definitions for unicode character sets.
 */
namespace ada::character_sets {
ada_really_inline constexpr bool bit_at(const uint8_t a[], uint8_t i);
}  // namespace ada::character_sets

#endif  // ADA_CHARACTER_SETS_H
/* end file include/ada/character_sets.h */
/* begin file include/ada/character_sets-inl.h */
/**
 * @file character_sets-inl.h
 * @brief Definitions of the character sets used by unicode functions.
 * @author Node.js
 * @see https://github.com/nodejs/node/blob/main/src/node_url_tables.cc
 */
#ifndef ADA_CHARACTER_SETS_INL_H
#define ADA_CHARACTER_SETS_INL_H


/**
 * These functions are not part of our public API and may
 * change at any time.
 * @private
 */
namespace ada::character_sets {

constexpr char hex[1024] =
    "%00\0%01\0%02\0%03\0%04\0%05\0%06\0%07\0"
    "%08\0%09\0%0A\0%0B\0%0C\0%0D\0%0E\0%0F\0"
    "%10\0%11\0%12\0%13\0%14\0%15\0%16\0%17\0"
    "%18\0%19\0%1A\0%1B\0%1C\0%1D\0%1E\0%1F\0"
    "%20\0%21\0%22\0%23\0%24\0%25\0%26\0%27\0"
    "%28\0%29\0%2A\0%2B\0%2C\0%2D\0%2E\0%2F\0"
    "%30\0%31\0%32\0%33\0%34\0%35\0%36\0%37\0"
    "%38\0%39\0%3A\0%3B\0%3C\0%3D\0%3E\0%3F\0"
    "%40\0%41\0%42\0%43\0%44\0%45\0%46\0%47\0"
    "%48\0%49\0%4A\0%4B\0%4C\0%4D\0%4E\0%4F\0"
    "%50\0%51\0%52\0%53\0%54\0%55\0%56\0%57\0"
    "%58\0%59\0%5A\0%5B\0%5C\0%5D\0%5E\0%5F\0"
    "%60\0%61\0%62\0%63\0%64\0%65\0%66\0%67\0"
    "%68\0%69\0%6A\0%6B\0%6C\0%6D\0%6E\0%6F\0"
    "%70\0%71\0%72\0%73\0%74\0%75\0%76\0%77\0"
    "%78\0%79\0%7A\0%7B\0%7C\0%7D\0%7E\0%7F\0"
    "%80\0%81\0%82\0%83\0%84\0%85\0%86\0%87\0"
    "%88\0%89\0%8A\0%8B\0%8C\0%8D\0%8E\0%8F\0"
    "%90\0%91\0%92\0%93\0%94\0%95\0%96\0%97\0"
    "%98\0%99\0%9A\0%9B\0%9C\0%9D\0%9E\0%9F\0"
    "%A0\0%A1\0%A2\0%A3\0%A4\0%A5\0%A6\0%A7\0"
    "%A8\0%A9\0%AA\0%AB\0%AC\0%AD\0%AE\0%AF\0"
    "%B0\0%B1\0%B2\0%B3\0%B4\0%B5\0%B6\0%B7\0"
    "%B8\0%B9\0%BA\0%BB\0%BC\0%BD\0%BE\0%BF\0"
    "%C0\0%C1\0%C2\0%C3\0%C4\0%C5\0%C6\0%C7\0"
    "%C8\0%C9\0%CA\0%CB\0%CC\0%CD\0%CE\0%CF\0"
    "%D0\0%D1\0%D2\0%D3\0%D4\0%D5\0%D6\0%D7\0"
    "%D8\0%D9\0%DA\0%DB\0%DC\0%DD\0%DE\0%DF\0"
    "%E0\0%E1\0%E2\0%E3\0%E4\0%E5\0%E6\0%E7\0"
    "%E8\0%E9\0%EA\0%EB\0%EC\0%ED\0%EE\0%EF\0"
    "%F0\0%F1\0%F2\0%F3\0%F4\0%F5\0%F6\0%F7\0"
    "%F8\0%F9\0%FA\0%FB\0%FC\0%FD\0%FE\0%FF";

constexpr uint8_t C0_CONTROL_PERCENT_ENCODE[32] = {
    // 00     01     02     03     04     05     06     07
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 08     09     0A     0B     0C     0D     0E     0F
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 10     11     12     13     14     15     16     17
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 18     19     1A     1B     1C     1D     1E     1F
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 20     21     22     23     24     25     26     27
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 28     29     2A     2B     2C     2D     2E     2F
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 30     31     32     33     34     35     36     37
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 38     39     3A     3B     3C     3D     3E     3F
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 40     41     42     43     44     45     46     47
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 48     49     4A     4B     4C     4D     4E     4F
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 50     51     52     53     54     55     56     57
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 58     59     5A     5B     5C     5D     5E     5F
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 60     61     62     63     64     65     66     67
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 68     69     6A     6B     6C     6D     6E     6F
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 70     71     72     73     74     75     76     77
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 78     79     7A     7B     7C     7D     7E     7F
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x80,
    // 80     81     82     83     84     85     86     87
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 88     89     8A     8B     8C     8D     8E     8F
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 90     91     92     93     94     95     96     97
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 98     99     9A     9B     9C     9D     9E     9F
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // A0     A1     A2     A3     A4     A5     A6     A7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // A8     A9     AA     AB     AC     AD     AE     AF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // B0     B1     B2     B3     B4     B5     B6     B7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // B8     B9     BA     BB     BC     BD     BE     BF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // C0     C1     C2     C3     C4     C5     C6     C7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // C8     C9     CA     CB     CC     CD     CE     CF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // D0     D1     D2     D3     D4     D5     D6     D7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // D8     D9     DA     DB     DC     DD     DE     DF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // E0     E1     E2     E3     E4     E5     E6     E7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // E8     E9     EA     EB     EC     ED     EE     EF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // F0     F1     F2     F3     F4     F5     F6     F7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // F8     F9     FA     FB     FC     FD     FE     FF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80};

constexpr uint8_t SPECIAL_QUERY_PERCENT_ENCODE[32] = {
    // 00     01     02     03     04     05     06     07
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 08     09     0A     0B     0C     0D     0E     0F
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 10     11     12     13     14     15     16     17
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 18     19     1A     1B     1C     1D     1E     1F
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 20     21     22     23     24     25     26     27
    0x01 | 0x00 | 0x04 | 0x08 | 0x00 | 0x00 | 0x00 | 0x80,
    // 28     29     2A     2B     2C     2D     2E     2F
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 30     31     32     33     34     35     36     37
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 38     39     3A     3B     3C     3D     3E     3F
    0x00 | 0x00 | 0x00 | 0x00 | 0x10 | 0x00 | 0x40 | 0x00,
    // 40     41     42     43     44     45     46     47
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 48     49     4A     4B     4C     4D     4E     4F
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 50     51     52     53     54     55     56     57
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 58     59     5A     5B     5C     5D     5E     5F
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 60     61     62     63     64     65     66     67
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 68     69     6A     6B     6C     6D     6E     6F
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 70     71     72     73     74     75     76     77
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 78     79     7A     7B     7C     7D     7E     7F
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x80,
    // 80     81     82     83     84     85     86     87
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 88     89     8A     8B     8C     8D     8E     8F
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 90     91     92     93     94     95     96     97
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 98     99     9A     9B     9C     9D     9E     9F
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // A0     A1     A2     A3     A4     A5     A6     A7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // A8     A9     AA     AB     AC     AD     AE     AF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // B0     B1     B2     B3     B4     B5     B6     B7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // B8     B9     BA     BB     BC     BD     BE     BF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // C0     C1     C2     C3     C4     C5     C6     C7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // C8     C9     CA     CB     CC     CD     CE     CF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // D0     D1     D2     D3     D4     D5     D6     D7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // D8     D9     DA     DB     DC     DD     DE     DF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // E0     E1     E2     E3     E4     E5     E6     E7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // E8     E9     EA     EB     EC     ED     EE     EF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // F0     F1     F2     F3     F4     F5     F6     F7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // F8     F9     FA     FB     FC     FD     FE     FF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80};

constexpr uint8_t QUERY_PERCENT_ENCODE[32] = {
    // 00     01     02     03     04     05     06     07
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 08     09     0A     0B     0C     0D     0E     0F
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 10     11     12     13     14     15     16     17
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 18     19     1A     1B     1C     1D     1E     1F
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 20     21     22     23     24     25     26     27
    0x01 | 0x00 | 0x04 | 0x08 | 0x00 | 0x00 | 0x00 | 0x00,
    // 28     29     2A     2B     2C     2D     2E     2F
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 30     31     32     33     34     35     36     37
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 38     39     3A     3B     3C     3D     3E     3F
    0x00 | 0x00 | 0x00 | 0x00 | 0x10 | 0x00 | 0x40 | 0x00,
    // 40     41     42     43     44     45     46     47
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 48     49     4A     4B     4C     4D     4E     4F
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 50     51     52     53     54     55     56     57
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 58     59     5A     5B     5C     5D     5E     5F
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 60     61     62     63     64     65     66     67
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 68     69     6A     6B     6C     6D     6E     6F
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 70     71     72     73     74     75     76     77
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 78     79     7A     7B     7C     7D     7E     7F
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x80,
    // 80     81     82     83     84     85     86     87
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 88     89     8A     8B     8C     8D     8E     8F
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 90     91     92     93     94     95     96     97
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 98     99     9A     9B     9C     9D     9E     9F
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // A0     A1     A2     A3     A4     A5     A6     A7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // A8     A9     AA     AB     AC     AD     AE     AF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // B0     B1     B2     B3     B4     B5     B6     B7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // B8     B9     BA     BB     BC     BD     BE     BF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // C0     C1     C2     C3     C4     C5     C6     C7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // C8     C9     CA     CB     CC     CD     CE     CF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // D0     D1     D2     D3     D4     D5     D6     D7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // D8     D9     DA     DB     DC     DD     DE     DF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // E0     E1     E2     E3     E4     E5     E6     E7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // E8     E9     EA     EB     EC     ED     EE     EF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // F0     F1     F2     F3     F4     F5     F6     F7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // F8     F9     FA     FB     FC     FD     FE     FF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80};

constexpr uint8_t FRAGMENT_PERCENT_ENCODE[32] = {
    // 00     01     02     03     04     05     06     07
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 08     09     0A     0B     0C     0D     0E     0F
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 10     11     12     13     14     15     16     17
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 18     19     1A     1B     1C     1D     1E     1F
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 20     21     22     23     24     25     26     27
    0x01 | 0x00 | 0x04 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 28     29     2A     2B     2C     2D     2E     2F
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 30     31     32     33     34     35     36     37
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 38     39     3A     3B     3C     3D     3E     3F
    0x00 | 0x00 | 0x00 | 0x00 | 0x10 | 0x00 | 0x40 | 0x00,
    // 40     41     42     43     44     45     46     47
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 48     49     4A     4B     4C     4D     4E     4F
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 50     51     52     53     54     55     56     57
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 58     59     5A     5B     5C     5D     5E     5F
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 60     61     62     63     64     65     66     67
    0x01 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 68     69     6A     6B     6C     6D     6E     6F
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 70     71     72     73     74     75     76     77
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 78     79     7A     7B     7C     7D     7E     7F
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x80,
    // 80     81     82     83     84     85     86     87
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 88     89     8A     8B     8C     8D     8E     8F
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 90     91     92     93     94     95     96     97
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 98     99     9A     9B     9C     9D     9E     9F
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // A0     A1     A2     A3     A4     A5     A6     A7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // A8     A9     AA     AB     AC     AD     AE     AF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // B0     B1     B2     B3     B4     B5     B6     B7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // B8     B9     BA     BB     BC     BD     BE     BF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // C0     C1     C2     C3     C4     C5     C6     C7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // C8     C9     CA     CB     CC     CD     CE     CF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // D0     D1     D2     D3     D4     D5     D6     D7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // D8     D9     DA     DB     DC     DD     DE     DF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // E0     E1     E2     E3     E4     E5     E6     E7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // E8     E9     EA     EB     EC     ED     EE     EF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // F0     F1     F2     F3     F4     F5     F6     F7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // F8     F9     FA     FB     FC     FD     FE     FF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80};

constexpr uint8_t USERINFO_PERCENT_ENCODE[32] = {
    // 00     01     02     03     04     05     06     07
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 08     09     0A     0B     0C     0D     0E     0F
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 10     11     12     13     14     15     16     17
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 18     19     1A     1B     1C     1D     1E     1F
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 20     21     22     23     24     25     26     27
    0x01 | 0x00 | 0x04 | 0x08 | 0x00 | 0x00 | 0x00 | 0x00,
    // 28     29     2A     2B     2C     2D     2E     2F
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x80,
    // 30     31     32     33     34     35     36     37
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 38     39     3A     3B     3C     3D     3E     3F
    0x00 | 0x00 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 40     41     42     43     44     45     46     47
    0x01 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 48     49     4A     4B     4C     4D     4E     4F
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 50     51     52     53     54     55     56     57
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 58     59     5A     5B     5C     5D     5E     5F
    0x00 | 0x00 | 0x00 | 0x08 | 0x10 | 0x20 | 0x40 | 0x00,
    // 60     61     62     63     64     65     66     67
    0x01 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 68     69     6A     6B     6C     6D     6E     6F
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 70     71     72     73     74     75     76     77
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 78     79     7A     7B     7C     7D     7E     7F
    0x00 | 0x00 | 0x00 | 0x08 | 0x10 | 0x20 | 0x00 | 0x80,
    // 80     81     82     83     84     85     86     87
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 88     89     8A     8B     8C     8D     8E     8F
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 90     91     92     93     94     95     96     97
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 98     99     9A     9B     9C     9D     9E     9F
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // A0     A1     A2     A3     A4     A5     A6     A7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // A8     A9     AA     AB     AC     AD     AE     AF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // B0     B1     B2     B3     B4     B5     B6     B7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // B8     B9     BA     BB     BC     BD     BE     BF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // C0     C1     C2     C3     C4     C5     C6     C7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // C8     C9     CA     CB     CC     CD     CE     CF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // D0     D1     D2     D3     D4     D5     D6     D7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // D8     D9     DA     DB     DC     DD     DE     DF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // E0     E1     E2     E3     E4     E5     E6     E7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // E8     E9     EA     EB     EC     ED     EE     EF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // F0     F1     F2     F3     F4     F5     F6     F7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // F8     F9     FA     FB     FC     FD     FE     FF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80};

constexpr uint8_t PATH_PERCENT_ENCODE[32] = {
    // 00     01     02     03     04     05     06     07
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 08     09     0A     0B     0C     0D     0E     0F
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 10     11     12     13     14     15     16     17
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 18     19     1A     1B     1C     1D     1E     1F
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 20     21     22     23     24     25     26     27
    0x01 | 0x00 | 0x04 | 0x08 | 0x00 | 0x00 | 0x00 | 0x00,
    // 28     29     2A     2B     2C     2D     2E     2F
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 30     31     32     33     34     35     36     37
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 38     39     3A     3B     3C     3D     3E     3F
    0x00 | 0x00 | 0x00 | 0x00 | 0x10 | 0x00 | 0x40 | 0x80,
    // 40     41     42     43     44     45     46     47
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 48     49     4A     4B     4C     4D     4E     4F
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 50     51     52     53     54     55     56     57
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 58     59     5A     5B     5C     5D     5E     5F
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x40 | 0x00,
    // 60     61     62     63     64     65     66     67
    0x01 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 68     69     6A     6B     6C     6D     6E     6F
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 70     71     72     73     74     75     76     77
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 78     79     7A     7B     7C     7D     7E     7F
    0x00 | 0x00 | 0x00 | 0x08 | 0x00 | 0x20 | 0x00 | 0x80,
    // 80     81     82     83     84     85     86     87
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 88     89     8A     8B     8C     8D     8E     8F
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 90     91     92     93     94     95     96     97
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 98     99     9A     9B     9C     9D     9E     9F
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // A0     A1     A2     A3     A4     A5     A6     A7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // A8     A9     AA     AB     AC     AD     AE     AF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // B0     B1     B2     B3     B4     B5     B6     B7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // B8     B9     BA     BB     BC     BD     BE     BF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // C0     C1     C2     C3     C4     C5     C6     C7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // C8     C9     CA     CB     CC     CD     CE     CF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // D0     D1     D2     D3     D4     D5     D6     D7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // D8     D9     DA     DB     DC     DD     DE     DF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // E0     E1     E2     E3     E4     E5     E6     E7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // E8     E9     EA     EB     EC     ED     EE     EF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // F0     F1     F2     F3     F4     F5     F6     F7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // F8     F9     FA     FB     FC     FD     FE     FF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80};

constexpr uint8_t WWW_FORM_URLENCODED_PERCENT_ENCODE[32] = {
    // 00     01     02     03     04     05     06     07
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 08     09     0A     0B     0C     0D     0E     0F
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 10     11     12     13     14     15     16     17
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 18     19     1A     1B     1C     1D     1E     1F
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 20     21     22     23     24     25     26     27
    0x00 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 28     29     2A     2B     2C     2D     2E     2F
    0x01 | 0x02 | 0x00 | 0x08 | 0x10 | 0x00 | 0x00 | 0x80,
    // 30     31     32     33     34     35     36     37
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 38     39     3A     3B     3C     3D     3E     3F
    0x00 | 0x00 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 40     41     42     43     44     45     46     47
    0x01 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 48     49     4A     4B     4C     4D     4E     4F
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 50     51     52     53     54     55     56     57
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 58     59     5A     5B     5C     5D     5E     5F
    0x00 | 0x00 | 0x00 | 0x08 | 0x10 | 0x20 | 0x40 | 0x00,
    // 60     61     62     63     64     65     66     67
    0x01 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 68     69     6A     6B     6C     6D     6E     6F
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 70     71     72     73     74     75     76     77
    0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
    // 78     79     7A     7B     7C     7D     7E     7F
    0x00 | 0x00 | 0x00 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 80     81     82     83     84     85     86     87
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 88     89     8A     8B     8C     8D     8E     8F
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 90     91     92     93     94     95     96     97
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // 98     99     9A     9B     9C     9D     9E     9F
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // A0     A1     A2     A3     A4     A5     A6     A7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // A8     A9     AA     AB     AC     AD     AE     AF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // B0     B1     B2     B3     B4     B5     B6     B7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // B8     B9     BA     BB     BC     BD     BE     BF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // C0     C1     C2     C3     C4     C5     C6     C7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // C8     C9     CA     CB     CC     CD     CE     CF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // D0     D1     D2     D3     D4     D5     D6     D7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // D8     D9     DA     DB     DC     DD     DE     DF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // E0     E1     E2     E3     E4     E5     E6     E7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // E8     E9     EA     EB     EC     ED     EE     EF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // F0     F1     F2     F3     F4     F5     F6     F7
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
    // F8     F9     FA     FB     FC     FD     FE     FF
    0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80};

ada_really_inline constexpr bool bit_at(const uint8_t a[], const uint8_t i) {
  return !!(a[i >> 3] & (1 << (i & 7)));
}

}  // namespace ada::character_sets

#endif  // ADA_CHARACTER_SETS_INL_H
/* end file include/ada/character_sets-inl.h */
/* begin file include/ada/checkers-inl.h */
/**
 * @file checkers-inl.h
 * @brief Definitions for URL specific checkers used within Ada.
 */
#ifndef ADA_CHECKERS_INL_H
#define ADA_CHECKERS_INL_H

#include <bit>
#include <cstdint>
#include <cstring>
#include <string_view>
/* begin file include/ada/checkers.h */
/**
 * @file checkers.h
 * @brief Declarations for URL specific checkers used within Ada.
 */
#ifndef ADA_CHECKERS_H
#define ADA_CHECKERS_H


#include <cstring>
#include <string_view>

/**
 * These functions are not part of our public API and may
 * change at any time.
 * @private
 * @namespace ada::checkers
 * @brief Includes the definitions for validation functions
 */
namespace ada::checkers {

/**
 * @private
 * Assuming that x is an ASCII letter, this function returns the lower case
 * equivalent.
 * @details More likely to be inlined by the compiler and constexpr.
 */
constexpr char to_lower(char x) noexcept;

/**
 * @private
 * Returns true if the character is an ASCII letter. Equivalent to std::isalpha
 * but more likely to be inlined by the compiler.
 *
 * @attention std::isalpha is not constexpr generally.
 */
constexpr bool is_alpha(char x) noexcept;

/**
 * @private
 * Check whether a string starts with 0x or 0X. The function is only
 * safe if input.size() >=2.
 *
 * @see has_hex_prefix
 */
constexpr bool has_hex_prefix_unsafe(std::string_view input);
/**
 * @private
 * Check whether a string starts with 0x or 0X.
 */
constexpr bool has_hex_prefix(std::string_view input);

/**
 * @private
 * Check whether x is an ASCII digit. More likely to be inlined than
 * std::isdigit.
 */
constexpr bool is_digit(char x) noexcept;

/**
 * @private
 * @details A string starts with a Windows drive letter if all of the following
 * are true:
 *
 *   - its length is greater than or equal to 2
 *   - its first two code points are a Windows drive letter
 *   - its length is 2 or its third code point is U+002F (/), U+005C (\), U+003F
 * (?), or U+0023 (#).
 *
 * https://url.spec.whatwg.org/#start-with-a-windows-drive-letter
 */
inline constexpr bool is_windows_drive_letter(std::string_view input) noexcept;

/**
 * @private
 * @details A normalized Windows drive letter is a Windows drive letter of which
 * the second code point is U+003A (:).
 */
inline constexpr bool is_normalized_windows_drive_letter(
    std::string_view input) noexcept;

/**
 * @private
 * Returns true if an input is an ipv4 address. It is assumed that the string
 * does not contain uppercase ASCII characters (the input should have been
 * lowered cased before calling this function) and is not empty.
 */
ada_really_inline constexpr bool is_ipv4(std::string_view view) noexcept;

/**
 * @private
 * Returns a bitset. If the first bit is set, then at least one character needs
 * percent encoding. If the second bit is set, a \\ is found. If the third bit
 * is set then we have a dot. If the fourth bit is set, then we have a percent
 * character.
 */
ada_really_inline constexpr uint8_t path_signature(
    std::string_view input) noexcept;

/**
 * @private
 * Returns true if the length of the domain name and its labels are according to
 * the specifications. The length of the domain must be 255 octets (253
 * characters not including the last 2 which are the empty label reserved at the
 * end). When the empty label is included (a dot at the end), the domain name
 * can have 254 characters. The length of a label must be at least 1 and at most
 * 63 characters.
 * @see section 3.1. of https://www.rfc-editor.org/rfc/rfc1034
 * @see https://www.unicode.org/reports/tr46/#ToASCII
 */
ada_really_inline constexpr bool verify_dns_length(
    std::string_view input) noexcept;

/**
 * @private
 * Fast-path parser for pure decimal IPv4 addresses (e.g., "192.168.1.1").
 * Returns the packed 32-bit IPv4 address on success, or a value > 0xFFFFFFFF
 * to indicate failure (caller should fall back to general parser).
 * This is optimized for the common case where the input is a well-formed
 * decimal IPv4 address with exactly 4 octets.
 */
ada_really_inline uint64_t try_parse_ipv4_fast(std::string_view input) noexcept;

/**
 * Sentinel value indicating try_parse_ipv4_fast() did not succeed.
 * Any value > 0xFFFFFFFF indicates the fast path should not be used.
 */
constexpr uint64_t ipv4_fast_fail = uint64_t(1) << 32;

}  // namespace ada::checkers

#endif  // ADA_CHECKERS_H
/* end file include/ada/checkers.h */

#if defined(ADA_AVX512)
#include <immintrin.h>
#endif

namespace ada::checkers {

constexpr bool has_hex_prefix_unsafe(std::string_view input) {
  // This is actually efficient code, see has_hex_prefix for the assembly.
  constexpr bool is_little_endian = std::endian::native == std::endian::little;
  constexpr uint16_t word0x = 0x7830;
  uint16_t two_first_bytes =
      static_cast<uint16_t>(input[0]) |
      static_cast<uint16_t>((static_cast<uint16_t>(input[1]) << 8));
  if constexpr (is_little_endian) {
    two_first_bytes |= 0x2000;
  } else {
    two_first_bytes |= 0x020;
  }
  return two_first_bytes == word0x;
}

constexpr bool has_hex_prefix(std::string_view input) {
  return input.size() >= 2 && has_hex_prefix_unsafe(input);
}

constexpr bool is_digit(char x) noexcept { return (x >= '0') & (x <= '9'); }

constexpr char to_lower(char x) noexcept { return (x | 0x20); }

constexpr bool is_alpha(char x) noexcept {
  return (to_lower(x) >= 'a') && (to_lower(x) <= 'z');
}

constexpr bool is_windows_drive_letter(std::string_view input) noexcept {
  return input.size() >= 2 &&
         (is_alpha(input[0]) && ((input[1] == ':') || (input[1] == '|'))) &&
         ((input.size() == 2) || (input[2] == '/' || input[2] == '\\' ||
                                  input[2] == '?' || input[2] == '#'));
}

constexpr bool is_normalized_windows_drive_letter(
    std::string_view input) noexcept {
  return input.size() == 2 && (is_alpha(input[0]) && (input[1] == ':'));
}

namespace detail {

// Unrolled pure-decimal IPv4. The common portable path for 7-16 byte hosts.
ada_really_inline uint64_t
parse_ipv4_decimal_scalar(const char* p, const char* pend) noexcept {
  uint32_t ipv4 = 0;
  for (int i = 0; i < 4; ++i) {
    if (p == pend) [[unlikely]] {
      return ipv4_fast_fail;
    }
    uint32_t val;
    char c = *p;
    if (c >= '0' && c <= '9') [[likely]] {
      val = static_cast<uint32_t>(c - '0');
      ++p;
    } else {
      return ipv4_fast_fail;
    }
    if (p < pend) {
      c = *p;
      if (c >= '0' && c <= '9') {
        if (val == 0) [[unlikely]] {
          return ipv4_fast_fail;
        }
        val = val * 10u + static_cast<uint32_t>(c - '0');
        ++p;
        if (p < pend) {
          c = *p;
          if (c >= '0' && c <= '9') {
            val = val * 10u + static_cast<uint32_t>(c - '0');
            ++p;
            if (val > 255u) [[unlikely]] {
              return ipv4_fast_fail;
            }
          }
        }
      }
    }
    ipv4 = (ipv4 << 8) | val;
    if (i < 3) {
      if (p == pend || *p != '.') [[unlikely]] {
        return ipv4_fast_fail;
      }
      ++p;
    }
  }
  if (p != pend) {
    if (p == pend - 1 && *p == '.') {
      return ipv4;
    }
    return ipv4_fast_fail;
  }
  return ipv4;
}

#if defined(ADA_AVX512)
// After SIMD validation: fewer rejection branches on convert.
ada_really_inline uint64_t
parse_ipv4_decimal_trusted(const char* p, const char* pend) noexcept {
  uint32_t ipv4 = 0;
  for (int i = 0; i < 4; ++i) {
    uint32_t val = static_cast<uint32_t>(*p - '0');
    ++p;
    if (p < pend && static_cast<unsigned char>(*p - '0') <= 9) {
      if (val == 0) [[unlikely]] {
        return ipv4_fast_fail;
      }
      val = val * 10u + static_cast<uint32_t>(*p - '0');
      ++p;
      if (p < pend && static_cast<unsigned char>(*p - '0') <= 9) {
        val = val * 10u + static_cast<uint32_t>(*p - '0');
        ++p;
        if (val > 255u) [[unlikely]] {
          return ipv4_fast_fail;
        }
      }
    }
    ipv4 = (ipv4 << 8) | val;
    if (i < 3) {
      ++p;  // trusted '.'
    }
  }
  return ipv4;  // trailing-dot already accounted for by caller via pend
}

// AVX-512 pure-decimal IPv4 (Lemire/Mula-style masked load + parallel checks).
// No over-read of the source string. Wins when the binary is built with
// -mavx512bw -mavx512vl (or -march that enables them).
ada_really_inline uint64_t try_parse_ipv4_avx512(const char* data,
                                                 size_t len) noexcept {
  const __mmask16 live = static_cast<__mmask16>((1u << len) - 1u);
  const __m128i input =
      _mm_maskz_loadu_epi8(live, reinterpret_cast<const void*>(data));
  const __mmask16 is_dot =
      _mm_mask_cmpeq_epi8_mask(live, input, _mm_set1_epi8('.'));
  const __m128i shifted = _mm_sub_epi8(input, _mm_set1_epi8('0'));
  const __mmask16 is_digit =
      _mm_mask_cmplt_epu8_mask(live, shifted, _mm_set1_epi8(10));
  if ((is_digit | is_dot) != live) {
    return ipv4_fast_fail;
  }
  const unsigned dot_count =
      static_cast<unsigned>(_mm_popcnt_u32(static_cast<unsigned>(is_dot)));
  size_t effective_len = len;
  if (dot_count == 3) {
    // ok
  } else if (dot_count == 4 && data[len - 1] == '.') {
    effective_len = len - 1;  // strip trailing dot for convert
  } else {
    return ipv4_fast_fail;
  }
  // Convert from a tiny stack copy so trusted peeks stay in-bounds.
  alignas(16) char buf[16]{};
  std::memcpy(buf, data, effective_len);
  return parse_ipv4_decimal_trusted(buf, buf + effective_len);
}
#endif  // ADA_AVX512

}  // namespace detail

/**
 * Fast pure-decimal IPv4 parse. Returns packed address or ipv4_fast_fail.
 * Accepts an optional single trailing dot.
 *
 * On AVX-512BW+VL targets, uses a masked-load SIMD kernel (no source
 * over-read) inspired by Lemire/Mula. Otherwise uses an unrolled scalar path
 * (typically faster than SSE2/NEON pre-validation for these 7-16 byte hosts).
 */
ada_really_inline uint64_t
try_parse_ipv4_fast(std::string_view input) noexcept {
  const size_t len = input.size();
  // Shortest pure decimal: "0.0.0.0" (7). Longest + trailing dot: 16.
  if (len < 7 || len > 16) [[unlikely]] {
    return ipv4_fast_fail;
  }
  const char* data = input.data();

#if defined(ADA_AVX512)
  return detail::try_parse_ipv4_avx512(data, len);
#else
  return detail::parse_ipv4_decimal_scalar(data, data + len);
#endif
}

}  // namespace ada::checkers

#endif  // ADA_CHECKERS_INL_H
/* end file include/ada/checkers-inl.h */
/* begin file include/ada/log.h */
/**
 * @file log.h
 * @brief Includes the definitions for logging.
 * @private Excluded from docs through the doxygen file.
 */
#ifndef ADA_LOG_H
#define ADA_LOG_H

// To enable logging, set ADA_LOGGING to 1:
#ifndef ADA_LOGGING
#define ADA_LOGGING 0
#endif

#if ADA_LOGGING
#include <iostream>
#endif  // ADA_LOGGING

namespace ada {

/**
 * Log a message. If you want to have no overhead when logging is disabled, use
 * the ada_log macro.
 * @private
 */
template <typename... Args>
ada_really_inline void log([[maybe_unused]] Args... args) {
#if ADA_LOGGING
  ((std::cout << "ADA_LOG: ") << ... << args) << std::endl;
#endif  // ADA_LOGGING
}
}  // namespace ada

#if ADA_LOGGING
#ifndef ada_log
#define ada_log(...)       \
  do {                     \
    ada::log(__VA_ARGS__); \
  } while (0)
#endif  // ada_log
#else
#define ada_log(...)
#endif  // ADA_LOGGING

#endif  // ADA_LOG_H
/* end file include/ada/log.h */
/* begin file include/ada/encoding_type.h */
/**
 * @file encoding_type.h
 * @brief Character encoding type definitions.
 *
 * Defines the encoding types supported for URL processing.
 *
 * @see https://encoding.spec.whatwg.org/
 */
#ifndef ADA_ENCODING_TYPE_H
#define ADA_ENCODING_TYPE_H

#include <string>

namespace ada {

/**
 * @brief Character encoding types for URL processing.
 *
 * Specifies the character encoding used for percent-decoding and other
 * string operations. UTF-8 is the most commonly used encoding for URLs.
 *
 * @see https://encoding.spec.whatwg.org/#encodings
 */
enum class encoding_type {
  UTF8,     /**< UTF-8 encoding (default for URLs) */
  UTF_16LE, /**< UTF-16 Little Endian encoding */
  UTF_16BE, /**< UTF-16 Big Endian encoding */
};

/**
 * Converts an encoding_type to its string representation.
 * @param type The encoding type to convert.
 * @return A string view of the encoding name.
 */
ada_warn_unused std::string_view to_string(encoding_type type);

}  // namespace ada

#endif  // ADA_ENCODING_TYPE_H
/* end file include/ada/encoding_type.h */
/* begin file include/ada/helpers.h */
/**
 * @file helpers.h
 * @brief Definitions for helper functions used within Ada.
 */
#ifndef ADA_HELPERS_H
#define ADA_HELPERS_H

/* begin file include/ada/url_base.h */
/**
 * @file url_base.h
 * @brief Base class and common definitions for URL types.
 *
 * This file defines the `url_base` abstract base class from which both
 * `ada::url` and `ada::url_aggregator` inherit. It also defines common
 * enumerations like `url_host_type`.
 */
#ifndef ADA_URL_BASE_H
#define ADA_URL_BASE_H

/* begin file include/ada/scheme.h */
/**
 * @file scheme.h
 * @brief URL scheme type definitions and utilities.
 *
 * This header defines the URL scheme types (http, https, etc.) and provides
 * functions to identify special schemes and their default ports according
 * to the WHATWG URL Standard.
 *
 * @see https://url.spec.whatwg.org/#special-scheme
 */
#ifndef ADA_SCHEME_H
#define ADA_SCHEME_H


#include <string>

/**
 * @namespace ada::scheme
 * @brief URL scheme utilities and constants.
 *
 * Provides functions for working with URL schemes, including identification
 * of special schemes and retrieval of default port numbers.
 */
namespace ada::scheme {

/**
 * @brief Enumeration of URL scheme types.
 *
 * Special schemes have specific parsing rules and default ports.
 * Using an enum allows efficient scheme comparisons without string operations.
 *
 * Default ports:
 * - HTTP: 80
 * - HTTPS: 443
 * - WS: 80
 * - WSS: 443
 * - FTP: 21
 * - FILE: (none)
 */
enum type : uint8_t {
  HTTP = 0,        /**< http:// scheme (port 80) */
  NOT_SPECIAL = 1, /**< Non-special scheme (no default port) */
  HTTPS = 2,       /**< https:// scheme (port 443) */
  WS = 3,          /**< ws:// WebSocket scheme (port 80) */
  FTP = 4,         /**< ftp:// scheme (port 21) */
  WSS = 5,         /**< wss:// secure WebSocket scheme (port 443) */
  FILE = 6         /**< file:// scheme (no default port) */
};

/**
 * Checks if a scheme string is a special scheme.
 * @param scheme The scheme string to check (e.g., "http", "https").
 * @return `true` if the scheme is special, `false` otherwise.
 * @see https://url.spec.whatwg.org/#special-scheme
 */
ada_really_inline constexpr bool is_special(std::string_view scheme);

/**
 * Returns the default port for a special scheme string.
 * @param scheme The scheme string (e.g., "http", "https").
 * @return The default port number, or 0 if not a special scheme.
 * @see https://url.spec.whatwg.org/#special-scheme
 */
inline uint16_t get_special_port(std::string_view scheme) noexcept;

/**
 * Returns the default port for a scheme type.
 * @param type The scheme type enum value.
 * @return The default port number, or 0 if not applicable.
 * @see https://url.spec.whatwg.org/#special-scheme
 */
constexpr uint16_t get_special_port(ada::scheme::type type) noexcept;

/**
 * Converts a scheme string to its type enum.
 * @param scheme The scheme string to convert.
 * @return The corresponding scheme type, or NOT_SPECIAL if not recognized.
 */
inline ada::scheme::type get_scheme_type(std::string_view scheme) noexcept;

}  // namespace ada::scheme

#endif  // ADA_SCHEME_H
/* end file include/ada/scheme.h */

#include <string>
#include <string_view>

namespace ada {

/**
 * @brief Enum representing the type of host in a URL.
 *
 * Used to distinguish between regular domain names, IPv4 addresses,
 * and IPv6 addresses for proper parsing and serialization.
 */
enum url_host_type : uint8_t {
  /** Regular domain name (e.g., "www.example.com") */
  DEFAULT = 0,
  /** IPv4 address (e.g., "127.0.0.1") */
  IPV4 = 1,
  /** IPv6 address (e.g., "[::1]" or "[2001:db8::1]") */
  IPV6 = 2,
};

/**
 * @brief Abstract base class for URL representations.
 *
 * The `url_base` class provides the common interface and state shared by
 * both `ada::url` and `ada::url_aggregator`. It contains basic URL attributes
 * like validity status and scheme type, but delegates component storage and
 * access to derived classes.
 *
 * @note This is an abstract class and cannot be instantiated directly.
 *       Use `ada::url` or `ada::url_aggregator` instead.
 *
 * @see url
 * @see url_aggregator
 */
struct url_base {
  virtual ~url_base() = default;

  /**
   * Indicates whether the URL was successfully parsed.
   * Set to `false` if parsing failed (e.g., invalid URL syntax).
   */
  bool is_valid{true};

  /**
   * Indicates whether the URL has an opaque path (non-hierarchical).
   * Opaque paths occur in non-special URLs like `mailto:` or `javascript:`.
   */
  bool has_opaque_path{false};

  /**
   * The type of the URL's host (domain, IPv4, or IPv6).
   */
  url_host_type host_type = url_host_type::DEFAULT;

  /**
   * @private
   * Internal representation of the URL's scheme type.
   */
  ada::scheme::type type{ada::scheme::type::NOT_SPECIAL};

  /**
   * Checks if the URL has a special scheme (http, https, ws, wss, ftp, file).
   * Special schemes have specific parsing rules and default ports.
   * @return `true` if the scheme is special, `false` otherwise.
   */
  [[nodiscard]] ada_really_inline bool is_special() const noexcept;

  /**
   * Returns the URL's origin (scheme + host + port for special URLs).
   * @return A newly allocated string containing the serialized origin.
   * @see https://url.spec.whatwg.org/#concept-url-origin
   */
  [[nodiscard]] virtual std::string get_origin() const = 0;

  /**
   * Validates whether the hostname is a valid domain according to RFC 1034.
   * Checks that the domain and its labels have valid lengths.
   * @return `true` if the domain is valid, `false` otherwise.
   */
  [[nodiscard]] virtual bool has_valid_domain() const noexcept = 0;

  /**
   * @private
   * Returns the default port for special schemes (e.g., 443 for https).
   * Returns 0 for file:// URLs or non-special schemes.
   */
  [[nodiscard]] inline uint16_t get_special_port() const noexcept;

  /**
   * @private
   * Returns the default port for the URL's scheme, or 0 if none.
   */
  [[nodiscard]] ada_really_inline uint16_t scheme_default_port() const noexcept;

  /**
   * @private
   * Parses a port number from the input string.
   * @param view The string containing the port to parse.
   * @param check_trailing_content Whether to validate no trailing characters.
   * @return Number of bytes consumed on success, 0 on failure.
   */
  virtual size_t parse_port(std::string_view view,
                            bool check_trailing_content) = 0;

  /** @private */
  virtual ada_really_inline size_t parse_port(std::string_view view) {
    return this->parse_port(view, false);
  }

  /**
   * Returns a JSON string representation of this URL for debugging.
   * @return A JSON-formatted string with URL information.
   */
  [[nodiscard]] virtual std::string to_string() const = 0;

  /** @private */
  virtual inline void clear_pathname() = 0;

  /** @private */
  virtual inline void clear_search() = 0;

  /** @private */
  [[nodiscard]] virtual inline bool has_hash() const noexcept = 0;

  /** @private */
  [[nodiscard]] virtual inline bool has_search() const noexcept = 0;

};  // url_base

}  // namespace ada

#endif
/* end file include/ada/url_base.h */

#include <string>
#include <string_view>
#include <optional>

#if ADA_DEVELOPMENT_CHECKS
#include <iostream>
#endif  // ADA_DEVELOPMENT_CHECKS

/**
 * These functions are not part of our public API and may
 * change at any time.
 *
 * @private
 * @namespace ada::helpers
 * @brief Includes the definitions for helper functions
 */
namespace ada::helpers {

/**
 * @private
 */
template <typename out_iter>
void encode_json(std::string_view view, out_iter out);

/**
 * @private
 * This function is used to prune a fragment from a url, and returning the
 * removed string if input has fragment.
 *
 * @details prune_hash seeks the first '#' and returns everything after it
 * as a string_view, and modifies (in place) the input so that it points at
 * everything before the '#'. If no '#' is found, the input is left unchanged
 * and std::nullopt is returned.
 *
 * @attention The function is non-allocating and it does not throw.
 * @returns Note that the returned string_view might be empty!
 */
ada_really_inline std::optional<std::string_view> prune_hash(
    std::string_view& input) noexcept;

/**
 * @private
 * Defined by the URL specification, shorten a URLs paths.
 * @see https://url.spec.whatwg.org/#shorten-a-urls-path
 * @returns Returns true if path is shortened.
 */
ada_really_inline bool shorten_path(std::string& path, ada::scheme::type type);

/**
 * @private
 * Defined by the URL specification, shorten a URLs paths.
 * @see https://url.spec.whatwg.org/#shorten-a-urls-path
 * @returns Returns true if path is shortened.
 */
ada_really_inline bool shorten_path(std::string_view& path,
                                    ada::scheme::type type);

/**
 * @private
 *
 * Parse the path from the provided input and append to the existing
 * (possibly empty) path. The input cannot contain tabs and spaces: it
 * is the user's responsibility to check.
 *
 * The input is expected to be UTF-8.
 *
 * @see https://url.spec.whatwg.org/
 */
ada_really_inline void parse_prepared_path(std::string_view input,
                                           ada::scheme::type type,
                                           std::string& path);

/**
 * @private
 * Remove and mutate all ASCII tab or newline characters from an input.
 */
ada_really_inline void remove_ascii_tab_or_newline(std::string& input);

/**
 * @private
 * Return the substring from input going from index pos to the end.
 */
ada_really_inline constexpr std::string_view substring(std::string_view input,
                                                       size_t pos);

/**
 * @private
 * Returns true if the string_view points within the string.
 */
bool overlaps(std::string_view input1, const std::string& input2) noexcept;

/**
 * @private
 * Return the substring from input going from index pos1 to the pos2 (non
 * included). The length of the substring is pos2 - pos1.
 */
ada_really_inline constexpr std::string_view substring(std::string_view input,
                                                       size_t pos1,
                                                       size_t pos2) {
  return input.substr(pos1, pos2 - pos1);
}

/**
 * @private
 * Modify the string_view so that it has the new size pos, assuming that pos <=
 * input.size(). This function cannot throw.
 */
ada_really_inline void resize(std::string_view& input, size_t pos) noexcept;

/**
 * @private
 * Returns a host's delimiter location depending on the state of the instance,
 * and whether a colon was found outside brackets. Used by the host parser.
 */
ada_really_inline std::pair<size_t, bool> get_host_delimiter_location(
    bool is_special, std::string_view& view) noexcept;

/**
 * @private
 * Removes leading and trailing C0 control and whitespace characters from
 * string.
 */
void trim_c0_whitespace(std::string_view& input) noexcept;

/**
 * @private
 * @see
 * https://url.spec.whatwg.org/#potentially-strip-trailing-spaces-from-an-opaque-path
 */
template <class url_type>
ada_really_inline void strip_trailing_spaces_from_opaque_path(url_type& url);

/**
 * @private
 * Finds the delimiter of a view in authority state.
 */
ada_really_inline size_t
find_authority_delimiter_special(std::string_view view) noexcept;

/**
 * @private
 * Finds the delimiter of a view in authority state.
 */
ada_really_inline size_t
find_authority_delimiter(std::string_view view) noexcept;

/**
 * @private
 */
template <typename T, typename... Args>
inline void inner_concat(std::string& buffer, T t) {
  buffer.append(t);
}

/**
 * @private
 */
template <typename T, typename... Args>
inline void inner_concat(std::string& buffer, T t, Args... args) {
  buffer.append(t);
  return inner_concat(buffer, args...);
}

/**
 * @private
 * Concatenate the arguments and return a string.
 * @returns a string
 */
template <typename... Args>
std::string concat(Args... args) {
  std::string answer;
  inner_concat(answer, args...);
  return answer;
}

/**
 * @private
 * @return Number of leading zeroes.
 */
inline int leading_zeroes(uint32_t input_num) noexcept {
#if ADA_REGULAR_VISUAL_STUDIO
  unsigned long leading_zero(0);
  unsigned long in(input_num);
  return _BitScanReverse(&leading_zero, in) ? int(31 - leading_zero) : 32;
#else
  return __builtin_clz(input_num);
#endif  // ADA_REGULAR_VISUAL_STUDIO
}

/**
 * @private
 * Counts the number of decimal digits necessary to represent x.
 * faster than std::to_string(x).size().
 * @return digit count
 */
inline int fast_digit_count(uint32_t x) noexcept {
  auto int_log2 = [](uint32_t z) -> int {
    return 31 - ada::helpers::leading_zeroes(z | 1);
  };
  // Compiles to very few instructions. Note that the
  // table is static and thus effectively a constant.
  // We leave it inside the function because it is meaningless
  // outside of it (this comes at no performance cost).
  const static uint64_t table[] = {
      4294967296,  8589934582,  8589934582,  8589934582,  12884901788,
      12884901788, 12884901788, 17179868184, 17179868184, 17179868184,
      21474826480, 21474826480, 21474826480, 21474826480, 25769703776,
      25769703776, 25769703776, 30063771072, 30063771072, 30063771072,
      34349738368, 34349738368, 34349738368, 34349738368, 38554705664,
      38554705664, 38554705664, 41949672960, 41949672960, 41949672960,
      42949672960, 42949672960};
  return int((x + table[int_log2(x)]) >> 32);
}
}  // namespace ada::helpers

#endif  // ADA_HELPERS_H
/* end file include/ada/helpers.h */
/* begin file include/ada/parser.h */
/**
 * @file parser.h
 * @brief Low-level URL parsing functions.
 *
 * This header provides the internal URL parsing implementation. Most users
 * should use `ada::parse()` from implementation.h instead of these functions
 * directly.
 *
 * @see implementation.h for the recommended public API
 */
#ifndef ADA_PARSER_H
#define ADA_PARSER_H

#include <string_view>

/** @private Forward declarations */
namespace ada {
struct url_aggregator;
struct url;
}  // namespace ada

/**
 * @namespace ada::parser
 * @brief Internal URL parsing implementation.
 *
 * Contains the core URL parsing algorithm as specified by the WHATWG URL
 * Standard. These functions are used internally by `ada::parse()`.
 */
namespace ada::parser {
/**
 * Parses a URL string into a URL object.
 *
 * @tparam result_type The type of URL object to create (url or url_aggregator).
 *
 * @param user_input The URL string to parse (must be valid UTF-8).
 * @param base_url Optional base URL for resolving relative URLs.
 *
 * @return The parsed URL object. Check `is_valid` to determine if parsing
 *         succeeded.
 *
 * @see https://url.spec.whatwg.org/#concept-basic-url-parser
 */
template <typename result_type = url_aggregator>
result_type parse_url(std::string_view user_input,
                      const result_type* base_url = nullptr);

extern template url_aggregator parse_url<url_aggregator>(
    std::string_view user_input, const url_aggregator* base_url);
extern template url parse_url<url>(std::string_view user_input,
                                   const url* base_url);

template <typename result_type = url_aggregator, bool store_values = true>
result_type parse_url_impl(std::string_view user_input,
                           const result_type* base_url = nullptr);

extern template url_aggregator parse_url_impl<url_aggregator, true>(
    std::string_view user_input, const url_aggregator* base_url);
extern template url_aggregator parse_url_impl<url_aggregator, false>(
    std::string_view user_input, const url_aggregator* base_url);
extern template url parse_url_impl<url, true>(std::string_view user_input,
                                              const url* base_url);

/** @private */
template <class result_type>
bool try_parse_simple_absolute(std::string_view input, result_type& out);

}  // namespace ada::parser

#endif  // ADA_PARSER_H
/* end file include/ada/parser.h */
/* begin file include/ada/parser-inl.h */
/**
 * @file parser-inl.h
 */
#ifndef ADA_PARSER_INL_H
#define ADA_PARSER_INL_H

/* begin file include/ada/url_pattern.h */
/**
 * @file url_pattern.h
 * @brief URLPattern API implementation.
 *
 * This header provides the URLPattern API as specified by the WHATWG URL
 * Pattern Standard. URLPattern allows matching URLs against patterns with
 * wildcards and named groups, similar to how regular expressions match strings.
 *
 * @see https://urlpattern.spec.whatwg.org/
 * @see https://developer.mozilla.org/en-US/docs/Web/API/URL_Pattern_API
 */
#ifndef ADA_URL_PATTERN_H
#define ADA_URL_PATTERN_H

/* begin file include/ada/implementation.h */
/**
 * @file implementation.h
 * @brief User-facing functions for URL parsing and manipulation.
 *
 * This header provides the primary public API for parsing URLs in Ada.
 * It includes the main `ada::parse()` function which is the recommended
 * entry point for most users.
 *
 * @see https://url.spec.whatwg.org/#api
 */
#ifndef ADA_IMPLEMENTATION_H
#define ADA_IMPLEMENTATION_H

#include <string>
#include <string_view>
#include <optional>

/* begin file include/ada/url.h */
/**
 * @file url.h
 * @brief Declaration for the `ada::url` class.
 *
 * This file contains the `ada::url` struct which represents a parsed URL
 * using separate `std::string` instances for each component. This
 * representation is more flexible but uses more memory than `url_aggregator`.
 *
 * @see url_aggregator.h for a more memory-efficient alternative
 */
#ifndef ADA_URL_H
#define ADA_URL_H

#include <algorithm>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>

/* begin file include/ada/url_components.h */
/**
 * @file url_components.h
 * @brief URL component offset representation for url_aggregator.
 *
 * This file defines the `url_components` struct which stores byte offsets
 * into a URL string buffer. It is used internally by `url_aggregator` to
 * efficiently locate URL components without storing separate strings.
 */
#ifndef ADA_URL_COMPONENTS_H
#define ADA_URL_COMPONENTS_H

namespace ada {

/**
 * @brief Stores byte offsets for URL components within a buffer.
 *
 * The `url_components` struct uses 32-bit offsets to track the boundaries
 * of each URL component within a single string buffer. This enables efficient
 * component extraction without additional memory allocations.
 *
 * Component layout in a URL:
 * ```
 * https://user:pass@example.com:1234/foo/bar?baz#quux
 *       |     |    |          | ^^^^|       |   |
 *       |     |    |          | |   |       |   `----- hash_start
 *       |     |    |          | |   |       `--------- search_start
 *       |     |    |          | |   `----------------- pathname_start
 *       |     |    |          | `--------------------- port
 *       |     |    |          `----------------------- host_end
 *       |     |    `---------------------------------- host_start
 *       |     `--------------------------------------- username_end
 *       `--------------------------------------------- protocol_end
 * ```
 *
 * @note The 32-bit offsets limit URLs to 4GB in length.
 * @note A value of `omitted` (UINT32_MAX) indicates the component is not
 * present.
 */
struct url_components {
  /** Sentinel value indicating a component is not present. */
  constexpr static uint32_t omitted = uint32_t(-1);

  url_components() = default;
  url_components(const url_components& u) = default;
  url_components(url_components&& u) noexcept = default;
  url_components& operator=(url_components&& u) noexcept = default;
  url_components& operator=(const url_components& u) = default;
  ~url_components() = default;

  /** Offset of the end of the protocol/scheme (position of ':'). */
  uint32_t protocol_end{0};

  /**
   * Offset of the end of the username.
   * Initialized to 0 (not `omitted`) to simplify username/password getters.
   */
  uint32_t username_end{0};

  /** Offset of the start of the host. */
  uint32_t host_start{0};

  /** Offset of the end of the host. */
  uint32_t host_end{0};

  /** Port number, or `omitted` if no port is specified. */
  uint32_t port{omitted};

  /** Offset of the start of the pathname. */
  uint32_t pathname_start{0};

  /** Offset of the '?' starting the query, or `omitted` if no query. */
  uint32_t search_start{omitted};

  /** Offset of the '#' starting the fragment, or `omitted` if no fragment. */
  uint32_t hash_start{omitted};

  /**
   * Validates that offsets are in ascending order and consistent.
   * Useful for debugging to detect internal corruption.
   * @return `true` if offsets are consistent, `false` otherwise.
   */
  [[nodiscard]] inline bool check_offset_consistency() const noexcept;

  /**
   * Returns a JSON string representation of the offsets for debugging.
   * @return A JSON-formatted string with all offset values.
   */
  [[nodiscard]] std::string to_string() const;

};  // struct url_components
}  // namespace ada
#endif
/* end file include/ada/url_components.h */

namespace ada {

struct url_aggregator;

// namespace parser {
// template <typename result_type>
// result_type parse_url(std::string_view user_input,
//                       const result_type* base_url = nullptr);
// template <typename result_type, bool store_values>
// result_type parse_url_impl(std::string_view user_input,
//                            const result_type* base_url = nullptr);
// }

/**
 * @brief Represents a parsed URL with individual string components.
 *
 * The `url` struct stores each URL component (scheme, username, password,
 * host, port, path, query, fragment) as a separate `std::string`. This
 * provides flexibility but incurs more memory allocations compared to
 * `url_aggregator`.
 *
 * **When to use `ada::url`:**
 * - When you need to frequently modify individual URL components
 * - When you want independent ownership of component strings
 *
 * **When to use `ada::url_aggregator` instead:**
 * - For read-mostly operations on parsed URLs
 * - When memory efficiency is important
 * - When you only need string_view access to components
 *
 * @note This type is returned when parsing with `ada::parse<ada::url>()`.
 *       By default, `ada::parse()` returns `ada::url_aggregator`.
 *
 * @see url_aggregator For a more memory-efficient URL representation
 * @see https://url.spec.whatwg.org/#url-representation
 */
struct url : url_base {
  url() = default;
  url(const url& u) = default;
  url(url&& u) noexcept = default;
  url& operator=(url&& u) noexcept = default;
  url& operator=(const url& u) = default;
  ~url() override = default;

  // Fields are ordered so that the most frequently accessed components
  // tend to occupy earlier cache lines and remain close together in memory.
  //
  // Note: The exact object layout (including cache-line boundaries, byte
  // offsets, and member sizes) is implementation- and platform-dependent.
  // This ordering expresses an intent for better cache locality but does not
  // guarantee any specific in-memory layout.

  /**
   * @private
   * A URL's host is null or a host. It is initially null.
   */
  std::optional<std::string> host{};

  /**
   * @private
   * A URL's path is either an ASCII string or a list of zero or more ASCII
   * strings, usually identifying a location.
   */
  std::string path{};

  /**
   * @private
   * A URL's query is either null or an ASCII string. It is initially null.
   */
  std::optional<std::string> query{};

  /**
   * @private
   * A URL's fragment is either null or an ASCII string that can be used for
   * further processing on the resource the URL's other components identify. It
   * is initially null.
   */
  std::optional<std::string> hash{};

  /**
   * @private
   * A URL's port is either null or a 16-bit unsigned integer that identifies a
   * networking port. It is initially null.
   */
  std::optional<uint16_t> port{};

  /**
   * @private
   * A URL's username is an ASCII string identifying a username. It is initially
   * the empty string.
   */
  std::string username{};

  /**
   * @private
   * A URL's password is an ASCII string identifying a password. It is initially
   * the empty string.
   */
  std::string password{};

  /**
   * Checks if the URL has an empty hostname (host is set but empty string).
   * @return `true` if host exists but is empty, `false` otherwise.
   */
  [[nodiscard]] inline bool has_empty_hostname() const noexcept;

  /**
   * Checks if the URL has a non-default port explicitly specified.
   * @return `true` if a port is present, `false` otherwise.
   */
  [[nodiscard]] inline bool has_port() const noexcept;

  /**
   * Checks if the URL has a hostname (including empty hostnames).
   * @return `true` if host is present, `false` otherwise.
   */
  [[nodiscard]] inline bool has_hostname() const noexcept;

  /**
   * Validates whether the hostname is a valid domain according to RFC 1034.
   * Checks that the domain and its labels have valid lengths (max 255 octets
   * total, max 63 octets per label).
   * @return `true` if the domain is valid, `false` otherwise.
   */
  [[nodiscard]] bool has_valid_domain() const noexcept override;

  /**
   * Returns a JSON string representation of this URL for debugging.
   * @return A JSON-formatted string with all URL components.
   */
  [[nodiscard]] std::string to_string() const override;

  /**
   * Returns the full serialized URL (the href).
   * @return The complete URL string (allocates a new string).
   * @see https://url.spec.whatwg.org/#dom-url-href
   */
  [[nodiscard]] ada_really_inline std::string get_href() const;

  /**
   * Returns the byte length of the serialized URL without allocating a string.
   * @return Size of the href in bytes.
   */
  [[nodiscard]] size_t get_href_size() const noexcept;

  /**
   * Returns the URL's origin as a string (scheme + host + port for special
   * URLs).
   * @return A newly allocated string containing the serialized origin.
   * @see https://url.spec.whatwg.org/#concept-url-origin
   */
  [[nodiscard]] std::string get_origin() const override;

  /**
   * Returns the URL's scheme followed by a colon (e.g., "https:").
   * @return A newly allocated string with the protocol.
   * @see https://url.spec.whatwg.org/#dom-url-protocol
   */
  [[nodiscard]] std::string get_protocol() const;

  /**
   * Returns the URL's host and port (e.g., "example.com:8080").
   * If no port is set, returns just the host. Returns empty string if no host.
   * @return A newly allocated string with host:port.
   * @see https://url.spec.whatwg.org/#dom-url-host
   */
  [[nodiscard]] std::string get_host() const;

  /**
   * Returns the URL's hostname (without port).
   * Returns empty string if no host is set.
   * @return A newly allocated string with the hostname.
   * @see https://url.spec.whatwg.org/#dom-url-hostname
   */
  [[nodiscard]] std::string get_hostname() const;

  /**
   * Returns the URL's path component.
   * @return A string_view pointing to the path.
   * @see https://url.spec.whatwg.org/#dom-url-pathname
   */
  [[nodiscard]] inline std::string_view get_pathname() const noexcept;

  /**
   * Returns the byte length of the pathname without creating a string.
   * @return Size of the pathname in bytes.
   * @see https://url.spec.whatwg.org/#dom-url-pathname
   */
  [[nodiscard]] ada_really_inline size_t get_pathname_length() const noexcept;

  /**
   * Returns the URL's query string prefixed with '?' (e.g., "?foo=bar").
   * Returns empty string if no query is set.
   * @return A newly allocated string with the search/query.
   * @see https://url.spec.whatwg.org/#dom-url-search
   */
  [[nodiscard]] std::string get_search() const;

  /**
   * Returns the URL's username component.
   * @return A constant reference to the username string.
   * @see https://url.spec.whatwg.org/#dom-url-username
   */
  [[nodiscard]] const std::string& get_username() const noexcept;

  /**
   * Sets the URL's username, percent-encoding special characters.
   * @param input The new username value.
   * @return `true` on success, `false` if the URL cannot have credentials.
   * @see https://url.spec.whatwg.org/#dom-url-username
   */
  bool set_username(std::string_view input);

  /**
   * Sets the URL's password, percent-encoding special characters.
   * @param input The new password value.
   * @return `true` on success, `false` if the URL cannot have credentials.
   * @see https://url.spec.whatwg.org/#dom-url-password
   */
  bool set_password(std::string_view input);

  /**
   * Sets the URL's port from a string (e.g., "8080").
   * @param input The port string. Empty string removes the port.
   * @return `true` on success, `false` if the URL cannot have a port.
   * @see https://url.spec.whatwg.org/#dom-url-port
   */
  bool set_port(std::string_view input);

  /**
   * Sets the URL's fragment/hash (the part after '#').
   * @param input The new hash value (with or without leading '#').
   * @see https://url.spec.whatwg.org/#dom-url-hash
   */
  void set_hash(std::string_view input);

  /**
   * Sets the URL's query string (the part after '?').
   * @param input The new query value (with or without leading '?').
   * @see https://url.spec.whatwg.org/#dom-url-search
   */
  void set_search(std::string_view input);

  /**
   * Sets the URL's pathname.
   * @param input The new path value.
   * @return `true` on success, `false` if the URL has an opaque path.
   * @see https://url.spec.whatwg.org/#dom-url-pathname
   */
  bool set_pathname(std::string_view input);

  /**
   * Sets the URL's host (hostname and optionally port).
   * @param input The new host value (e.g., "example.com:8080").
   * @return `true` on success, `false` if parsing fails.
   * @see https://url.spec.whatwg.org/#dom-url-host
   */
  bool set_host(std::string_view input);

  /**
   * Sets the URL's hostname (without port).
   * @param input The new hostname value.
   * @return `true` on success, `false` if parsing fails.
   * @see https://url.spec.whatwg.org/#dom-url-hostname
   */
  bool set_hostname(std::string_view input);

  /**
   * Sets the URL's protocol/scheme.
   * @param input The new protocol (with or without trailing ':').
   * @return `true` on success, `false` if the scheme is invalid.
   * @see https://url.spec.whatwg.org/#dom-url-protocol
   */
  bool set_protocol(std::string_view input);

  /**
   * Replaces the entire URL by parsing a new href string.
   * @param input The new URL string to parse.
   * @return `true` on success, `false` if parsing fails.
   * @see https://url.spec.whatwg.org/#dom-url-href
   */
  bool set_href(std::string_view input);

  /**
   * Returns the URL's password component.
   * @return A constant reference to the password string.
   * @see https://url.spec.whatwg.org/#dom-url-password
   */
  [[nodiscard]] const std::string& get_password() const noexcept;

  /**
   * Returns the URL's port as a string (e.g., "8080").
   * Returns empty string if no port is set.
   * @return A newly allocated string with the port.
   * @see https://url.spec.whatwg.org/#dom-url-port
   */
  [[nodiscard]] std::string get_port() const;

  /**
   * Returns the URL's fragment prefixed with '#' (e.g., "#section").
   * Returns empty string if no fragment is set.
   * @return A newly allocated string with the hash.
   * @see https://url.spec.whatwg.org/#dom-url-hash
   */
  [[nodiscard]] std::string get_hash() const;

  /**
   * Checks if the URL has credentials (non-empty username or password).
   * @return `true` if username or password is non-empty, `false` otherwise.
   */
  [[nodiscard]] ada_really_inline bool has_credentials() const noexcept;

  /**
   * Returns the URL component offsets for efficient serialization.
   *
   * The components represent byte offsets into the serialized URL:
   * ```
   * https://user:pass@example.com:1234/foo/bar?baz#quux
   *       |     |    |          | ^^^^|       |   |
   *       |     |    |          | |   |       |   `----- hash_start
   *       |     |    |          | |   |       `--------- search_start
   *       |     |    |          | |   `----------------- pathname_start
   *       |     |    |          | `--------------------- port
   *       |     |    |          `----------------------- host_end
   *       |     |    `---------------------------------- host_start
   *       |     `--------------------------------------- username_end
   *       `--------------------------------------------- protocol_end
   * ```
   * @return A newly constructed url_components struct.
   * @see https://github.com/servo/rust-url
   */
  [[nodiscard]] ada_really_inline ada::url_components get_components() const;

  /**
   * Checks if the URL has a fragment/hash component.
   * @return `true` if hash is present, `false` otherwise.
   */
  [[nodiscard]] inline bool has_hash() const noexcept override;

  /**
   * Checks if the URL has a query/search component.
   * @return `true` if query is present, `false` otherwise.
   */
  [[nodiscard]] inline bool has_search() const noexcept override;

 private:
  friend ada::url ada::parser::parse_url<ada::url>(std::string_view,
                                                   const ada::url*);
  friend ada::url_aggregator ada::parser::parse_url<ada::url_aggregator>(
      std::string_view, const ada::url_aggregator*);
  friend void ada::helpers::strip_trailing_spaces_from_opaque_path<ada::url>(
      ada::url& url);

  friend ada::url ada::parser::parse_url_impl<ada::url, true>(std::string_view,
                                                              const ada::url*);
  friend ada::url_aggregator ada::parser::parse_url_impl<
      ada::url_aggregator, true>(std::string_view, const ada::url_aggregator*);
  friend ada::url_aggregator ada::parser::parse_url_impl<
      ada::url_aggregator, false>(std::string_view, const ada::url_aggregator*);
  template <class result_type>
  friend bool ada::parser::try_parse_simple_absolute(std::string_view,
                                                     result_type&);

  inline void update_unencoded_base_hash(std::string_view input);
  inline void update_base_hostname(std::string_view input);
  inline void update_base_search(std::string_view input,
                                 const uint8_t query_percent_encode_set[]);
  inline void update_base_search(std::optional<std::string>&& input);
  inline void update_base_pathname(std::string_view input);
  inline void update_base_username(std::string_view input);
  inline void update_base_password(std::string_view input);
  inline void update_base_port(std::optional<uint16_t> input);

  /**
   * Sets the host or hostname according to override condition.
   * Return true on success.
   * @see https://url.spec.whatwg.org/#hostname-state
   */
  template <bool override_hostname = false>
  bool set_host_or_hostname(std::string_view input);

  /**
   * Return true on success.
   * @see https://url.spec.whatwg.org/#concept-ipv4-parser
   */
  [[nodiscard]] bool parse_ipv4(std::string_view input);

  /**
   * Return true on success.
   * @see https://url.spec.whatwg.org/#concept-ipv6-parser
   */
  [[nodiscard]] bool parse_ipv6(std::string_view input);

  /**
   * Return true on success.
   * @see https://url.spec.whatwg.org/#concept-opaque-host-parser
   */
  [[nodiscard]] bool parse_opaque_host(std::string_view input);

  /**
   * A URL's scheme is an ASCII string that identifies the type of URL and can
   * be used to dispatch a URL for further processing after parsing. It is
   * initially the empty string. We only set non_special_scheme when the scheme
   * is non-special, otherwise we avoid constructing string.
   *
   * Special schemes are stored in ada::scheme::details::is_special_list so we
   * typically do not need to store them in each url instance.
   */
  std::string non_special_scheme{};

  /**
   * A URL cannot have a username/password/port if its host is null or the empty
   * string, or its scheme is "file".
   */
  [[nodiscard]] inline bool cannot_have_credentials_or_port() const;

  ada_really_inline size_t parse_port(
      std::string_view view, bool check_trailing_content) noexcept override;

  ada_really_inline size_t parse_port(std::string_view view) noexcept override {
    return this->parse_port(view, false);
  }

  /**
   * Parse the host from the provided input. We assume that
   * the input does not contain spaces or tabs. Control
   * characters and spaces are not trimmed (they should have
   * been removed if needed).
   * Return true on success.
   * @see https://url.spec.whatwg.org/#host-parsing
   */
  [[nodiscard]] ada_really_inline bool parse_host(std::string_view input);

  template <bool has_state_override = false>
  [[nodiscard]] ada_really_inline bool parse_scheme(std::string_view input);

  inline void clear_pathname() override;
  inline void clear_search() override;
  inline void set_protocol_as_file();

  /**
   * Parse the path from the provided input.
   * Return true on success. Control characters not
   * trimmed from the ends (they should have
   * been removed if needed).
   *
   * The input is expected to be UTF-8.
   *
   * @see https://url.spec.whatwg.org/
   */
  ada_really_inline void parse_path(std::string_view input);

  /**
   * Set the scheme for this URL. The provided scheme should be a valid
   * scheme string, be lower-cased, not contain spaces or tabs. It should
   * have no spurious trailing or leading content.
   */
  inline void set_scheme(std::string&& new_scheme) noexcept;

  /**
   * Take the scheme from another URL. The scheme string is moved from the
   * provided url.
   */
  inline void copy_scheme(ada::url&& u);

  /**
   * Take the scheme from another URL. The scheme string is copied from the
   * provided url.
   */
  inline void copy_scheme(const ada::url& u);

};  // struct url

inline std::ostream& operator<<(std::ostream& out, const ada::url& u);
}  // namespace ada

#endif  // ADA_URL_H
/* end file include/ada/url.h */

namespace ada {

/**
 * Result type for URL parsing operations.
 *
 * @tparam result_type The URL type to return (default: `ada::url_aggregator`)
 *
 * @example
 * ```cpp
 * ada::result<ada::url_aggregator> result = ada::parse("https://example.com");
 * if (result) {
 *     // Success: use result.value() or *result
 * } else {
 *     // Error: handle error
 * }
 * ```
 */
template <class result_type = ada::url_aggregator>
using result = std::optional<result_type>;

/**
 * Parses a URL string according to the WHATWG URL Standard.
 *
 * This is the main entry point for URL parsing in Ada. The function takes
 * a string input and optionally a base URL for resolving relative URLs.
 *
 * @tparam result_type The URL type to return. Can be either `ada::url` or
 *         `ada::url_aggregator` (default). The `url_aggregator` type is more
 *         memory-efficient as it stores components as offsets into a single
 *         buffer.
 *
 * @param input The URL string to parse. Must be valid ASCII or UTF-8 encoded.
 *        Leading and trailing whitespace is automatically trimmed.
 * @param base_url Optional pointer to a base URL for resolving relative URLs.
 *        If nullptr (default), only absolute URLs can be parsed successfully.
 *
 * @return A `result<result_type>` containing either the parsed URL on success,
 *         or an error code on failure. Use the boolean conversion or
 *         `has_value()` to check for success.
 *
 * @note The parser is fully compliant with the WHATWG URL Standard.
 *
 * Parsing fails if the input or the resulting normalized URL exceeds
 * `get_max_input_length()` bytes (default ~4 GB, configurable via
 * `set_max_input_length()`). This accounts for percent-encoding expansion:
 * a short input that normalizes into a long URL is still rejected.
 *
 * @example
 * ```cpp
 * // Parse an absolute URL
 * auto url = ada::parse("https://user:pass@example.com:8080/path?query#hash");
 * if (url) {
 *     std::cout << url->get_hostname(); // "example.com"
 *     std::cout << url->get_pathname(); // "/path"
 * }
 *
 * // Parse a relative URL with a base
 * auto base = ada::parse("https://example.com/dir/");
 * if (base) {
 *     auto relative = ada::parse("../other/page", &*base);
 *     if (relative) {
 *         std::cout << relative->get_href(); //
 * "https://example.com/other/page"
 *     }
 * }
 * ```
 *
 * @see https://url.spec.whatwg.org/#url-parsing
 */
template <class result_type = ada::url_aggregator>
ada_warn_unused ada::result<result_type> parse(
    std::string_view input, const result_type* base_url = nullptr);

extern template ada::result<url> parse<url>(std::string_view input,
                                            const url* base_url);
extern template ada::result<url_aggregator> parse<url_aggregator>(
    std::string_view input, const url_aggregator* base_url);

/**
 * Checks whether a URL string can be successfully parsed.
 *
 * Equivalent to `parse(input, base).has_value()` for every input, including
 * when `set_max_input_length` rejects a normalized href that exceeds the limit.
 *
 * @param input The URL string to validate. Must be valid ASCII or UTF-8.
 * @param base_input Optional base URL string for relative inputs.
 * @return `true` if parsing would succeed, `false` otherwise.
 * @see https://url.spec.whatwg.org/#dom-url-canparse
 */
bool can_parse(std::string_view input,
               const std::string_view* base_input = nullptr);

/**
 * Converts a file system path to a file:// URL.
 *
 * Creates a properly formatted file URL from a local file system path.
 * Handles platform-specific path separators and percent-encoding.
 *
 * Respects `get_max_input_length()`: if the input path or the resulting
 * `file://` href would exceed the limit, returns an empty string.
 *
 * @param path The file system path to convert. Must be valid ASCII or UTF-8.
 *
 * @return A file:// URL string representing the given path, or empty on
 *         length-limit rejection.
 */
std::string href_from_file(std::string_view path);

/**
 * Sets the maximum allowed length for URLs.
 *
 * Both the raw input and the resulting normalized URL (the href) are checked
 * against this limit. Parsing or setter calls that would produce a URL
 * exceeding this length are rejected. The same limit also applies to
 * `href_from_file` and to query strings passed to `url_search_params`
 * construction / `reset`. The value must fit in a uint32_t.
 * The default is std::numeric_limits<uint32_t>::max() (approximately 4 GB).
 *
 * @param length The new maximum URL length in bytes.
 */
void set_max_input_length(uint32_t length);

/**
 * Returns the current maximum allowed length for URLs.
 *
 * @return The current maximum URL length in bytes.
 */
uint32_t get_max_input_length();

}  // namespace ada

#endif  // ADA_IMPLEMENTATION_H
/* end file include/ada/implementation.h */

#include <ostream>
#include <string>
#include <string_view>

#if ADA_TESTING
#include <iostream>
#endif  // ADA_TESTING

#endif
/* end file include/ada/url_pattern.h */

#endif  // ADA_PARSER_INL_H
/* end file include/ada/parser-inl.h */
/* begin file include/ada/scheme-inl.h */
/**
 * @file scheme-inl.h
 * @brief Definitions for the URL scheme.
 */
#ifndef ADA_SCHEME_INL_H
#define ADA_SCHEME_INL_H


namespace ada::scheme {

/**
 * @namespace ada::scheme::details
 * @brief Includes the definitions for scheme specific entities
 */
namespace details {
// for use with is_special and get_special_port
// Spaces, if present, are removed from URL.
constexpr std::string_view is_special_list[] = {"http", " ",   "https", "ws",
                                                "ftp",  "wss", "file",  " "};
// for use with get_special_port
constexpr uint16_t special_ports[] = {80, 0, 443, 80, 21, 443, 0, 0};

// @private
// convert a string_view to a 64-bit integer key for fast comparison
constexpr uint64_t make_key(std::string_view sv) {
  uint64_t val = 0;
  for (size_t i = 0; i < sv.size(); i++)
    val |= (uint64_t)(uint8_t)sv[i] << (i * 8);
  return val;
}
// precomputed keys for the special schemes, indexed by a hash of the input
// string
constexpr uint64_t scheme_keys[] = {
    make_key("http"),   // 0: HTTP
    0,                  // 1: sentinel
    make_key("https"),  // 2: HTTPS
    make_key("ws"),     // 3: WS
    make_key("ftp"),    // 4: FTP
    make_key("wss"),    // 5: WSS
    make_key("file"),   // 6: FILE
    0,                  // 7: sentinel
};

// @private
// branchless load of up to 5 characters into a uint64_t, padding with zeros if
// n < 5
inline uint64_t branchless_load5(const char* p, size_t n) {
  uint64_t input = (uint8_t)p[0];
  input |= ((uint64_t)(uint8_t)p[n > 1] << 8) & (0 - (uint64_t)(n > 1));
  input |= ((uint64_t)(uint8_t)p[(n > 2) * 2] << 16) & (0 - (uint64_t)(n > 2));
  input |= ((uint64_t)(uint8_t)p[(n > 3) * 3] << 24) & (0 - (uint64_t)(n > 3));
  input |= ((uint64_t)(uint8_t)p[(n > 4) * 4] << 32) & (0 - (uint64_t)(n > 4));
  return input;
}
}  // namespace details

/****
 * @private
 * In is_special, get_scheme_type, and get_special_port, we
 * use a standard hashing technique to find the index of the scheme in
 * the is_special_list. The hashing technique is based on the size of
 * the scheme and the first character of the scheme. It ensures that we
 * do at most one string comparison per call. If the protocol is
 * predictible (e.g., it is always "http"), we can get a better average
 * performance by using a simpler approach where we loop and compare
 * scheme with all possible protocols starting with the most likely
 * protocol. Doing multiple comparisons may have a poor worst case
 * performance, however. In this instance, we choose a potentially
 * slightly lower best-case performance for a better worst-case
 * performance. We can revisit this choice at any time.
 *
 * Reference:
 * Schmidt, Douglas C. "Gperf: A perfect hash function generator."
 * More C++ gems 17 (2000).
 *
 * Reference: https://en.wikipedia.org/wiki/Perfect_hash_function
 *
 * Reference: https://github.com/ada-url/ada/issues/617
 ****/

ada_really_inline constexpr bool is_special(std::string_view scheme) {
  if (scheme.empty()) {
    return false;
  }
  int hash_value = (2 * scheme.size() + (unsigned)(scheme[0])) & 7;
  const std::string_view target = details::is_special_list[hash_value];
  return (target[0] == scheme[0]) && (target.substr(1) == scheme.substr(1));
}
inline uint16_t get_special_port(std::string_view scheme) noexcept {
  if (scheme.empty()) {
    return 0;
  }
  int hash_value = (2 * scheme.size() + (unsigned)(scheme[0])) & 7;
  const std::string_view target = details::is_special_list[hash_value];
  if (scheme.size() == target.size() &&
      details::branchless_load5(scheme.data(), scheme.size()) ==
          details::scheme_keys[hash_value]) {
    return details::special_ports[hash_value];
  } else {
    return 0;
  }
}
constexpr uint16_t get_special_port(ada::scheme::type type) noexcept {
  return details::special_ports[int(type)];
}
inline ada::scheme::type get_scheme_type(std::string_view scheme) noexcept {
  if (scheme.empty()) {
    return ada::scheme::NOT_SPECIAL;
  }
  int hash_value = (2 * scheme.size() + (unsigned)(scheme[0])) & 7;
  const std::string_view target = details::is_special_list[hash_value];
  if (scheme.size() == target.size() &&
      details::branchless_load5(scheme.data(), scheme.size()) ==
          details::scheme_keys[hash_value]) {
    return ada::scheme::type(hash_value);
  } else {
    return ada::scheme::NOT_SPECIAL;
  }
}

}  // namespace ada::scheme

#endif  // ADA_SCHEME_INL_H
/* end file include/ada/scheme-inl.h */
/* begin file include/ada/serializers.h */
/**
 * @file serializers.h
 * @brief IP address serialization utilities.
 *
 * This header provides functions for converting IP addresses to their
 * string representations according to the WHATWG URL Standard.
 */
#ifndef ADA_SERIALIZERS_H
#define ADA_SERIALIZERS_H


#include <array>
#include <string>

/**
 * @namespace ada::serializers
 * @brief IP address serialization functions.
 *
 * Contains utilities for serializing IPv4 and IPv6 addresses to strings.
 */
namespace ada::serializers {

/**
 * Finds the longest consecutive sequence of zero pieces in an IPv6 address.
 * Used for :: compression in IPv6 serialization.
 *
 * @param address The 8 16-bit pieces of the IPv6 address.
 * @param[out] compress Index of the start of the longest zero sequence.
 * @param[out] compress_length Length of the longest zero sequence.
 */
void find_longest_sequence_of_ipv6_pieces(
    const std::array<uint16_t, 8>& address, size_t& compress,
    size_t& compress_length) noexcept;

/**
 * Serializes an IPv6 address to its string representation.
 *
 * @param address The 8 16-bit pieces of the IPv6 address.
 * @return The serialized IPv6 string (e.g., "2001:db8::1").
 * @see https://url.spec.whatwg.org/#concept-ipv6-serializer
 */
std::string ipv6(const std::array<uint16_t, 8>& address);

/**
 * Serializes an IPv4 address to its dotted-decimal string representation.
 *
 * @param address The 32-bit IPv4 address as an integer.
 * @return The serialized IPv4 string (e.g., "192.168.1.1").
 * @see https://url.spec.whatwg.org/#concept-ipv4-serializer
 */
std::string ipv4(uint64_t address);

}  // namespace ada::serializers

#endif  // ADA_SERIALIZERS_H
/* end file include/ada/serializers.h */
/* begin file include/ada/state.h */
/**
 * @file state.h
 * @brief URL parser state machine states.
 *
 * Defines the states used by the URL parsing state machine as specified
 * in the WHATWG URL Standard.
 *
 * @see https://url.spec.whatwg.org/#url-parsing
 */
#ifndef ADA_STATE_H
#define ADA_STATE_H


#include <string>

namespace ada {

/**
 * @brief States in the URL parsing state machine.
 *
 * The URL parser processes input through a sequence of states, each handling
 * a specific part of the URL syntax.
 *
 * @see https://url.spec.whatwg.org/#url-parsing
 */
enum class state {
  /**
   * @see https://url.spec.whatwg.org/#authority-state
   */
  AUTHORITY,

  /**
   * @see https://url.spec.whatwg.org/#scheme-start-state
   */
  SCHEME_START,

  /**
   * @see https://url.spec.whatwg.org/#scheme-state
   */
  SCHEME,

  /**
   * @see https://url.spec.whatwg.org/#host-state
   */
  HOST,

  /**
   * @see https://url.spec.whatwg.org/#no-scheme-state
   */
  NO_SCHEME,

  /**
   * @see https://url.spec.whatwg.org/#fragment-state
   */
  FRAGMENT,

  /**
   * @see https://url.spec.whatwg.org/#relative-state
   */
  RELATIVE_SCHEME,

  /**
   * @see https://url.spec.whatwg.org/#relative-slash-state
   */
  RELATIVE_SLASH,

  /**
   * @see https://url.spec.whatwg.org/#file-state
   */
  FILE,

  /**
   * @see https://url.spec.whatwg.org/#file-host-state
   */
  FILE_HOST,

  /**
   * @see https://url.spec.whatwg.org/#file-slash-state
   */
  FILE_SLASH,

  /**
   * @see https://url.spec.whatwg.org/#path-or-authority-state
   */
  PATH_OR_AUTHORITY,

  /**
   * @see https://url.spec.whatwg.org/#special-authority-ignore-slashes-state
   */
  SPECIAL_AUTHORITY_IGNORE_SLASHES,

  /**
   * @see https://url.spec.whatwg.org/#special-authority-slashes-state
   */
  SPECIAL_AUTHORITY_SLASHES,

  /**
   * @see https://url.spec.whatwg.org/#special-relative-or-authority-state
   */
  SPECIAL_RELATIVE_OR_AUTHORITY,

  /**
   * @see https://url.spec.whatwg.org/#query-state
   */
  QUERY,

  /**
   * @see https://url.spec.whatwg.org/#path-state
   */
  PATH,

  /**
   * @see https://url.spec.whatwg.org/#path-start-state
   */
  PATH_START,

  /**
   * @see https://url.spec.whatwg.org/#cannot-be-a-base-url-path-state
   */
  OPAQUE_PATH,

  /**
   * @see https://url.spec.whatwg.org/#port-state
   */
  PORT,
};

/**
 * Converts a parser state to its string name for debugging.
 * @param s The state to convert.
 * @return A string representation of the state.
 */
ada_warn_unused std::string to_string(ada::state s);

}  // namespace ada

#endif  // ADA_STATE_H
/* end file include/ada/state.h */
/* begin file include/ada/unicode.h */
/**
 * @file unicode.h
 * @brief Definitions for all unicode specific functions.
 */
#ifndef ADA_UNICODE_H
#define ADA_UNICODE_H


#include <string>
#include <string_view>
#include <optional>

/**
 * Unicode operations. These functions are not part of our public API and may
 * change at any time.
 *
 * @private
 * @namespace ada::unicode
 * @brief Includes the definitions for unicode operations
 */
namespace ada::unicode {

/**
 * @private
 * We receive a UTF-8 string representing a domain name.
 * If the string is percent encoded, we apply percent decoding.
 *
 * Given a domain, we need to identify its labels.
 * They are separated by label-separators:
 *
 * U+002E (.) FULL STOP
 * U+FF0E FULLWIDTH FULL STOP
 * U+3002 IDEOGRAPHIC FULL STOP
 * U+FF61 HALFWIDTH IDEOGRAPHIC FULL STOP
 *
 * They are all mapped to U+002E.
 *
 * We process each label into a string that should not exceed 63 octets.
 * If the string is already punycode (starts with "xn--"), then we must
 * scan it to look for unallowed code points.
 * Otherwise, if the string is not pure ASCII, we need to transcode it
 * to punycode by following RFC 3454 which requires us to
 * - Map characters  (see section 3),
 * - Normalize (see section 4),
 * - Reject forbidden characters,
 * - Check for right-to-left characters and if so, check all requirements (see
 * section 6),
 * - Optionally reject based on unassigned code points (section 7).
 *
 * The Unicode standard provides a table of code points with a mapping, a list
 * of forbidden code points and so forth. This table is subject to change and
 * will vary based on the implementation. For Unicode 15, the table is at
 * https://www.unicode.org/Public/idna/15.0.0/IdnaMappingTable.txt
 * If you use ICU, they parse this table and map it to code using a Python
 * script.
 *
 * The resulting strings should not exceed 255 octets according to RFC 1035
 * section 2.3.4. ICU checks for label size and domain size, but these errors
 * are ignored.
 *
 * @see https://url.spec.whatwg.org/#concept-domain-to-ascii
 *
 */
bool to_ascii(std::optional<std::string>& out, std::string_view plain,
              size_t first_percent);

/**
 * @private
 * Checks if the input has tab or newline characters.
 *
 * @attention The has_tabs_or_newline function is a bottleneck and it is simple
 * enough that compilers like GCC can 'autovectorize it'.
 */
ada_really_inline bool has_tabs_or_newline(
    std::string_view user_input) noexcept;

/**
 * @private
 * Checks if the input is a forbidden host code point.
 * @see https://url.spec.whatwg.org/#forbidden-host-code-point
 */
ada_really_inline constexpr bool is_forbidden_host_code_point(char c) noexcept;

/**
 * @private
 * Checks if the input contains a forbidden domain code point.
 * @see https://url.spec.whatwg.org/#forbidden-domain-code-point
 */
ada_really_inline constexpr bool contains_forbidden_domain_code_point(
    const char* input, size_t length) noexcept;

/**
 * @private
 * Checks if the input contains a forbidden domain code point in which case
 * the first bit is set to 1. If the input contains an upper case ASCII letter,
 * then the second bit is set to 1.
 * @see https://url.spec.whatwg.org/#forbidden-domain-code-point
 */
ada_really_inline constexpr uint8_t
contains_forbidden_domain_code_point_or_upper(const char* input,
                                              size_t length) noexcept;

/**
 * @private
 * Checks if the input is a forbidden domain code point.
 * @see https://url.spec.whatwg.org/#forbidden-domain-code-point
 */
ada_really_inline constexpr bool is_forbidden_domain_code_point(
    char c) noexcept;

/**
 * @private
 * Checks if the input is alphanumeric, '+', '-' or '.'
 */
ada_really_inline constexpr bool is_alnum_plus(char c) noexcept;

/**
 * @private
 * @details An ASCII hex digit is an ASCII upper hex digit or ASCII lower hex
 * digit. An ASCII upper hex digit is an ASCII digit or a code point in the
 * range U+0041 (A) to U+0046 (F), inclusive. An ASCII lower hex digit is an
 * ASCII digit or a code point in the range U+0061 (a) to U+0066 (f), inclusive.
 */
ada_really_inline constexpr bool is_ascii_hex_digit(char c) noexcept;

/**
 * @private
 * An ASCII digit is a code point in the range U+0030 (0) to U+0039 (9),
 * inclusive.
 */
ada_really_inline constexpr bool is_ascii_digit(char c) noexcept;

/**
 * @private
 * @details If a char is between U+0000 and U+007F inclusive, then it's an ASCII
 * character.
 */
ada_really_inline constexpr bool is_ascii(char32_t c) noexcept;

/**
 * @private
 * Checks if the input is a C0 control or space character.
 *
 * @details A C0 control or space is a C0 control or U+0020 SPACE.
 * A C0 control is a code point in the range U+0000 NULL to U+001F INFORMATION
 * SEPARATOR ONE, inclusive.
 */
ada_really_inline constexpr bool is_c0_control_or_space(char c) noexcept;

/**
 * @private
 * Checks if the input is a ASCII tab or newline character.
 *
 * @details An ASCII tab or newline is U+0009 TAB, U+000A LF, or U+000D CR.
 */
ada_really_inline constexpr bool is_ascii_tab_or_newline(char c) noexcept;

/**
 * @private
 * @details A double-dot path segment must be ".." or an ASCII case-insensitive
 * match for ".%2e", "%2e.", or "%2e%2e".
 */
ada_really_inline bool is_double_dot_path_segment(
    std::string_view input) noexcept;

/**
 * @private
 * @details A single-dot path segment must be "." or an ASCII case-insensitive
 * match for "%2e".
 */
ada_really_inline constexpr bool is_single_dot_path_segment(
    std::string_view input) noexcept;

/**
 * @private
 * @details ipv4 character might contain 0-9 or a-f character ranges.
 */
ada_really_inline constexpr bool is_lowercase_hex(char c) noexcept;

/**
 * @private
 * @details Convert hex to binary. Caller is responsible to ensure that
 * the parameter is an hexadecimal digit (0-9, A-F, a-f).
 */
ada_really_inline unsigned constexpr convert_hex_to_binary(char c) noexcept;

/**
 * @private
 * first_percent should be  = input.find('%')
 *
 * @todo It would be faster as noexcept maybe, but it could be unsafe since.
 * @author Node.js
 * @see https://github.com/nodejs/node/blob/main/src/node_url.cc#L245
 * @see https://encoding.spec.whatwg.org/#utf-8-decode-without-bom
 */
std::string percent_decode(std::string_view input, size_t first_percent);

/**
 * Decode an application/x-www-form-urlencoded component: map '+' to space,
 * then percent-decode. Single allocation; no intermediate string.
 *
 * @param input A form-urlencoded component (key or value).
 * @return The decoded string.
 * @see https://url.spec.whatwg.org/#concept-urlencoded-parser
 */
std::string form_urlencoded_decode(std::string_view input);

/**
 * @private
 * Returns a percent-encoding string whether percent encoding was needed or not.
 * @see https://github.com/nodejs/node/blob/main/src/node_url.cc#L226
 */
std::string percent_encode(std::string_view input,
                           const uint8_t character_set[]);
/**
 * @private
 * Returns a percent-encoded string version of input, while starting the percent
 * encoding at the provided index.
 * @see https://github.com/nodejs/node/blob/main/src/node_url.cc#L226
 */
std::string percent_encode(std::string_view input,
                           const uint8_t character_set[], size_t index);
/**
 * @private
 * Returns true if percent encoding was needed, in which case, we store
 * the percent-encoded content in 'out'. If the boolean 'append' is set to
 * true, the content is appended to 'out'.
 * If percent encoding is not needed, out is left unchanged.
 * @see https://github.com/nodejs/node/blob/main/src/node_url.cc#L226
 */
template <bool append>
bool percent_encode(std::string_view input, const uint8_t character_set[],
                    std::string& out);
/**
 * @private
 * Returns the index at which percent encoding should start, or (equivalently),
 * the length of the prefix that does not require percent encoding.
 */
ada_really_inline size_t percent_encode_index(std::string_view input,
                                              const uint8_t character_set[]);
/**
 * @private
 * Lowers the string in-place, assuming that the content is ASCII.
 * Return true if the content was ASCII.
 */
inline bool to_lower_ascii(char* input, size_t length) noexcept;
}  // namespace ada::unicode

#endif  // ADA_UNICODE_H
/* end file include/ada/unicode.h */
/* begin file include/ada/url_base-inl.h */
/**
 * @file url_base-inl.h
 * @brief Inline functions for url base
 */
#ifndef ADA_URL_BASE_INL_H
#define ADA_URL_BASE_INL_H


#include <string>
#if ADA_REGULAR_VISUAL_STUDIO
#include <intrin.h>
#endif  // ADA_REGULAR_VISUAL_STUDIO

namespace ada {

[[nodiscard]] ada_really_inline bool url_base::is_special()
    const noexcept {
  return type != ada::scheme::NOT_SPECIAL;
}

[[nodiscard]] inline uint16_t url_base::get_special_port() const noexcept {
  return ada::scheme::get_special_port(type);
}

[[nodiscard]] ada_really_inline uint16_t
url_base::scheme_default_port() const noexcept {
  return scheme::get_special_port(type);
}

}  // namespace ada

#endif  // ADA_URL_BASE_INL_H
/* end file include/ada/url_base-inl.h */
/* begin file include/ada/url-inl.h */
/**
 * @file url-inl.h
 * @brief Definitions for the URL
 */
#ifndef ADA_URL_INL_H
#define ADA_URL_INL_H


#include <charconv>
#include <cstring>
#include <optional>
#include <string>
#if ADA_REGULAR_VISUAL_STUDIO
#include <intrin.h>
#endif  // ADA_REGULAR_VISUAL_STUDIO

namespace ada {
[[nodiscard]] ada_really_inline bool url::has_credentials() const noexcept {
  return !username.empty() || !password.empty();
}
[[nodiscard]] ada_really_inline bool url::has_port() const noexcept {
  return port.has_value();
}
[[nodiscard]] inline bool url::cannot_have_credentials_or_port() const {
  return !host.has_value() || host->empty() || type == ada::scheme::type::FILE;
}
[[nodiscard]] inline bool url::has_empty_hostname() const noexcept {
  if (!host.has_value()) {
    return false;
  }
  return host->empty();
}
[[nodiscard]] inline bool url::has_hostname() const noexcept {
  return host.has_value();
}
inline std::ostream& operator<<(std::ostream& out, const ada::url& u) {
  return out << u.to_string();
}

[[nodiscard]] size_t url::get_pathname_length() const noexcept {
  return path.size();
}

[[nodiscard]] inline std::string_view url::get_pathname() const noexcept {
  return path;
}

[[nodiscard]] ada_really_inline ada::url_components url::get_components()
    const {
  url_components out{};

  // protocol ends with ':'. for example: "https:"
  out.protocol_end = uint32_t(get_protocol().size());

  // Trailing index is always the next character of the current one.
  // NOLINTNEXTLINE(clang-analyzer-deadcode.DeadStores)
  size_t running_index = out.protocol_end;

  if (host.has_value()) {
    // 2 characters for "//" and 1 character for starting index
    out.host_start = out.protocol_end + 2;

    if (has_credentials()) {
      out.username_end = uint32_t(out.host_start + username.size());

      out.host_start += uint32_t(username.size());

      if (!password.empty()) {
        out.host_start += uint32_t(password.size() + 1);
      }

      out.host_end = uint32_t(out.host_start + host->size());
    } else {
      out.username_end = out.host_start;

      // Host does not start with "@" if it does not include credentials.
      out.host_end = uint32_t(out.host_start + host->size()) - 1;
    }

    running_index = out.host_end + 1;
  } else {
    // Update host start and end date to the same index, since it does not
    // exist.
    out.host_start = out.protocol_end;
    out.host_end = out.host_start;

    if (!has_opaque_path && path.starts_with("//")) {
      // If url's host is null, url does not have an opaque path, url's path's
      // size is greater than 1, and url's path[0] is the empty string, then
      // append U+002F (/) followed by U+002E (.) to output.
      running_index = out.protocol_end + 2;
    } else {
      running_index = out.protocol_end;
    }
  }

  if (port.has_value()) {
    out.port = *port;
    running_index += helpers::fast_digit_count(*port) + 1;  // Port omits ':'
  }

  out.pathname_start = uint32_t(running_index);

  running_index += path.size();

  if (query.has_value()) {
    out.search_start = uint32_t(running_index);
    running_index += get_search().size();
    if (get_search().empty()) {
      running_index++;
    }
  }

  if (hash.has_value()) {
    out.hash_start = uint32_t(running_index);
  }

  return out;
}

inline void url::update_base_hostname(std::string_view input) { host = input; }

inline void url::update_unencoded_base_hash(std::string_view input) {
  // We do the percent encoding
  hash = unicode::percent_encode(input,
                                 ada::character_sets::FRAGMENT_PERCENT_ENCODE);
}

inline void url::update_base_search(std::string_view input,
                                    const uint8_t query_percent_encode_set[]) {
  query = ada::unicode::percent_encode(input, query_percent_encode_set);
}

inline void url::update_base_search(std::optional<std::string>&& input) {
  query = std::move(input);
}

inline void url::update_base_pathname(const std::string_view input) {
  path = input;
}

inline void url::update_base_username(const std::string_view input) {
  username = input;
}

inline void url::update_base_password(const std::string_view input) {
  password = input;
}

inline void url::update_base_port(std::optional<uint16_t> input) {
  port = input;
}

inline void url::clear_pathname() { path.clear(); }

inline void url::clear_search() { query = std::nullopt; }

[[nodiscard]] inline bool url::has_hash() const noexcept {
  return hash.has_value();
}

[[nodiscard]] inline bool url::has_search() const noexcept {
  return query.has_value();
}

inline void url::set_protocol_as_file() { type = ada::scheme::type::FILE; }

inline void url::set_scheme(std::string&& new_scheme) noexcept {
  type = ada::scheme::get_scheme_type(new_scheme);
  // We only move the 'scheme' if it is non-special.
  if (!is_special()) {
    non_special_scheme = std::move(new_scheme);
  }
}

inline void url::copy_scheme(ada::url&& u) {
  non_special_scheme = u.non_special_scheme;
  type = u.type;
}

inline void url::copy_scheme(const ada::url& u) {
  non_special_scheme = u.non_special_scheme;
  type = u.type;
}

[[nodiscard]] ada_really_inline std::string url::get_href() const {
  if (is_special() && host.has_value() && username.empty() &&
      password.empty() && !port.has_value()) [[likely]] {
    const std::string_view scheme = ada::scheme::details::is_special_list[type];
    const size_t host_size = host->size();
    const size_t path_size = path.size();
    const size_t query_size = query.has_value() ? query->size() : 0;
    const size_t hash_size = hash.has_value() ? hash->size() : 0;
    const size_t total = scheme.size() + 3 + host_size + path_size +
                         (query.has_value() ? query_size + 1 : 0) +
                         (hash.has_value() ? hash_size + 1 : 0);
    std::string output(total, '\0');
    char* p = output.data();
    std::memcpy(p, scheme.data(), scheme.size());
    p += scheme.size();
    p[0] = ':';
    p[1] = '/';
    p[2] = '/';
    p += 3;
    // NOLINTNEXTLINE(bugprone-not-null-terminated-result)
    std::memcpy(p, host->data(), host_size);
    p += host_size;
    std::memcpy(p, path.data(), path_size);
    p += path_size;
    if (query.has_value()) {
      *p++ = '?';
      // NOLINTNEXTLINE(bugprone-not-null-terminated-result)
      std::memcpy(p, query->data(), query_size);
      p += query_size;
    }
    if (hash.has_value()) {
      *p++ = '#';
      // NOLINTNEXTLINE(bugprone-not-null-terminated-result)
      std::memcpy(p, hash->data(), hash_size);
    }
    return output;
  }

  std::string output;
  output.reserve(get_href_size());

  if (is_special()) {
    output.append(ada::scheme::details::is_special_list[type]);
    output += ':';
  } else {
    output.append(non_special_scheme);
    output += ':';
  }

  if (host.has_value()) {
    output += '/';
    output += '/';
    if (has_credentials()) {
      output.append(username);
      if (!password.empty()) {
        output += ':';
        output.append(password);
      }
      output += '@';
    }
    output.append(*host);
    if (port.has_value()) {
      output += ':';
      char port_buf[5];
      auto [ptr, ec] = std::to_chars(port_buf, port_buf + 5, *port);
      (void)ec;
      output.append(port_buf, static_cast<size_t>(ptr - port_buf));
    }
  } else if (!has_opaque_path && path.starts_with("//")) {
    // If url's host is null, url does not have an opaque path, url's path's
    // size is greater than 1, and url's path[0] is the empty string, then
    // append U+002F (/) followed by U+002E (.) to output.
    output += '/';
    output += '.';
  }
  output.append(path);
  if (query.has_value()) {
    output += '?';
    output.append(*query);
  }
  if (hash.has_value()) {
    output += '#';
    output.append(*hash);
  }
  return output;
}

[[nodiscard]] inline size_t url::get_href_size() const noexcept {
  size_t size = 0;
  if (is_special()) {
    size += ada::scheme::details::is_special_list[type].size() + 1;
  } else {
    size += non_special_scheme.size() + 1;
  }
  if (host.has_value()) {
    size += host->size();
    size += 2;
    if (has_credentials()) {
      size += username.size();
      if (!password.empty()) {
        size += 1 + password.size();
      }
      size += 1;
    }
    if (port.has_value()) {
      size += 1;
      uint16_t p = *port;
      size += (p >= 10000)  ? 5
              : (p >= 1000) ? 4
              : (p >= 100)  ? 3
              : (p >= 10)   ? 2
                            : 1;
    }
  } else if (!has_opaque_path && path.starts_with("//")) {
    size += 2;
  }
  size += path.size();
  if (query.has_value()) {
    size += 1 + query->size();
  }
  if (hash.has_value()) {
    size += 1 + hash->size();
  }
  return size;
}

ada_really_inline size_t url::parse_port(std::string_view view,
                                         bool check_trailing_content) noexcept {
  ada_log("parse_port('", view, "') ", view.size());
  if (!view.empty() && view[0] == '-') {
    ada_log("parse_port: view[0] == '0' && view.size() > 1");
    is_valid = false;
    return 0;
  }
  uint16_t parsed_port{};
  auto r = std::from_chars(view.data(), view.data() + view.size(), parsed_port);
  if (r.ec == std::errc::result_out_of_range) {
    ada_log("parse_port: r.ec == std::errc::result_out_of_range");
    is_valid = false;
    return 0;
  }
  ada_log("parse_port: ", parsed_port);
  const auto consumed = size_t(r.ptr - view.data());
  ada_log("parse_port: consumed ", consumed);
  if (check_trailing_content) {
    is_valid &=
        (consumed == view.size() || view[consumed] == '/' ||
         view[consumed] == '?' || (is_special() && view[consumed] == '\\'));
  }
  ada_log("parse_port: is_valid = ", is_valid);
  if (is_valid) {
    // scheme_default_port can return 0, and we should allow 0 as a base port.
    auto default_port = scheme_default_port();
    bool is_port_valid = (default_port == 0 && parsed_port == 0) ||
                         (default_port != parsed_port);
    port = (r.ec == std::errc() && is_port_valid) ? std::optional(parsed_port)
                                                  : std::nullopt;
  }
  return consumed;
}

}  // namespace ada

#endif  // ADA_URL_H
/* end file include/ada/url-inl.h */
/* begin file include/ada/url_components-inl.h */
/**
 * @file url_components.h
 * @brief Declaration for the URL Components
 */
#ifndef ADA_URL_COMPONENTS_INL_H
#define ADA_URL_COMPONENTS_INL_H


namespace ada {

[[nodiscard]] inline bool url_components::check_offset_consistency()
    const noexcept {
  /**
   * https://user:pass@example.com:1234/foo/bar?baz#quux
   *       |     |    |          | ^^^^|       |   |
   *       |     |    |          | |   |       |   `----- hash_start
   *       |     |    |          | |   |       `--------- search_start
   *       |     |    |          | |   `----------------- pathname_start
   *       |     |    |          | `--------------------- port
   *       |     |    |          `----------------------- host_end
   *       |     |    `---------------------------------- host_start
   *       |     `--------------------------------------- username_end
   *       `--------------------------------------------- protocol_end
   */
  // These conditions can be made more strict.
  if (protocol_end == url_components::omitted) {
    return false;
  }
  uint32_t index = protocol_end;

  if (username_end == url_components::omitted) {
    return false;
  }
  if (username_end < index) {
    return false;
  }
  index = username_end;

  if (host_start == url_components::omitted) {
    return false;
  }
  if (host_start < index) {
    return false;
  }
  index = host_start;

  if (port != url_components::omitted) {
    if (port > 0xffff) {
      return false;
    }
    uint32_t port_length = helpers::fast_digit_count(port) + 1;
    if (index + port_length < index) {
      return false;
    }
    index += port_length;
  }

  if (pathname_start == url_components::omitted) {
    return false;
  }
  if (pathname_start < index) {
    return false;
  }
  index = pathname_start;

  if (search_start != url_components::omitted) {
    if (search_start < index) {
      return false;
    }
    index = search_start;
  }

  if (hash_start != url_components::omitted) {
    if (hash_start < index) {
      return false;
    }
  }

  return true;
}

}  // namespace ada
#endif
/* end file include/ada/url_components-inl.h */
/* begin file include/ada/url_aggregator.h */
/**
 * @file url_aggregator.h
 * @brief Declaration for the `ada::url_aggregator` class.
 *
 * This file contains the `ada::url_aggregator` struct which represents a parsed
 * URL using a single buffer with component offsets. This is the default and
 * most memory-efficient URL representation in Ada.
 *
 * @see url.h for an alternative representation using separate strings
 */
#ifndef ADA_URL_AGGREGATOR_H
#define ADA_URL_AGGREGATOR_H

#include <ostream>
#include <string>
#include <string_view>


namespace ada {

namespace parser {}

/**
 * @brief Memory-efficient URL representation using a single buffer.
 *
 * The `url_aggregator` stores the entire normalized URL in a single string
 * buffer and tracks component boundaries using offsets. This design minimizes
 * memory allocations and is ideal for read-mostly access patterns.
 *
 * Getter methods return `std::string_view` pointing into the internal buffer.
 * These views are lightweight (no allocation) but become invalid if the
 * url_aggregator is modified or destroyed.
 *
 * @warning Views returned by getters (e.g., `get_pathname()`) are invalidated
 * when any setter is called. Do not use a getter's result as input to a
 * setter on the same object without copying first.
 *
 * @note This is the default URL type returned by `ada::parse()`.
 *
 * @see url For an alternative using separate std::string instances
 */
struct url_aggregator : url_base {
  url_aggregator() = default;
  url_aggregator(const url_aggregator& u) = default;
  url_aggregator(url_aggregator&& u) noexcept = default;
  url_aggregator& operator=(url_aggregator&& u) noexcept = default;
  url_aggregator& operator=(const url_aggregator& u) = default;
  ~url_aggregator() override = default;

  /**
   * The setter functions follow the steps defined in the URL Standard.
   *
   * The url_aggregator has a single buffer that contains the entire normalized
   * URL. The various components are represented as offsets into that buffer.
   * When you call get_pathname(), for example, you get a std::string_view that
   * points into that buffer. If the url_aggregator is modified, the buffer may
   * be reallocated, and the std::string_view you obtained earlier may become
   * invalid. In particular, this implies that you cannot modify the URL using
   * a setter function with a std::string_view that points into the
   * url_aggregator E.g., the following is incorrect:
   * url->set_hostname(url->get_pathname()).
   * You must first copy the pathname to a separate string.
   * std::string pathname(url->get_pathname());
   * url->set_hostname(pathname);
   *
   * The caller is responsible for ensuring that the url_aggregator is not
   * modified while any std::string_view obtained from it is in use.
   */
  bool set_href(std::string_view input);
  bool set_host(std::string_view input);
  bool set_hostname(std::string_view input);
  bool set_protocol(std::string_view input);
  bool set_username(std::string_view input);
  bool set_password(std::string_view input);
  bool set_port(std::string_view input);
  bool set_pathname(std::string_view input);
  void set_search(std::string_view input);
  void set_hash(std::string_view input);

  /**
   * Validates whether the hostname is a valid domain according to RFC 1034.
   * @return `true` if the domain is valid, `false` otherwise.
   */
  [[nodiscard]] bool has_valid_domain() const noexcept override;

  /**
   * Returns the URL's origin (scheme + host + port for special URLs).
   * @return A newly allocated string containing the serialized origin.
   * @see https://url.spec.whatwg.org/#concept-url-origin
   */
  [[nodiscard]] std::string get_origin() const override;

  [[nodiscard]] const std::string& get_buffer() const { return buffer; }

  /**
   * Returns the full serialized URL (the href) as a string_view.
   * Does not allocate memory. The returned view becomes invalid if this
   * url_aggregator is modified or destroyed.
   * @return A string_view into the internal buffer.
   * @see https://url.spec.whatwg.org/#dom-url-href
   */
  [[nodiscard]] inline std::string_view get_href() const noexcept
      ada_lifetime_bound;

  /**
   * Returns the byte length of the serialized URL without allocating a string.
   * @return Size of the href in bytes.
   */
  [[nodiscard]] inline size_t get_href_size() const noexcept;

  /**
   * Returns the URL's username component.
   * Does not allocate memory. The returned view becomes invalid if this
   * url_aggregator is modified or destroyed.
   * @return A string_view of the username.
   * @see https://url.spec.whatwg.org/#dom-url-username
   */
  [[nodiscard]] std::string_view get_username() const ada_lifetime_bound;

  /**
   * Returns the URL's password component.
   * Does not allocate memory. The returned view becomes invalid if this
   * url_aggregator is modified or destroyed.
   * @return A string_view of the password.
   * @see https://url.spec.whatwg.org/#dom-url-password
   */
  [[nodiscard]] std::string_view get_password() const ada_lifetime_bound;

  /**
   * Returns the URL's port as a string (e.g., "8080").
   * Does not allocate memory. Returns empty view if no port is set.
   * The returned view becomes invalid if this url_aggregator is modified.
   * @return A string_view of the port.
   * @see https://url.spec.whatwg.org/#dom-url-port
   */
  [[nodiscard]] std::string_view get_port() const ada_lifetime_bound;

  /**
   * Returns the URL's fragment prefixed with '#' (e.g., "#section").
   * Does not allocate memory. Returns empty view if no fragment is set.
   * The returned view becomes invalid if this url_aggregator is modified.
   * @return A string_view of the hash.
   * @see https://url.spec.whatwg.org/#dom-url-hash
   */
  [[nodiscard]] std::string_view get_hash() const ada_lifetime_bound;

  /**
   * Returns the URL's host and port (e.g., "example.com:8080").
   * Does not allocate memory. Returns empty view if no host is set.
   * The returned view becomes invalid if this url_aggregator is modified.
   * @return A string_view of host:port.
   * @see https://url.spec.whatwg.org/#dom-url-host
   */
  [[nodiscard]] std::string_view get_host() const ada_lifetime_bound;

  /**
   * Returns the URL's hostname (without port).
   * Does not allocate memory. Returns empty view if no host is set.
   * The returned view becomes invalid if this url_aggregator is modified.
   * @return A string_view of the hostname.
   * @see https://url.spec.whatwg.org/#dom-url-hostname
   */
  [[nodiscard]] std::string_view get_hostname() const ada_lifetime_bound;

  /**
   * Returns the URL's path component.
   * Does not allocate memory. The returned view becomes invalid if this
   * url_aggregator is modified or destroyed.
   * @return A string_view of the pathname.
   * @see https://url.spec.whatwg.org/#dom-url-pathname
   */
  [[nodiscard]] inline std::string_view get_pathname() const
      ada_lifetime_bound;

  /**
   * Returns the byte length of the pathname without creating a string.
   * @return Size of the pathname in bytes.
   * @see https://url.spec.whatwg.org/#dom-url-pathname
   */
  [[nodiscard]] ada_really_inline uint32_t get_pathname_length() const noexcept;

  /**
   * Returns the URL's query string prefixed with '?' (e.g., "?foo=bar").
   * Does not allocate memory. Returns empty view if no query is set.
   * The returned view becomes invalid if this url_aggregator is modified.
   * @return A string_view of the search/query.
   * @see https://url.spec.whatwg.org/#dom-url-search
   */
  [[nodiscard]] std::string_view get_search() const ada_lifetime_bound;

  /**
   * Returns the URL's scheme followed by a colon (e.g., "https:").
   * Does not allocate memory. The returned view becomes invalid if this
   * url_aggregator is modified or destroyed.
   * @return A string_view of the protocol.
   * @see https://url.spec.whatwg.org/#dom-url-protocol
   */
  [[nodiscard]] std::string_view get_protocol() const ada_lifetime_bound;

  /**
   * Checks if the URL has credentials (non-empty username or password).
   * @return `true` if username or password is non-empty, `false` otherwise.
   */
  [[nodiscard]] ada_really_inline bool has_credentials()
      const noexcept;

  /**
   * Returns the URL component offsets for efficient serialization.
   *
   * The components represent byte offsets into the serialized URL:
   * ```
   * https://user:pass@example.com:1234/foo/bar?baz#quux
   *       |     |    |          | ^^^^|       |   |
   *       |     |    |          | |   |       |   `----- hash_start
   *       |     |    |          | |   |       `--------- search_start
   *       |     |    |          | |   `----------------- pathname_start
   *       |     |    |          | `--------------------- port
   *       |     |    |          `----------------------- host_end
   *       |     |    `---------------------------------- host_start
   *       |     `--------------------------------------- username_end
   *       `--------------------------------------------- protocol_end
   * ```
   * @return A constant reference to the url_components struct.
   * @see https://github.com/servo/rust-url
   */
  [[nodiscard]] ada_really_inline const url_components& get_components()
      const noexcept;

  /**
   * Returns a JSON string representation of this URL for debugging.
   * @return A JSON-formatted string with all URL components.
   */
  [[nodiscard]] std::string to_string() const override;

  /**
   * Returns a visual diagram showing component boundaries in the URL.
   * Useful for debugging and understanding URL structure.
   * @return A multi-line string diagram.
   */
  [[nodiscard]] std::string to_diagram() const;

  /**
   * Validates internal consistency of component offsets (for debugging).
   * @return `true` if offsets are consistent, `false` if corrupted.
   */
  [[nodiscard]] inline bool validate() const noexcept;

  /**
   * Checks if the URL has an empty hostname (host is set but empty string).
   * @return `true` if host exists but is empty, `false` otherwise.
   */
  [[nodiscard]] inline bool has_empty_hostname() const noexcept;

  /**
   * Checks if the URL has a hostname (including empty hostnames).
   * @return `true` if host is present, `false` otherwise.
   */
  [[nodiscard]] inline bool has_hostname() const noexcept;

  /**
   * Checks if the URL has a non-empty username.
   * @return `true` if username is non-empty, `false` otherwise.
   */
  [[nodiscard]] inline bool has_non_empty_username() const noexcept;

  /**
   * Checks if the URL has a non-empty password.
   * @return `true` if password is non-empty, `false` otherwise.
   */
  [[nodiscard]] inline bool has_non_empty_password() const noexcept;

  /**
   * Checks if the URL has a non-default port explicitly specified.
   * @return `true` if a port is present, `false` otherwise.
   */
  [[nodiscard]] inline bool has_port() const noexcept;

  /**
   * Checks if the URL has a password component (may be empty).
   * @return `true` if password is present, `false` otherwise.
   */
  [[nodiscard]] inline bool has_password() const noexcept;

  /**
   * Checks if the URL has a fragment/hash component.
   * @return `true` if hash is present, `false` otherwise.
   */
  [[nodiscard]] inline bool has_hash() const noexcept override;

  /**
   * Checks if the URL has a query/search component.
   * @return `true` if query is present, `false` otherwise.
   */
  [[nodiscard]] inline bool has_search() const noexcept override;

  /**
   * Removes the port from the URL.
   */
  inline void clear_port();

  /**
   * Removes the hash/fragment from the URL.
   */
  inline void clear_hash();

  /**
   * Removes the query/search string from the URL.
   */
  inline void clear_search() override;

 private:
  // helper methods
  friend void helpers::strip_trailing_spaces_from_opaque_path<url_aggregator>(
      url_aggregator& url);
  // parse_url methods
  friend url_aggregator parser::parse_url<url_aggregator>(
      std::string_view, const url_aggregator*);

  friend url_aggregator parser::parse_url_impl<url_aggregator, true>(
      std::string_view, const url_aggregator*);
  friend url_aggregator parser::parse_url_impl<url_aggregator, false>(
      std::string_view, const url_aggregator*);
  template <class result_type>
  friend bool parser::try_parse_simple_absolute(std::string_view, result_type&);

  // components is declared before buffer so that the offset fields land
  // close to url_base in memory, improving cache locality for getter calls.
  // Note: exact cache-line placement is implementation- and platform-dependent.
  url_components components{};
  std::string buffer{};

  /**
   * Returns true if neither the search, nor the hash nor the pathname
   * have been set.
   * @return true if the buffer is ready to receive the path.
   */
  [[nodiscard]] ada_really_inline bool is_at_path() const noexcept;

  inline void add_authority_slashes_if_needed();

  /**
   * To optimize performance, you may indicate how much memory to allocate
   * within this instance.
   */
  inline void reserve(uint32_t capacity);

  ada_really_inline size_t parse_port(std::string_view view,
                                      bool check_trailing_content) override;

  ada_really_inline size_t parse_port(std::string_view view) override {
    return this->parse_port(view, false);
  }

  /**
   * Return true on success. The 'in_place' parameter indicates whether the
   * the string_view input is pointing in the buffer. When in_place is false,
   * we must nearly always update the buffer.
   * @see https://url.spec.whatwg.org/#concept-ipv4-parser
   */
  [[nodiscard]] bool parse_ipv4(std::string_view input, bool in_place);

  /**
   * Return true on success.
   * @see https://url.spec.whatwg.org/#concept-ipv6-parser
   */
  [[nodiscard]] bool parse_ipv6(std::string_view input);

  /**
   * Return true on success.
   * @see https://url.spec.whatwg.org/#concept-opaque-host-parser
   */
  [[nodiscard]] bool parse_opaque_host(std::string_view input);

  ada_really_inline void parse_path(std::string_view input);

  /**
   * A URL cannot have a username/password/port if its host is null or the empty
   * string, or its scheme is "file".
   */
  [[nodiscard]] inline bool cannot_have_credentials_or_port() const;

  template <bool override_hostname = false>
  bool set_host_or_hostname(std::string_view input);

  ada_really_inline bool parse_host(std::string_view input);

  inline void update_base_authority(std::string_view base_buffer,
                                    const url_components& base);
  inline void update_unencoded_base_hash(std::string_view input);
  inline void update_base_hostname(std::string_view input);
  inline void update_base_search(std::string_view input);
  inline void update_base_search(std::string_view input,
                                 const uint8_t* query_percent_encode_set);
  inline void update_base_pathname(std::string_view input);
  inline void update_base_username(std::string_view input);
  inline void append_base_username(std::string_view input);
  inline void update_base_password(std::string_view input);
  inline void append_base_password(std::string_view input);
  inline void update_base_port(uint32_t input);
  inline void append_base_pathname(std::string_view input);
  [[nodiscard]] inline uint32_t retrieve_base_port() const;
  inline void clear_hostname();
  inline void clear_password();
  inline void clear_pathname() override;
  [[nodiscard]] inline bool has_dash_dot() const noexcept;
  void delete_dash_dot();
  inline void consume_prepared_path(std::string_view input);
  template <bool has_state_override = false>
  [[nodiscard]] ada_really_inline bool parse_scheme_with_colon(
      std::string_view input);
  ada_really_inline uint32_t replace_and_resize(uint32_t start, uint32_t end,
                                                std::string_view input);
  [[nodiscard]] inline bool has_authority() const noexcept;
  inline void set_protocol_as_file();
  inline void set_scheme(std::string_view new_scheme);
  /**
   * Fast function to set the scheme from a view with a colon in the
   * buffer, does not change type.
   */
  inline void set_scheme_from_view_with_colon(
      std::string_view new_scheme_with_colon);
  inline void copy_scheme(const url_aggregator& u);

  inline void update_host_to_base_host(const std::string_view input);

};  // url_aggregator

inline std::ostream& operator<<(std::ostream& out, const url& u);
}  // namespace ada

#endif
/* end file include/ada/url_aggregator.h */
/* begin file include/ada/url_aggregator-inl.h */
/**
 * @file url_aggregator-inl.h
 * @brief Inline functions for url aggregator
 */
#ifndef ADA_URL_AGGREGATOR_INL_H
#define ADA_URL_AGGREGATOR_INL_H

/* begin file include/ada/unicode-inl.h */
/**
 * @file unicode-inl.h
 * @brief Definitions for unicode operations.
 */
#ifndef ADA_UNICODE_INL_H
#define ADA_UNICODE_INL_H

/**
 * Unicode operations. These functions are not part of our public API and may
 * change at any time.
 *
 * private
 * @namespace ada::unicode
 * @brief Includes the declarations for unicode operations
 */
namespace ada::unicode {
ada_really_inline size_t percent_encode_index(const std::string_view input,
                                              const uint8_t character_set[]) {
  const char* data = input.data();
  const size_t size = input.size();

  // Process 8 bytes at a time using unrolled loop
  size_t i = 0;
  for (; i + 8 <= size; i += 8) {
    unsigned char chunk[8];
    std::memcpy(&chunk, data + i,
                8);  // entices compiler to unconditionally process 8 characters

    // Check 8 characters at once
    for (size_t j = 0; j < 8; j++) {
      if (character_sets::bit_at(character_set, chunk[j])) {
        return i + j;
      }
    }
  }

  // Handle remaining bytes
  for (; i < size; i++) {
    if (character_sets::bit_at(character_set, data[i])) {
      return i;
    }
  }

  return size;
}
}  // namespace ada::unicode

#endif  // ADA_UNICODE_INL_H
/* end file include/ada/unicode-inl.h */

#include <charconv>
#include <ostream>
#include <string_view>

namespace ada {

inline void url_aggregator::update_base_authority(
    std::string_view base_buffer, const ada::url_components& base) {
  std::string_view input = base_buffer.substr(
      base.protocol_end, base.host_start - base.protocol_end);
  ada_log("url_aggregator::update_base_authority ", input);

  bool input_starts_with_dash = input.starts_with("//");
  uint32_t diff = components.host_start - components.protocol_end;

  buffer.erase(components.protocol_end,
               components.host_start - components.protocol_end);
  components.username_end = components.protocol_end;

  if (input_starts_with_dash) {
    input.remove_prefix(2);
    diff += 2;  // add "//"
    buffer.insert(components.protocol_end, "//");
    components.username_end += 2;
  }

  size_t password_delimiter = input.find(':');

  // Check if input contains both username and password by checking the
  // delimiter: ":" A typical input that contains authority would be "user:pass"
  if (password_delimiter != std::string_view::npos) {
    // Insert both username and password
    std::string_view username = input.substr(0, password_delimiter);
    std::string_view password = input.substr(password_delimiter + 1);

    buffer.insert(components.protocol_end + diff, username);
    diff += uint32_t(username.size());
    buffer.insert(components.protocol_end + diff, ":");
    components.username_end = components.protocol_end + diff;
    buffer.insert(components.protocol_end + diff + 1, password);
    diff += uint32_t(password.size()) + 1;
  } else if (!input.empty()) {
    // Insert only username
    buffer.insert(components.protocol_end + diff, input);
    components.username_end =
        components.protocol_end + diff + uint32_t(input.size());
    diff += uint32_t(input.size());
  }

  components.host_start += diff;

  if (buffer.size() > base.host_start && buffer[base.host_start] != '@') {
    buffer.insert(components.host_start, "@");
    diff++;
  }
  components.host_end += diff;
  components.pathname_start += diff;
  if (components.search_start != url_components::omitted) {
    components.search_start += diff;
  }
  if (components.hash_start != url_components::omitted) {
    components.hash_start += diff;
  }
}

inline void url_aggregator::update_unencoded_base_hash(std::string_view input) {
  ada_log("url_aggregator::update_unencoded_base_hash ", input, " [",
          input.size(), " bytes], buffer is '", buffer, "' [", buffer.size(),
          " bytes] components.hash_start = ", components.hash_start);
  ADA_ASSERT_TRUE(validate());
  ADA_ASSERT_TRUE(!helpers::overlaps(input, buffer));
  if (components.hash_start != url_components::omitted) {
    buffer.resize(components.hash_start);
  }
  components.hash_start = uint32_t(buffer.size());
  buffer += "#";
  bool encoding_required = unicode::percent_encode<true>(
      input, ada::character_sets::FRAGMENT_PERCENT_ENCODE, buffer);
  // When encoding_required is false, then buffer is left unchanged, and percent
  // encoding was not deemed required.
  if (!encoding_required) {
    buffer.append(input);
  }
  ada_log("url_aggregator::update_unencoded_base_hash final buffer is '",
          buffer, "' [", buffer.size(), " bytes]");
  ADA_ASSERT_TRUE(validate());
}

ada_really_inline uint32_t url_aggregator::replace_and_resize(
    uint32_t start, uint32_t end, std::string_view input) {
  uint32_t current_length = end - start;
  uint32_t input_size = uint32_t(input.size());
  uint32_t new_difference = input_size - current_length;

  if (current_length == 0) {
    buffer.insert(start, input);
  } else if (input_size == current_length) {
    buffer.replace(start, input_size, input);
  } else if (input_size < current_length) {
    buffer.erase(start, current_length - input_size);
    buffer.replace(start, input_size, input);
  } else {
    buffer.replace(start, current_length, input.substr(0, current_length));
    buffer.insert(start + current_length, input.substr(current_length));
  }

  return new_difference;
}

inline void url_aggregator::update_base_hostname(const std::string_view input) {
  ada_log("url_aggregator::update_base_hostname ", input, " [", input.size(),
          " bytes], buffer is '", buffer, "' [", buffer.size(), " bytes]");
  ADA_ASSERT_TRUE(validate());
  ADA_ASSERT_TRUE(!helpers::overlaps(input, buffer));

  // This next line is required for when parsing a URL like `foo://`
  add_authority_slashes_if_needed();

  bool has_credentials = components.protocol_end + 2 < components.host_start;
  uint32_t new_difference =
      replace_and_resize(components.host_start, components.host_end, input);

  if (has_credentials) {
    buffer.insert(components.host_start, "@");
    new_difference++;
  }
  components.host_end += new_difference;
  components.pathname_start += new_difference;
  if (components.search_start != url_components::omitted) {
    components.search_start += new_difference;
  }
  if (components.hash_start != url_components::omitted) {
    components.hash_start += new_difference;
  }
  ADA_ASSERT_TRUE(validate());
}

[[nodiscard]] ada_really_inline uint32_t
url_aggregator::get_pathname_length() const noexcept {
  ada_log("url_aggregator::get_pathname_length");
  uint32_t ending_index = uint32_t(buffer.size());
  if (components.search_start != url_components::omitted) {
    ending_index = components.search_start;
  } else if (components.hash_start != url_components::omitted) {
    ending_index = components.hash_start;
  }
  return ending_index - components.pathname_start;
}

[[nodiscard]] ada_really_inline bool url_aggregator::is_at_path()
    const noexcept {
  return buffer.size() == components.pathname_start;
}

inline void url_aggregator::update_base_search(std::string_view input) {
  ada_log("url_aggregator::update_base_search ", input);
  ADA_ASSERT_TRUE(validate());
  ADA_ASSERT_TRUE(!helpers::overlaps(input, buffer));
  if (input.empty()) {
    clear_search();
    return;
  }

  if (input[0] == '?') {
    input.remove_prefix(1);
  }

  if (components.hash_start == url_components::omitted) {
    if (components.search_start == url_components::omitted) {
      components.search_start = uint32_t(buffer.size());
      buffer += "?";
    } else {
      buffer.resize(components.search_start + 1);
    }

    buffer.append(input);
  } else {
    if (components.search_start == url_components::omitted) {
      components.search_start = components.hash_start;
    } else {
      buffer.erase(components.search_start,
                   components.hash_start - components.search_start);
      components.hash_start = components.search_start;
    }

    buffer.insert(components.search_start, "?");
    buffer.insert(components.search_start + 1, input);
    components.hash_start += uint32_t(input.size() + 1);  // Do not forget `?`
  }

  ADA_ASSERT_TRUE(validate());
}

inline void url_aggregator::update_base_search(
    std::string_view input, const uint8_t query_percent_encode_set[]) {
  ada_log("url_aggregator::update_base_search ", input,
          " with encoding parameter ", to_string(), "\n", to_diagram());
  ADA_ASSERT_TRUE(validate());
  ADA_ASSERT_TRUE(!helpers::overlaps(input, buffer));

  if (components.hash_start == url_components::omitted) {
    if (components.search_start == url_components::omitted) {
      components.search_start = uint32_t(buffer.size());
      buffer += "?";
    } else {
      buffer.resize(components.search_start + 1);
    }

    bool encoding_required =
        unicode::percent_encode<true>(input, query_percent_encode_set, buffer);
    // When encoding_required is false, then buffer is left unchanged, and
    // percent encoding was not deemed required.
    if (!encoding_required) {
      buffer.append(input);
    }
  } else {
    if (components.search_start == url_components::omitted) {
      components.search_start = components.hash_start;
    } else {
      buffer.erase(components.search_start,
                   components.hash_start - components.search_start);
      components.hash_start = components.search_start;
    }

    buffer.insert(components.search_start, "?");
    size_t idx =
        ada::unicode::percent_encode_index(input, query_percent_encode_set);
    if (idx == input.size()) {
      buffer.insert(components.search_start + 1, input);
      components.hash_start += uint32_t(input.size() + 1);  // Do not forget `?`
    } else {
      buffer.insert(components.search_start + 1, input, 0, idx);
      input.remove_prefix(idx);
      // We only create a temporary string if we need percent encoding and
      // we attempt to create as small a temporary string as we can.
      std::string encoded =
          ada::unicode::percent_encode(input, query_percent_encode_set);
      buffer.insert(components.search_start + idx + 1, encoded);
      components.hash_start +=
          uint32_t(encoded.size() + idx + 1);  // Do not forget `?`
    }
  }

  ADA_ASSERT_TRUE(validate());
}

inline void url_aggregator::update_base_pathname(const std::string_view input) {
  ada_log("url_aggregator::update_base_pathname '", input, "' [", input.size(),
          " bytes] \n", to_diagram());
  ADA_ASSERT_TRUE(!helpers::overlaps(input, buffer));
  ADA_ASSERT_TRUE(validate());

  const bool begins_with_dashdash = input.starts_with("//");
  if (!begins_with_dashdash && has_dash_dot()) {
    // We must delete the ./
    delete_dash_dot();
  }

  if (begins_with_dashdash && !has_opaque_path && !has_authority() &&
      !has_dash_dot()) {
    // If url's host is null, url does not have an opaque path, url's path's
    // size is greater than 1, then append U+002F (/) followed by U+002E (.) to
    // output.
    buffer.insert(components.pathname_start, "/.");
    components.pathname_start += 2;
    if (components.search_start != url_components::omitted) {
      components.search_start += 2;
    }
    if (components.hash_start != url_components::omitted) {
      components.hash_start += 2;
    }
  }

  uint32_t difference = replace_and_resize(
      components.pathname_start,
      components.pathname_start + get_pathname_length(), input);
  if (components.search_start != url_components::omitted) {
    components.search_start += difference;
  }
  if (components.hash_start != url_components::omitted) {
    components.hash_start += difference;
  }
  ADA_ASSERT_TRUE(validate());
}

inline void url_aggregator::append_base_pathname(const std::string_view input) {
  ada_log("url_aggregator::append_base_pathname ", input, " ", to_string(),
          "\n", to_diagram());
  ADA_ASSERT_TRUE(validate());
  ADA_ASSERT_TRUE(!helpers::overlaps(input, buffer));
#if ADA_DEVELOPMENT_CHECKS
  // computing the expected password.
  std::string path_expected(get_pathname());
  path_expected.append(input);
#endif  // ADA_DEVELOPMENT_CHECKS
  uint32_t ending_index = uint32_t(buffer.size());
  if (components.search_start != url_components::omitted) {
    ending_index = components.search_start;
  } else if (components.hash_start != url_components::omitted) {
    ending_index = components.hash_start;
  }
  buffer.insert(ending_index, input);

  if (components.search_start != url_components::omitted) {
    components.search_start += uint32_t(input.size());
  }
  if (components.hash_start != url_components::omitted) {
    components.hash_start += uint32_t(input.size());
  }
#if ADA_DEVELOPMENT_CHECKS
  std::string path_after = std::string(get_pathname());
  ADA_ASSERT_EQUAL(
      path_expected, path_after,
      "append_base_pathname problem after inserting " + std::string(input));
#endif  // ADA_DEVELOPMENT_CHECKS
  ADA_ASSERT_TRUE(validate());
}

inline void url_aggregator::update_base_username(const std::string_view input) {
  ada_log("url_aggregator::update_base_username '", input, "' ", to_string(),
          "\n", to_diagram());
  ADA_ASSERT_TRUE(validate());
  ADA_ASSERT_TRUE(!helpers::overlaps(input, buffer));

  add_authority_slashes_if_needed();

  bool has_password = has_non_empty_password();
  bool host_starts_with_at = buffer.size() > components.host_start &&
                             buffer[components.host_start] == '@';
  uint32_t diff = replace_and_resize(components.protocol_end + 2,
                                     components.username_end, input);

  components.username_end += diff;
  components.host_start += diff;

  if (!input.empty() && !host_starts_with_at) {
    buffer.insert(components.host_start, "@");
    diff++;
  } else if (input.empty() && host_starts_with_at && !has_password) {
    // Input is empty, there is no password, and we need to remove "@" from
    // hostname
    buffer.erase(components.host_start, 1);
    diff--;
  }

  components.host_end += diff;
  components.pathname_start += diff;
  if (components.search_start != url_components::omitted) {
    components.search_start += diff;
  }
  if (components.hash_start != url_components::omitted) {
    components.hash_start += diff;
  }
  ADA_ASSERT_TRUE(validate());
}

inline void url_aggregator::append_base_username(const std::string_view input) {
  ada_log("url_aggregator::append_base_username ", input);
  ADA_ASSERT_TRUE(validate());
  ADA_ASSERT_TRUE(!helpers::overlaps(input, buffer));
#if ADA_DEVELOPMENT_CHECKS
  // computing the expected password.
  std::string username_expected(get_username());
  username_expected.append(input);
#endif  // ADA_DEVELOPMENT_CHECKS
  add_authority_slashes_if_needed();

  // If input is empty, do nothing.
  if (input.empty()) {
    return;
  }

  uint32_t difference = uint32_t(input.size());
  buffer.insert(components.username_end, input);
  components.username_end += difference;
  components.host_start += difference;

  if (buffer[components.host_start] != '@' &&
      components.host_start != components.host_end) {
    buffer.insert(components.host_start, "@");
    difference++;
  }

  components.host_end += difference;
  components.pathname_start += difference;
  if (components.search_start != url_components::omitted) {
    components.search_start += difference;
  }
  if (components.hash_start != url_components::omitted) {
    components.hash_start += difference;
  }
#if ADA_DEVELOPMENT_CHECKS
  std::string username_after(get_username());
  ADA_ASSERT_EQUAL(
      username_expected, username_after,
      "append_base_username problem after inserting " + std::string(input));
#endif  // ADA_DEVELOPMENT_CHECKS
  ADA_ASSERT_TRUE(validate());
}

inline void url_aggregator::clear_password() {
  ada_log("url_aggregator::clear_password ", to_string());
  ADA_ASSERT_TRUE(validate());
  if (!has_password()) {
    return;
  }

  uint32_t diff = components.host_start - components.username_end;
  buffer.erase(components.username_end, diff);
  components.host_start -= diff;
  components.host_end -= diff;
  components.pathname_start -= diff;
  if (components.search_start != url_components::omitted) {
    components.search_start -= diff;
  }
  if (components.hash_start != url_components::omitted) {
    components.hash_start -= diff;
  }
}

inline void url_aggregator::update_base_password(const std::string_view input) {
  ada_log("url_aggregator::update_base_password ", input);
  ADA_ASSERT_TRUE(validate());
  ADA_ASSERT_TRUE(!helpers::overlaps(input, buffer));

  add_authority_slashes_if_needed();

  // TODO: Optimization opportunity. Merge the following removal functions.
  if (input.empty()) {
    clear_password();

    // Remove username too, if it is empty.
    if (!has_non_empty_username()) {
      update_base_username("");
    }

    return;
  }

  bool password_exists = has_password();
  uint32_t difference = uint32_t(input.size());

  if (password_exists) {
    uint32_t current_length =
        components.host_start - components.username_end - 1;
    buffer.erase(components.username_end + 1, current_length);
    difference -= current_length;
  } else {
    buffer.insert(components.username_end, ":");
    difference++;
  }

  buffer.insert(components.username_end + 1, input);
  components.host_start += difference;

  // The following line is required to add "@" to hostname. When updating
  // password if hostname does not start with "@", it is "update_base_password"s
  // responsibility to set it.
  if (buffer[components.host_start] != '@') {
    buffer.insert(components.host_start, "@");
    difference++;
  }

  components.host_end += difference;
  components.pathname_start += difference;
  if (components.search_start != url_components::omitted) {
    components.search_start += difference;
  }
  if (components.hash_start != url_components::omitted) {
    components.hash_start += difference;
  }
  ADA_ASSERT_TRUE(validate());
}

inline void url_aggregator::append_base_password(const std::string_view input) {
  ada_log("url_aggregator::append_base_password ", input, " ", to_string(),
          "\n", to_diagram());
  ADA_ASSERT_TRUE(validate());
  ADA_ASSERT_TRUE(!helpers::overlaps(input, buffer));
#if ADA_DEVELOPMENT_CHECKS
  // computing the expected password.
  std::string password_expected = std::string(get_password());
  password_expected.append(input);
#endif  // ADA_DEVELOPMENT_CHECKS
  add_authority_slashes_if_needed();

  // If input is empty, do nothing.
  if (input.empty()) {
    return;
  }

  uint32_t difference = uint32_t(input.size());
  if (has_password()) {
    buffer.insert(components.host_start, input);
  } else {
    difference++;  // Increment for ":"
    buffer.insert(components.username_end, ":");
    buffer.insert(components.username_end + 1, input);
  }
  components.host_start += difference;

  // The following line is required to add "@" to hostname. When updating
  // password if hostname does not start with "@", it is "append_base_password"s
  // responsibility to set it.
  if (buffer[components.host_start] != '@') {
    buffer.insert(components.host_start, "@");
    difference++;
  }

  components.host_end += difference;
  components.pathname_start += difference;
  if (components.search_start != url_components::omitted) {
    components.search_start += difference;
  }
  if (components.hash_start != url_components::omitted) {
    components.hash_start += difference;
  }
#if ADA_DEVELOPMENT_CHECKS
  std::string password_after(get_password());
  ADA_ASSERT_EQUAL(
      password_expected, password_after,
      "append_base_password problem after inserting " + std::string(input));
#endif  // ADA_DEVELOPMENT_CHECKS
  ADA_ASSERT_TRUE(validate());
}

inline void url_aggregator::update_base_port(uint32_t input) {
  ada_log("url_aggregator::update_base_port");
  ADA_ASSERT_TRUE(validate());
  if (input == url_components::omitted) {
    clear_port();
    return;
  }
  // calling std::to_string(input.value()) is unfortunate given that the port
  // value is probably already available as a string.
  std::string value = helpers::concat(":", std::to_string(input));
  uint32_t difference = uint32_t(value.size());

  if (components.port != url_components::omitted) {
    difference -= components.pathname_start - components.host_end;
    buffer.erase(components.host_end,
                 components.pathname_start - components.host_end);
  }

  buffer.insert(components.host_end, value);
  components.pathname_start += difference;
  if (components.search_start != url_components::omitted) {
    components.search_start += difference;
  }
  if (components.hash_start != url_components::omitted) {
    components.hash_start += difference;
  }
  components.port = input;
  ADA_ASSERT_TRUE(validate());
}

inline void url_aggregator::clear_port() {
  ada_log("url_aggregator::clear_port");
  ADA_ASSERT_TRUE(validate());
  if (components.port == url_components::omitted) {
    return;
  }
  uint32_t length = components.pathname_start - components.host_end;
  buffer.erase(components.host_end, length);
  components.pathname_start -= length;
  if (components.search_start != url_components::omitted) {
    components.search_start -= length;
  }
  if (components.hash_start != url_components::omitted) {
    components.hash_start -= length;
  }
  components.port = url_components::omitted;
  ADA_ASSERT_TRUE(validate());
}

[[nodiscard]] inline uint32_t url_aggregator::retrieve_base_port() const {
  ada_log("url_aggregator::retrieve_base_port");
  return components.port;
}

inline void url_aggregator::clear_search() {
  ada_log("url_aggregator::clear_search");
  ADA_ASSERT_TRUE(validate());
  if (components.search_start == url_components::omitted) {
    return;
  }

  if (components.hash_start == url_components::omitted) {
    buffer.resize(components.search_start);
  } else {
    buffer.erase(components.search_start,
                 components.hash_start - components.search_start);
    components.hash_start = components.search_start;
  }

  components.search_start = url_components::omitted;

#if ADA_DEVELOPMENT_CHECKS
  ADA_ASSERT_EQUAL(get_search(), "",
                   "search should have been cleared on buffer=" + buffer +
                       " with " + components.to_string() + "\n" + to_diagram());
#endif
  ADA_ASSERT_TRUE(validate());
}

inline void url_aggregator::clear_hash() {
  ada_log("url_aggregator::clear_hash");
  ADA_ASSERT_TRUE(validate());
  if (components.hash_start == url_components::omitted) {
    return;
  }
  buffer.resize(components.hash_start);
  components.hash_start = url_components::omitted;

#if ADA_DEVELOPMENT_CHECKS
  ADA_ASSERT_EQUAL(get_hash(), "",
                   "hash should have been cleared on buffer=" + buffer +
                       " with " + components.to_string() + "\n" + to_diagram());
#endif
  ADA_ASSERT_TRUE(validate());
}

inline void url_aggregator::clear_pathname() {
  ada_log("url_aggregator::clear_pathname");
  ADA_ASSERT_TRUE(validate());
  uint32_t ending_index = uint32_t(buffer.size());
  if (components.search_start != url_components::omitted) {
    ending_index = components.search_start;
  } else if (components.hash_start != url_components::omitted) {
    ending_index = components.hash_start;
  }
  uint32_t pathname_length = ending_index - components.pathname_start;
  buffer.erase(components.pathname_start, pathname_length);
  uint32_t difference = pathname_length;
  if (components.pathname_start == components.host_end + 2 &&
      buffer[components.host_end] == '/' &&
      buffer[components.host_end + 1] == '.') {
    components.pathname_start -= 2;
    buffer.erase(components.host_end, 2);
    difference += 2;
  }
  if (components.search_start != url_components::omitted) {
    components.search_start -= difference;
  }
  if (components.hash_start != url_components::omitted) {
    components.hash_start -= difference;
  }
  ada_log("url_aggregator::clear_pathname completed, running checks...");
#if ADA_DEVELOPMENT_CHECKS
  ADA_ASSERT_EQUAL(get_pathname(), "",
                   "pathname should have been cleared on buffer=" + buffer +
                       " with " + components.to_string() + "\n" + to_diagram());
#endif
  ADA_ASSERT_TRUE(validate());
  ada_log("url_aggregator::clear_pathname completed, running checks... ok");
}

inline void url_aggregator::clear_hostname() {
  ada_log("url_aggregator::clear_hostname");
  ADA_ASSERT_TRUE(validate());
  if (!has_authority()) {
    return;
  }
  ADA_ASSERT_TRUE(has_authority());

  uint32_t hostname_length = components.host_end - components.host_start;
  uint32_t start = components.host_start;

  // If hostname starts with "@", we should not remove that character.
  if (hostname_length > 0 && buffer[start] == '@') {
    start++;
    hostname_length--;
  }
  buffer.erase(start, hostname_length);
  components.host_end = start;
  components.pathname_start -= hostname_length;
  if (components.search_start != url_components::omitted) {
    components.search_start -= hostname_length;
  }
  if (components.hash_start != url_components::omitted) {
    components.hash_start -= hostname_length;
  }
#if ADA_DEVELOPMENT_CHECKS
  ADA_ASSERT_EQUAL(get_hostname(), "",
                   "hostname should have been cleared on buffer=" + buffer +
                       " with " + components.to_string() + "\n" + to_diagram());
#endif
  ADA_ASSERT_TRUE(has_authority());
  ADA_ASSERT_EQUAL(has_empty_hostname(), true,
                   "hostname should have been cleared on buffer=" + buffer +
                       " with " + components.to_string() + "\n" + to_diagram());
  ADA_ASSERT_TRUE(validate());
}

[[nodiscard]] inline bool url_aggregator::has_hash() const noexcept {
  ada_log("url_aggregator::has_hash");
  return components.hash_start != url_components::omitted;
}

[[nodiscard]] inline bool url_aggregator::has_search() const noexcept {
  ada_log("url_aggregator::has_search");
  return components.search_start != url_components::omitted;
}

inline bool url_aggregator::has_credentials() const noexcept {
  ada_log("url_aggregator::has_credentials");
  return has_non_empty_username() || has_non_empty_password();
}

inline bool url_aggregator::cannot_have_credentials_or_port() const {
  ada_log("url_aggregator::cannot_have_credentials_or_port");
  return type == ada::scheme::type::FILE ||
         components.host_start == components.host_end;
}

[[nodiscard]] ada_really_inline const ada::url_components&
url_aggregator::get_components() const noexcept {
  return components;
}

[[nodiscard]] inline bool ada::url_aggregator::has_authority()
    const noexcept {
  ada_log("url_aggregator::has_authority");
  // Performance: instead of doing this potentially expensive check, we could
  // have a boolean in the struct.
  return components.protocol_end + 2 <= components.host_start &&
         buffer[components.protocol_end] == '/' &&
         buffer[components.protocol_end + 1] == '/';
}

inline void ada::url_aggregator::add_authority_slashes_if_needed() {
  ada_log("url_aggregator::add_authority_slashes_if_needed");
  ADA_ASSERT_TRUE(validate());
  // Protocol setter will insert `http:` to the URL. It is up to hostname setter
  // to insert
  // `//` initially to the buffer, since it depends on the hostname existence.
  if (has_authority()) {
    return;
  }
  // Performance: the common case is components.protocol_end == buffer.size()
  // Optimization opportunity: in many cases, the "//" is part of the input and
  // the insert could be fused with another insert.
  buffer.insert(components.protocol_end, "//");
  components.username_end += 2;
  components.host_start += 2;
  components.host_end += 2;
  components.pathname_start += 2;
  if (components.search_start != url_components::omitted) {
    components.search_start += 2;
  }
  if (components.hash_start != url_components::omitted) {
    components.hash_start += 2;
  }
  ADA_ASSERT_TRUE(validate());
}

inline void ada::url_aggregator::reserve(uint32_t capacity) {
  buffer.reserve(capacity);
}

inline bool url_aggregator::has_non_empty_username() const noexcept {
  ada_log("url_aggregator::has_non_empty_username");
  return components.protocol_end + 2 < components.username_end;
}

inline bool url_aggregator::has_non_empty_password() const noexcept {
  ada_log("url_aggregator::has_non_empty_password");
  return components.host_start > components.username_end;
}

inline bool url_aggregator::has_password() const noexcept {
  ada_log("url_aggregator::has_password");
  // This function does not care about the length of the password
  return components.host_start > components.username_end &&
         buffer[components.username_end] == ':';
}

inline bool url_aggregator::has_empty_hostname() const noexcept {
  if (!has_hostname()) {
    return false;
  }
  if (components.host_start == components.host_end) {
    return true;
  }
  if (components.host_end > components.host_start + 1) {
    return false;
  }
  return components.username_end != components.host_start;
}

inline bool url_aggregator::has_hostname() const noexcept {
  return has_authority();
}

inline bool url_aggregator::has_port() const noexcept {
  ada_log("url_aggregator::has_port");
  // A URL cannot have a username/password/port if its host is null or the empty
  // string, or its scheme is "file".
  return has_hostname() && components.pathname_start != components.host_end;
}

[[nodiscard]] inline bool url_aggregator::has_dash_dot() const noexcept {
  // If url's host is null, url does not have an opaque path, url's path's size
  // is greater than 1, and url's path[0] is the empty string, then append
  // U+002F (/) followed by U+002E (.) to output.
  ada_log("url_aggregator::has_dash_dot");
#if ADA_DEVELOPMENT_CHECKS
  // If pathname_start and host_end are exactly two characters apart, then we
  // either have a one-digit port such as http://test.com:5?param=1 or else we
  // have a /.: sequence such as "non-spec:/.//". We test that this is the case.
  if (components.pathname_start == components.host_end + 2) {
    ADA_ASSERT_TRUE((buffer[components.host_end] == '/' &&
                     buffer[components.host_end + 1] == '.') ||
                    (buffer[components.host_end] == ':' &&
                     checkers::is_digit(buffer[components.host_end + 1])));
  }
  if (components.pathname_start == components.host_end + 2 &&
      buffer[components.host_end] == '/' &&
      buffer[components.host_end + 1] == '.') {
    ADA_ASSERT_TRUE(components.pathname_start + 1 < buffer.size());
    ADA_ASSERT_TRUE(buffer[components.pathname_start] == '/');
    ADA_ASSERT_TRUE(buffer[components.pathname_start + 1] == '/');
  }
#endif
  // Performance: it should be uncommon for components.pathname_start ==
  // components.host_end + 2 to be true. So we put this check first in the
  // sequence. Most times, we do not have an opaque path. Checking for '/.' is
  // more expensive, but should be uncommon.
  return components.pathname_start == components.host_end + 2 &&
         !has_opaque_path && buffer[components.host_end] == '/' &&
         buffer[components.host_end + 1] == '.';
}

[[nodiscard]] inline std::string_view url_aggregator::get_href()
    const noexcept ada_lifetime_bound {
  ada_log("url_aggregator::get_href");
  return buffer;
}

[[nodiscard]] inline size_t url_aggregator::get_href_size() const noexcept {
  return buffer.size();
}

ada_really_inline size_t
url_aggregator::parse_port(std::string_view view, bool check_trailing_content) {
  ada_log("url_aggregator::parse_port('", view, "') ", view.size());
  if (!view.empty() && view[0] == '-') {
    ada_log("parse_port: view[0] == '0' && view.size() > 1");
    is_valid = false;
    return 0;
  }
  uint16_t parsed_port{};
  auto r = std::from_chars(view.data(), view.data() + view.size(), parsed_port);
  if (r.ec == std::errc::result_out_of_range) {
    ada_log("parse_port: r.ec == std::errc::result_out_of_range");
    is_valid = false;
    return 0;
  }
  ada_log("parse_port: ", parsed_port);
  const size_t consumed = size_t(r.ptr - view.data());
  ada_log("parse_port: consumed ", consumed);
  if (check_trailing_content) {
    is_valid &=
        (consumed == view.size() || view[consumed] == '/' ||
         view[consumed] == '?' || (is_special() && view[consumed] == '\\'));
  }
  ada_log("parse_port: is_valid = ", is_valid);
  if (is_valid) {
    ada_log("parse_port", r.ec == std::errc());
    // scheme_default_port can return 0, and we should allow 0 as a base port.
    auto default_port = scheme_default_port();
    bool is_port_valid = (default_port == 0 && parsed_port == 0) ||
                         (default_port != parsed_port);
    if (r.ec == std::errc() && is_port_valid) {
      update_base_port(parsed_port);
    } else {
      clear_port();
    }
  }
  return consumed;
}

inline void url_aggregator::set_protocol_as_file() {
  ada_log("url_aggregator::set_protocol_as_file ");
  ADA_ASSERT_TRUE(validate());
  type = ada::scheme::type::FILE;
  // next line could overflow but unsigned arithmetic has well-defined
  // overflows.
  uint32_t new_difference = 5 - components.protocol_end;

  if (buffer.empty()) {
    buffer.append("file:");
  } else {
    buffer.erase(0, components.protocol_end);
    buffer.insert(0, "file:");
  }
  components.protocol_end = 5;

  // Update the rest of the components.
  components.username_end += new_difference;
  components.host_start += new_difference;
  components.host_end += new_difference;
  components.pathname_start += new_difference;
  if (components.search_start != url_components::omitted) {
    components.search_start += new_difference;
  }
  if (components.hash_start != url_components::omitted) {
    components.hash_start += new_difference;
  }
  ADA_ASSERT_TRUE(validate());
}

[[nodiscard]] inline bool url_aggregator::validate() const noexcept {
  if (!is_valid) {
    return true;
  }
  if (!components.check_offset_consistency()) {
    ada_log("url_aggregator::validate inconsistent components \n",
            to_diagram());
    return false;
  }
  // We have a credible components struct, but let us investivate more
  // carefully:
  /**
   * https://user:pass@example.com:1234/foo/bar?baz#quux
   *       |     |    |          | ^^^^|       |   |
   *       |     |    |          | |   |       |   `----- hash_start
   *       |     |    |          | |   |       `--------- search_start
   *       |     |    |          | |   `----------------- pathname_start
   *       |     |    |          | `--------------------- port
   *       |     |    |          `----------------------- host_end
   *       |     |    `---------------------------------- host_start
   *       |     `--------------------------------------- username_end
   *       `--------------------------------------------- protocol_end
   */
  if (components.protocol_end == url_components::omitted) {
    ada_log("url_aggregator::validate omitted protocol_end \n", to_diagram());
    return false;
  }
  if (components.username_end == url_components::omitted) {
    ada_log("url_aggregator::validate omitted username_end \n", to_diagram());
    return false;
  }
  if (components.host_start == url_components::omitted) {
    ada_log("url_aggregator::validate omitted host_start \n", to_diagram());
    return false;
  }
  if (components.host_end == url_components::omitted) {
    ada_log("url_aggregator::validate omitted host_end \n", to_diagram());
    return false;
  }
  if (components.pathname_start == url_components::omitted) {
    ada_log("url_aggregator::validate omitted pathname_start \n", to_diagram());
    return false;
  }

  if (components.protocol_end > buffer.size()) {
    ada_log("url_aggregator::validate protocol_end overflow \n", to_diagram());
    return false;
  }
  if (components.username_end > buffer.size()) {
    ada_log("url_aggregator::validate username_end overflow \n", to_diagram());
    return false;
  }
  if (components.host_start > buffer.size()) {
    ada_log("url_aggregator::validate host_start overflow \n", to_diagram());
    return false;
  }
  if (components.host_end > buffer.size()) {
    ada_log("url_aggregator::validate host_end overflow \n", to_diagram());
    return false;
  }
  if (components.pathname_start > buffer.size()) {
    ada_log("url_aggregator::validate pathname_start overflow \n",
            to_diagram());
    return false;
  }

  if (components.protocol_end > 0) {
    if (buffer[components.protocol_end - 1] != ':') {
      ada_log(
          "url_aggregator::validate missing : at the end of the protocol \n",
          to_diagram());
      return false;
    }
  }

  if (components.username_end != buffer.size() &&
      components.username_end > components.protocol_end + 2) {
    if (buffer[components.username_end] != ':' &&
        buffer[components.username_end] != '@') {
      ada_log(
          "url_aggregator::validate missing : or @ at the end of the username "
          "\n",
          to_diagram());
      return false;
    }
  }

  if (components.host_start != buffer.size()) {
    if (components.host_start > components.username_end) {
      if (buffer[components.host_start] != '@') {
        ada_log(
            "url_aggregator::validate missing @ at the end of the password \n",
            to_diagram());
        return false;
      }
    } else if (components.host_start == components.username_end &&
               components.host_end > components.host_start) {
      if (components.host_start == components.protocol_end + 2) {
        if (buffer[components.protocol_end] != '/' ||
            buffer[components.protocol_end + 1] != '/') {
          ada_log(
              "url_aggregator::validate missing // between protocol and host "
              "\n",
              to_diagram());
          return false;
        }
      } else {
        if (components.host_start > components.protocol_end &&
            buffer[components.host_start] != '@') {
          ada_log(
              "url_aggregator::validate missing @ at the end of the username "
              "\n",
              to_diagram());
          return false;
        }
      }
    } else {
      if (components.host_end != components.host_start) {
        ada_log("url_aggregator::validate expected omitted host \n",
                to_diagram());
        return false;
      }
    }
  }
  if (components.host_end != buffer.size() &&
      components.pathname_start > components.host_end) {
    if (components.pathname_start == components.host_end + 2 &&
        buffer[components.host_end] == '/' &&
        buffer[components.host_end + 1] == '.') {
      if (components.pathname_start + 1 >= buffer.size() ||
          buffer[components.pathname_start] != '/' ||
          buffer[components.pathname_start + 1] != '/') {
        ada_log(
            "url_aggregator::validate expected the path to begin with // \n",
            to_diagram());
        return false;
      }
    } else if (buffer[components.host_end] != ':') {
      ada_log("url_aggregator::validate missing : at the port \n",
              to_diagram());
      return false;
    }
  }
  if (components.pathname_start != buffer.size() &&
      components.pathname_start < components.search_start &&
      components.pathname_start < components.hash_start && !has_opaque_path) {
    if (buffer[components.pathname_start] != '/') {
      ada_log("url_aggregator::validate missing / at the path \n",
              to_diagram());
      return false;
    }
  }
  if (components.search_start != url_components::omitted) {
    if (buffer[components.search_start] != '?') {
      ada_log("url_aggregator::validate missing ? at the search \n",
              to_diagram());
      return false;
    }
  }
  if (components.hash_start != url_components::omitted) {
    if (buffer[components.hash_start] != '#') {
      ada_log("url_aggregator::validate missing # at the hash \n",
              to_diagram());
      return false;
    }
  }

  return true;
}

[[nodiscard]] inline std::string_view url_aggregator::get_pathname() const
    ada_lifetime_bound {
  ada_log("url_aggregator::get_pathname pathname_start = ",
          components.pathname_start, " buffer.size() = ", buffer.size(),
          " components.search_start = ", components.search_start,
          " components.hash_start = ", components.hash_start);
  auto ending_index = uint32_t(buffer.size());
  if (components.search_start != url_components::omitted) {
    ending_index = components.search_start;
  } else if (components.hash_start != url_components::omitted) {
    ending_index = components.hash_start;
  }
  return helpers::substring(buffer, components.pathname_start, ending_index);
}

inline std::ostream& operator<<(std::ostream& out,
                                const ada::url_aggregator& u) {
  return out << u.to_string();
}

void url_aggregator::update_host_to_base_host(const std::string_view input) {
  ada_log("url_aggregator::update_host_to_base_host ", input);
  ADA_ASSERT_TRUE(validate());
  ADA_ASSERT_TRUE(!helpers::overlaps(input, buffer));
  if (type != ada::scheme::type::FILE) {
    // Let host be the result of host parsing host_view with url is not special.
    if (input.empty() && !is_special()) {
      if (has_hostname()) {
        clear_hostname();
      } else if (has_dash_dot()) {
        add_authority_slashes_if_needed();
        delete_dash_dot();
      }
      return;
    }
  }
  update_base_hostname(input);
  ADA_ASSERT_TRUE(validate());
  return;
}
}  // namespace ada

#endif  // ADA_URL_AGGREGATOR_INL_H
/* end file include/ada/url_aggregator-inl.h */

// Public API
/* begin file include/ada/ada_version.h */
/**
 * @file ada_version.h
 * @brief Definitions for Ada's version number.
 */
#ifndef ADA_ADA_VERSION_H
#define ADA_ADA_VERSION_H

#define ADA_VERSION "4.0.0"

namespace ada {

enum {
  ADA_VERSION_MAJOR = 4,
  ADA_VERSION_MINOR = 0,
  ADA_VERSION_REVISION = 0,
};

}  // namespace ada

#endif  // ADA_ADA_VERSION_H
/* end file include/ada/ada_version.h */

#endif  // ADA_H
/* end file include/ada.h */
