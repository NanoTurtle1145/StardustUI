#include "../../includes/window.hpp"
#include "../../includes/components/button.hpp"
#include "../../includes/components/lable.hpp"
#include "../../includes/sytle.hpp"

namespace {
Lable* g_status_label = nullptr;

void on_button_click() {
    if (g_status_label != nullptr) {
        g_status_label->set_text("Button clicked!");
    }
}
}

static int helloworld_main_impl(int, char**, char**) {
    append_debug_log("helloworld: main enter\n");
    Window window("Hello, World!", 400, 300);
    append_debug_log("helloworld: window constructed\n");

    Sytel base_style;
    base_style.set_color(0x000000FF);
    base_style.set_size(24);

    Sytel hover_style;
    hover_style.set_color(0xFF0000FF);

    SytelRules rules;
    rules.set_base_sytel(base_style);
    rules.set_on_hover_sytel(hover_style);

    Lable hello_label("Hello, World!", 24, 0x000000FF);
    hello_label.set_style_rules(rules);
    hello_label.set_pos(100, 72);

    Lable status_label("Click the button", 16, 0x444444FF);
    status_label.set_pos(112, 220);
    g_status_label = &status_label;

    Sytel button_base;
    button_base.set_color(0x000000FF);
    button_base.set_size(16);
    button_base.set_background_color(0xDCDCDCFF);
    button_base.set_border_color(0x707070FF);
    button_base.set_border_width(1);
    button_base.set_padding(12);

    Sytel button_hover;
    button_hover.set_background_color(0xFFB347FF);

    Sytel button_click;
    button_click.set_background_color(0xFF8C42FF);
    button_click.set_color(0xFFFFFFFF);

    SytelRules button_rules;
    button_rules.set_base_sytel(button_base);
    button_rules.set_on_hover_sytel(button_hover);
    button_rules.set_on_click_sytel(button_click);

    Button button("Click Me", 160, 48, button_rules);
    button.set_pos(112, 140);
    button.callback(on_button_click);

    window.addComponent(hello_label);
    window.addComponent(button);
    window.addComponent(status_label);
    window.show();
    append_debug_log("helloworld: window.show returned\n");
    return 0;
}

#if defined(STARDUSTUI_WINDOWS) || defined(STARDUSTUI_LINUX)
int main(int argc, char *argv[], char *envp[])
{
    return helloworld_main_impl(argc, argv, envp);
}
#else
extern "C" int helloworld_main_cpp(int argc, char *argv[], char *envp[]) asm("_Z4mainiPPcS0_");
extern "C" int helloworld_main_cpp(int argc, char *argv[], char *envp[])
{
    return helloworld_main_impl(argc, argv, envp);
}
#endif
