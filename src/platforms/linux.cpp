#include "../../platforms/linux.hpp"
#include "../../includes/vector.hpp"

#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <cstdio>
#include <cstdlib>

namespace {
struct DrawCommand {
    enum Type {
        Pixel,
        Text
    };

    Type type;
    int x;
    int y;
    unsigned int color;
    unsigned int size;
    stardustui::string text;

    DrawCommand() : type(Pixel), x(0), y(0), color(0), size(0), text() {}
};

struct FontEntry {
    unsigned int size;
    XftFont *font;

    FontEntry() : size(0), font(nullptr) {}
};

struct WindowState {
    Display *display;
    int screen;
    Window window;
    GC graphics_context;
    XftDraw *xft_draw;
    Atom delete_message;
    stardustui::vector<DrawCommand> commands;
    stardustui::vector<FontEntry> fonts;

    WindowState()
        : display(nullptr),
          screen(0),
          window(0),
          graphics_context(0),
          xft_draw(nullptr),
          delete_message(0),
          commands(),
          fonts() {}
};

stardustui::vector<WindowState*> g_windows;
char g_last_error[256];

void set_last_error(const char *message)
{
    if (message == nullptr) {
        g_last_error[0] = '\0';
        return;
    }

    int index = 0;
    while (message[index] != '\0' && index + 1 < static_cast<int>(sizeof(g_last_error))) {
        g_last_error[index] = message[index];
        ++index;
    }
    g_last_error[index] = '\0';
}

WindowState *to_state(unsigned long long handle)
{
    return reinterpret_cast<WindowState*>(handle);
}

unsigned long long from_state(WindowState *state)
{
    return reinterpret_cast<unsigned long long>(state);
}

bool has_state(WindowState *state)
{
    for (int index = 0; index < g_windows.size(); ++index) {
        if (g_windows[index] == state) {
            return true;
        }
    }

    return false;
}

unsigned long to_pixel(unsigned int color)
{
    unsigned int red = (color >> 24) & 0xFF;
    unsigned int green = (color >> 16) & 0xFF;
    unsigned int blue = (color >> 8) & 0xFF;
    return (red << 16) | (green << 8) | blue;
}

XftFont *load_font(WindowState *state, unsigned int size)
{
    if (state == nullptr || state->display == nullptr) {
        return nullptr;
    }

    unsigned int pixel_size = size == 0 ? 12 : size;
    for (int index = 0; index < state->fonts.size(); ++index) {
        if (state->fonts[index].size == pixel_size) {
            return state->fonts[index].font;
        }
    }

    char font_name[128];
    std::snprintf(
        font_name,
        sizeof(font_name),
        "Sans-%u",
        pixel_size);

    XftFont *font = XftFontOpenName(state->display, state->screen, font_name);

    if (font == nullptr) {
        std::snprintf(
            font_name,
            sizeof(font_name),
            "DejaVu Sans-%u",
            pixel_size);
        font = XftFontOpenName(state->display, state->screen, font_name);
    }

    if (font == nullptr) {
        font = XftFontOpenName(state->display, state->screen, "monospace-12");
    }

    if (font != nullptr) {
        FontEntry entry;
        entry.size = pixel_size;
        entry.font = font;
        state->fonts.push_back(entry);
    } else {
        log_serial("stardustui: Linux failed to load any X11 font\n");
    }
    return font;
}

void draw_command(WindowState *state, const DrawCommand& command)
{
    if (state == nullptr || state->display == nullptr || state->window == 0 || state->graphics_context == 0) {
        return;
    }

    XSetForeground(state->display, state->graphics_context, to_pixel(command.color));

    if (command.type == DrawCommand::Pixel) {
        XDrawPoint(state->display, state->window, state->graphics_context, command.x, command.y);
    } else {
        XftFont *font = load_font(state, command.size);
        int baseline = command.y + static_cast<int>(command.size == 0 ? 12 : command.size);

        if (font != nullptr) {
            baseline = command.y + font->ascent;
            XRenderColor render_color{};
            render_color.red = static_cast<unsigned short>(((command.color >> 24) & 0xFF) * 257);
            render_color.green = static_cast<unsigned short>(((command.color >> 16) & 0xFF) * 257);
            render_color.blue = static_cast<unsigned short>(((command.color >> 8) & 0xFF) * 257);
            render_color.alpha = 0xFFFF;

            XftColor xft_color{};
            if (XftColorAllocValue(
                    state->display,
                    DefaultVisual(state->display, state->screen),
                    DefaultColormap(state->display, state->screen),
                    &render_color,
                    &xft_color)) {
                XftDrawStringUtf8(
                    state->xft_draw,
                    &xft_color,
                    font,
                    command.x,
                    baseline,
                    reinterpret_cast<const FcChar8*>(command.text.c_str()),
                    command.text.length());
                XftColorFree(
                    state->display,
                    DefaultVisual(state->display, state->screen),
                    DefaultColormap(state->display, state->screen),
                    &xft_color);
            }
        } else {
            XDrawString(
                state->display,
                state->window,
                state->graphics_context,
                command.x,
                baseline,
                command.text.c_str(),
                command.text.length());
        }
    }
}

void redraw(WindowState *state)
{
    if (state == nullptr) {
        return;
    }

    for (int index = 0; index < state->commands.size(); ++index) {
        draw_command(state, state->commands[index]);
    }
    XFlush(state->display);
}

void remove_state(WindowState *state)
{
    for (int index = 0; index < g_windows.size(); ++index) {
        if (g_windows[index] == state) {
            g_windows[index] = nullptr;
            return;
        }
    }
}
}

