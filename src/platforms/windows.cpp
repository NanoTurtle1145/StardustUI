#include "../../platforms/platform.hpp"
#include "../../includes/file.hpp"
#include "../../includes/text/text_renderer.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <cstdio>
#include <windows.h>
#include <windowsx.h>

namespace stardustui {

// File platform adapter used by stardustui::File.

bool file_exists_platform(const char* path)
{
    FILE* file = fopen(path, "rb");
    if (file == nullptr) {
        return false;
    }

    fclose(file);
    return true;
}

bool file_remove_platform(const char* path)
{
    return ::remove(path) == 0;
}

bool file_read_bytes_platform(const char* path, File::byte*& out_data, int& out_size)
{
    out_data = nullptr;
    out_size = 0;

    FILE* file = fopen(path, "rb");
    if (file == nullptr) {
        return false;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return false;
    }

    long length = ftell(file);
    if (length < 0 || length > 0x7fffffffL) {
        fclose(file);
        return false;
    }

    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return false;
    }

    if (length == 0) {
        fclose(file);
        return true;
    }

    out_data = new File::byte[(int)length];
    if (out_data == nullptr) {
        fclose(file);
        return false;
    }

    const unsigned int read_count = fread(out_data, 1, (unsigned int)length, file);
    fclose(file);

    if (read_count != (unsigned int)length) {
        delete[] out_data;
        out_data = nullptr;
        return false;
    }

    out_size = (int)length;
    return true;
}

bool file_write_bytes_platform(const char* path, const File::byte* data, int size)
{
    FILE* file = fopen(path, "wb");
    if (file == nullptr) {
        return false;
    }

    if (size == 0) {
        fclose(file);
        return true;
    }

    const unsigned int written = fwrite(data, 1, (unsigned int)size, file);
    fclose(file);
    return written == (unsigned int)size;
}

bool file_append_text_platform(const char* path, const char* text, int length)
{
    FILE* file = fopen(path, "ab");
    if (file == nullptr) {
        return false;
    }

    const unsigned int written = fwrite(text, 1, (unsigned int)length, file);
    fclose(file);
    return written == (unsigned int)length;
}

bool set_text_font_path(const stardustui::string& path)
{
    return Font::set_default_font_path(path);
}

bool set_text_font_memory(const stardustui::File::byte* data, int size)
{
    return Font::set_default_font_memory(data, size);
}

void clear_text_font()
{
    Font::clear_default_font();
}

}

namespace {
const wchar_t kWindowClassName[] = L"StardustUIWindow";

enum DrawCommandType {
    DrawCommandPixel,
    DrawCommandRect,
    DrawCommandText
};

struct DrawCommand {
    DrawCommandType type;
    int x;
    int y;
    int width;
    int height;
    unsigned int color;
    unsigned int size;
    stardustui::string text;

    DrawCommand() : type(DrawCommandPixel), x(0), y(0), width(0), height(0), color(0), size(0), text() {}
};

struct WindowState {
    HWND handle;
    window_message_proc message_proc;
    stardustui::vector<DrawCommand> commands;

