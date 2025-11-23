/*
 * Copyright (c) 2022-2025 Samuel Ugochukwu <sammycageagle@gmail.com>
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

#include <stdbool.h>

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

#if defined(__GNUC__) && (__GNUC__ > 2)
#define PLUTOBOOK_PRINTF_FORMAT(fmt_index, va_index) __attribute__((format(printf, fmt_index, va_index)))
#else
#define PLUTOBOOK_PRINTF_FORMAT(fmt_index, va_index)
#endif

#define PLUTOBOOK_VERSION_MAJOR 0
#define PLUTOBOOK_VERSION_MINOR 11
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
 * @brief Returns a string containing the build metadata of the plutobook library.
 * @return A pointer to a string containing the build metadata of the plutobook library.
 */
PLUTOBOOK_API const char* plutobook_build_info(void);

/**
 * This macro defines an index that is guaranteed to exceed the valid page count.
 * It is typically utilized as a sentinel value to represent an unbounded or maximum value, indicating
 * that there is no limit or that the maximum possible value is intended.
 */
#define PLUTOBOOK_MAX_PAGE_COUNT 0xFFFFFFFFU

/**
 * This macro defines an index that is guaranteed to be below the valid page count.
 * It is typically utilized as a sentinel value to represent an unbounded or minimum value, indicating
 * that there is no lower limit or that the minimum possible value is intended.
 */
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

#ifdef __cplusplus
#define PLUTOBOOK_MAKE_STRUCT(type, ...) type{__VA_ARGS__}
#else
#define PLUTOBOOK_MAKE_STRUCT(type, ...) (type){__VA_ARGS__}
#endif

/**
 * @brief Defines the dimensions of a page in points (1/72 inch).
 */
typedef struct _plutobook_page_size {
    float width;  /**< Page width in points */
    float height; /**< Page height in points */
} plutobook_page_size_t;

#define PLUTOBOOK_MAKE_PAGE_SIZE(width, height) PLUTOBOOK_MAKE_STRUCT(plutobook_page_size_t, width, height)

/**
 * @brief Predefined macros for common paper sizes.
 */
#define PLUTOBOOK_PAGE_SIZE_NAMED(name) PLUTOBOOK_MAKE_PAGE_SIZE(PLUTOBOOK_PAGE_WIDTH_##name, PLUTOBOOK_PAGE_HEIGHT_##name)

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
 * @brief Defines the margins of a page in points (1/72 inch).
 */
typedef struct _plutobook_page_margins {
    float top;    /**< Top margin in points */
    float right;  /**< Right margin in points */
    float bottom; /**< Bottom margin in points */
    float left;   /**< Left margin in points */
} plutobook_page_margins_t;

#define PLUTOBOOK_MAKE_PAGE_MARGINS(top, right, bottom, left) PLUTOBOOK_MAKE_STRUCT(plutobook_page_margins_t, top, right, bottom, left)

/**
 * @brief Predefined macros for common margin settings.
 */
#define PLUTOBOOK_PAGE_MARGINS_NONE PLUTOBOOK_MAKE_PAGE_MARGINS(0, 0, 0, 0)
#define PLUTOBOOK_PAGE_MARGINS_NORMAL PLUTOBOOK_MAKE_PAGE_MARGINS(72, 72, 72, 72)
#define PLUTOBOOK_PAGE_MARGINS_NARROW PLUTOBOOK_MAKE_PAGE_MARGINS(36, 36, 36, 36)
#define PLUTOBOOK_PAGE_MARGINS_MODERATE PLUTOBOOK_MAKE_PAGE_MARGINS(72, 54, 72, 54)
#define PLUTOBOOK_PAGE_MARGINS_WIDE PLUTOBOOK_MAKE_PAGE_MARGINS(72, 144, 72, 144)

/**
 * @brief Defines status codes that indicate the result of a stream operation.
 */
typedef enum _plutobook_stream_status {
    PLUTOBOOK_STREAM_STATUS_SUCCESS = 0,
    PLUTOBOOK_STREAM_STATUS_READ_ERROR = 10,
    PLUTOBOOK_STREAM_STATUS_WRITE_ERROR = 11,
} plutobook_stream_status_t;

/**
 * @brief This type represents a function called when writing data to an output stream.
 *
 * @param closure user-defined closure for the callback.
 * @param data buffer containing the data to write.
 * @param length the number of bytes to write.
 * @return `PLUTOBOOK_STREAM_STATUS_SUCCESS` on success, or `PLUTOBOOK_STREAM_STATUS_WRITE_ERROR` on failure.
 */
typedef plutobook_stream_status_t (*plutobook_stream_write_callback_t)(void* closure, const char* data, unsigned int length);

typedef struct _cairo_surface cairo_surface_t;
typedef struct _cairo cairo_t;

/**
 * @brief Represents a 2D drawing interface for creating and manipulating graphical content.
 */
typedef struct _plutobook_canvas plutobook_canvas_t;

/**
 * @brief Destroys the canvas and frees its associated resources.
 *
 * @param canvas A pointer to a `plutobook_canvas_t` object.
 */
PLUTOBOOK_API void plutobook_canvas_destroy(plutobook_canvas_t* canvas);

