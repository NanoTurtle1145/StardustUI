#include "../../includes/window.hpp"
#include "../../includes/components/button.hpp"
#include "../../includes/components/canvas.hpp"
#include "../../includes/components/flex.hpp"
#include "../../includes/components/lable.hpp"
#include "../../includes/components/textbox.hpp"
#include "../../includes/sytle.hpp"
#include "../../includes/theme.hpp"
#include "../../platforms/platform.hpp"

namespace {
TextBox* g_history_box = nullptr;
TextBox* g_input_box = nullptr;
Lable* g_status_label = nullptr;

void append_text(stardustui::string& target, const char* text) {
    if (text != nullptr) {
        target.append(text);
    }
}

void append_line(stardustui::string& target, const char* text) {
    append_text(target, text);
    target.push_char('\n');
}

void draw_sidebar_avatar(Canvas& canvas, unsigned int color, unsigned int accent) {
    canvas.fill_rect(0, 0, canvas.get_width(), canvas.get_height(), color);
    canvas.fill_rect(4, 4, canvas.get_width() - 8, canvas.get_height() - 8, accent);
}

void draw_avatar_one(Canvas& canvas) {
    draw_sidebar_avatar(canvas, 0x2D6AE3FF, 0xAFC9FFFF);
}

void draw_avatar_two(Canvas& canvas) {
    draw_sidebar_avatar(canvas, 0x178F65FF, 0xA8E5CDFF);
}

void draw_avatar_three(Canvas& canvas) {
    draw_sidebar_avatar(canvas, 0xC6671FFF, 0xFFD0AEFF);
}

void paint_static_avatar(Canvas& canvas, void (*drawer)(Canvas&)) {
    canvas.clear();
    if (drawer != nullptr) {
        drawer(canvas);
    }
}

void on_send_click() {
    if (g_history_box == nullptr || g_input_box == nullptr || g_status_label == nullptr) {
        return;
    }

    const stardustui::string& input = g_input_box->get_text();
    if (input.length() <= 0) {
        g_status_label->set_text("Type something before sending");
        return;
    }

    stardustui::string next_history;
    const stardustui::string& current_history = g_history_box->get_text();
    if (current_history.length() > 0) {
        next_history.append(current_history.c_str());
        next_history.push_char('\n');
    }
    next_history.append("Me: ");
    next_history.append(input.c_str());

    g_history_box->set_text(next_history);
    g_input_box->set_text("");
    g_status_label->set_text("Message sent");
}

SytelRules make_panel_rules(unsigned int background,
                            unsigned int border,
                            unsigned int border_width = 1,
                            unsigned int radius = 20) {
    Sytel panel;
    panel.set_background_color(background);
    panel.set_border_color(border);
    panel.set_border_width(border_width);
    panel.set_radius(radius);

    SytelRules rules;
    rules.set_base_sytel(panel);
    return rules;
}
}

