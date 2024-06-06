/*
 * Copyright (c) 2022 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef PLUTOBOOK_H
#define PLUTOBOOK_H

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(PLUTOBOOK_BUILD_STATIC) && (defined(_WIN32) || defined(__CYGWIN__))
#define PLUTOBOOK_EXPORT __declspec(dllexport)
#define PLUTOBOOK_IMPORT __declspec(dllimport)
#elif defined(__GNUC__) && (__GNUC__ >= 4)
#define PLUTOBOOK_EXPORT __attribute__((__visibility__("default")))
#define PLUTOBOOK_IMPORT
#else
#define PLUTOBOOK_EXPORT
#define PLUTOBOOK_IMPORT
#endif

#ifdef PLUTOBOOK_BUILD
#define PLUTOBOOK_API PLUTOBOOK_EXPORT
#else
#define PLUTOBOOK_API PLUTOBOOK_IMPORT
#endif

#define PLUTOBOOK_VERSION_MAJOR 0
#define PLUTOBOOK_VERSION_MINOR 0
#define PLUTOBOOK_VERSION_MICRO 1

#define PLUTOBOOK_VERSION_ENCODE(major, minor, micro) (((major) * 10000) + ((minor) * 100) + ((micro) * 1))
#define PLUTOBOOK_VERSION PLUTOBOOK_VERSION_ENCODE(PLUTOBOOK_VERSION_MAJOR, PLUTOBOOK_VERSION_MINOR, PLUTOBOOK_VERSION_MICRO)

#define PLUTOBOOK_VERSION_XSTRINGIZE(major, minor, micro) #major"."#minor"."#micro
#define PLUTOBOOK_VERSION_STRINGIZE(major, minor, micro) PLUTOBOOK_VERSION_XSTRINGIZE(major, minor, micro)
#define PLUTOBOOK_VERSION_STRING PLUTOBOOK_VERSION_STRINGIZE(PLUTOBOOK_VERSION_MAJOR, PLUTOBOOK_VERSION_MINOR, PLUTOBOOK_VERSION_MICRO)

/**
 * @brief Returns the version of the plutobook library encoded in a single integer.
 *
 * This function retrieves the version of the plutobook library and encodes it into a single integer.
 * The version number is represented in a format suitable for comparison.
 *
 * @return The version of the plutobook library encoded in a single integer.
 */
PLUTOBOOK_API int plutobook_version(void);

/**
 * @brief Returns the version of the plutobook library as a human-readable string in the format "X.Y.Z".
 *
 * This function retrieves the version of the plutobook library and returns it as a human-readable string
 * in the format "X.Y.Z", where X represents the major version, Y represents the minor version, and Z represents
 * the patch version.
 *
 * @return A pointer to a string containing the version of the plutobook library in the format "X.Y.Z".
 */
PLUTOBOOK_API const char* plutobook_version_string(void);

/**
 * @brief plutobook_about
 * @return
 */
PLUTOBOOK_API const char* plutobook_about(void);

/**
 * This macro defines an index that is guaranteed to exceed the valid page count.
 * It is typically utilized as a sentinel value to represent an unbounded or maximum value, indicating
 * that there is no limit or that the maximum possible value is intended.
 */
#define PLUTOBOOK_MAX_PAGE_COUNT 0xFFFFFFFFU
#define PLUTOBOOK_MIN_PAGE_COUNT 0x00000000U

/**
 * Defines conversion factors for various units to points (pt) and vice versa.
 * These conversion factors allow easy conversion between different units and points.
 *
 * Example Usage:
 *   - To convert 12 inches to points: 12 * PLUTOBOOK_UNITS_IN
 *   - To convert 12 points to inches: 12 / PLUTOBOOK_UNITS_IN
 */
#define PLUTOBOOK_UNITS_PT 1.f
#define PLUTOBOOK_UNITS_PC 12.f
#define PLUTOBOOK_UNITS_IN 72.f
#define PLUTOBOOK_UNITS_CM (72.f / 2.54f)
#define PLUTOBOOK_UNITS_MM (72.f / 25.4f)
#define PLUTOBOOK_UNITS_PX (72.f / 96.0f)

/**
 * @brief Represents the size of a page in points (1/72 inch).
 */
typedef struct _plutobook_page_size {
    float width;
    float height;
} plutobook_page_size_t;

/**
 * @brief Predefined plutobook_page_size_t objects for common paper sizes.
 */
