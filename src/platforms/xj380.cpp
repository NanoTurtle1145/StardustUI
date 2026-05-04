#include "../../platforms/platform.hpp"
#include "../../includes/string.hpp"
#include "../../includes/text/text_renderer.hpp"
#include "../../platforms/xj380/xapi/xguiapi.h"
#include "../../platforms/xj380/xapi/xtuiapi.h"
#include "../../platforms/xj380/xapi/xposix.h"
#include "../../platforms/xj380/xapi/liballoc/alloc.h"

using namespace stardustui;

namespace stardustui {

namespace {
enum Xj380TextMode {
    Xj380TextModeCustomPath,
    Xj380TextModeCustomMemory
};

Xj380TextMode g_xj380_text_mode = Xj380TextModeCustomPath;

bool split_directory_and_file(const char* path, stardustui::string& out_directory, stardustui::string& out_file)
{
    if (path == nullptr || path[0] == '\0') {
        return false;
    }

    const char* last_separator = nullptr;
    for (int index = 0; path[index] != '\0'; ++index) {
        if (path[index] == '/' || path[index] == '\\') {
            last_separator = path + index;
        }
    }

    if (last_separator == nullptr) {
        out_directory.assign(".");
        out_file.assign(path);
        return out_file.length() > 0;
    }

    out_directory.assign("");
    for (const char* cursor = path; cursor < last_separator; ++cursor) {
        if (!out_directory.push_char(*cursor)) {
            return false;
        }
    }

    if (out_directory.length() == 0) {
        out_directory.assign("/");
    }

    out_file.assign(last_separator + 1);
    return out_file.length() > 0;
}

bool query_file_length(const char* path, unsigned long long& out_length)
{
    out_length = 0;
    if (path == nullptr || path[0] == '\0') {
        return false;
    }

    stardustui::string directory_path;
    stardustui::string file_name;
    if (split_directory_and_file(path, directory_path, file_name)) {
        UINT32 count = 0;
        DirNode nodes[255];
        xapi_SearchFile(directory_path.data(), &count, nodes);
        if (count != 404) {
            for (UINT32 index = 0; index < count && index < 255; ++index) {
                if (stardustui::string(nodes[index].filename).equals(file_name.c_str())) {
                    out_length = nodes[index].length;
                    return true;
                }
            }
        }
    }

    XFILE* file = xapi_OpenFile((char*)path);
    if (file == nullptr) {
        return false;
    }

    out_length = file->length;
    xapi_CloseFile(file);
    return true;
}
}

// File platform adapter used by stardustui::File.

bool file_exists_platform(const char* path)
{
    unsigned long long length = 0;
    return query_file_length(path, length);
}

bool file_remove_platform(const char* path)
{
    return (long long)xapi_DeleteFile((char*)path) >= 0;
}

bool file_read_bytes_platform(const char* path, File::byte*& out_data, int& out_size)
{
    out_data = nullptr;
    out_size = 0;
    if (path == nullptr || path[0] == '\0') {
        return false;
    }

    XFILE* file = xapi_OpenFile((char*)path);
    if (file == nullptr) {
        return false;
    }

    const unsigned long long length = file->length;
    if (length == 0) {
        xapi_CloseFile(file);
        return true;
    }

    if (length > 0x7fffffffULL) {
        xapi_CloseFile(file);
        return false;
    }

    out_data = new File::byte[(int)length];
    if (out_data == nullptr) {
        xapi_CloseFile(file);
        return false;
    }

    const long long read_result = (long long)xapi_ReadFile((char*)path, (char*)out_data, length, 0);
    xapi_CloseFile(file);
    if (read_result < 0 || (unsigned long long)read_result != length) {
        delete[] out_data;
        out_data = nullptr;
        return false;
    }

    out_size = (int)length;
    return true;
}

bool file_write_bytes_platform(const char* path, const File::byte* data, int size)
{
    xapi_DeleteFile((char*)path);
    xapi_CreateFile((char*)path);

    if (size == 0) {
        return true;
    }

    return (long long)xapi_WriteFile((char*)path, (char*)data, (unsigned long long)size, 0) >= 0;
}

bool file_append_text_platform(const char* path, const char* text, int length)
{
    XFILE* file = xapi_OpenFile((char*)path);
    unsigned long long offset = 0;
    if (file != nullptr) {
        offset = file->length;
        xapi_CloseFile(file);
    } else {
        xapi_CreateFile((char*)path);
    }

    return (long long)xapi_WriteFile((char*)path, (char*)text, (unsigned long long)length, offset) >= 0;
}

}

