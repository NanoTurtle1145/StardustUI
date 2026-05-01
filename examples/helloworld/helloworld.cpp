#include "../../includes/window.hpp"
#include "../../includes/components/lable.hpp"

static int helloworld_main_impl(int, char**, char**) {
    append_debug_log("helloworld: main enter\n");
    Window window("Hello, World!", 400, 300);
    append_debug_log("helloworld: window constructed\n");
    Lable hello_label("Hello, World!", 24, 0x000000FF);
    hello_label.set_pos(100, 100);
    window.addComponent(hello_label);
    window.show();
    append_debug_log("helloworld: window.show returned\n");
    return 0;
}

extern "C" int helloworld_main_cpp(int argc, char *argv[], char *envp[]) asm("_Z4mainiPPcS0_");
extern "C" int helloworld_main_cpp(int argc, char *argv[], char *envp[])
{
    return helloworld_main_impl(argc, argv, envp);
}
