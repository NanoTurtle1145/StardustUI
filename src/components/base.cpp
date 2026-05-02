#include "../../includes/components/base.hpp"
base_component::base_component()
    : mouse_active(false),
      click_active(false),
      hover_active(false),
      x(0),
      y(0) {}

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

bool base_component::contains(int, int) const {
    return false;
}