using operator_size_t = decltype(sizeof(0));
namespace {
window_message_proc g_window_message_proc = nullptr;

struct CachedPlatformTextBitmap {
    stardustui::string text;
    unsigned int color;
    unsigned int size;
    int width;
    int height;
    stardustui::vector<XCOLORA> pixels;

    CachedPlatformTextBitmap() : text(), color(0), size(0), width(0), height(0), pixels() {}
};

bool should_cache_platform_text(const stardustui::string& text, int width, int height)
{
    if (width <= 0 || height <= 0) {
        return false;
    }
#ifdef XJ380
    (void)text;
    (void)width;
    (void)height;
    return false;
#else
    if (text.length() > 64) {
        return false;
    }
    if (width * height > 8192) {
        return false;
    }
#endif
    return true;
}

stardustui::vector<CachedPlatformTextBitmap>& platform_text_cache()
{
    static stardustui::vector<CachedPlatformTextBitmap>* cache = nullptr;
    if (cache == nullptr) {
        cache = new stardustui::vector<CachedPlatformTextBitmap>();
    }
    return *cache;
}

void clear_platform_text_cache()
{
    platform_text_cache().release_storage();
}

void reset_platform_text_cache()
{
    clear_platform_text_cache();
}

CachedPlatformTextBitmap* find_cached_platform_text(const stardustui::string& text, unsigned int color, unsigned int size)
{
    stardustui::vector<CachedPlatformTextBitmap>& cache = platform_text_cache();
    for (int index = 0; index < cache.size(); ++index) {
        CachedPlatformTextBitmap& entry = cache[index];
        if (entry.color == color && entry.size == size && entry.text.equals(text.c_str())) {
            return &entry;
        }
    }
    return nullptr;
}

CachedPlatformTextBitmap* cache_platform_text(const stardustui::string& text,
                                              unsigned int color,
                                              unsigned int size,
                                              const stardustui::text::TextBitmap& bitmap)
{
    if (!should_cache_platform_text(text, bitmap.width, bitmap.height)) {
        return nullptr;
    }

    CachedPlatformTextBitmap entry;
    entry.text = text;
    entry.color = color;
    entry.size = size;
    entry.width = bitmap.width;
    entry.height = bitmap.height;
    const int pixel_count = bitmap.width * bitmap.height;
    if (pixel_count > 0 && !entry.pixels.reserve(pixel_count)) {
        return nullptr;
    }

    for (int index = 0; index < pixel_count; ++index) {
        XCOLORA pixel{};
        const unsigned int source = bitmap.pixels[index];
        pixel.Red = static_cast<UINT8>((source >> 24) & 0xFFu);
        pixel.Green = static_cast<UINT8>((source >> 16) & 0xFFu);
        pixel.Blue = static_cast<UINT8>((source >> 8) & 0xFFu);
        pixel.Alpha = static_cast<UINT8>(source & 0xFFu);
        if (!entry.pixels.push_back(pixel)) {
            return nullptr;
        }
    }

    stardustui::vector<CachedPlatformTextBitmap>& cache = platform_text_cache();
    if (!cache.push_back(entry)) {
        return nullptr;
    }
    return &cache[cache.size() - 1];
}

void dispatch_xj380_message(unsigned long long type, unsigned long long h_data, unsigned long long l_data)
{
    if (g_window_message_proc == nullptr) {
        return;
    }

    if (type == MSG_MOVE) {
        g_window_message_proc(kWindowMessageMove, h_data, l_data);
        return;
    }

    if (type == MSG_LBUTTON) {
        g_window_message_proc(kWindowMessageLeftButtonClick, h_data, l_data);
        return;
    }

    if (type == MSG_CHAR) {
        g_window_message_proc(kWindowMessageChar, 0, l_data);
        return;
    }

    if (type == MSG_SPCHAR) {
        g_window_message_proc(kWindowMessageSpecialChar, 0, l_data);
    }
}

unsigned char blend_channel(unsigned char source, unsigned char destination, unsigned int alpha)
{
    const unsigned int inverse_alpha = 255u - alpha;
    return static_cast<unsigned char>((static_cast<unsigned int>(source) * alpha +
                                       static_cast<unsigned int>(destination) * inverse_alpha) / 255u);
}

bool blend_and_write_platform_text(unsigned long long handle,
                                   int x,
                                   int y,
                                   int width,
                                   int height,
                                   const XCOLORA* source_pixels)
{
    if (handle == 0 || source_pixels == nullptr || width <= 0 || height <= 0) {
        return false;
    }

    unsigned long long window_width = 0;
    unsigned long long window_height = 0;
    xapi_GetWindowSize(handle, &window_width, &window_height);

    int content_width = static_cast<int>(window_width);
    int content_height = static_cast<int>(window_height);
    if (content_width <= 0 || content_height <= 0) {
        return false;
    }

    content_width -= 24;
    content_height -= 47;
    if (content_width <= 0 || content_height <= 0) {
        return false;
    }

    int source_x = 0;
    int source_y = 0;
    int clipped_x = x;
    int clipped_y = y;
    int clipped_width = width;
    int clipped_height = height;

    if (clipped_x < 0) {
        source_x = -clipped_x;
        clipped_width += clipped_x;
        clipped_x = 0;
    }
    if (clipped_y < 0) {
        source_y = -clipped_y;
        clipped_height += clipped_y;
        clipped_y = 0;
    }
    if (clipped_x >= content_width || clipped_y >= content_height) {
        return false;
    }
    if (clipped_x + clipped_width > content_width) {
        clipped_width = content_width - clipped_x;
    }
    if (clipped_y + clipped_height > content_height) {
        clipped_height = content_height - clipped_y;
    }
    if (clipped_width <= 0 || clipped_height <= 0) {
        return false;
    }

    const int pixel_count = clipped_width * clipped_height;
    XCOLOR* destination_pixels = new XCOLOR[pixel_count];
    if (destination_pixels == nullptr) {
        return false;
    }

    xapi_ReadBuffer(handle,
                    static_cast<UINT32>(clipped_x),
                    static_cast<UINT32>(clipped_y),
                    static_cast<UINT32>(clipped_width),
                    static_cast<UINT32>(clipped_height),
                    destination_pixels);

    for (int row = 0; row < clipped_height; ++row) {
        for (int column = 0; column < clipped_width; ++column) {
            const int source_index = (source_y + row) * width + (source_x + column);
            const int destination_index = row * clipped_width + column;
            const XCOLORA& source = source_pixels[source_index];
            if (source.Alpha == 0) {
                continue;
            }

            if (source.Alpha == 255) {
                destination_pixels[destination_index].Red = source.Red;
                destination_pixels[destination_index].Green = source.Green;
                destination_pixels[destination_index].Blue = source.Blue;
                continue;
            }

            destination_pixels[destination_index].Red =
                blend_channel(source.Red, destination_pixels[destination_index].Red, source.Alpha);
            destination_pixels[destination_index].Green =
                blend_channel(source.Green, destination_pixels[destination_index].Green, source.Alpha);
            destination_pixels[destination_index].Blue =
                blend_channel(source.Blue, destination_pixels[destination_index].Blue, source.Alpha);
        }
    }

    xapi_WriteBuffer(handle,
                     static_cast<UINT32>(clipped_x),
                     static_cast<UINT32>(clipped_y),
                     static_cast<UINT32>(clipped_width),
                     static_cast<UINT32>(clipped_height),
                     destination_pixels);
    delete[] destination_pixels;
    return true;
}

bool write_platform_text_on_solid_background(unsigned long long handle,
                                             int x,
                                             int y,
                                             int width,
                                             int height,
                                             unsigned int background_color,
                                             const XCOLORA* source_pixels)
{
    if (handle == 0 || source_pixels == nullptr || width <= 0 || height <= 0) {
        return false;
    }

    unsigned long long window_width = 0;
    unsigned long long window_height = 0;
    xapi_GetWindowSize(handle, &window_width, &window_height);

    int content_width = static_cast<int>(window_width) - 24;
    int content_height = static_cast<int>(window_height) - 47;
    if (content_width <= 0 || content_height <= 0) {
        return false;
    }

    int source_x = 0;
    int source_y = 0;
    int clipped_x = x;
    int clipped_y = y;
    int clipped_width = width;
    int clipped_height = height;

    if (clipped_x < 0) {
        source_x = -clipped_x;
        clipped_width += clipped_x;
        clipped_x = 0;
    }
    if (clipped_y < 0) {
        source_y = -clipped_y;
        clipped_height += clipped_y;
        clipped_y = 0;
    }
    if (clipped_x >= content_width || clipped_y >= content_height) {
        return false;
    }
    if (clipped_x + clipped_width > content_width) {
        clipped_width = content_width - clipped_x;
    }
    if (clipped_y + clipped_height > content_height) {
        clipped_height = content_height - clipped_y;
    }
    if (clipped_width <= 0 || clipped_height <= 0) {
        return false;
    }

    const int pixel_count = clipped_width * clipped_height;
    XCOLOR* destination_pixels = new XCOLOR[pixel_count];
    if (destination_pixels == nullptr) {
        return false;
    }

    const unsigned char background_red = static_cast<unsigned char>((background_color >> 24) & 0xFFu);
    const unsigned char background_green = static_cast<unsigned char>((background_color >> 16) & 0xFFu);
    const unsigned char background_blue = static_cast<unsigned char>((background_color >> 8) & 0xFFu);

    for (int row = 0; row < clipped_height; ++row) {
        for (int column = 0; column < clipped_width; ++column) {
            const int source_index = (source_y + row) * width + (source_x + column);
            const int destination_index = row * clipped_width + column;
            const XCOLORA& source = source_pixels[source_index];

            if (source.Alpha == 0) {
                destination_pixels[destination_index].Red = background_red;
                destination_pixels[destination_index].Green = background_green;
                destination_pixels[destination_index].Blue = background_blue;
                continue;
            }

            if (source.Alpha == 255) {
                destination_pixels[destination_index].Red = source.Red;
                destination_pixels[destination_index].Green = source.Green;
                destination_pixels[destination_index].Blue = source.Blue;
                continue;
            }

            destination_pixels[destination_index].Red =
                blend_channel(source.Red, background_red, source.Alpha);
            destination_pixels[destination_index].Green =
                blend_channel(source.Green, background_green, source.Alpha);
            destination_pixels[destination_index].Blue =
                blend_channel(source.Blue, background_blue, source.Alpha);
        }
    }

    xapi_WriteBuffer(handle,
                     static_cast<UINT32>(clipped_x),
                     static_cast<UINT32>(clipped_y),
                     static_cast<UINT32>(clipped_width),
                     static_cast<UINT32>(clipped_height),
                     destination_pixels);
    delete[] destination_pixels;
    return true;
}

}