#define PLUTOBOOK_PAGE_SIZE_NAMED(name) ((plutobook_page_size_t){PLUTOBOOK_PAGE_WIDTH_##name, PLUTOBOOK_PAGE_HEIGHT_##name})

#define PLUTOBOOK_PAGE_WIDTH_NONE 0.f
#define PLUTOBOOK_PAGE_HEIGHT_NONE 0.f
#define PLUTOBOOK_PAGE_SIZE_NONE PLUTOBOOK_PAGE_SIZE_NAMED(NONE)

#define PLUTOBOOK_PAGE_WIDTH_A3 (297 * PLUTOBOOK_UNITS_MM)
#define PLUTOBOOK_PAGE_HEIGHT_A3 (420 * PLUTOBOOK_UNITS_MM)
#define PLUTOBOOK_PAGE_SIZE_A3 PLUTOBOOK_PAGE_SIZE_NAMED(A3)

#define PLUTOBOOK_PAGE_WIDTH_A4 (210 * PLUTOBOOK_UNITS_MM)
#define PLUTOBOOK_PAGE_HEIGHT_A4 (297 * PLUTOBOOK_UNITS_MM)
#define PLUTOBOOK_PAGE_SIZE_A4 PLUTOBOOK_PAGE_SIZE_NAMED(A4)

#define PLUTOBOOK_PAGE_WIDTH_A5 (148 * PLUTOBOOK_UNITS_MM)
#define PLUTOBOOK_PAGE_HEIGHT_A5 (210 * PLUTOBOOK_UNITS_MM)
#define PLUTOBOOK_PAGE_SIZE_A5 PLUTOBOOK_PAGE_SIZE_NAMED(A5)

#define PLUTOBOOK_PAGE_WIDTH_B4 (250 * PLUTOBOOK_UNITS_MM)
#define PLUTOBOOK_PAGE_HEIGHT_B4 (353 * PLUTOBOOK_UNITS_MM)
#define PLUTOBOOK_PAGE_SIZE_B4 PLUTOBOOK_PAGE_SIZE_NAMED(B4)

#define PLUTOBOOK_PAGE_WIDTH_B5 (176 * PLUTOBOOK_UNITS_MM)
#define PLUTOBOOK_PAGE_HEIGHT_B5 (250 * PLUTOBOOK_UNITS_MM)
#define PLUTOBOOK_PAGE_SIZE_B5 PLUTOBOOK_PAGE_SIZE_NAMED(B5)

#define PLUTOBOOK_PAGE_WIDTH_LETTER (8.5f * PLUTOBOOK_UNITS_IN)
#define PLUTOBOOK_PAGE_HEIGHT_LETTER (11 * PLUTOBOOK_UNITS_IN)
#define PLUTOBOOK_PAGE_SIZE_LETTER PLUTOBOOK_PAGE_SIZE_NAMED(LETTER)

#define PLUTOBOOK_PAGE_WIDTH_LEGAL (8.5f * PLUTOBOOK_UNITS_IN)
#define PLUTOBOOK_PAGE_HEIGHT_LEGAL (14 * PLUTOBOOK_UNITS_IN)
#define PLUTOBOOK_PAGE_SIZE_LEGAL PLUTOBOOK_PAGE_SIZE_NAMED(LEGAL)

#define PLUTOBOOK_PAGE_WIDTH_LEDGER (11 * PLUTOBOOK_UNITS_IN)
#define PLUTOBOOK_PAGE_HEIGHT_LEDGER (17 * PLUTOBOOK_UNITS_IN)
#define PLUTOBOOK_PAGE_SIZE_LEDGER PLUTOBOOK_PAGE_SIZE_NAMED(LEDGER)

/**
 * @brief Represents the margins of a page in points (1/72 inch).
 */
typedef struct _plutobook_page_margins {
    float top;
    float right;
    float bottom;
    float left;
} plutobook_page_margins_t;

/**
 * @brief Predefined plutobook_page_margins_t objects for common margin settings.
 */
#define PLUTOBOOK_PAGE_MARGINS_NONE ((plutobook_page_margins_t){0, 0, 0, 0})
#define PLUTOBOOK_PAGE_MARGINS_NORMAL ((plutobook_page_margins_t){72, 72, 72, 72})
#define PLUTOBOOK_PAGE_MARGINS_NARROW ((plutobook_page_margins_t){36, 36, 36, 36})
#define PLUTOBOOK_PAGE_MARGINS_MODERATE ((plutobook_page_margins_t){72, 54, 72, 54})
#define PLUTOBOOK_PAGE_MARGINS_WIDE ((plutobook_page_margins_t){72, 144, 72, 144})

