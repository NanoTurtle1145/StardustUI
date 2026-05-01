#pragma once
#include "base.hpp"
#include "../string.hpp"
class Lable : public base_component
{
public:
    Lable(const stardustui::string& text, unsigned int size, unsigned int color);
    ~Lable() override;
    void draw(unsigned long long handle) override;
private:
    stardustui::string text;
    unsigned int size;
    unsigned int color;
};