    WindowState() : handle(nullptr), message_proc(nullptr), commands() {}
};

stardustui::vector<WindowState*> g_windows;

HWND to_hwnd(unsigned long long handle)
{
    return reinterpret_cast<HWND>(handle);
}

unsigned long long from_hwnd(HWND handle)
{
    return reinterpret_cast<unsigned long long>(handle);
}

HINSTANCE get_instance()
{
    return GetModuleHandleW(nullptr);
}

void to_wide(const char *text, wchar_t *buffer, int buffer_size)
{
    if (buffer == nullptr || buffer_size <= 0) {
        return;
    }

    buffer[0] = L'\0';
    if (text == nullptr) {
        return;
    }

    int written = MultiByteToWideChar(CP_UTF8, 0, text, -1, buffer, buffer_size);
    if (written > 0) {
        return;
    }

    written = MultiByteToWideChar(CP_ACP, 0, text, -1, buffer, buffer_size);
    if (written > 0) {
        return;
    }

    int index = 0;
    while (text[index] != '\0' && index + 1 < buffer_size) {
        buffer[index] = static_cast<unsigned char>(text[index]);
        ++index;
    }
    buffer[index] = L'\0';
}

COLORREF to_colorref(unsigned int color)
{
    unsigned int red = (color >> 24) & 0xFF;
    unsigned int green = (color >> 16) & 0xFF;
    unsigned int blue = (color >> 8) & 0xFF;
    return RGB(red, green, blue);
}

WindowState *find_state(HWND handle)
{
    for (int index = 0; index < g_windows.size(); ++index) {
        WindowState *state = g_windows[index];
        if (state != nullptr && state->handle == handle) {
            return state;
        }
    }

    return nullptr;
}

void remove_state(HWND handle)
{
    for (int index = 0; index < g_windows.size(); ++index) {
        WindowState *state = g_windows[index];
        if (state == nullptr || state->handle != handle) {
            continue;
        }

        delete state;
        g_windows[index] = nullptr;
        return;
    }
}

void render_command(HDC device_context, const DrawCommand& command)
{
    if (command.type == DrawCommandPixel) {
        SetPixel(device_context, command.x, command.y, to_colorref(command.color));
        return;
    }

    if (command.type == DrawCommandRect) {
        RECT rect{};
        rect.left = command.x;
        rect.top = command.y;
        rect.right = command.x + command.width;
        rect.bottom = command.y + command.height;
        HBRUSH brush = CreateSolidBrush(to_colorref(command.color));
        if (brush != nullptr) {
            FillRect(device_context, &rect, brush);
            DeleteObject(brush);
        }
        return;
    }

    stardustui::text::TextBitmap bitmap;
    if (!stardustui::text::rasterize_text(command.text, command.color, command.size == 0 ? 12 : command.size, bitmap)) {
        return;
    }
    if (bitmap.width <= 0 || bitmap.height <= 0) {
        return;
    }

    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = bitmap.width;
    info.bmiHeader.biHeight = -bitmap.height;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;

    void* dib_bits = nullptr;
    HBITMAP dib = CreateDIBSection(device_context, &info, DIB_RGB_COLORS, &dib_bits, nullptr, 0);
    if (dib == nullptr || dib_bits == nullptr) {
        if (dib != nullptr) {
            DeleteObject(dib);
        }
        return;
    }

    unsigned int* destination = static_cast<unsigned int*>(dib_bits);
    for (int y = 0; y < bitmap.height; ++y) {
        for (int x = 0; x < bitmap.width; ++x) {
            const unsigned int pixel = bitmap.pixels[y * bitmap.width + x];
            const unsigned int red = (pixel >> 24) & 0xFFu;
            const unsigned int green = (pixel >> 16) & 0xFFu;
            const unsigned int blue = (pixel >> 8) & 0xFFu;
            const unsigned int alpha = pixel & 0xFFu;
            destination[y * bitmap.width + x] = (alpha << 24) | (red << 16) | (green << 8) | blue;
        }
    }

    HDC memory_dc = CreateCompatibleDC(device_context);
    if (memory_dc == nullptr) {
        DeleteObject(dib);
        return;
    }

    HGDIOBJ old_bitmap = SelectObject(memory_dc, dib);
    BLENDFUNCTION blend{};
    blend.BlendOp = AC_SRC_OVER;
    blend.SourceConstantAlpha = 255;
    blend.AlphaFormat = AC_SRC_ALPHA;
    AlphaBlend(device_context,
               command.x,
               command.y,
               bitmap.width,
               bitmap.height,
               memory_dc,
               0,
               0,
               bitmap.width,
               bitmap.height,
               blend);

    if (old_bitmap != nullptr) {
        SelectObject(memory_dc, old_bitmap);
    }
    DeleteDC(memory_dc);
    DeleteObject(dib);
}

void render_all_commands(HDC device_context, WindowState *state, const RECT *paint_rect)
{
    if (device_context == nullptr || state == nullptr) {
        return;
    }

    RECT client_rect{};
    GetClientRect(state->handle, &client_rect);

    const int width = client_rect.right - client_rect.left;
    const int height = client_rect.bottom - client_rect.top;
    if (width <= 0 || height <= 0) {
        return;
    }

    HDC memory_dc = CreateCompatibleDC(device_context);
    if (memory_dc == nullptr) {
        for (int index = 0; index < state->commands.size(); ++index) {
            render_command(device_context, state->commands[index]);
        }
        return;
    }

    HBITMAP bitmap = CreateCompatibleBitmap(device_context, width, height);
    if (bitmap == nullptr) {
        DeleteDC(memory_dc);
        for (int index = 0; index < state->commands.size(); ++index) {
            render_command(device_context, state->commands[index]);
        }
        return;
    }

    HGDIOBJ old_bitmap = SelectObject(memory_dc, bitmap);
    HBRUSH background = CreateSolidBrush(RGB(255, 255, 255));
    if (background != nullptr) {
        FillRect(memory_dc, &client_rect, background);
        DeleteObject(background);
    }

    for (int index = 0; index < state->commands.size(); ++index) {
        render_command(memory_dc, state->commands[index]);
    }

    RECT blit_rect = client_rect;
    if (paint_rect != nullptr) {
        blit_rect = *paint_rect;
    }

    const int blit_width = blit_rect.right - blit_rect.left;
    const int blit_height = blit_rect.bottom - blit_rect.top;
    if (blit_width > 0 && blit_height > 0) {
        BitBlt(device_context,
               blit_rect.left,
               blit_rect.top,
               blit_width,
               blit_height,
               memory_dc,
               blit_rect.left,
               blit_rect.top,
               SRCCOPY);
    }

    if (old_bitmap != nullptr) {
        SelectObject(memory_dc, old_bitmap);
    }
    DeleteObject(bitmap);
    DeleteDC(memory_dc);
}

LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch (message) {
    case WM_PAINT: {
        PAINTSTRUCT paint{};
        HDC device_context = BeginPaint(hwnd, &paint);
        WindowState *state = find_state(hwnd);
        if (state != nullptr) {
            render_all_commands(device_context, state, &paint.rcPaint);
        }
        EndPaint(hwnd, &paint);
        return 0;
    }
    case WM_ERASEBKGND:
        return 1;
    case WM_MOUSEMOVE: {
        WindowState *state = find_state(hwnd);
        if (state != nullptr && state->message_proc != nullptr) {
            state->message_proc(
                kWindowMessageMove,
                static_cast<unsigned long long>(GET_X_LPARAM(lparam)),
                static_cast<unsigned long long>(GET_Y_LPARAM(lparam)));
        }
        return 0;
    }
    case WM_LBUTTONDOWN: {
        WindowState *state = find_state(hwnd);
        if (state != nullptr && state->message_proc != nullptr) {
            SetCapture(hwnd);
            state->message_proc(
                kWindowMessageLeftButtonDown,
                static_cast<unsigned long long>(GET_X_LPARAM(lparam)),
                static_cast<unsigned long long>(GET_Y_LPARAM(lparam)));
        }
        return 0;
    }
    case WM_LBUTTONUP: {
        WindowState *state = find_state(hwnd);
        if (state != nullptr && state->message_proc != nullptr) {
            ReleaseCapture();
            state->message_proc(
                kWindowMessageLeftButtonUp,
                static_cast<unsigned long long>(GET_X_LPARAM(lparam)),
                static_cast<unsigned long long>(GET_Y_LPARAM(lparam)));
        }
        return 0;
    }
    case WM_CHAR: {
        WindowState *state = find_state(hwnd);
        if (state != nullptr && state->message_proc != nullptr) {
            const char ch = static_cast<char>(wparam & 0xFF);
            if (ch == '\b' || ch == '\r' || ch == '\n') {
                state->message_proc(kWindowMessageSpecialChar, 0, static_cast<unsigned long long>(ch == '\r' ? '\n' : ch));
            } else {
                state->message_proc(kWindowMessageChar, 0, static_cast<unsigned long long>(ch));
            }
        }
        return 0;
    }
    case WM_DESTROY:
        remove_state(hwnd);
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProcW(hwnd, message, wparam, lparam);
    }
}

