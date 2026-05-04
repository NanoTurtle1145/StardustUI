#pragma once

namespace stardustui {
template<typename T>
class vector {
public:
	vector() : items(nullptr), count(0), storage(0) {}

	~vector() {
		delete[] this->items;
	}

	vector(const vector& other) : items(nullptr), count(0), storage(0) {
		copy_from(other);
	}

	vector& operator=(const vector& other) {
		if (this != &other) {
			vector copy(other);
			swap(copy);
		}
		return *this;
	}

	vector(vector&& other) noexcept : items(other.items), count(other.count), storage(other.storage) {
		other.items = nullptr;
		other.count = 0;
		other.storage = 0;
	}

	vector& operator=(vector&& other) noexcept {
		if (this != &other) {
			delete[] this->items;
			this->items = other.items;
			this->count = other.count;
			this->storage = other.storage;
			other.items = nullptr;
			other.count = 0;
			other.storage = 0;
		}
		return *this;
	}

	bool push_back(const T& value) {
		if (this->count >= this->storage && !reserve(this->storage == 0 ? 8 : this->storage * 2)) {
			return false;
		}

		this->items[this->count++] = value;
		return true;
	}

	bool reserve(int new_capacity) {
		if (new_capacity <= this->storage) {
			return true;
		}

		T* new_items = new T[new_capacity];
		if (new_items == nullptr) {
			return false;
		}

		for (int index = 0; index < this->count; ++index) {
			new_items[index] = this->items[index];
		}

		delete[] this->items;
		this->items = new_items;
		this->storage = new_capacity;
		return true;
	}

	int size() const {
		return this->count;
	}

	int capacity() const {
		return this->storage;
	}

	bool empty() const {
		return this->count == 0;
	}

	void clear() {
		this->count = 0;
	}

	void release_storage() {
		delete[] this->items;
		this->items = nullptr;
		this->count = 0;
		this->storage = 0;
	}

	T& operator[](int index) {
		return this->items[index];
	}

	const T& operator[](int index) const {
		return this->items[index];
	}

	T* at(int index) {
		if (index < 0 || index >= this->count) {
			return nullptr;
		}

		return &this->items[index];
	}

	const T* at(int index) const {
		if (index < 0 || index >= this->count) {
			return nullptr;
		}

		return &this->items[index];
	}

private:
	void copy_from(const vector& other) {
		if (other.storage == 0) {
			return;
		}

		T* new_items = new T[other.storage];
		if (new_items == nullptr) {
			return;
		}

		for (int index = 0; index < other.count; ++index) {
			new_items[index] = other.items[index];
		}

		this->items = new_items;
		this->count = other.count;
		this->storage = other.storage;
	}

	void swap(vector& other) noexcept {
		T* old_items = this->items;
		this->items = other.items;
		other.items = old_items;

		int old_count = this->count;
		this->count = other.count;
		other.count = old_count;

		int old_storage = this->storage;
		this->storage = other.storage;
		other.storage = old_storage;
	}

	T* items;
	int count;
	int storage;
};
}
