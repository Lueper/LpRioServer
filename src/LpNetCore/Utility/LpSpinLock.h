#pragma once

#include <atomic>

class LpSpinLock {
public:
	void Lock() {
		int expected = 0;
		int desired = 1;

		while (m_lock.compare_exchange_strong(expected, desired, std::memory_order_acquire) == false) {
			expected = 0;
		}
	}

	void UnLock() {
		m_lock.store(0, std::memory_order_release);
	}

private:
	std::atomic<int> m_lock{};
};