bool create_window(char *title, int width, int height, unsigned long long *handle)
{
    if (title == nullptr || handle == nullptr || width <= 0 || height <= 0) {
        set_last_error("invalid window title, size, or handle output");
        return false;
    }

    Display *display = XOpenDisplay(nullptr);
    if (display == nullptr) {
        set_last_error("cannot open X display; check DISPLAY and X11/XWayland availability");
        return false;
    }

    WindowState *state = new WindowState();
    if (state == nullptr) {
        set_last_error("failed to allocate window state");
        XCloseDisplay(display);
        return false;
    }

    state->display = display;
    state->screen = DefaultScreen(display);

    unsigned long white = WhitePixel(display, state->screen);
    unsigned long black = BlackPixel(display, state->screen);

    state->window = XCreateSimpleWindow(
        display,
        RootWindow(display, state->screen),
        0,
        0,
        static_cast<unsigned int>(width),
        static_cast<unsigned int>(height),
        1,
        black,
        white);

    if (state->window == 0) {
        set_last_error("XCreateSimpleWindow failed");
        delete state;
        XCloseDisplay(display);
        return false;
    }

    XStoreName(display, state->window, title);
    XSelectInput(display, state->window, ExposureMask | KeyPressMask | StructureNotifyMask);

    state->delete_message = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, state->window, &state->delete_message, 1);

    state->graphics_context = XCreateGC(display, state->window, 0, nullptr);
    if (state->graphics_context == 0) {
        set_last_error("XCreateGC failed");
        XDestroyWindow(display, state->window);
        delete state;
        XCloseDisplay(display);
        return false;
    }

    state->xft_draw = XftDrawCreate(
        display,
        state->window,
        DefaultVisual(display, state->screen),
        DefaultColormap(display, state->screen));
    if (state->xft_draw == nullptr) {
        set_last_error("XftDrawCreate failed");
        XFreeGC(display, state->graphics_context);
        XDestroyWindow(display, state->window);
        delete state;
        XCloseDisplay(display);
        return false;
    }

    XMapWindow(display, state->window);
    XFlush(display);

    *handle = from_state(state);
    g_windows.push_back(state);
    set_last_error(nullptr);
    return true;
}

void print_error(const char *message)
{
    std::fprintf(stderr, "Error: %s\n", message == nullptr ? "Unknown error" : message);
    if (g_last_error[0] != '\0') {
        std::fprintf(stderr, "Linux platform detail: %s\n", g_last_error);
    }
}

void log_serial(const char *message)
{
    if (message != nullptr) {
        std::fputs(message, stderr);
    }
}

void append_debug_log(const char *message)
{
    log_serial(message);
}

void refresh_window(unsigned long long handle)
{
    WindowState *state = to_state(handle);
    redraw(state);
}

void wait_window()
{
    while (true) {
        bool any_window = false;

        for (int index = 0; index < g_windows.size(); ++index) {
            WindowState *state = g_windows[index];
            if (state == nullptr || state->display == nullptr) {
                continue;
            }

            any_window = true;
            XEvent event{};
            XNextEvent(state->display, &event);

            if (event.type == Expose) {
                redraw(state);
            } else if (event.type == ClientMessage && static_cast<Atom>(event.xclient.data.l[0]) == state->delete_message) {
                delete_window(from_state(state));
            } else if (event.type == DestroyNotify) {
                remove_state(state);
            }
        }

        if (!any_window) {
            return;
        }
    }
}

bool delete_window(unsigned long long handle)
{
    WindowState *state = to_state(handle);
    if (state == nullptr || !has_state(state)) {
        return false;
    }

    remove_state(state);

    if (state->display != nullptr) {
        for (int index = 0; index < state->fonts.size(); ++index) {
            if (state->fonts[index].font != nullptr) {
                XftFontClose(state->display, state->fonts[index].font);
            }
        }
        if (state->xft_draw != nullptr) {
            XftDrawDestroy(state->xft_draw);
        }
        if (state->graphics_context != 0) {
            XFreeGC(state->display, state->graphics_context);
        }
        if (state->window != 0) {
            XDestroyWindow(state->display, state->window);
        }
        XCloseDisplay(state->display);
    }

    delete state;
    return true;
}

void draw_pixel(unsigned long long handle, int x, int y, unsigned int color)
{
    WindowState *state = to_state(handle);
    if (state == nullptr) {
        return;
    }

    DrawCommand command;
    command.type = DrawCommand::Pixel;
    command.x = x;
    command.y = y;
    command.color = color;
    state->commands.push_back(command);

    draw_command(state, command);
    XFlush(state->display);
}

void draw_text(unsigned long long handle, int x, int y, unsigned int color, unsigned int size, const stardustui::string& text)
{
    WindowState *state = to_state(handle);
    if (state == nullptr) {
        return;
    }

    DrawCommand command;
    command.type = DrawCommand::Text;
    command.x = x;
    command.y = y;
    command.color = color;
    command.size = size;
    command.text = text;
    state->commands.push_back(command);

    draw_command(state, command);
    XFlush(state->display);
}
