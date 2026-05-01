#pragma once
#include "../settings.hpp"
#ifdef  XJ380
#include "../platforms/xj380.hpp"
#endif
#include "components/base.hpp"
#include "string.hpp"
#include "vector.hpp"

class Window {
  public:
	Window(const char* title, int width, int height);
	~Window();
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;
	void show();
	void hide();
	int getWidth();
	int getHeight();
	const char* getTitle();
	void error(const char*);
	void addComponent(base_component& component);
	void addComponent(base_component* component);
private:
	stardustui::string title;
	int width;
	int height;
	unsigned long long handle;
	stardustui::vector<base_component*> components; 
};