/**
 * @brief Flushes any pending drawing operations on the canvas.
 *
 * @param canvas A pointer to a `plutobook_canvas_t` object.
 */
PLUTOBOOK_API void plutobook_canvas_flush(plutobook_canvas_t* canvas);

/**
 * @brief Finishes all drawing operations and performs cleanup on the canvas.
 *
 * @param canvas A pointer to a `plutobook_canvas_t` object.
 */
PLUTOBOOK_API void plutobook_canvas_finish(plutobook_canvas_t* canvas);

/**
 * @brief Translates the canvas by a given offset, moving its origin.
 *
 * @param canvas A pointer to a `plutobook_canvas_t` object.
 * @param tx The horizontal translation offset.
 * @param ty The vertical translation offset.
 */
PLUTOBOOK_API void plutobook_canvas_translate(plutobook_canvas_t* canvas, float tx, float ty);

/**
 * @brief Scales the canvas by the specified factors in the horizontal and vertical directions.
 *
 * @param canvas A pointer to a `plutobook_canvas_t` object.
 * @param sx The scaling factor in the horizontal direction.
 * @param sy The scaling factor in the vertical direction.
 */
PLUTOBOOK_API void plutobook_canvas_scale(plutobook_canvas_t* canvas, float sx, float sy);

/**
 * @brief Rotates the canvas around the current origin by the specified angle.
 *
 * @param canvas A pointer to a `plutobook_canvas_t` object.
 * @param angle The rotation angle in radians.
 */
PLUTOBOOK_API void plutobook_canvas_rotate(plutobook_canvas_t* canvas, float angle);

/**
 * @brief Multiplies the current transformation matrix with the specified matrix.
 *
 * @param canvas A pointer to a `plutobook_canvas_t` object.
 * @param a The horizontal scaling factor.
 * @param b The horizontal skewing factor.
 * @param c The vertical skewing factor.
 * @param d The vertical scaling factor.
 * @param e The horizontal translation offset.
 * @param f The vertical translation offset.
 */
PLUTOBOOK_API void plutobook_canvas_transform(plutobook_canvas_t* canvas, float a, float b, float c, float d, float e, float f);

/**
 * @brief Resets the transformation matrix to the specified matrix.
 *
 * @param canvas A pointer to a `plutobook_canvas_t` object.
 * @param a The horizontal scaling factor.
 * @param b The horizontal skewing factor.
 * @param c The vertical skewing factor.
 * @param d The vertical scaling factor.
 * @param e The horizontal translation offset.
 * @param f The vertical translation offset.
 */
PLUTOBOOK_API void plutobook_canvas_set_matrix(plutobook_canvas_t* canvas, float a, float b, float c, float d, float e, float f);

/**
 * @brief Resets the current transformation matrix to the identity matrix.
 *
 * @param canvas A pointer to a `plutobook_canvas_t` object.
 */
PLUTOBOOK_API void plutobook_canvas_reset_matrix(plutobook_canvas_t* canvas);

/**
 * @brief Intersects the current clip region with the specified rectangle.
 *
 * @param canvas A pointer to a `plutobook_canvas_t` object.
 * @param x The x-coordinate of the rectangle’s top-left corner.
 * @param y The y-coordinate of the rectangle’s top-left corner.
 * @param width The width of the rectangle.
 * @param height The height of the rectangle.
 */
PLUTOBOOK_API void plutobook_canvas_clip_rect(plutobook_canvas_t* canvas, float x, float y, float width, float height);

/**
 * @brief Clears the canvas surface with the specified color.
 *
 * @param canvas A pointer to a `plutobook_canvas_t` object.
 * @param red The red component of the color, in the range [0, 1].
 * @param green The green component of the color, in the range [0, 1].
 * @param blue The blue component of the color, in the range [0, 1].
 * @param alpha The alpha (transparency) component of the color, in the range [0, 1].
 */
PLUTOBOOK_API void plutobook_canvas_clear_surface(plutobook_canvas_t* canvas, float red, float green, float blue, float alpha);

/**
 * @brief Saves the current state of the canvas.
 *
 * @param canvas A pointer to a `plutobook_canvas_t` object.
 */
PLUTOBOOK_API void plutobook_canvas_save_state(plutobook_canvas_t* canvas);

/**
 * @brief Restores the most recently saved state of the canvas.
 *
 * @param canvas A pointer to a `plutobook_canvas_t` object.
 */
PLUTOBOOK_API void plutobook_canvas_restore_state(plutobook_canvas_t* canvas);

/**
 * @brief Gets the underlying cairo surface associated with the canvas.
 *
 * @param canvas A pointer to a `plutobook_canvas_t` object.
 * @return A pointer to the underlying `cairo_surface_t` object.
 */
PLUTOBOOK_API cairo_surface_t* plutobook_canvas_get_surface(const plutobook_canvas_t* canvas);

/**
 * @brief Gets the underlying cairo context associated with the canvas.
 *
 * @param canvas A pointer to a `plutobook_canvas_t` object.
 * @return A pointer to the underlying `cairo_t` object.
 */
PLUTOBOOK_API cairo_t* plutobook_canvas_get_context(const plutobook_canvas_t* canvas);