bool register_window_class()
{
    static bool registered = false;
    if (registered) {
        return true;
    }

    WNDCLASSW window_class{};
    window_class.lpfnWndProc = window_proc;
    window_class.hInstance = get_instance();
    window_class.lpszClassName = kWindowClassName;
    window_class.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
    window_class.hbrBackground = nullptr;

    ATOM result = RegisterClassW(&window_class);
    registered = result != 0 || GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
    return registered;
}
}

bool create_window(char *title, int width, int height, unsigned long long *handle)
{
    if (title == nullptr || handle == nullptr || width <= 0 || height <= 0) {
        return false;
    }

    if (!register_window_class()) {
        return false;
    }

    wchar_t wide_title[512];
    to_wide(title, wide_title, static_cast<int>(sizeof(wide_title) / sizeof(wide_title[0])));

    DWORD style = WS_OVERLAPPEDWINDOW;
    RECT rect{0, 0, width, height};
    AdjustWindowRect(&rect, style, FALSE);

    HWND native_handle = CreateWindowExW(
        0,
        kWindowClassName,
        wide_title,
        style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        nullptr,
        nullptr,
        get_instance(),
        nullptr);

    if (native_handle == nullptr) {
        return false;
    }

    *handle = from_hwnd(native_handle);
    WindowState *state = new WindowState();
    if (state != nullptr) {
        state->handle = native_handle;
        g_windows.push_back(state);
    }
    ShowWindow(native_handle, SW_SHOW);
    UpdateWindow(native_handle);
    return true;
}

