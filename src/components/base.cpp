#include "../../includes/components/base.hpp"
base_component::base_component()
    : mouse_active(false),
      click_active(false),
      hover_active(false),
      redraw_requested(false),
      x(0),
      y(0),
      width(0),
      height(0) {}

base_component::~base_component() = default;
void base_component::draw(unsigned long long) {}

void base_component::update() {
    if(this->callback_func){
        this->callback_func();
    }
}

void base_component::callback(void (*func)()) {
    this->callback_func = func;
}

void base_component::set_style_rules(const SytelRules& rules) {
    this->style_rules = rules;
}

const SytelRules& base_component::get_style_rules() const {
    return this->style_rules;
}

void base_component::clear_style_rules() {
    this->style_rules.clear();
}

void base_component::set_mouse_state(bool active) {
    this->mouse_active = active;
}

void base_component::set_click_state(bool active) {
    this->click_active = active;
}

void base_component::set_hover_state(bool active) {
    this->hover_active = active;
}

bool base_component::is_mouse_active() const {
    return this->mouse_active;
}

bool base_component::is_click_active() const {
    return this->click_active;
}

bool base_component::is_hover_active() const {
    return this->hover_active;
}

Sytel base_component::resolve_style() const {
    return this->style_rules.resolve(this->mouse_active, this->click_active, this->hover_active);
}

int base_component::get_preferred_width() const {
    return static_cast<int>(this->width);
}

int base_component::get_preferred_height() const {
    return static_cast<int>(this->height);
}

bool base_component::contains(int x, int y) const {
    return x >= static_cast<int>(this->x) &&
           y >= static_cast<int>(this->y) &&
           x < static_cast<int>(this->x + this->width) &&
           y < static_cast<int>(this->y + this->height);
}

void base_component::set_bounds(int x, int y, int width, int height) {
    this->x = static_cast<unsigned int>(x < 0 ? 0 : x);
    this->y = static_cast<unsigned int>(y < 0 ? 0 : y);
    this->width = static_cast<unsigned int>(width < 0 ? 0 : width);
    this->height = static_cast<unsigned int>(height < 0 ? 0 : height);
}

int base_component::get_width() const {
    return static_cast<int>(this->width);
}

int base_component::get_height() const {
    return static_cast<int>(this->height);
}

void base_component::request_redraw() {
    this->redraw_requested = true;
}

bool base_component::consume_redraw_request() {
    const bool requested = this->redraw_requested;
    this->redraw_requested = false;
    return requested;
}

bool base_component::has_pending_redraw() const {
    return this->redraw_requested;
}