/**
 * @brief Defines different memory formats for image data.
 */
typedef enum _plutobook_image_format {
    PLUTOBOOK_IMAGE_FORMAT_INVALID = -1,
    PLUTOBOOK_IMAGE_FORMAT_ARGB32 = 0,
    PLUTOBOOK_IMAGE_FORMAT_RGB24 = 1,
    PLUTOBOOK_IMAGE_FORMAT_A8 = 2,
    PLUTOBOOK_IMAGE_FORMAT_A1 = 3
} plutobook_image_format_t;

/**
 * @brief Creates a new canvas for drawing to image data with the specified dimensions and format.
 *
 * @param width The width of the image in pixels.
 * @param height The height of the image in pixels.
 * @param format The image format used for the canvas.
 * @return A pointer to a newly created `plutobook_canvas_t` object, or `NULL` on failure.
 */
PLUTOBOOK_API plutobook_canvas_t* plutobook_image_canvas_create(int width, int height, plutobook_image_format_t format);

/**
 * @brief Creates a new canvas for drawing to existing image data.
 *
 * @param data A pointer to the raw image data.
 * @param width The width of the image in pixels.
 * @param height The height of the image in pixels.
 * @param stride The number of bytes in one row of the image, including padding.
 * @param format The image format used for the canvas.
 * @return A pointer to a newly created `plutobook_canvas_t` object, or `NULL` on failure.
 */
PLUTOBOOK_API plutobook_canvas_t* plutobook_image_canvas_create_for_data(unsigned char* data, int width, int height, int stride, plutobook_image_format_t format);

/**
 * @brief Retrieves the image data from the canvas.
 *
 * @param canvas A pointer to a `plutobook_canvas_t` object.
 * @return A pointer to the image data.
 */
PLUTOBOOK_API unsigned char* plutobook_image_canvas_get_data(const plutobook_canvas_t* canvas);

/**
 * @brief Retrieves the format of the image data on the canvas.
 *
 * @param canvas A pointer to a `plutobook_canvas_t` object.
 * @return The image format of the canvas.
 */
PLUTOBOOK_API plutobook_image_format_t plutobook_image_canvas_get_format(const plutobook_canvas_t* canvas);

/**
 * @brief Retrieves the width of the image data on the canvas.
 *
 * @param canvas A pointer to a `plutobook_canvas_t` object.
 * @return The width of the canvas image in pixels.
 */
PLUTOBOOK_API int plutobook_image_canvas_get_width(const plutobook_canvas_t* canvas);

/**
 * @brief Retrieves the height of the image data on the canvas.
 *
 * @param canvas A pointer to a `plutobook_canvas_t` object.
 * @return The height of the canvas image in pixels.
 */
PLUTOBOOK_API int plutobook_image_canvas_get_height(const plutobook_canvas_t* canvas);

/**
 * @brief Retrieves the stride (the number of bytes per row) of the image data on the canvas.
 *
 * @param canvas A pointer to a `plutobook_canvas_t` object.
 * @return The stride of the canvas image in bytes.
 */
PLUTOBOOK_API int plutobook_image_canvas_get_stride(const plutobook_canvas_t* canvas);

/**
 * @brief Writes the image data from the canvas to a PNG file.
 *
 * @param canvas A pointer to a `plutobook_canvas_t` object.
 * @param filename The path to the file where the PNG image will be saved.
 * @return `true` on success, or `false` on failure.
 */
PLUTOBOOK_API bool plutobook_image_canvas_write_to_png(const plutobook_canvas_t* canvas, const char* filename);

/**
 * @brief Writes the image data from the canvas to a PNG stream using a custom write callback.
 *
 * @param canvas A pointer to a `plutobook_canvas_t` object.
 * @param callback The callback function for writing the image data to a stream.
 * @param closure A user-defined closure passed to the callback.
 * @return `true` on success, or `false` on failure.
 */
PLUTOBOOK_API bool plutobook_image_canvas_write_to_png_stream(const plutobook_canvas_t* canvas, plutobook_stream_write_callback_t callback, void* closure);

/**
 * @brief Defines different metadata fields for a PDF document.
 */
typedef enum _plutobook_pdf_metadata {
    PLUTOBOOK_PDF_METADATA_TITLE,
    PLUTOBOOK_PDF_METADATA_AUTHOR,
    PLUTOBOOK_PDF_METADATA_SUBJECT,
    PLUTOBOOK_PDF_METADATA_KEYWORDS,
    PLUTOBOOK_PDF_METADATA_CREATOR,
    PLUTOBOOK_PDF_METADATA_CREATION_DATE,
    PLUTOBOOK_PDF_METADATA_MODIFICATION_DATE
} plutobook_pdf_metadata_t;

/**
 * @brief Creates a new canvas for generating a PDF file.
 *
 * @param filename A path to the output PDF file.
 * @param size The page size for the PDF.
 * @return A pointer to a newly created `plutobook_canvas_t` object, or `NULL` on failure.
 */
PLUTOBOOK_API plutobook_canvas_t* plutobook_pdf_canvas_create(const char* filename, plutobook_page_size_t size);

