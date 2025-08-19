#include "plutobook.hpp"

#include <cairo-pdf.h>

#include <cstdarg>
#include <cstring>
#include <cstdlib>

int plutobook_version(void)
{
    return PLUTOBOOK_VERSION;
}

const char* plutobook_version_string(void)
{
    return PLUTOBOOK_VERSION_STRING;
}

#if defined(_WIN32)
#define SYSTEM_NAME "Windows"
#elif defined(__APPLE__) && defined(__MACH__)
#define SYSTEM_NAME "macOS"
#elif defined(__linux__)
#define SYSTEM_NAME "Linux"
#elif defined(__unix__)
#define SYSTEM_NAME "Unix"
#else
#define SYSTEM_NAME "Unknown"
#endif

#if defined(__x86_64__) || defined(_M_X64)
#define MACHINE_NAME "x86_64"
#elif defined(__aarch64__) || defined(_M_ARM64)
#define MACHINE_NAME "ARM64"
#elif defined(__i386__) || defined(_M_IX86)
#define MACHINE_NAME "x86"
#elif defined(__arm__) || defined(_M_ARM)
#define MACHINE_NAME "ARM"
#else
#define MACHINE_NAME "Unknown"
#endif

#if defined(__clang__)
#define COMPILER_NAME "Clang " __clang_version__
#elif defined(__GNUC__)
#define COMPILER_NAME "GCC " __VERSION__
#elif defined(_MSC_VER)
#define STRINGIZE(x) #x
#define STRINGIFY(x) STRINGIZE(x)
#define COMPILER_NAME "MSVC " STRINGIFY(_MSC_VER)
#else
#define COMPILER_NAME "Unknown"
#endif

const char* plutobook_build_info()
{
    return "System: " SYSTEM_NAME "\n"
           "Machine: " MACHINE_NAME "\n"
           "Compiler: " COMPILER_NAME "\n"
           "Built: " __DATE__ " " __TIME__ "\n"
           "Features:"
#ifdef PLUTOBOOK_HAS_CURL
           " Curl"
#endif
#ifdef PLUTOBOOK_HAS_TURBOJPEG
           " TurboJPEG"
#endif
#ifdef PLUTOBOOK_HAS_WEBP
           " WebP"
#endif
           "\n\n"
           "PlutoBook version: " PLUTOBOOK_VERSION_STRING "\n"
           "Cairo version: " CAIRO_VERSION_STRING "\n";
}

struct _plutobook_canvas {
    cairo_surface_t* surface;
    cairo_t* context;
};

static plutobook_canvas_t* plutobook_canvas_create(cairo_surface_t* surface)
{
    auto context = cairo_create(surface);
    if(auto status = cairo_status(context)) {
        plutobook_set_error_message("canvas error: %s", cairo_status_to_string(status));
        cairo_surface_destroy(surface);
        return nullptr;
    }

    auto canvas = (plutobook_canvas_t*)(std::malloc(sizeof(plutobook_canvas_t)));
    if(canvas == nullptr) {
        plutobook_set_error_message("canvas allocation failed");
        return nullptr;
    }

    canvas->surface = surface;
    canvas->context = context;
    return canvas;
}

void plutobook_canvas_destroy(plutobook_canvas_t* canvas)
{
    if(canvas == nullptr)
        return;
    cairo_destroy(canvas->context);
    cairo_surface_destroy(canvas->surface);
    std::free(canvas);
}

void plutobook_canvas_flush(plutobook_canvas_t* canvas)
{
    if(canvas == nullptr)
        return;
    cairo_surface_flush(canvas->surface);
}

void plutobook_canvas_finish(plutobook_canvas_t* canvas)
{
    if(canvas == nullptr)
        return;
    cairo_surface_finish(canvas->surface);
}

void plutobook_canvas_translate(plutobook_canvas_t* canvas, float tx, float ty)
{
    if(canvas == nullptr)
        return;
    cairo_translate(canvas->context, tx, ty);
}

