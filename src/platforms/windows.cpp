#include "../../platforms/windows.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace {
const wchar_t kWindowClassName[] = L"StardustUIWindow";

enum DrawCommandType {
    DrawCommandPixel,
    DrawCommandText
};

struct DrawCommand {
    DrawCommandType type;
    int x;
    int y;
    unsigned int color;
    unsigned int size;
    stardustui::string text;

    DrawCommand() : type(DrawCommandPixel), x(0), y(0), color(0), size(0), text() {}
};

struct WindowState {
    HWND handle;
    stardustui::vector<DrawCommand> commands;

    WindowState() : handle(nullptr), commands() {}
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

    wchar_t wide_text[1024];
    to_wide(command.text.c_str(), wide_text, static_cast<int>(sizeof(wide_text) / sizeof(wide_text[0])));

    HFONT font = CreateFontW(
        -static_cast<int>(command.size == 0 ? 12 : command.size),
        0,
        0,
        0,
        FW_NORMAL,
        FALSE,
        FALSE,
        FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        L"Segoe UI");

    HGDIOBJ old_font = nullptr;
    if (font != nullptr) {
        old_font = SelectObject(device_context, font);
    }

    SetBkMode(device_context, TRANSPARENT);
    SetTextColor(device_context, to_colorref(command.color));
    TextOutW(device_context, command.x, command.y, wide_text, lstrlenW(wide_text));

    if (old_font != nullptr) {
        SelectObject(device_context, old_font);
    }
    if (font != nullptr) {
        DeleteObject(font);
    }
}

LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch (message) {
    case WM_PAINT: {
        PAINTSTRUCT paint{};
        HDC device_context = BeginPaint(hwnd, &paint);
        WindowState *state = find_state(hwnd);
        if (state != nullptr) {
            for (int index = 0; index < state->commands.size(); ++index) {
                render_command(device_context, state->commands[index]);
            }
        }
        EndPaint(hwnd, &paint);
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
    window_class.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);

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
        InvalidateRect(window, nullptr, TRUE);
        UpdateWindow(window);
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

    HDC device_context = GetDC(window);
    if (device_context == nullptr) {
        return;
    }

    render_command(device_context, command);
    ReleaseDC(window, device_context);
}

void draw_text(unsigned long long handle, int x, int y, unsigned int color, unsigned int size, const stardustui::string& text)
{
    HWND window = to_hwnd(handle);
    if (window == nullptr) {
        return;
    }

    HDC device_context = GetDC(window);
    if (device_context == nullptr) {
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

    render_command(device_context, command);
    ReleaseDC(window, device_context);
}