/**
 * @brief Creates a new canvas for generating a PDF and writes it to a stream.
 *
 * @param callback A callback function to write the data to a stream.
 * @param closure A user-defined pointer passed to the callback function.
 * @param size The page size for the PDF.
 * @return A pointer to a newly created `plutobook_canvas_t` object, or `NULL` on failure.
 */
PLUTOBOOK_API plutobook_canvas_t* plutobook_pdf_canvas_create_for_stream(plutobook_stream_write_callback_t callback, void* closure, plutobook_page_size_t size);

/**
 * @brief Sets the metadata of the PDF document.
 *
 * The `PDF_METADATA_CREATION_DATE` and `PDF_METADATA_MODIFICATION_DATE` values must be in ISO-8601 format: YYYY-MM-DDThh:mm:ss.
 * An optional timezone of the form "[+/-]hh:mm" or "Z" for UTC time can be appended. All other metadata values can be any string.
 *
 * @param canvas A pointer to a `plutobook_canvas_t` object.
 * @param metadata The metadata type to set.
 * @param value The value of the metadata field.
 */
PLUTOBOOK_API void plutobook_pdf_canvas_set_metadata(plutobook_canvas_t* canvas, plutobook_pdf_metadata_t metadata, const char* value);

/**
 * @brief Sets the size of the PDF page.
 *
 * This function should only be called before any drawing operations are performed on the current page.
 * The simplest way to do this is by calling this function immediately after creating the canvas or 
 * immediately after completing a page with `plutobook_pdf_canvas_show_page`.
 *
 * @param canvas A pointer to a `plutobook_canvas_t` object.
 * @param size The desired size of the PDF page.
 */
PLUTOBOOK_API void plutobook_pdf_canvas_set_size(plutobook_canvas_t* canvas, plutobook_page_size_t size);

/**
 * @brief Finalizes the current page and prepares the canvas for a new page.
 *
 * @param canvas A pointer to a `plutobook_canvas_t` object.
 */
PLUTOBOOK_API void plutobook_pdf_canvas_show_page(plutobook_canvas_t* canvas);

/**
 * @brief A callback function type for resource data destruction.
 *
 * @param data A pointer to the resource data to be destroyed.
 */
typedef void (*plutobook_resource_destroy_callback_t)(void* data);

/**
 * @brief A structure representing the resource data.
 */
typedef struct _plutobook_resource_data plutobook_resource_data_t;

/**
 * @brief Creates a new `plutobook_resource_data_t` object by copying the provided content.
 *
 * @param content The content of the resource.
 * @param content_length The length of the content in bytes.
 * @param mime_type The MIME type of the content.
 * @param text_encoding The text encoding used for the content.
 * @return A pointer to a newly created `plutobook_resource_data_t` object, or `NULL` on failure.
 */
PLUTOBOOK_API plutobook_resource_data_t* plutobook_resource_data_create(const char* content, unsigned int content_length, const char* mime_type, const char* text_encoding);

/**
 * @brief Creates a new `plutobook_resource_data_t` object using the provided content "as is", and uses the `destroy_callback` to free the resource when it's no longer needed.
 *
 * @param content The content of the resource.
 * @param content_length The length of the content in bytes.
 * @param mime_type The MIME type of the content.
 * @param text_encoding The text encoding used for the content.
 * @param destroy_callback A callback function that will be called to free the resource when it's no longer needed.
 * @param closure A user-defined pointer that will be passed to the `destroy_callback` when called.
 * @return A pointer to a newly created `plutobook_resource_data_t` object, or `NULL` on failure.
 */
PLUTOBOOK_API plutobook_resource_data_t* plutobook_resource_data_create_without_copy(const char* content, unsigned int content_length, const char* mime_type, const char* text_encoding, plutobook_resource_destroy_callback_t destroy_callback, void* closure);

/**
 * @brief Increases the reference count of a resource data object.
 *
 * This function returns a reference to the given `plutobook_resource_data_t` object, incrementing its reference count.
 * This is useful for managing the lifetime of resource data objects in a reference-counted system.
 *
 * @param resource A pointer to the `plutobook_resource_data_t` object to reference.
 * @return A pointer to the referenced `plutobook_resource_data_t` object.
 */
PLUTOBOOK_API plutobook_resource_data_t* plutobook_resource_data_reference(plutobook_resource_data_t* resource);

/**
 * @brief Destroys a resource data object and frees its associated memory.
 *
 * This function decreases the reference count of the `plutobook_resource_data_t` object, and if the reference count
 * drops to zero, the resource is destroyed and its memory is freed.
 *
 * @param resource A pointer to the `plutobook_resource_data_t` object to destroy.
 */
PLUTOBOOK_API void plutobook_resource_data_destroy(plutobook_resource_data_t* resource);

/**
 * @brief Gets the current reference count of a resource data object.
 *
 * This function returns the number of references to the `plutobook_resource_data_t` object, helping manage the
 * object's lifetime and determine if it can be safely destroyed.
 *
 * @param resource A pointer to the `plutobook_resource_data_t` object.
 * @return The current reference count of the `plutobook_resource_data_t` object.
 */
PLUTOBOOK_API unsigned int plutobook_resource_data_get_reference_count(const plutobook_resource_data_t* resource);

