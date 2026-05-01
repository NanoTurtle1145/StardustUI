#include "../../includes/components/base.hpp"
base_component::base_component() = default;
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