void plutobook_canvas_scale(plutobook_canvas_t* canvas, float sx, float sy)
{
    if(canvas == nullptr)
        return;
    cairo_scale(canvas->context, sx, sy);
}

void plutobook_canvas_rotate(plutobook_canvas_t* canvas, float angle)
{
    if(canvas == nullptr)
        return;
    cairo_rotate(canvas->context, angle);
}

void plutobook_canvas_transform(plutobook_canvas_t* canvas, float a, float b, float c, float d, float e, float f)
{
    if(canvas == nullptr)
        return;
    cairo_matrix_t matrix = {a, b, c, d, e, f};
    cairo_transform(canvas->context, &matrix);
}

void plutobook_canvas_set_matrix(plutobook_canvas_t* canvas, float a, float b, float c, float d, float e, float f)
{
    if(canvas == nullptr)
        return;
    cairo_matrix_t matrix = {a, b, c, d, e, f};
    cairo_set_matrix(canvas->context, &matrix);
}

void plutobook_canvas_reset_matrix(plutobook_canvas_t* canvas)
{
    if(canvas == nullptr)
        return;
    cairo_identity_matrix(canvas->context);
}

void plutobook_canvas_clip_rect(plutobook_canvas_t* canvas, float x, float y, float width, float height)
{
    if(canvas == nullptr)
        return;
    cairo_rectangle(canvas->context, x, y, width, height);
    cairo_clip(canvas->context);
}