/**
 * @brief Retrieves the content of the resource.
 * @param resource A pointer to a `plutobook_resource_data_t` object.
 * @return The content of the resource.
 */
PLUTOBOOK_API const char* plutobook_resource_data_get_content(const plutobook_resource_data_t* resource);

/**
 * @brief Retrieves the length of the resource content.
 * @param resource A pointer to a `plutobook_resource_data_t` object.
 * @return The length of the resource content in bytes.
 */
PLUTOBOOK_API unsigned int plutobook_resource_data_get_content_length(const plutobook_resource_data_t* resource);

/**
 * @brief Retrieves the MIME type of the resource content.
 * @param resource A pointer to a `plutobook_resource_data_t` object.
 * @return The MIME type of the resource content.
 */
PLUTOBOOK_API const char* plutobook_resource_data_get_mime_type(const plutobook_resource_data_t* resource);

/**
 * @brief Retrieves the text encoding used for the resource content.
 * @param resource A pointer to a `plutobook_resource_data_t` object.
 * @return The text encoding used for the resource content.
 */
PLUTOBOOK_API const char* plutobook_resource_data_get_text_encoding(const plutobook_resource_data_t* resource);

/**
 * @brief Defines a callback type for fetching resource data from a URL.
 *
 * The callback should return a pointer to a `plutobook_resource_data_t` object, which contains the content fetched from the given URL.
 *
 * @param closure A user-defined pointer that will be passed to the callback (can be used for custom state or data).
 * @param url The URL of the resource to fetch.
 * @return A pointer to a `plutobook_resource_data_t` object containing the fetched resource, or `NULL` if an error occurs.
 */
typedef plutobook_resource_data_t* (*plutobook_resource_fetch_callback_t)(void* closure, const char* url);

/**
 * @brief Fetches resource data from a given URL using the default resource fetcher.
 *
 * This function uses a predefined mechanism to fetch resource data from the specified URL and return it as a `plutobook_resource_data_t` object.
 *
 * @param url The URL of the resource to fetch.
 * @return A pointer to a `plutobook_resource_data_t` object containing the fetched content, or `NULL` if the fetch operation fails.
 */
PLUTOBOOK_API plutobook_resource_data_t* plutobook_fetch_url(const char* url);

/**
 * @brief Sets the path to a file containing trusted CA certificates.
 *
 * If not set, no custom CA file is used.
 *
 * @param path Path to the CA certificate bundle file.
 */
PLUTOBOOK_API void plutobook_set_ssl_cainfo(const char* path);

/**
 * @brief Sets the path to a directory containing trusted CA certificates.
 *
 * If not set, no custom CA path is used.
 *
 * @param path Path to the directory with CA certificates.
 */
PLUTOBOOK_API void plutobook_set_ssl_capath(const char* path);

/**
 * @brief Enables or disables SSL peer certificate verification.
 *
 * If not set, verification is enabled by default.
 *
 * @param verify Set to true to verify the peer, false to disable verification.
 */
PLUTOBOOK_API void plutobook_set_ssl_verify_peer(bool verify);

/**
 * @brief Enables or disables SSL host name verification.
 *
 * If not set, verification is enabled by default.
 *
 * @param verify Set to true to verify the host, false to disable verification.
 */
PLUTOBOOK_API void plutobook_set_ssl_verify_host(bool verify);

/**
 * @brief Enables or disables automatic following of HTTP redirects.
 *
 * If not set, following redirects is enabled by default.
 *
 * @param follow Set to true to follow redirects, false to disable.
 */
PLUTOBOOK_API void plutobook_set_http_follow_redirects(bool follow);

/**
 * @brief Sets the maximum number of redirects to follow.
 *
 * If not set, the default maximum is 30.
 *
 * @param amount The maximum number of redirects.
 */
PLUTOBOOK_API void plutobook_set_http_max_redirects(int amount);

/**
 * @brief Sets the maximum time allowed for an HTTP request.
 *
 * If not set, the default timeout is 300 seconds.
 *
 * @param timeout Timeout duration in seconds.
 */
PLUTOBOOK_API void plutobook_set_http_timeout(int timeout);

/**
 * @brief Defines the different media types used for CSS @media queries.
 */
typedef enum _plutobook_media_type {
    PLUTOBOOK_MEDIA_TYPE_PRINT,
    PLUTOBOOK_MEDIA_TYPE_SCREEN
} plutobook_media_type_t;

/**
 * @brief Represents a plutobook document.
 */
typedef struct _plutobook plutobook_t;

/**
 * @brief Creates a new `plutobook_t` object with the specified page size, margins, and media type.
 *
 * @param size The initial page size.
 * @param margins The initial page margins.
 * @param media The media type used for media queries.
 * @return A pointer to the newly created `plutobook_t` object, or `NULL` on failure.
 */
PLUTOBOOK_API plutobook_t* plutobook_create(plutobook_page_size_t size, plutobook_page_margins_t margins, plutobook_media_type_t media);

/**
 * @brief Destroys a `plutobook_t` object and frees all associated resources.
 *
 * @param book A pointer to the `plutobook_t` object to destroy.
 */
PLUTOBOOK_API void plutobook_destroy(plutobook_t* book);