typedef enum _plutobook_log_level {
    PLUTOBOOK_LOG_LEVEL_TRACE = 0,
    PLUTOBOOK_LOG_LEVEL_DEBUG = 1,
    PLUTOBOOK_LOG_LEVEL_INFO = 2,
    PLUTOBOOK_LOG_LEVEL_WARN = 3,
    PLUTOBOOK_LOG_LEVEL_ERROR = 4,
    PLUTOBOOK_LOG_LEVEL_CRITICAL = 5,
    PLUTOBOOK_LOG_LEVEL_OFF = 6
} plutobook_log_level_t;

PLUTOBOOK_API plutobook_log_level_t plutobook_get_log_level(void);
PLUTOBOOK_API void plutobook_set_log_level(plutobook_log_level_t level);

typedef enum _plutobook_status {
    PLUTOBOOK_STATUS_SUCCESS = 0,
    PLUTOBOOK_STATUS_MEMORY_ERROR = 1,
    PLUTOBOOK_STATUS_LOAD_ERROR = 10,
    PLUTOBOOK_STATUS_WRITE_ERROR = 11,
    PLUTOBOOK_STATUS_CANVAS_ERROR = 12
} plutobook_status_t;

typedef plutobook_status_t (*plutobook_stream_write_callback_t)(void* closure, const char* data, unsigned int length);

typedef struct _plutobook_canvas plutobook_canvas_t;
typedef struct _cairo_surface cairo_surface_t;
typedef struct _cairo cairo_t;

PLUTOBOOK_API void plutobook_canvas_destroy(plutobook_canvas_t* canvas);
PLUTOBOOK_API void plutobook_canvas_flush(plutobook_canvas_t* canvas);
PLUTOBOOK_API void plutobook_canvas_finish(plutobook_canvas_t* canvas);
PLUTOBOOK_API void plutobook_canvas_translate(plutobook_canvas_t* canvas, float tx, float ty);
PLUTOBOOK_API void plutobook_canvas_scale(plutobook_canvas_t* canvas, float sx, float sy);
PLUTOBOOK_API void plutobook_canvas_rotate(plutobook_canvas_t* canvas, float angle);
PLUTOBOOK_API void plutobook_canvas_transform(plutobook_canvas_t* canvas, float a, float b, float c, float d, float e, float f);
PLUTOBOOK_API void plutobook_canvas_set_matrix(plutobook_canvas_t* canvas, float a, float b, float c, float d, float e, float f);
PLUTOBOOK_API void plutobook_canvas_reset_matrix(plutobook_canvas_t* canvas);
PLUTOBOOK_API void plutobook_canvas_clip_rect(plutobook_canvas_t* canvas, float x, float y, float width, float height);
PLUTOBOOK_API void plutobook_canvas_clear_surface(plutobook_canvas_t* canvas, float red, float green, float blue, float alpha);
PLUTOBOOK_API void plutobook_canvas_save_state(plutobook_canvas_t* canvas);
PLUTOBOOK_API void plutobook_canvas_restore_state(plutobook_canvas_t* canvas);
PLUTOBOOK_API cairo_surface_t* plutobook_canvas_get_surface(const plutobook_canvas_t* canvas);
PLUTOBOOK_API cairo_t* plutobook_canvas_get_context(const plutobook_canvas_t* canvas);
PLUTOBOOK_API plutobook_status_t plutobook_canvas_get_status(const plutobook_canvas_t* canvas);

typedef enum _plutobook_image_format {
    PLUTOBOOK_IMAGE_FORMAT_INVALID = -1,
    PLUTOBOOK_IMAGE_FORMAT_ARGB32 = 0,
    PLUTOBOOK_IMAGE_FORMAT_RGB24 = 1,
    PLUTOBOOK_IMAGE_FORMAT_A8 = 2,
    PLUTOBOOK_IMAGE_FORMAT_A1 = 3
} plutobook_image_format_t;