void *operator new(operator_size_t size)
{
    return malloc(size);
}

void *operator new[](operator_size_t size)
{
    return malloc(size);
}

void operator delete(void *ptr) noexcept
{
    free(ptr);
}

void operator delete[](void *ptr) noexcept
{
    free(ptr);
}

void operator delete(void *ptr, operator_size_t) noexcept
{
    free(ptr);
}

void operator delete[](void *ptr, operator_size_t) noexcept
{
    free(ptr);
}

bool create_window(char *title, int width, int height, unsigned long long *handle)
{
    if (title == nullptr || handle == nullptr || width <= 0 || height <= 0) return false;

    XWINDOW xwin{};
    xwin.width  = width;
    xwin.height = height;
    xwin.title  = title;
    xwin.sets   = XWIN_NORMAL;

    HDLE native_handle{};
    xapi_CreateWindow(&native_handle, &xwin);
    *handle = native_handle;
    return true;
}

void print_error(const char *message)
{
    static char kErrorFormat[] = "Error: %s";
    xapi_Printf(kErrorFormat, message);
}

void log_serial(const char *message)
{
    xapi_OutputSerial((char *)message);
}

void append_debug_log(const char *message)
{
    (void)message;
}

void refresh_window(unsigned long long handle)
{
    xapi_RefreshWindow(handle);
}