/**
 * @brief Clears the content of the document.
 *
 * @param book A pointer to a `plutobook_t` object.
 */
PLUTOBOOK_API void plutobook_clear_content(plutobook_t* book);

/**
 * @brief Sets the metadata of the PDF document.
 *
 * The `PDF_METADATA_CREATION_DATE` and `PDF_METADATA_MODIFICATION_DATE` values must be in ISO-8601 format: YYYY-MM-DDThh:mm:ss.
 * An optional timezone of the form "[+/-]hh:mm" or "Z" for UTC time can be appended. All other metadata values can be any string.
 *
 * @param book A pointer to a `plutobook_t` object.
 * @param metadata The metadata type to set.
 * @param value The value of the metadata field.
 */
PLUTOBOOK_API void plutobook_set_metadata(plutobook_t* book, plutobook_pdf_metadata_t metadata, const char* value);

/**
 * @brief Gets the value of the specified metadata.
 *
 * @param book A pointer to a `plutobook_t` object.
 * @param metadata The type of metadata to get.
 * @return The value of the specified metadata.
 */
PLUTOBOOK_API const char* plutobook_get_metadata(const plutobook_t* book, plutobook_pdf_metadata_t metadata);

/**
 * @brief Returns the width of the viewport.
 *
 * @param book A pointer to a `plutobook_t` object.
 * @return The width of the viewport in pixels.
 */
PLUTOBOOK_API float plutobook_get_viewport_width(const plutobook_t* book);

/**
 * @brief Returns the height of the viewport.
 *
 * @param book A pointer to a `plutobook_t` object.
 * @return The height of the viewport in pixels.
 */
PLUTOBOOK_API float plutobook_get_viewport_height(const plutobook_t* book);

/**
 * @brief Returns the width of the document.
 *
 * @param book A pointer to a `plutobook_t` object.
 * @return The width of the document in pixels.
 */
PLUTOBOOK_API float plutobook_get_document_width(const plutobook_t* book);

/**
 * @brief Returns the height of the document.
 *
 * @param book A pointer to a `plutobook_t` object.
 * @return The height of the document in pixels.
 */
PLUTOBOOK_API float plutobook_get_document_height(const plutobook_t* book);

/**
 * @brief Returns the initial page size.
 *
 * @param book A pointer to a `plutobook_t` object.
 * @return The initial page size.
 */
PLUTOBOOK_API plutobook_page_size_t plutobook_get_page_size(const plutobook_t* book);

/**
 * @brief Returns the initial page margins.
 *
 * @param book A pointer to a `plutobook_t` object.
 * @return The initial page margins.
 */
PLUTOBOOK_API plutobook_page_margins_t plutobook_get_page_margins(const plutobook_t* book);

/**
 * @brief Returns the media type used for media queries.
 *
 * @param book A pointer to a `plutobook_t` object.
 * @return The media type used for media queries.
 */
PLUTOBOOK_API plutobook_media_type_t plutobook_get_media_type(const plutobook_t* book);

/**
 * @brief Returns the number of pages in the document.
 *
 * @param book A pointer to a `plutobook_t` object.
 * @return The number of pages in the document.
 */
PLUTOBOOK_API unsigned int plutobook_get_page_count(const plutobook_t* book);

/**
 * @brief Returns the page size at the specified index.
 *
 * @param book A pointer to a `plutobook_t` object.
 * @param index The index of the page.
 * @return The size of the page at the specified index.
 */
PLUTOBOOK_API plutobook_page_size_t plutobook_get_page_size_at(const plutobook_t* book, unsigned int index);

/**
 * @brief Loads the document from the specified URL.
 *
 * @param book A pointer to a `plutobook_t` object.
 * @param url The URL to load the document from.
 * @param user_style An optional user-defined style to apply.
 * @param user_script An optional user-defined script to run after the document has loaded.
 * @return `true` on success, or `false` on failure.
 */
PLUTOBOOK_API bool plutobook_load_url(plutobook_t* book, const char* url, const char* user_style, const char* user_script);

/**
 * @brief Loads the document from the specified data.
 *
 * @param book A pointer to a `plutobook_t` object.
 * @param data The data to load the document from.
 * @param length The length of the data in bytes.
 * @param mime_type The MIME type of the data.
 * @param text_encoding The text encoding of the data.
 * @param user_style An optional user-defined style to apply.
 * @param user_script An optional user-defined script to run after the document has loaded.
 * @param base_url The base URL for resolving relative URLs.
 * @return `true` on success, or `false` on failure.
 */
PLUTOBOOK_API bool plutobook_load_data(plutobook_t* book, const char* data, unsigned int length, const char* mime_type, const char* text_encoding, const char* user_style, const char* user_script, const char* base_url);

/**
 * @brief Loads the document from the specified image data.
 *
 * @param book A pointer to a `plutobook_t` object.
 * @param data The image data to load the document from.
 * @param length The length of the image data in bytes.
 * @param mime_type The MIME type of the image data.
 * @param text_encoding The text encoding of the image data.
 * @param user_style An optional user-defined style to apply.
 * @param user_script An optional user-defined script to run after the document has loaded.
 * @param base_url The base URL for resolving relative URLs.
 * @return `true` on success, or `false` on failure.
 */
