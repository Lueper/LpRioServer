#pragma once

#include <mutex>

template <typename T>
class LpSingleton {
protected:
	LpSingleton() {}
	virtual ~LpSingleton() {}

public:
	static T* Instance() {
		if (m_instance == nullptr) {
			std::lock_guard<std::mutex> lock(m_mutex);

			if (m_instance == nullptr) {
				m_instance = new T;
			}
		}
		return m_instance;
	};

	static void DestroyInstance() {
		if (m_instance != nullptr) {
			delete m_instance;
			m_instance = nullptr;
		}
	}

private:
	static T* m_instance;
	static std::mutex m_mutex;
};

template <typename T>
T* LpSingleton<T>::m_instance = 0;
template <typename T>
std::mutex LpSingleton<T>::m_mutex;