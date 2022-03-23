#pragma once
#include <vector>
#include "/scripts/code/dcheck.hpp"
#include <stdexcept>

template<class T>
class Vector : public std::vector<T> {
	using super = std::vector<T>;
	public:
		using std::vector<T>::vector;
		Vector(Vector&& other) = default;
		Vector(const Vector& other) = default;

		Vector(super&& other) : super(other)  {}
		Vector(const super& other) : super(other) {}

		Vector& operator=(const std::vector<T>& other) {
			super::operator=(other); return *this;
		}
		Vector& operator=(const Vector<T>& other) {
			super::operator=(other); return *this;
		}
		Vector& operator=(std::vector<T>&& other) { 
			super::operator=(other); return *this;
		}
		Vector& operator=(Vector<T>&& other) { 
			super::operator=(other); return *this;
		}
#ifndef NDEBUG
	T& operator[](size_t n) /*override*/ { 
		DCHECK_LT(n, this->size());
		return super::at(n); 
	}
	const T& operator[](size_t n) const /*override*/ { 
		DCHECK_LT(n, this->size());
		return super::at(n);
	}
#endif
};