PLUTOBOOK_API bool plutobook_load_image(plutobook_t* book, const char* data, unsigned int length, const char* mime_type, const char* text_encoding, const char* user_style, const char* user_script, const char* base_url);

/**
 * @brief Loads the document from the specified XML data.
 *
 * @param book A pointer to a `plutobook_t` object.
 * @param data The XML data to load the document from, encoded in UTF-8.
 * @param length The length of the XML data in bytes, or `-1` if null-terminated.
 * @param user_style An optional user-defined style to apply.
 * @param user_script An optional user-defined script to run after the document has loaded.
 * @param base_url The base URL for resolving relative URLs.
 * @return `true` on success, or `false` on failure.
 */
PLUTOBOOK_API bool plutobook_load_xml(plutobook_t* book, const char* data, int length, const char* user_style, const char* user_script, const char* base_url);

/**
 * @brief Loads the document from the specified HTML data.
 *
 * @param book A pointer to a `plutobook_t` object.
 * @param data The HTML data to load the document from, encoded in UTF-8.
 * @param length The length of the HTML data in bytes, or `-1` if null-terminated.
 * @param user_style An optional user-defined style to apply.
 * @param user_script An optional user-defined script to run after the document has loaded.
 * @param base_url The base URL for resolving relative URLs.
 * @return `true` on success, or `false` on failure.
 */
PLUTOBOOK_API bool plutobook_load_html(plutobook_t* book, const char* data, int length, const char* user_style, const char* user_script, const char* base_url);

/**
 * @brief Renders the specified page to the given canvas.
 *
 * @param book A pointer to a `plutobook_t` object.
 * @param canvas The canvas to render the page on.
 * @param page_index The index of the page to render.
 */
PLUTOBOOK_API void plutobook_render_page(const plutobook_t* book, plutobook_canvas_t* canvas, unsigned int page_index);

/**
 * @brief Renders the specified page to the given cairo context.
 *
 * @param book A pointer to a `plutobook_t` object.
 * @param context The cairo context to render the page on.
 * @param page_index The index of the page to render.
 */
PLUTOBOOK_API void plutobook_render_page_cairo(const plutobook_t* book, cairo_t* context, unsigned int page_index);

/**
 * @brief Renders the entire document to the given canvas.
 *
 * @param book A pointer to a `plutobook_t` object.
 * @param canvas The canvas to render the entire document on.
 */
PLUTOBOOK_API void plutobook_render_document(const plutobook_t* book, plutobook_canvas_t* canvas);

/**
 * @brief Renders the entire document to the given cairo context.
 *
 * @param book A pointer to a `plutobook_t` object.
 * @param canvas The cairo context to render the entire document on.
 */
PLUTOBOOK_API void plutobook_render_document_cairo(const plutobook_t* book, cairo_t* context);

/**
 * @brief Renders a specific rectangular portion of the document to the given canvas.
 *
 * @param book A pointer to a `plutobook_t` object.
 * @param canvas The canvas to render the document portion on.
 * @param x The x-coordinate of the top-left corner of the rectangle.
 * @param y The y-coordinate of the top-left corner of the rectangle.
 * @param width The width of the rectangle to render.
 * @param height The height of the rectangle to render.
 */
PLUTOBOOK_API void plutobook_render_document_rect(const plutobook_t* book, plutobook_canvas_t* canvas, float x, float y, float width, float height);

/**
 * @brief Renders a specific rectangular portion of the document to the given cairo context.
 *
 * @param book A pointer to a `plutobook_t` object.
 * @param canvas The cairo context to render the document portion on.
 * @param x The x-coordinate of the top-left corner of the rectangle.
 * @param y The y-coordinate of the top-left corner of the rectangle.
 * @param width The width of the rectangle to render.
 * @param height The height of the rectangle to render.
 */
PLUTOBOOK_API void plutobook_render_document_rect_cairo(const plutobook_t* book, cairo_t* context, float x, float y, float width, float height);

/**
 * @brief Writes the entire document to a PDF file.
 *
 * @param book A pointer to a `plutobook_t` object.
 * @param filename The file path where the PDF document will be written.
 * @return `true` on success, or `false` on failure.
 */
PLUTOBOOK_API bool plutobook_write_to_pdf(const plutobook_t* book, const char* filename);

/**
 * @brief Writes a range of pages from the document to a PDF file.
 *
 * @param book A pointer to a `plutobook_t` object.
 * @param filename The file path where the PDF document will be written.
 * @param page_start The first page in the range to be written (inclusive).
 * @param page_end The last page in the range to be written (inclusive).
 * @param page_step The increment used to advance through the pages in the range.
 * @return `true` on success, or `false` on failure.
 */
PLUTOBOOK_API bool plutobook_write_to_pdf_range(const plutobook_t* book, const char* filename, unsigned int page_start, unsigned int page_end, int page_step);

/**
 * @brief Writes the entire document to a PDF stream using a callback function.
 *
 * @param book A pointer to a `plutobook_t` object.
 * @param callback A callback function used for writing the PDF stream.
 * @param closure A user-defined pointer passed to the callback function for additional data.
 * @return `true` on success, or `false` on failure.
 */
