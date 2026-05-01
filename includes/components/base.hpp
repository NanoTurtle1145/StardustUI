#pragma once
#ifdef  XJ380
#include "../../platforms/xj380.hpp"
#endif
class base_component
{
public:
    base_component();
    virtual ~base_component();
    base_component(const base_component&) = delete;
    base_component& operator=(const base_component&) = delete;
    base_component(base_component&&) = delete;
    base_component& operator=(base_component&&) = delete;
    virtual void draw(unsigned long long handle);
    virtual void update();
    virtual void callback(void (*func)());
    void set_pos(int x,int y){
        this->x=x;
        this->y=y;
    }
    void get_pos(int &x,int &y) const {
        x=this->x;
        y=this->y;
    }
protected:
    void (*callback_func)() = nullptr;   
    unsigned int x,y;
};