void set_window_message_processor(unsigned long long handle, window_message_proc proc)
{
    g_window_message_proc = proc;
    SetMsgPrcor(handle, dispatch_xj380_message);
}

void wait_window()
{
    while (true) {
        __asm__ __volatile__("pause");
    }
}

void pump_window_events()
{
}

bool is_window_open(unsigned long long handle)
{
    return handle != 0;
}

bool delete_window(unsigned long long handle)
{
    if (handle == 0) return false;

    xapi_CloseWindow(handle);
    return true;
}
void draw_pixel(unsigned long long handle, int x, int y, unsigned int color)
{
    xapi_DrawPoint(handle, x, y, color);
}

void draw_rect(unsigned long long handle, int x, int y, int width, int height, unsigned int color)
{
    if (width <= 0 || height <= 0) return;

    xapi_DrawRect(handle,
                  static_cast<UINT32>(x),
                  static_cast<UINT32>(y),
                  static_cast<UINT32>(x + width - 1),
                  static_cast<UINT32>(y + height - 1),
                  color,
                  true);
}

void clear_draw_commands(unsigned long long)
{
}

void draw_text(unsigned long long handle, int x, int y, unsigned int color, unsigned int size, const stardustui::string& text)
{
    const unsigned int resolved_size = size == 0 ? 12 : size;
    CachedPlatformTextBitmap* cached = find_cached_platform_text(text, color, resolved_size);
    XCOLORA* transient_pixels = nullptr;
    int transient_width = 0;
    int transient_height = 0;
    if (cached == nullptr) {
        stardustui::text::TextBitmap bitmap;
        if (!stardustui::text::rasterize_text(text, color, resolved_size, bitmap)) {
            return;
        }
        if (bitmap.width <= 0 || bitmap.height <= 0) {
            return;
        }

        cached = cache_platform_text(text, color, resolved_size, bitmap);
        if (cached == nullptr) {
            const int pixel_count = bitmap.width * bitmap.height;
            transient_pixels = new XCOLORA[pixel_count];
            if (transient_pixels == nullptr) {
                return;
            }
            transient_width = bitmap.width;
            transient_height = bitmap.height;
            for (int index = 0; index < pixel_count; ++index) {
                const unsigned int source = bitmap.pixels[index];
                transient_pixels[index].Red = static_cast<UINT8>((source >> 24) & 0xFFu);
                transient_pixels[index].Green = static_cast<UINT8>((source >> 16) & 0xFFu);
                transient_pixels[index].Blue = static_cast<UINT8>((source >> 8) & 0xFFu);
                transient_pixels[index].Alpha = static_cast<UINT8>(source & 0xFFu);
            }

            blend_and_write_platform_text(handle, x, y, transient_width, transient_height, transient_pixels);
            delete[] transient_pixels;
            return;
        }
    }

    if (cached->width <= 0 || cached->height <= 0 || cached->pixels.size() <= 0) {
        return;
    }

    blend_and_write_platform_text(handle,
                                  x,
                                  y,
                                  cached->width,
                                  cached->height,
                                  cached->pixels.size() > 0 ? &cached->pixels[0] : nullptr);
}