void print_error(const char *message)
{
    MessageBoxA(nullptr, message == nullptr ? "Unknown error" : message, "StardustUI", MB_ICONERROR | MB_OK);
}

void log_serial(const char *message)
{
    if (message != nullptr) {
        OutputDebugStringA(message);
    }
}

void append_debug_log(const char *message)
{
    log_serial(message);
}

void refresh_window(unsigned long long handle)
{
    HWND window = to_hwnd(handle);
    if (window != nullptr) {
        InvalidateRect(window, nullptr, FALSE);
        UpdateWindow(window);
    }
}

void set_window_message_processor(unsigned long long handle, window_message_proc proc)
{
    HWND window = to_hwnd(handle);
    WindowState *state = find_state(window);
    if (state != nullptr) {
        state->message_proc = proc;
    }
}

void wait_window()
{
    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
}

void pump_window_events()
{
    MSG message{};
    while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
        if (message.message == WM_QUIT) {
            break;
        }
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
}

bool is_window_open(unsigned long long handle)
{
    HWND window = to_hwnd(handle);
    return window != nullptr && IsWindow(window) != 0;
}

bool delete_window(unsigned long long handle)
{
    HWND window = to_hwnd(handle);
    if (window == nullptr || !IsWindow(window)) {
        return false;
    }

    return DestroyWindow(window) != 0;
}

void draw_pixel(unsigned long long handle, int x, int y, unsigned int color)
{
    HWND window = to_hwnd(handle);
    if (window == nullptr) {
        return;
    }

    DrawCommand command;
    command.type = DrawCommandPixel;
    command.x = x;
    command.y = y;
    command.color = color;
    WindowState *state = find_state(window);
    if (state != nullptr) {
        state->commands.push_back(command);
    }
}

void draw_rect(unsigned long long handle, int x, int y, int width, int height, unsigned int color)
{
    HWND window = to_hwnd(handle);
    if (window == nullptr || width <= 0 || height <= 0) {
        return;
    }

    DrawCommand command;
    command.type = DrawCommandRect;
    command.x = x;
    command.y = y;
    command.width = width;
    command.height = height;
    command.color = color;

    WindowState *state = find_state(window);
    if (state != nullptr) {
        state->commands.push_back(command);
    }
}

void clear_draw_commands(unsigned long long handle)
{
    HWND window = to_hwnd(handle);
    WindowState *state = find_state(window);
    if (state != nullptr) {
        state->commands.clear();
    }
}

void draw_text(unsigned long long handle, int x, int y, unsigned int color, unsigned int size, const stardustui::string& text)
{
    HWND window = to_hwnd(handle);
    if (window == nullptr) {
        return;
    }

    DrawCommand command;
    command.type = DrawCommandText;
    command.x = x;
    command.y = y;
    command.color = color;
    command.size = size;
    command.text = text;
    WindowState *state = find_state(window);
    if (state != nullptr) {
        state->commands.push_back(command);
    }
}

void draw_text_on_solid_background(unsigned long long handle,
                                   int x,
                                   int y,
                                   unsigned int color,
                                   unsigned int size,
                                   unsigned int,
                                   const stardustui::string& text)
{
    draw_text(handle, x, y, color, size, text);
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
    Sleep(static_cast<DWORD>(ms));
}
