#include "../../includes/components/lable.hpp"
#include "../../settings.hpp"
#ifdef  XJ380
#include "../../platforms/xj380.hpp"
#endif
#ifdef STARDUSTUI_WINDOWS
#include "../../platforms/windows.hpp"
#endif
#ifdef STARDUSTUI_LINUX
#include "../../platforms/linux.hpp"
#endif
Lable::Lable(const stardustui::string& text, unsigned int size, unsigned int color)
    : text(text), size(size), color(color) {}

Lable::~Lable() = default;

void Lable::draw(unsigned long long handle)
{
    draw_text(handle, this->x, this->y, this->color, this->size, this->text);
}