static int layout_main_impl(int argc, char** argv, char**) {
    const char* theme_to_load = "md3-light";
    if (argc > 1 && argv != nullptr && argv[1] != nullptr && argv[1][0] != '\0') {
        theme_to_load = argv[1];
    }
    stardustui::Theme::load_theme(theme_to_load);
    const stardustui::Colors& colors = stardustui::Theme::colors();
    stardustui::string window_title("StardustUI Chat Demo");
    if (stardustui::Theme::name().length() > 0) {
        window_title.append(" - ");
        window_title.append(stardustui::Theme::name().c_str());
    }
    Window window(window_title.c_str(), 1040, 680, true);

    FlexLayout root(1000, 640);
    root.set_pos(20, 20);
    root.set_anchors(base_component::AnchorLeft |
                     base_component::AnchorTop |
                     base_component::AnchorRight |
                     base_component::AnchorBottom);
    root.set_direction(FlexLayout::Row);
    root.set_gap(0);
    root.set_style_rules(make_panel_rules(colors.background, colors.background, 0));

    FlexLayout sidebar(260, 0);
    sidebar.set_direction(FlexLayout::Column);
    sidebar.set_gap(12);
    sidebar.set_padding(18);
    sidebar.set_style_rules(make_panel_rules(colors.surface_variant, colors.outline_variant));

    Lable sidebar_title("Conversations", 24, colors.on_surface);
    Lable sidebar_hint("Pinned teams and channels", 14, colors.on_surface_variant);

    Sytel contact_base;
    contact_base.set_color(colors.on_surface);
    contact_base.set_size(15);
    contact_base.set_background_color(colors.surface);
    contact_base.set_border_color(colors.outline_variant);
    contact_base.set_border_width(1);
    contact_base.set_padding(12);
    contact_base.set_radius(20);

    Sytel contact_hover;
    contact_hover.set_background_color(colors.primary_container);
    contact_hover.set_border_color(colors.primary);

    Sytel contact_click;
    contact_click.set_background_color(colors.primary);
    contact_click.set_color(colors.on_primary);
    contact_click.set_border_color(colors.primary);

    SytelRules contact_rules;
    contact_rules.set_base_sytel(contact_base);
    contact_rules.set_on_hover_sytel(contact_hover);
    contact_rules.set_on_click_sytel(contact_click);

    FlexLayout contact_one(0, 54);
    contact_one.set_direction(FlexLayout::Row);
    contact_one.set_gap(10);
    contact_one.set_align_items(FlexLayout::AlignCenter);
    Canvas avatar_one(28, 28);
    paint_static_avatar(avatar_one, draw_avatar_one);
    Button chat_one("Project Nebula", 0, 44, contact_rules);
    contact_one.addComponent(avatar_one, 0);
    contact_one.addComponent(chat_one, 1);

    FlexLayout contact_two(0, 54);
    contact_two.set_direction(FlexLayout::Row);
    contact_two.set_gap(10);
    contact_two.set_align_items(FlexLayout::AlignCenter);
    Canvas avatar_two(28, 28);
    paint_static_avatar(avatar_two, draw_avatar_two);
    Button chat_two("Rendering Squad", 0, 44, contact_rules);
    contact_two.addComponent(avatar_two, 0);
    contact_two.addComponent(chat_two, 1);

    FlexLayout contact_three(0, 54);
    contact_three.set_direction(FlexLayout::Row);
    contact_three.set_gap(10);
    contact_three.set_align_items(FlexLayout::AlignCenter);
    Canvas avatar_three(28, 28);
    paint_static_avatar(avatar_three, draw_avatar_three);
    Button chat_three("Design Review", 0, 44, contact_rules);
    contact_three.addComponent(avatar_three, 0);
    contact_three.addComponent(chat_three, 1);

    sidebar.addComponent(sidebar_title, 0);
    sidebar.addComponent(sidebar_hint, 0);
    sidebar.addComponent(contact_one, 0);
    sidebar.addComponent(contact_two, 0);
    sidebar.addComponent(contact_three, 0);

    FlexLayout main_column(0, 0);
    main_column.set_direction(FlexLayout::Column);
    main_column.set_gap(12);
    main_column.set_padding(12);
    main_column.set_style_rules(make_panel_rules(colors.background, colors.background, 0));

    FlexLayout header_content(0, 84);
    header_content.set_direction(FlexLayout::Row);
    header_content.set_padding(20);
    header_content.set_align_items(FlexLayout::AlignCenter);
    header_content.set_justify_content(FlexLayout::JustifySpaceBetween);
    header_content.set_style_rules(make_panel_rules(colors.surface, colors.outline_variant));

    Lable room_title("Project Nebula", 28, colors.on_surface);
    Lable room_meta("4 people online", 14, colors.on_surface_variant);

    Sytel action_base;
    action_base.set_color(colors.on_secondary_container);
    action_base.set_size(14);
    action_base.set_background_color(colors.secondary_container);
    action_base.set_border_color(colors.secondary_container);
    action_base.set_border_width(1);
    action_base.set_padding(10);
    action_base.set_radius(20);

    Sytel action_hover;
    action_hover.set_background_color(colors.secondary);
    action_hover.set_border_color(colors.secondary);
    action_hover.set_color(colors.on_secondary);

    SytelRules action_rules;
    action_rules.set_base_sytel(action_base);
    action_rules.set_on_hover_sytel(action_hover);

    Button details_button("Details", 110, 40, action_rules);

    FlexLayout header_labels(0, 44);
    header_labels.set_direction(FlexLayout::Column);
    header_labels.set_gap(4);
    header_labels.addComponent(room_title, 0);
    header_labels.addComponent(room_meta, 0);

    header_content.addComponent(header_labels, 1);
    header_content.addComponent(details_button, 0);

    Sytel history_base;
    history_base.set_color(colors.on_surface);
    history_base.set_size(16);
    history_base.set_background_color(colors.surface);
    history_base.set_border_color(colors.outline_variant);
    history_base.set_border_width(1);
    history_base.set_padding(14);
    history_base.set_radius(16);

    Sytel history_hover;
    history_hover.set_border_color(colors.primary);

    SytelRules history_rules;
    history_rules.set_base_sytel(history_base);
    history_rules.set_on_hover_sytel(history_hover);

    TextBox history_box(0, 0, false, history_rules);
    stardustui::string history_text;
    append_line(history_text, "Mina: Build is ready. 中文测试");
    append_line(history_text, "Leo: Linux layout is stable.");
    append_line(history_text, "Rin: Windows needs one more pass.");
    append_line(history_text, "Mina: Keep this as the flex demo.");
    history_box.set_text(history_text);
    g_history_box = &history_box;

    FlexLayout composer(0, 150);
    composer.set_direction(FlexLayout::Column);
    composer.set_gap(12);
    composer.set_padding(16);
    composer.set_style_rules(make_panel_rules(colors.surface, colors.outline_variant));

    Lable composer_hint("Message", 14, colors.on_surface_variant);

    Sytel input_base = history_base;
    input_base.set_background_color(colors.background);

    Sytel input_click;
    input_click.set_border_color(colors.primary);

    SytelRules input_rules;
    input_rules.set_base_sytel(input_base);
    input_rules.set_on_hover_sytel(history_hover);
    input_rules.set_on_click_sytel(input_click);

    FlexLayout composer_row(0, 92);
    composer_row.set_direction(FlexLayout::Row);
    composer_row.set_gap(12);
    composer_row.set_align_items(FlexLayout::AlignStretch);
    composer_row.set_style_rules(make_panel_rules(colors.surface, colors.surface, 0));

    TextBox input_box(0, 92, true, input_rules);
    input_box.set_text("");
    g_input_box = &input_box;

    Sytel send_base;
    send_base.set_color(colors.on_primary);
    send_base.set_size(16);
    send_base.set_background_color(colors.primary);
    send_base.set_border_color(colors.primary);
    send_base.set_border_width(1);
    send_base.set_padding(12);
    send_base.set_radius(24);

    Sytel send_hover;
    send_hover.set_background_color(colors.secondary);
    send_hover.set_border_color(colors.secondary);
    send_hover.set_color(colors.on_secondary);

    Sytel send_click;
    send_click.set_background_color(colors.tertiary);
    send_click.set_border_color(colors.tertiary);
    send_click.set_color(colors.on_tertiary);

    SytelRules send_rules;
    send_rules.set_base_sytel(send_base);
    send_rules.set_on_hover_sytel(send_hover);
    send_rules.set_on_click_sytel(send_click);

    Button send_button("Send", 120, 92, send_rules);
    send_button.callback(on_send_click);

    Lable status_label("Flex chat demo ready", 13, colors.on_surface_variant);
    g_status_label = &status_label;

    composer_row.addComponent(input_box, 1);
    composer_row.addComponent(send_button, 0);
    composer.addComponent(composer_hint, 0);
    composer.addComponent(composer_row, 1);
    composer.addComponent(status_label, 0);

    main_column.addComponent(header_content, 0);
    main_column.addComponent(history_box, 1);
    main_column.addComponent(composer, 0);

    window.addComponent(root);
    root.addComponent(sidebar, 0);
    root.addComponent(main_column, 1);
    window.show();
    return 0;
}

#if defined(STARDUSTUI_WINDOWS) || defined(STARDUSTUI_LINUX)
int main(int argc, char *argv[], char *envp[])
{
    return layout_main_impl(argc, argv, envp);
}
#else
extern "C" int layout_main_cpp(int argc, char *argv[], char *envp[]) asm("_Z4mainiPPcS0_");
extern "C" int layout_main_cpp(int argc, char *argv[], char *envp[])
{
    return layout_main_impl(argc, argv, envp);
}
#endif