void draw_text_on_solid_background(unsigned long long handle,
                                   int x,
                                   int y,
                                   unsigned int color,
                                   unsigned int size,
                                   unsigned int background_color,
                                   const stardustui::string& text)
{
    const unsigned int resolved_size = size == 0 ? 12 : size;
    CachedPlatformTextBitmap* cached = find_cached_platform_text(text, color, resolved_size);
    XCOLORA* transient_pixels = nullptr;
    int transient_width = 0;
    int transient_height = 0;
    if (cached == nullptr) {
        stardustui::text::TextBitmap bitmap;
        if (!stardustui::text::rasterize_text(text, color, resolved_size, bitmap)) {
            return;
        }
        if (bitmap.width <= 0 || bitmap.height <= 0) {
            return;
        }

        cached = cache_platform_text(text, color, resolved_size, bitmap);
        if (cached == nullptr) {
            const int pixel_count = bitmap.width * bitmap.height;
            transient_pixels = new XCOLORA[pixel_count];
            if (transient_pixels == nullptr) {
                return;
            }
            transient_width = bitmap.width;
            transient_height = bitmap.height;
            for (int index = 0; index < pixel_count; ++index) {
                const unsigned int source = bitmap.pixels[index];
                transient_pixels[index].Red = static_cast<UINT8>((source >> 24) & 0xFFu);
                transient_pixels[index].Green = static_cast<UINT8>((source >> 16) & 0xFFu);
                transient_pixels[index].Blue = static_cast<UINT8>((source >> 8) & 0xFFu);
                transient_pixels[index].Alpha = static_cast<UINT8>(source & 0xFFu);
            }

            write_platform_text_on_solid_background(handle,
                                                    x,
                                                    y,
                                                    transient_width,
                                                    transient_height,
                                                    background_color,
                                                    transient_pixels);
            delete[] transient_pixels;
            return;
        }
    }

    if (cached->width <= 0 || cached->height <= 0 || cached->pixels.size() <= 0) {
        return;
    }

    write_platform_text_on_solid_background(handle,
                                            x,
                                            y,
                                            cached->width,
                                            cached->height,
                                            background_color,
                                            cached->pixels.size() > 0 ? &cached->pixels[0] : nullptr);
}