PLUTOBOOK_API plutobook_canvas_t* plutobook_image_canvas_create(int width, int height, plutobook_image_format_t format);
PLUTOBOOK_API plutobook_canvas_t* plutobook_image_canvas_create_for_data(unsigned char* data, int width, int height, int stride, plutobook_image_format_t format);
PLUTOBOOK_API unsigned char* plutobook_image_canvas_get_data(const plutobook_canvas_t* canvas);
PLUTOBOOK_API plutobook_image_format_t plutobook_image_canvas_get_format(const plutobook_canvas_t* canvas);
PLUTOBOOK_API int plutobook_image_canvas_get_width(const plutobook_canvas_t* canvas);
PLUTOBOOK_API int plutobook_image_canvas_get_height(const plutobook_canvas_t* canvas);
PLUTOBOOK_API int plutobook_image_canvas_get_stride(const plutobook_canvas_t* canvas);
PLUTOBOOK_API plutobook_status_t plutobook_image_canvas_write_to_png(const plutobook_canvas_t* canvas, const char* filename);
PLUTOBOOK_API plutobook_status_t plutobook_image_canvas_write_to_png_stream(const plutobook_canvas_t* canvas, plutobook_stream_write_callback_t callback, void* closure);

typedef enum _plutobook_pdf_metadata {
    PLUTOBOOK_PDF_METADATA_TITLE,
    PLUTOBOOK_PDF_METADATA_AUTHOR,
    PLUTOBOOK_PDF_METADATA_SUBJECT,
    PLUTOBOOK_PDF_METADATA_KEYWORDS,
    PLUTOBOOK_PDF_METADATA_CREATOR,
    PLUTOBOOK_PDF_METADATA_CREATION_DATE,
    PLUTOBOOK_PDF_METADATA_MODIFICATION_DATE
} plutobook_pdf_metadata_t;

PLUTOBOOK_API plutobook_canvas_t* plutobook_pdf_canvas_create(const char* filename, plutobook_page_size_t size);
PLUTOBOOK_API plutobook_canvas_t* plutobook_pdf_canvas_create_for_stream(plutobook_stream_write_callback_t callback, void* closure, plutobook_page_size_t size);
PLUTOBOOK_API void plutobook_pdf_canvas_set_metadata(plutobook_canvas_t* canvas, plutobook_pdf_metadata_t name, const char* value);
PLUTOBOOK_API void plutobook_pdf_canvas_set_size(plutobook_canvas_t* canvas, plutobook_page_size_t size);
PLUTOBOOK_API void plutobook_pdf_canvas_show_page(plutobook_canvas_t* canvas);

typedef struct _plutobook_resource_data plutobook_resource_data_t;

PLUTOBOOK_API plutobook_resource_data_t* plutobook_resource_data_create(const char* content, unsigned int content_length, const char* mime_type, const char* text_encoding);
PLUTOBOOK_API plutobook_resource_data_t* plutobook_resource_data_reference(plutobook_resource_data_t* resource);
PLUTOBOOK_API void plutobook_resource_data_destroy(plutobook_resource_data_t* resource);
PLUTOBOOK_API unsigned int plutobook_resource_data_get_reference_count(const plutobook_resource_data_t* resource);
PLUTOBOOK_API const char* plutobook_resource_data_get_content(const plutobook_resource_data_t* resource);
PLUTOBOOK_API unsigned int plutobook_resource_data_get_content_length(const plutobook_resource_data_t* resource);
PLUTOBOOK_API const char* plutobook_resource_data_get_mime_type(const plutobook_resource_data_t* resource);
PLUTOBOOK_API const char* plutobook_resource_data_get_text_encoding(const plutobook_resource_data_t* resource);

typedef plutobook_resource_data_t* (*plutobook_resource_load_callback_t)(void* closure, const char* url);

PLUTOBOOK_API plutobook_resource_data_t* plutobook_default_resource_fetcher_load_url(const char* url);
PLUTOBOOK_API void plutobook_set_custom_resource_fetcher(plutobook_resource_load_callback_t callback, void* closure);
PLUTOBOOK_API plutobook_resource_load_callback_t plutobook_get_custom_resource_fetcher_callback(void);
PLUTOBOOK_API void* plutobook_get_custom_resource_fetcher_closure(void);

typedef enum _plutobook_media_type {
    PLUTOBOOK_MEDIA_TYPE_PRINT,
    PLUTOBOOK_MEDIA_TYPE_SCREEN
} plutobook_media_type_t;

typedef struct _plutobook plutobook_t;