void plutobook_canvas_clear_surface(plutobook_canvas_t* canvas, float red, float green, float blue, float alpha)
{
    if(canvas == nullptr)
        return;
    cairo_save(canvas->context);
    cairo_set_operator(canvas->context, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgba(canvas->context, red, green, blue, alpha);
    cairo_paint(canvas->context);
    cairo_restore(canvas->context);
}

void plutobook_canvas_save_state(plutobook_canvas_t* canvas)
{
    if(canvas == nullptr)
        return;
    cairo_save(canvas->context);
}

void plutobook_canvas_restore_state(plutobook_canvas_t* canvas)
{
    if(canvas == nullptr)
        return;
    cairo_restore(canvas->context);
}

cairo_surface_t* plutobook_canvas_get_surface(const plutobook_canvas_t* canvas)
{
    if(canvas == nullptr)
        return nullptr;
    return canvas->surface;
}

cairo_t* plutobook_canvas_get_context(const plutobook_canvas_t* canvas)
{
    if(canvas == nullptr)
        return nullptr;
    return canvas->context;
}

static_assert(PLUTOBOOK_IMAGE_FORMAT_INVALID == static_cast<plutobook_image_format_t>(CAIRO_FORMAT_INVALID), "unexpected plutobook_image_format_t value");
static_assert(PLUTOBOOK_IMAGE_FORMAT_ARGB32 == static_cast<plutobook_image_format_t>(CAIRO_FORMAT_ARGB32), "unexpected plutobook_image_format_t value");
static_assert(PLUTOBOOK_IMAGE_FORMAT_RGB24 == static_cast<plutobook_image_format_t>(CAIRO_FORMAT_RGB24), "unexpected plutobook_image_format_t value");
static_assert(PLUTOBOOK_IMAGE_FORMAT_A8 == static_cast<plutobook_image_format_t>(CAIRO_FORMAT_A8), "unexpected plutobook_image_format_t value");
static_assert(PLUTOBOOK_IMAGE_FORMAT_A1 == static_cast<plutobook_image_format_t>(CAIRO_FORMAT_A1), "unexpected plutobook_image_format_t value");

plutobook_canvas_t* plutobook_image_canvas_create(int width, int height, plutobook_image_format_t format)
{
    return plutobook_canvas_create(cairo_image_surface_create((cairo_format_t)(format), width, height));
}

plutobook_canvas_t* plutobook_image_canvas_create_for_data(unsigned char* data, int width, int height, int stride, plutobook_image_format_t format)
{
    return plutobook_canvas_create(cairo_image_surface_create_for_data(data, (cairo_format_t)(format), width, height, stride));
}

unsigned char* plutobook_image_canvas_get_data(const plutobook_canvas_t* canvas)
{
    if(canvas == nullptr)
        return nullptr;
    return cairo_image_surface_get_data(canvas->surface);
}

plutobook_image_format_t plutobook_image_canvas_get_format(const plutobook_canvas_t* canvas)
{
    if(canvas == nullptr)
        return PLUTOBOOK_IMAGE_FORMAT_INVALID;
    return (plutobook_image_format_t)(cairo_image_surface_get_format(canvas->surface));
}

int plutobook_image_canvas_get_width(const plutobook_canvas_t* canvas)
{
    if(canvas == nullptr)
        return 0;
    return cairo_image_surface_get_width(canvas->surface);
}

int plutobook_image_canvas_get_height(const plutobook_canvas_t* canvas)
{
    if(canvas == nullptr)
        return 0;
    return cairo_image_surface_get_height(canvas->surface);
}

int plutobook_image_canvas_get_stride(const plutobook_canvas_t* canvas)
{
    if(canvas == nullptr)
        return 0;
    return cairo_image_surface_get_stride(canvas->surface);
}

bool plutobook_image_canvas_write_to_png(const plutobook_canvas_t* canvas, const char* filename)
{
    if(canvas == nullptr) {
        plutobook_set_error_message("image encode error '%s': canvas is null", filename);
        return false;
    }

    if(auto status = cairo_surface_write_to_png(canvas->surface, filename)) {
        plutobook_set_error_message("image encode error '%s': %s", filename, cairo_status_to_string(status));
        return false;
    }

    return true;
}

static_assert(PLUTOBOOK_STREAM_STATUS_SUCCESS == static_cast<plutobook_stream_status_t>(CAIRO_STATUS_SUCCESS), "unexpected plutobook_stream_status_t value");
static_assert(PLUTOBOOK_STREAM_STATUS_READ_ERROR == static_cast<plutobook_stream_status_t>(CAIRO_STATUS_READ_ERROR), "unexpected plutobook_stream_status_t value");
static_assert(PLUTOBOOK_STREAM_STATUS_WRITE_ERROR == static_cast<plutobook_stream_status_t>(CAIRO_STATUS_WRITE_ERROR), "unexpected plutobook_stream_status_t value");

bool plutobook_image_canvas_write_to_png_stream(const plutobook_canvas_t* canvas, plutobook_stream_write_callback_t callback, void* closure)
{
    if(canvas == nullptr) {
        plutobook_set_error_message("image encode error: canvas is null");
        return false;
    }

    if(auto status = cairo_surface_write_to_png_stream(canvas->surface, (cairo_write_func_t)(callback), closure)) {
        plutobook_set_error_message("image encode error: %s", cairo_status_to_string(status));
        return false;
    }

    return true;
}

plutobook_canvas_t* plutobook_pdf_canvas_create(const char* filename, plutobook_page_size_t size)
{
    return plutobook_canvas_create(cairo_pdf_surface_create(filename, size.width, size.height));
}

plutobook_canvas_t* plutobook_pdf_canvas_create_for_stream(plutobook_stream_write_callback_t callback, void* closure, plutobook_page_size_t size)
{
    return plutobook_canvas_create(cairo_pdf_surface_create_for_stream((cairo_write_func_t)(callback), closure, size.width, size.height));
}

static_assert(PLUTOBOOK_PDF_METADATA_TITLE == static_cast<plutobook_pdf_metadata_t>(CAIRO_PDF_METADATA_TITLE), "unexpected plutobook_pdf_metadata_t value");
static_assert(PLUTOBOOK_PDF_METADATA_AUTHOR == static_cast<plutobook_pdf_metadata_t>(CAIRO_PDF_METADATA_AUTHOR), "unexpected plutobook_pdf_metadata_t value");
static_assert(PLUTOBOOK_PDF_METADATA_SUBJECT == static_cast<plutobook_pdf_metadata_t>(CAIRO_PDF_METADATA_SUBJECT), "unexpected plutobook_pdf_metadata_t value");
static_assert(PLUTOBOOK_PDF_METADATA_KEYWORDS == static_cast<plutobook_pdf_metadata_t>(CAIRO_PDF_METADATA_KEYWORDS), "unexpected plutobook_pdf_metadata_t value");
static_assert(PLUTOBOOK_PDF_METADATA_CREATOR == static_cast<plutobook_pdf_metadata_t>(CAIRO_PDF_METADATA_CREATOR), "unexpected plutobook_pdf_metadata_t value");
static_assert(PLUTOBOOK_PDF_METADATA_CREATION_DATE == static_cast<plutobook_pdf_metadata_t>(CAIRO_PDF_METADATA_CREATE_DATE), "unexpected plutobook_pdf_metadata_t value");
static_assert(PLUTOBOOK_PDF_METADATA_MODIFICATION_DATE == static_cast<plutobook_pdf_metadata_t>(CAIRO_PDF_METADATA_MOD_DATE), "unexpected plutobook_pdf_metadata_t value");

void plutobook_pdf_canvas_set_metadata(plutobook_canvas_t* canvas, plutobook_pdf_metadata_t metadata, const char* value)
{
    if(canvas == nullptr)
        return;
    cairo_pdf_surface_set_metadata(canvas->surface, (cairo_pdf_metadata_t)(metadata), value);
}

void plutobook_pdf_canvas_set_size(plutobook_canvas_t* canvas, plutobook_page_size_t size)
{
    if(canvas == nullptr)
        return;
    cairo_pdf_surface_set_size(canvas->surface, size.width, size.height);
}

void plutobook_pdf_canvas_show_page(plutobook_canvas_t* canvas)
{
    if(canvas == nullptr)
        return;
    cairo_show_page(canvas->context);
}

struct _plutobook_resource_data {
    unsigned int ref_count;
    unsigned int content_length;
    char* mime_type;
    char* text_encoding;
    char* content;
    plutobook_resource_destroy_callback_t destroy_callback;
    void* closure;
};

static plutobook_resource_data_t* plutobook_resource_data_create_uninitialized(unsigned int content_length, const char* mime_type, const char* text_encoding)
{
    auto mime_type_length = std::strlen(mime_type) + 1ul;
    auto text_encoding_length = std::strlen(text_encoding) + 1ul;
    auto resource = (plutobook_resource_data_t*)(std::malloc(mime_type_length + text_encoding_length + content_length + sizeof(plutobook_resource_data_t)));
    if(resource == nullptr) {
        plutobook_set_error_message("resource data allocation failed");
        return nullptr;
    }

    resource->ref_count = 1u;
    resource->mime_type = (char*)(resource + 1);
    resource->text_encoding = resource->mime_type + mime_type_length;
    resource->content = resource->text_encoding + text_encoding_length;
    std::memcpy(resource->mime_type, mime_type, mime_type_length);
    std::memcpy(resource->text_encoding, text_encoding, text_encoding_length);
    return resource;
}

plutobook_resource_data_t* plutobook_resource_data_create(const char* content, unsigned int content_length, const char* mime_type, const char* text_encoding)
{
    auto resource = plutobook_resource_data_create_uninitialized(content_length, mime_type, text_encoding);
    if(resource == nullptr)
        return nullptr;
    std::memcpy(resource->content, content, content_length);
    resource->content_length = content_length;
    resource->destroy_callback = nullptr;
    resource->closure = nullptr;
    return resource;
}

plutobook_resource_data_t* plutobook_resource_data_create_without_copy(const char* content, unsigned int content_length, const char* mime_type, const char* text_encoding, plutobook_resource_destroy_callback_t destroy_callback, void* closure)
{
    auto resource = plutobook_resource_data_create_uninitialized(0, mime_type, text_encoding);
    if(resource == nullptr)
        return nullptr;
    resource->content = (char*)(content);
    resource->content_length = content_length;
    resource->destroy_callback = destroy_callback;
    resource->closure = closure;
    return resource;
}

plutobook_resource_data_t* plutobook_resource_data_reference(plutobook_resource_data_t* resource)
{
    if(resource == nullptr)
        return nullptr;
    ++resource->ref_count;
    return resource;
}

void plutobook_resource_data_destroy(plutobook_resource_data_t* resource)
{
    if(resource == nullptr)
        return;
    if(--resource->ref_count == 0) {
        if(resource->destroy_callback)
            resource->destroy_callback(resource->closure);
        std::free(resource);
    }
}

unsigned int plutobook_resource_data_get_reference_count(const plutobook_resource_data_t* resource)
{
    if(resource == nullptr)
        return 0;
    return resource->ref_count;
}

const char* plutobook_resource_data_get_content(const plutobook_resource_data_t* resource)
{
    if(resource == nullptr)
        return nullptr;
    return resource->content;
}

unsigned int plutobook_resource_data_get_content_length(const plutobook_resource_data_t* resource)
{
    if(resource == nullptr)
        return 0;
    return resource->content_length;
}

const char* plutobook_resource_data_get_mime_type(const plutobook_resource_data_t* resource)
{
    if(resource == nullptr)
        return "";
    return resource->mime_type;
}

const char* plutobook_resource_data_get_text_encoding(const plutobook_resource_data_t* resource)
{
    if(resource == nullptr)
        return "";
    return resource->text_encoding;
}

plutobook_resource_data_t* plutobook_fetch_url(const char* url)
{
    return plutobook::defaultResourceFetcher()->fetchUrl(url).release();
}

void plutobook_set_ssl_cainfo(const char* path)
{
    plutobook::defaultResourceFetcher()->setCAInfo(path);
}

void plutobook_set_ssl_capath(const char* path)
{
    plutobook::defaultResourceFetcher()->setCAPath(path);
}

void plutobook_set_ssl_verify_peer(bool verify)
{
    plutobook::defaultResourceFetcher()->setVerifyPeer(verify);
}

void plutobook_set_ssl_verify_host(bool verify)
{
    plutobook::defaultResourceFetcher()->setVerifyHost(verify);
}

void plutobook_set_http_follow_redirects(bool follow)
{
    plutobook::defaultResourceFetcher()->setFollowRedirects(follow);
}

void plutobook_set_http_max_redirects(int amount)
{
    plutobook::defaultResourceFetcher()->setMaxRedirects(amount);
}

void plutobook_set_http_timeout(int timeout)
{
    plutobook::defaultResourceFetcher()->setTimeout(timeout);
}

struct _plutobook final : public plutobook::Book, public plutobook::ResourceFetcher {
    _plutobook(plutobook_page_size_t size, plutobook_page_margins_t margins, plutobook_media_type_t media);
    plutobook::ResourceData fetchUrl(const std::string& url) final;
    plutobook_resource_fetch_callback_t custom_resource_fetcher_callback{nullptr};
    void* custom_resource_fetcher_closure{nullptr};
};

_plutobook::_plutobook(plutobook_page_size_t size, plutobook_page_margins_t margins, plutobook_media_type_t media)
    : Book(size, margins, (plutobook::MediaType)(media))
{
}

plutobook::ResourceData _plutobook::fetchUrl(const std::string& url)
{
    if(custom_resource_fetcher_callback == nullptr)
        return plutobook::defaultResourceFetcher()->fetchUrl(url);
    return plutobook::ResourceData(custom_resource_fetcher_callback(custom_resource_fetcher_closure, url.data()));
}

plutobook_t* plutobook_create(plutobook_page_size_t size, plutobook_page_margins_t margins, plutobook_media_type_t media)
{
    return new _plutobook(size, margins, media);
}

void plutobook_destroy(plutobook_t* book)
{
    delete book;
}

void plutobook_clear_content(plutobook_t* book)
{
    book->clearContent();
}

void plutobook_set_metadata(plutobook_t* book, plutobook_pdf_metadata_t metadata, const char* value)
{
    switch(metadata) {
    case PLUTOBOOK_PDF_METADATA_TITLE:
        book->setTitle(value);
        break;
    case PLUTOBOOK_PDF_METADATA_AUTHOR:
        book->setAuthor(value);
        break;
    case PLUTOBOOK_PDF_METADATA_SUBJECT:
        book->setSubject(value);
        break;
    case PLUTOBOOK_PDF_METADATA_KEYWORDS:
        book->setKeywords(value);
        break;
    case PLUTOBOOK_PDF_METADATA_CREATOR:
        book->setCreator(value);
        break;
    case PLUTOBOOK_PDF_METADATA_CREATION_DATE:
        book->setCreationDate(value);
        break;
    case PLUTOBOOK_PDF_METADATA_MODIFICATION_DATE:
        book->setModificationDate(value);
        break;
    }
}

const char* plutobook_get_metadata(const plutobook_t* book, plutobook_pdf_metadata_t metadata)
{
    switch(metadata) {
    case PLUTOBOOK_PDF_METADATA_TITLE:
        return book->title().data();
    case PLUTOBOOK_PDF_METADATA_AUTHOR:
        return book->author().data();
    case PLUTOBOOK_PDF_METADATA_SUBJECT:
        return book->subject().data();
    case PLUTOBOOK_PDF_METADATA_KEYWORDS:
        return book->keywords().data();
    case PLUTOBOOK_PDF_METADATA_CREATOR:
        return book->creator().data();
    case PLUTOBOOK_PDF_METADATA_CREATION_DATE:
        return book->creationDate().data();
    case PLUTOBOOK_PDF_METADATA_MODIFICATION_DATE:
        return book->modificationDate().data();
    }

    return nullptr;
}

float plutobook_get_viewport_width(const plutobook_t* book)
{
    return book->viewportWidth();
}

float plutobook_get_viewport_height(const plutobook_t* book)
{
    return book->viewportHeight();
}

float plutobook_get_document_width(const plutobook_t* book)
{
    return book->documentWidth();
}

float plutobook_get_document_height(const plutobook_t* book)
{
    return book->documentHeight();
}

plutobook_page_size_t plutobook_get_page_size(const plutobook_t* book)
{
    return book->pageSize();
}

plutobook_page_margins_t plutobook_get_page_margins(const plutobook_t* book)
{
    return book->pageMargins();
}

plutobook_media_type_t plutobook_get_media_type(const plutobook_t* book)
{
    return (plutobook_media_type_t)(book->mediaType());
}

unsigned int plutobook_get_page_count(const plutobook_t* book)
{
    return book->pageCount();
}

plutobook_page_size_t plutobook_get_page_size_at(const plutobook_t* book, unsigned int index)
{
    return book->pageSizeAt(index);
}

bool plutobook_load_url(plutobook_t* book, const char* url, const char* user_style, const char* user_script)
{
    return book->loadUrl(url, user_style, user_script);
}

bool plutobook_load_data(plutobook_t* book, const char* data, unsigned int size, const char* mime_type, const char* text_encoding, const char* user_style, const char* user_script, const char* base_url)
{
    return book->loadData(data, size, mime_type, text_encoding, user_style, user_script, base_url);
}

bool plutobook_load_image(plutobook_t* book, const char* data, unsigned int size, const char* mime_type, const char* text_encoding, const char* user_style, const char* user_script, const char* base_url)
{
    return book->loadImage(data, size, mime_type, text_encoding, user_style, user_script, base_url);
}

bool plutobook_load_xml(plutobook_t* book, const char* data, int length, const char* user_style, const char* user_script, const char* base_url)
{
    if(length == -1)
        length = std::strlen(data);
    std::string_view content(data, length);
    return book->loadXml(content, user_style, user_script, base_url);
}

bool plutobook_load_html(plutobook_t* book, const char* data, int length, const char* user_style, const char* user_script, const char* base_url)
{
    if(length == -1)
        length = std::strlen(data);
    std::string_view content(data, length);
    return book->loadHtml(content, user_style, user_script, base_url);
}

void plutobook_render_page(const plutobook_t* book, plutobook_canvas_t* canvas, unsigned int page_index)
{
    plutobook_render_page_cairo(book, canvas->context, page_index);
}

void plutobook_render_page_cairo(const plutobook_t* book, cairo_t* context, unsigned int page_index)
{
    book->renderPage(context, page_index);
}

void plutobook_render_document(const plutobook_t* book, plutobook_canvas_t* canvas)
{
    plutobook_render_document_cairo(book, canvas->context);
}

void plutobook_render_document_cairo(const plutobook_t* book, cairo_t* context)
{
    book->renderDocument(context);
}

void plutobook_render_document_rect(const plutobook_t* book, plutobook_canvas_t* canvas, float x, float y, float width, float height)
{
    plutobook_render_document_rect_cairo(book, canvas->context, x, y, width, height);
}

void plutobook_render_document_rect_cairo(const plutobook_t* book, cairo_t* context, float x, float y, float width, float height)
{
    book->renderDocument(context, x, y, width, height);
}

bool plutobook_write_to_pdf(const plutobook_t* book, const char* filename)
{
    return plutobook_write_to_pdf_range(book, filename, PLUTOBOOK_MIN_PAGE_COUNT, PLUTOBOOK_MAX_PAGE_COUNT, 1);
}

bool plutobook_write_to_pdf_range(const plutobook_t* book, const char* filename, unsigned int from_page, unsigned int to_page, int page_step)
{
    return book->writeToPdf(filename, from_page, to_page, page_step);
}

bool plutobook_write_to_pdf_stream(const plutobook_t* book, plutobook_stream_write_callback_t callback, void* closure)
{
    return plutobook_write_to_pdf_stream_range(book, callback, closure, PLUTOBOOK_MIN_PAGE_COUNT, PLUTOBOOK_MAX_PAGE_COUNT, 1);
}

bool plutobook_write_to_pdf_stream_range(const plutobook_t* book, plutobook_stream_write_callback_t callback, void* closure, unsigned int from_page, unsigned int to_page, int page_step)
{
    return book->writeToPdf(callback, closure, from_page, to_page, page_step);
}

bool plutobook_write_to_png(const plutobook_t* book, const char* filename, plutobook_image_format_t format)
{
    return book->writeToPng(filename, (plutobook::ImageFormat)(format));
}

bool plutobook_write_to_png_stream(const plutobook_t* book, plutobook_stream_write_callback_t callback, void* closure, plutobook_image_format_t format)
{
    return book->writeToPng(callback, closure, (plutobook::ImageFormat)(format));
}

void plutobook_set_custom_resource_fetcher(plutobook_t* book, plutobook_resource_fetch_callback_t callback, void* closure)
{
    book->setCustomResourceFetcher(book);
    book->custom_resource_fetcher_callback = callback;
    book->custom_resource_fetcher_closure = closure;
}

plutobook_resource_fetch_callback_t plutobook_get_custom_resource_fetcher_callback(const plutobook_t* book)
{
    return book->custom_resource_fetcher_callback;
}

void* plutobook_get_custom_resource_fetcher_closure(const plutobook_t* book)
{
    return book->custom_resource_fetcher_closure;
}

constexpr int kErrorBufferSize = 512;

thread_local char plutobook_error_message[kErrorBufferSize];

void plutobook_set_error_message(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    char error_message[kErrorBufferSize];
    vsnprintf(error_message, kErrorBufferSize, format, args);
    strcpy(plutobook_error_message, error_message);
    va_end(args);
}

const char* plutobook_get_error_message(void)
{
    return plutobook_error_message;
}

void plutobook_clear_error_message(void)
{
    std::memset(plutobook_error_message, 0, sizeof(plutobook_error_message));
}
