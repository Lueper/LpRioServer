#pragma once

#include <concurrent_queue.h>

template <typename T>
class LpPool {
public:
	LpPool() {}

	LpPool(size_t size) {
		for (size_t i = 0; i < size; i++) {
			m_pool.push(Alloc());
		}
	}

	virtual ~LpPool() {
		T* obj = nullptr;
		while (m_pool.try_pop(obj)) {
			delete obj;
		}
	}

	T* Alloc() {
		return new T();
	}

	T* Pop() {
		T* obj = nullptr;
		if (m_pool.try_pop(obj) == false)
			obj = Alloc();
		
		return obj;
	}

	void Push(T* obj) {
		m_pool.push(obj);
	}

private:
	concurrency::concurrent_queue<T*> m_pool;
};