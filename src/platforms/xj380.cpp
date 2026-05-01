#include "../../platforms/xj380.hpp"
#include "../../includes/string.hpp"
#include "../../platforms/xj380/xapi/xguiapi.h"
#include "../../platforms/xj380/xapi/xtuiapi.h"
#include "../../platforms/xj380/xapi/liballoc/alloc.h"

using namespace stardustui;

using operator_size_t = decltype(sizeof(0));
namespace {
char kDebugLogPath[] = "/system/stardustui.log";
char kDebugLogBuffer[4096];
unsigned long long kDebugLogLength = 0;
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
    if (message == nullptr) return;

    unsigned long long index = 0;
    while (message[index] != '\0' && kDebugLogLength + index + 1 < sizeof(kDebugLogBuffer)) {
        kDebugLogBuffer[kDebugLogLength + index] = message[index];
        ++index;
    }

    if (index == 0) return;

    kDebugLogLength += index;
    kDebugLogBuffer[kDebugLogLength] = '\0';
    xapi_CreateFile(kDebugLogPath);
    xapi_WriteFile(kDebugLogPath, kDebugLogBuffer, kDebugLogLength, 0);
    log_serial(message);
}

void refresh_window(unsigned long long handle)
{
    xapi_RefreshWindow(handle);
}

void wait_window()
{
    while (true) {
        __asm__ __volatile__("pause");
    }
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
void draw_text(unsigned long long handle, int x, int y, unsigned int color, UINT32 size, const stardustui::string& text)
{
    xapi_DrawText(handle, x, y, (char*)text.c_str(), size, color);
}
