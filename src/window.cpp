#include "../includes/window.hpp"

Window::Window(const char* title, int width, int height) {
	this->title.assign(title);
	this->width = width;
	this->height = height;
	this->handle = 0;
}

Window::~Window() {
	append_debug_log("stardustui: Window destructor\n");
	if (this->handle != 0) {
		append_debug_log("stardustui: destructor closing handle\n");
		delete_window(this->handle);
	}
}

void Window::show() {
	static constexpr char kCreateWindowFailed[] = "Failed to create window";
	append_debug_log("stardustui: Window::show enter\n");
	if (!create_window(this->title.data(), this->width, this->height, &this->handle)) {
		append_debug_log("stardustui: create_window failed\n");
		error(kCreateWindowFailed);
		return;
	}

	append_debug_log("stardustui: create_window ok\n");
	refresh_window(this->handle);
	append_debug_log("stardustui: refresh_window ok\n");
	for (int i = 0; i < components.size(); ++i) {
		components[i]->draw(this->handle);
	}
	append_debug_log("stardustui: wait_window enter\n");
	wait_window();
}

void Window::hide() {
	if (this->handle != 0) {
		append_debug_log("stardustui: Window::hide closing handle\n");
		delete_window(this->handle);
		this->handle = 0;
	}
}

int Window::getWidth() {
	return this->width;
}

int Window::getHeight() {
	return this->height;
}

const char* Window::getTitle() {
	return this->title.c_str();
}

void Window::error(const char* msg) {
	print_error(msg);
}

void Window::addComponent(base_component& component) {
	addComponent(&component);
}

void Window::addComponent(base_component* component) {
	if (component == nullptr) {
		return;
	}

	if (!this->components.push_back(component)) {
		error("Failed to grow component storage");
		return;
	}
}