PLUTOBOOK_API plutobook_t* plutobook_create(plutobook_page_size_t size, plutobook_page_margins_t margins, plutobook_media_type_t media);
PLUTOBOOK_API void plutobook_destroy(plutobook_t* book);
PLUTOBOOK_API void plutobook_clear_content(plutobook_t* book);

PLUTOBOOK_API void plutobook_set_metadata(plutobook_t* book, plutobook_pdf_metadata_t name, const char* value);
PLUTOBOOK_API const char* plutobook_get_metadata(const plutobook_t* book, plutobook_pdf_metadata_t name);

PLUTOBOOK_API float plutobook_get_viewport_width(const plutobook_t* book);
PLUTOBOOK_API float plutobook_get_viewport_height(const plutobook_t* book);
PLUTOBOOK_API float plutobook_get_document_width(const plutobook_t* book);
PLUTOBOOK_API float plutobook_get_document_height(const plutobook_t* book);

PLUTOBOOK_API plutobook_page_size_t plutobook_get_page_size(const plutobook_t* book);
PLUTOBOOK_API plutobook_page_margins_t plutobook_get_page_margins(const plutobook_t* book);
PLUTOBOOK_API plutobook_page_size_t plutobook_get_page_size_at(const plutobook_t* book, unsigned int index);
PLUTOBOOK_API plutobook_media_type_t plutobook_get_media_type(const plutobook_t* book);
PLUTOBOOK_API unsigned int plutobook_get_page_count(const plutobook_t* book);

PLUTOBOOK_API plutobook_status_t plutobook_load_url(plutobook_t* book, const char* url, const char* user_style, const char* user_script);
PLUTOBOOK_API plutobook_status_t plutobook_load_data(plutobook_t* book, const char* data, unsigned int length, const char* mime_type, const char* text_encoding, const char* user_style, const char* user_script, const char* base_url);
PLUTOBOOK_API plutobook_status_t plutobook_load_image(plutobook_t* book, const char* data, unsigned int length, const char* mime_type, const char* text_encoding, const char* user_style, const char* user_script, const char* base_url);
PLUTOBOOK_API plutobook_status_t plutobook_load_xml(plutobook_t* book, const char* data, int length, const char* user_style, const char* user_script, const char* base_url);
PLUTOBOOK_API plutobook_status_t plutobook_load_html(plutobook_t* book, const char* data, int length, const char* user_style, const char* user_script, const char* base_url);

PLUTOBOOK_API void plutobook_render_page(const plutobook_t* book, plutobook_canvas_t* canvas, unsigned int page_index);
PLUTOBOOK_API void plutobook_render_page_cairo(const plutobook_t* book, cairo_t* context, unsigned int page_index);
PLUTOBOOK_API void plutobook_render_document(const plutobook_t* book, plutobook_canvas_t* canvas);
PLUTOBOOK_API void plutobook_render_document_cairo(const plutobook_t* book, cairo_t* context);
PLUTOBOOK_API void plutobook_render_document_rect(const plutobook_t* book, plutobook_canvas_t* canvas, float x, float y, float width, float height);
PLUTOBOOK_API void plutobook_render_document_rect_cairo(const plutobook_t* book, cairo_t* context, float x, float y, float width, float height);

PLUTOBOOK_API plutobook_status_t plutobook_write_to_pdf(const plutobook_t* book, const char* filename);
PLUTOBOOK_API plutobook_status_t plutobook_write_to_pdf_range(const plutobook_t* book, const char* filename, unsigned int from_page, unsigned int to_page, int page_step);
PLUTOBOOK_API plutobook_status_t plutobook_write_to_pdf_stream(const plutobook_t* book, plutobook_stream_write_callback_t callback, void* closure);
PLUTOBOOK_API plutobook_status_t plutobook_write_to_pdf_stream_range(const plutobook_t* book, plutobook_stream_write_callback_t callback, void* closure, unsigned int from_page, unsigned int to_page, int page_step);

PLUTOBOOK_API plutobook_status_t plutobook_write_to_png(const plutobook_t* book, const char* filename, plutobook_image_format_t format);
PLUTOBOOK_API plutobook_status_t plutobook_write_to_png_stream(const plutobook_t* book, plutobook_stream_write_callback_t callback, void* closure, plutobook_image_format_t format);

#ifdef __cplusplus
}
#endif

#endif // PLUTOBOOK_H