unsigned int calc_text_width(const stardustui::string& text, unsigned int size)
{
    unsigned int width = 0;
    unsigned int height = 0;
    if (!stardustui::text::measure_text(text, size == 0 ? 12 : size, width, height)) {
        return static_cast<unsigned int>(text.length() * (size == 0 ? 12 : size));
    }
    return width;
}

unsigned int calc_text_height(const stardustui::string& text, unsigned int size)
{
    unsigned int width = 0;
    unsigned int height = 0;
    if (!stardustui::text::measure_text(text, size == 0 ? 12 : size, width, height)) {
        return size == 0 ? 12U : size;
    }
    return height;
}

void sleep_ms(unsigned long long ms)
{
    xapi_Sleep(ms);
}

namespace stardustui {

bool set_text_font_path(const stardustui::string& path)
{
    g_xj380_text_mode = Xj380TextModeCustomPath;
    reset_platform_text_cache();
    return Font::set_default_font_path(path);
}

bool set_text_font_memory(const stardustui::File::byte* data, int size)
{
    g_xj380_text_mode = Xj380TextModeCustomMemory;
    reset_platform_text_cache();
    return Font::set_default_font_memory(data, size);
}

void clear_text_font()
{
    g_xj380_text_mode = Xj380TextModeCustomPath;
    reset_platform_text_cache();
    Font::clear_default_font();
}

}
