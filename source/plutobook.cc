#include "plutobook.hpp"

#include <cairo-pdf.h>

#include <cstring>
#include <cstdlib>

int plutobook_version()
{
    return PLUTOBOOK_VERSION;
}

const char* plutobook_version_string()
{
    return PLUTOBOOK_VERSION_STRING;
}

const char* plutobook_about()
{
    return "PlutoBook " PLUTOBOOK_VERSION_STRING " (https://github.com/plutoprint)";
}

struct _plutobook_canvas {
    cairo_surface_t* surface;
    cairo_t* context;
};

static plutobook_canvas_t* plutobook_canvas_create(cairo_surface_t* surface)
{
    auto context = cairo_create(surface);
    if(cairo_status(context)) {
        return nullptr;
    }

    auto canvas = (plutobook_canvas_t*)(std::malloc(sizeof(plutobook_canvas_t)));
    if(canvas == nullptr)
        return nullptr;
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
    if(canvas == NULL)
        return;
    cairo_surface_flush(canvas->surface);
}

void plutobook_canvas_finish(plutobook_canvas_t* canvas)
{
    if(canvas == NULL)
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
    cairo_set_source_rgba(canvas->context, red, green, blue, alpha);
    cairo_set_operator(canvas->context, CAIRO_OPERATOR_SOURCE);
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

static_assert(CAIRO_STATUS_SUCCESS == static_cast<cairo_status_t>(PLUTOBOOK_STATUS_SUCCESS), "Status codes must align");
static_assert(CAIRO_STATUS_NO_MEMORY == static_cast<cairo_status_t>(PLUTOBOOK_STATUS_MEMORY_ERROR), "Status codes must align");
static_assert(CAIRO_STATUS_READ_ERROR == static_cast<cairo_status_t>(PLUTOBOOK_STATUS_LOAD_ERROR), "Status codes must align");
static_assert(CAIRO_STATUS_WRITE_ERROR == static_cast<cairo_status_t>(PLUTOBOOK_STATUS_WRITE_ERROR), "Status codes must align");

plutobook_status_t plutobook_canvas_get_status(const plutobook_canvas_t* canvas)
{
    if(canvas == nullptr || cairo_status(canvas->context) || cairo_surface_status(canvas->surface))
        return PLUTOBOOK_STATUS_CANVAS_ERROR;
    return PLUTOBOOK_STATUS_SUCCESS;
}

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

plutobook_status_t plutobook_image_canvas_write_to_png(const plutobook_canvas_t* canvas, const char* filename)
{
    if(canvas == nullptr)
        return PLUTOBOOK_STATUS_WRITE_ERROR;
    if(cairo_surface_write_to_png(canvas->surface, filename))
        return PLUTOBOOK_STATUS_WRITE_ERROR;
    return PLUTOBOOK_STATUS_SUCCESS;
}

plutobook_status_t plutobook_image_canvas_write_to_png_stream(const plutobook_canvas_t* canvas, plutobook_stream_write_callback_t callback, void* closure)
{
    if(canvas == nullptr)
        return PLUTOBOOK_STATUS_WRITE_ERROR;
    if(cairo_surface_write_to_png_stream(canvas->surface, (cairo_write_func_t)(callback), closure))
        return PLUTOBOOK_STATUS_WRITE_ERROR;
    return PLUTOBOOK_STATUS_SUCCESS;
}

plutobook_canvas_t* plutobook_pdf_canvas_create(const char* filename, plutobook_page_size_t size)
{
    return plutobook_canvas_create(cairo_pdf_surface_create(filename, size.width, size.height));
}

plutobook_canvas_t* plutobook_pdf_canvas_create_for_stream(plutobook_stream_write_callback_t callback, void* closure, plutobook_page_size_t size)
{
    return plutobook_canvas_create(cairo_pdf_surface_create_for_stream((cairo_write_func_t)(callback), closure, size.width, size.height));
}

void plutobook_pdf_canvas_set_metadata(plutobook_canvas_t* canvas, plutobook_pdf_metadata_t name, const char* value)
{
    if(canvas == nullptr)
        return;
    cairo_pdf_surface_set_metadata(canvas->surface, (cairo_pdf_metadata_t)(name), value);
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

static plutobook_resource_data_t* plutobook_resource_data_create(unsigned int content_length, const char* mime_type, const char* text_encoding)
{
    auto mime_type_length = std::strlen(mime_type) + 1ul;
    auto text_encoding_length = std::strlen(text_encoding) + 1ul;
    auto resource = (plutobook_resource_data_t*)(std::malloc(mime_type_length + text_encoding_length + content_length + sizeof(plutobook_resource_data_t)));
    if(resource == nullptr)
        return nullptr;
    resource->ref_count = 1u;
    resource->content_length = content_length;
    resource->mime_type = (char*)(resource + 1);
    resource->text_encoding = resource->mime_type + mime_type_length;
    resource->content = resource->text_encoding + text_encoding_length;
    resource->destroy_callback = nullptr;
    resource->closure = nullptr;
    std::memcpy(resource->mime_type, mime_type, mime_type_length);
    std::memcpy(resource->text_encoding, text_encoding, text_encoding_length);
    return resource;
}

plutobook_resource_data_t* plutobook_resource_data_create_with_copy(const char* content, unsigned int content_length, const char* mime_type, const char* text_encoding)
{
    auto resource = plutobook_resource_data_create(content_length, mime_type, text_encoding);
    if(resource == nullptr)
        return nullptr;
    std::memcpy(resource->content, content, content_length);
    return resource;
}

plutobook_resource_data_t* plutobook_resource_data_create_without_copy(const char* content, unsigned int content_length, const char* mime_type, const char* text_encoding, plutobook_resource_destroy_callback_t destroy_callback, void* closure)
{
    auto resource = plutobook_resource_data_create(0, mime_type, text_encoding);
    if(resource == nullptr)
        return nullptr;
    resource->content_length = content_length;
    resource->content = (char*)(content);
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
        return nullptr;
    return resource->mime_type;
}

const char* plutobook_resource_data_get_text_encoding(const plutobook_resource_data_t* resource)
{
    if(resource == nullptr)
        return nullptr;
    return resource->text_encoding;
}

plutobook_resource_data_t* plutobook_default_resource_fetcher_load_url(const char* url)
{
    return plutobook::defaultResourceFetcher()->loadUrl(url).release();
}

struct _plutobook final : public plutobook::ResourceFetcher {
    _plutobook(plutobook_page_size_t size, plutobook_page_margins_t margins, plutobook_media_type_t media);
    plutobook::ResourceData loadUrl(const std::string& url) final;
    plutobook::Book book;
    plutobook_resource_load_callback_t custom_resource_fetcher_callback{nullptr};
    void* custom_resource_fetcher_closure{nullptr};
};

_plutobook::_plutobook(plutobook_page_size_t size, plutobook_page_margins_t margins, plutobook_media_type_t media)
    : book(size, margins, (plutobook::MediaType)(media))
{
}

plutobook::ResourceData _plutobook::loadUrl(const std::string& url)
{
    if(custom_resource_fetcher_callback == nullptr)
        return plutobook::defaultResourceFetcher()->loadUrl(url);
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

inline const plutobook::Book& getBook(const plutobook_t* book)
{
    return book->book;
}

inline plutobook::Book& getBook(plutobook_t* book)
{
    return book->book;
}

void plutobook_clear_content(plutobook_t* book)
{
    getBook(book).clearContent();
}

void plutobook_set_metadata(plutobook_t* book, plutobook_pdf_metadata_t name, const char* value)
{
    switch(name) {
    case PLUTOBOOK_PDF_METADATA_TITLE:
        getBook(book).setTitle(value);
        break;
    case PLUTOBOOK_PDF_METADATA_AUTHOR:
        getBook(book).setAuthor(value);
        break;
    case PLUTOBOOK_PDF_METADATA_SUBJECT:
        getBook(book).setSubject(value);
        break;
    case PLUTOBOOK_PDF_METADATA_KEYWORDS:
        getBook(book).setKeywords(value);
        break;
    case PLUTOBOOK_PDF_METADATA_CREATOR:
        getBook(book).setCreator(value);
        break;
    case PLUTOBOOK_PDF_METADATA_CREATION_DATE:
        getBook(book).setCreationDate(value);
        break;
    case PLUTOBOOK_PDF_METADATA_MODIFICATION_DATE:
        getBook(book).setModificationDate(value);
        break;
    }
}

const char* plutobook_get_metadata(const plutobook_t* book, plutobook_pdf_metadata_t name)
{
    switch(name) {
    case PLUTOBOOK_PDF_METADATA_TITLE:
        return getBook(book).title().data();
    case PLUTOBOOK_PDF_METADATA_AUTHOR:
        return getBook(book).author().data();
    case PLUTOBOOK_PDF_METADATA_SUBJECT:
        return getBook(book).subject().data();
    case PLUTOBOOK_PDF_METADATA_KEYWORDS:
        return getBook(book).keywords().data();
    case PLUTOBOOK_PDF_METADATA_CREATOR:
        return getBook(book).creator().data();
    case PLUTOBOOK_PDF_METADATA_CREATION_DATE:
        return getBook(book).creationDate().data();
    case PLUTOBOOK_PDF_METADATA_MODIFICATION_DATE:
        return getBook(book).modificationDate().data();
    }

    return nullptr;
}

float plutobook_get_viewport_width(const plutobook_t* book)
{
    return getBook(book).viewportWidth();
}

float plutobook_get_viewport_height(const plutobook_t* book)
{
    return getBook(book).viewportHeight();
}

float plutobook_get_document_width(const plutobook_t* book)
{
    return getBook(book).documentWidth();
}

float plutobook_get_document_height(const plutobook_t* book)
{
    return getBook(book).documentHeight();
}

plutobook_page_size_t plutobook_get_page_size(const plutobook_t* book)
{
    return getBook(book).pageSize();
}

plutobook_page_margins_t plutobook_get_page_margins(const plutobook_t* book)
{
    return getBook(book).pageMargins();
}

unsigned int plutobook_get_page_count(const plutobook_t* book)
{
    return getBook(book).pageCount();
}

plutobook_page_size_t plutobook_get_page_size_at(const plutobook_t* book, unsigned int index)
{
    return getBook(book).pageSizeAt(index);
}

plutobook_media_type_t plutobook_get_media_type(const plutobook_t* book)
{
    return (plutobook_media_type_t)(getBook(book).mediaType());
}

plutobook_status_t plutobook_load_url(plutobook_t* book, const char* url, const char* user_style, const char* user_script)
{
    if(getBook(book).loadUrl(url, user_style, user_script))
        return PLUTOBOOK_STATUS_SUCCESS;
    return PLUTOBOOK_STATUS_LOAD_ERROR;
}

plutobook_status_t plutobook_load_data(plutobook_t* book, const char* data, unsigned int size, const char* mime_type, const char* text_encoding, const char* user_style, const char* user_script, const char* base_url)
{
    if(getBook(book).loadData(data, size, mime_type, text_encoding, user_style, user_script, base_url))
        return PLUTOBOOK_STATUS_SUCCESS;
    return PLUTOBOOK_STATUS_LOAD_ERROR;
}

plutobook_status_t plutobook_load_image(plutobook_t* book, const char* data, unsigned int size, const char* mime_type, const char* text_encoding, const char* user_style, const char* user_script, const char* base_url)
{
    if(getBook(book).loadImage(data, size, mime_type, text_encoding, user_style, user_script, base_url))
        return PLUTOBOOK_STATUS_SUCCESS;
    return PLUTOBOOK_STATUS_LOAD_ERROR;
}

plutobook_status_t plutobook_load_xml(plutobook_t* book, const char* data, int length, const char* user_style, const char* user_script, const char* base_url)
{
    if(length == -1)
        length = std::strlen(data);
    std::string_view content(data, length);
    if(getBook(book).loadXml(content, user_style, user_script, base_url))
        return PLUTOBOOK_STATUS_SUCCESS;
    return PLUTOBOOK_STATUS_LOAD_ERROR;
}

plutobook_status_t plutobook_load_html(plutobook_t* book, const char* data, int length, const char* user_style, const char* user_script, const char* base_url)
{
    if(length == -1)
        length = std::strlen(data);
    std::string_view content(data, length);
    if(getBook(book).loadHtml(content, user_style, user_script, base_url))
        return PLUTOBOOK_STATUS_SUCCESS;
    return PLUTOBOOK_STATUS_LOAD_ERROR;
}

void plutobook_render_page(const plutobook_t* book, plutobook_canvas_t* canvas, unsigned int page_index)
{
    plutobook_render_page_cairo(book, canvas->context, page_index);
}

void plutobook_render_page_cairo(const plutobook_t* book, cairo_t* context, unsigned int page_index)
{
    getBook(book).renderPage(context, page_index);
}

void plutobook_render_document(const plutobook_t* book, plutobook_canvas_t* canvas)
{
    plutobook_render_document_cairo(book, canvas->context);
}

void plutobook_render_document_cairo(const plutobook_t* book, cairo_t* context)
{
    getBook(book).renderDocument(context);
}

void plutobook_render_document_rect(const plutobook_t* book, plutobook_canvas_t* canvas, float x, float y, float width, float height)
{
    plutobook_render_document_rect_cairo(book, canvas->context, x, y, width, height);
}

void plutobook_render_document_rect_cairo(const plutobook_t* book, cairo_t* context, float x, float y, float width, float height)
{
    getBook(book).renderDocument(context, x, y, width, height);
}

plutobook_status_t plutobook_write_to_pdf(const plutobook_t* book, const char* filename)
{
    return plutobook_write_to_pdf_range(book, filename, PLUTOBOOK_MIN_PAGE_COUNT, PLUTOBOOK_MAX_PAGE_COUNT, 1);
}

plutobook_status_t plutobook_write_to_pdf_range(const plutobook_t* book, const char* filename, unsigned int from_page, unsigned int to_page, int page_step)
{
    if(getBook(book).writeToPdf(filename, from_page, to_page, page_step))
        return PLUTOBOOK_STATUS_SUCCESS;
    return PLUTOBOOK_STATUS_WRITE_ERROR;
}

plutobook_status_t plutobook_write_to_pdf_stream(const plutobook_t* book, plutobook_stream_write_callback_t callback, void* closure)
{
    return plutobook_write_to_pdf_stream_range(book, callback, closure, PLUTOBOOK_MIN_PAGE_COUNT, PLUTOBOOK_MAX_PAGE_COUNT, 1);
}

plutobook_status_t plutobook_write_to_pdf_stream_range(const plutobook_t* book, plutobook_stream_write_callback_t callback, void* closure, unsigned int from_page, unsigned int to_page, int page_step)
{
    if(getBook(book).writeToPdf(callback, closure, from_page, to_page, page_step))
        return PLUTOBOOK_STATUS_SUCCESS;
    return PLUTOBOOK_STATUS_WRITE_ERROR;
}

plutobook_status_t plutobook_write_to_png(const plutobook_t* book, const char* filename, plutobook_image_format_t format)
{
    if(getBook(book).writeToPng(filename, (plutobook::ImageFormat)(format)))
        return PLUTOBOOK_STATUS_SUCCESS;
    return PLUTOBOOK_STATUS_WRITE_ERROR;
}

plutobook_status_t plutobook_write_to_png_stream(const plutobook_t* book, plutobook_stream_write_callback_t callback, void* closure, plutobook_image_format_t format)
{
    if(getBook(book).writeToPng(callback, closure, (plutobook::ImageFormat)(format)))
        return PLUTOBOOK_STATUS_SUCCESS;
    return PLUTOBOOK_STATUS_WRITE_ERROR;
}

void plutobook_set_custom_resource_fetcher(plutobook_t* book, plutobook_resource_load_callback_t callback, void* closure)
{
    getBook(book).setCustomResourceFetcher(book);
    book->custom_resource_fetcher_callback = callback;
    book->custom_resource_fetcher_closure = closure;
}

plutobook_resource_load_callback_t plutobook_get_custom_resource_fetcher_callback(const plutobook_t* book)
{
    return book->custom_resource_fetcher_callback;
}

void* plutobook_get_custom_resource_fetcher_closure(const plutobook_t* book)
{
    return book->custom_resource_fetcher_closure;
}
