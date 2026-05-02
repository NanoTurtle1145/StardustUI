#include "../../includes/components/lable.hpp"
#include "../../settings.hpp"
#ifdef  XJ380
#include "../../platforms/xj380.hpp"
#endif
Lable::Lable(const stardustui::string& text, unsigned int size, unsigned int color)
    : text(text), size(size), color(color) {}

Lable::Lable(const stardustui::string& text, unsigned int size, const SytelRules& style)
    : text(text), size(size), color(0) {
    this->set_style_rules(style);
}

Lable::~Lable() = default;

bool Lable::contains(int x, int y) const {
    const Sytel style = this->resolve_style();
    const unsigned int resolved_size = style.get_size(this->size);
    const unsigned int width = calc_text_width(this->text, resolved_size);
    const unsigned int height = resolved_size;

    return x >= static_cast<int>(this->x) &&
           y >= static_cast<int>(this->y) &&
           x < static_cast<int>(this->x + width) &&
           y < static_cast<int>(this->y + height);
}

void Lable::set_text(const stardustui::string& text) {
    this->text = text;
}

const stardustui::string& Lable::get_text() const {
    return this->text;
}

void Lable::draw(unsigned long long handle)
{
    const Sytel style = this->resolve_style();
    const unsigned int resolved_color = style.get_color(this->color);
    const unsigned int resolved_size = style.get_size(this->size);

    draw_text(handle, this->x, this->y, resolved_color, resolved_size, this->text);
}