PLUTOBOOK_API bool plutobook_write_to_pdf_stream(const plutobook_t* book, plutobook_stream_write_callback_t callback, void* closure);

/**
 * @brief Writes a specified range of pages from the document to a PDF stream using a callback function.
 *
 * @param book A pointer to a `plutobook_t` object.
 * @param callback A callback function used for writing the PDF stream.
 * @param closure A user-defined pointer passed to the callback function for additional data.
 * @param page_start The first page in the range to be written (inclusive).
 * @param page_end The last page in the range to be written (inclusive).
 * @param page_step The increment used to advance through the pages in the range.
 * @return `true` on success, or `false` on failure.
 */
PLUTOBOOK_API bool plutobook_write_to_pdf_stream_range(const plutobook_t* book, plutobook_stream_write_callback_t callback, void* closure, unsigned int page_start, unsigned int page_end, int page_step);

/**
 * @brief Writes the entire document to a PNG image file.
 *
 * @param book A pointer to a `plutobook_t` object.
 * @param filename The file path where the PNG image will be written.
 * @param width The desired width in pixels, or -1 to auto-scale based on the document size.
 * @param height The desired height in pixels, or -1 to auto-scale based on the document size.
 * @return `true` on success, or `false` on failure.
 */
PLUTOBOOK_API bool plutobook_write_to_png(const plutobook_t* book, const char* filename, int width, int height);

/**
 * @brief Writes the entire document to a PNG image stream using a callback function.
 *
 * @param book A pointer to a `plutobook_t` object.
 * @param callback A callback function to handle the image data stream.
 * @param closure A pointer to user-defined data to pass to the callback function.
 * @param width The desired width in pixels, or -1 to auto-scale based on the document size.
 * @param height The desired height in pixels, or -1 to auto-scale based on the document size.
 * @return `true` on success, or `false` on failure.
 */
PLUTOBOOK_API bool plutobook_write_to_png_stream(const plutobook_t* book, plutobook_stream_write_callback_t callback, void* closure, int width, int height);

/**
 * @brief Sets a custom resource fetcher callback for the document.
 *
 * @param book A pointer to a `plutobook_t` object.
 * @param callback A function pointer to the custom resource fetch callback.
 * @param closure A pointer to user-defined data to pass to the callback function.
 */
PLUTOBOOK_API void plutobook_set_custom_resource_fetcher(plutobook_t* book, plutobook_resource_fetch_callback_t callback, void* closure);

/**
 * @brief Gets the custom resource fetcher callback set for the document.
 *
 * @param book A pointer to a `plutobook_t` object.
 * @return A function pointer to the custom resource fetch callback, or `NULL` if no callback is set.
 */
PLUTOBOOK_API plutobook_resource_fetch_callback_t plutobook_get_custom_resource_fetcher_callback(const plutobook_t* book);

/**
 * @brief Gets the user-defined closure data passed to the custom resource fetcher callback.
 *
 * @param book A pointer to a `plutobook_t` object.
 * @return A pointer to the closure data, or `NULL` if no closure is set.
 */
PLUTOBOOK_API void* plutobook_get_custom_resource_fetcher_closure(const plutobook_t* book);

/**
 * @brief Sets the error message for the current thread.
 *
 * It replaces any previously set error message for the current thread.
 *
 * @param format A `printf`-style format string specifying the error message.
 * @param ... Arguments corresponding to the format string.
 */
PLUTOBOOK_API void plutobook_set_error_message(const char* format, ...) PLUTOBOOK_PRINTF_FORMAT(1, 2);

/**
 * @brief Retrieves the last error message that occurred on the current thread.
 *
 * This function returns a message describing the most recent error that occurred
 * in the current thread as set by `plutobook_set_error_message()`. If multiple errors
 * occur before this function is called, only the most recent message is returned.
 *
 * @note This function does not indicate whether an error has occurred.
 * You must check the return values of PlutoBook API functions to determine if an
 * operation failed. Do not rely solely on `plutobook_get_error_message()` to detect errors.
 *
 * @note PlutoBook does not clear the error message on successful API calls. It is your
 * responsibility to check return values and determine when to clear or ignore the message.
 *
 * @note Error messages are stored in thread-local storage. An error message set in one thread
 * will not interfere with messages or behavior in another thread.
 *
 * @return A pointer to the current error message string.
 */
PLUTOBOOK_API const char* plutobook_get_error_message(void);

/**
 * @brief @brief Clears any previously set error message for the current thread.
 */
PLUTOBOOK_API void plutobook_clear_error_message(void);

/**
 * @brief Sets the `FONTCONFIG_PATH` environment variable for the current process.
 *
 * This function specifies the directory that Fontconfig should use to locate
 * its configuration files.
 *
 * @param path A null-terminated string specifying the directory containing Fontconfig
 * configuration files.
 *
 * @note This function must be called before creating any `plutobook_t` instance
 * to ensure that Fontconfig uses the specified configuration path.
 */
PLUTOBOOK_API void plutobook_set_fontconfig_path(const char* path);

#ifdef __cplusplus
}
#endif

#endif // PLUTOBOOK_H
