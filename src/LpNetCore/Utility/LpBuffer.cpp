#include "pch.h"

#include "LpBuffer.h"

LpBuffer::LpBuffer(uint32_t _size)
	: m_maxSize(_size), m_useSize(0), m_lCur(0), m_rCur(0), m_buffer(nullptr), m_expandCount(10), m_spinLock() {
	m_buffer = new char[m_maxSize];
	memset(m_buffer, 0, m_maxSize);
}

LpBuffer::~LpBuffer() {
	delete[] m_buffer;
}

void LpBuffer::Clear() {
	m_spinLock.Lock();

	m_useSize = 0;
	m_lCur = 0;
	m_rCur = 0;

	m_spinLock.UnLock();
}

void LpBuffer::Push(char* _data, uint32_t _size) {
	m_spinLock.Lock();

	if (_data == nullptr || _size <= 0) {
		m_spinLock.UnLock();
		return;
	}

	if (_size >= m_maxSize - m_useSize) {
		for (int i = 0; i < m_expandCount; i++) {
			Expand();
			if (_size <= m_maxSize - m_useSize) {
				break;
			}
		}

		if (_size > m_maxSize - m_useSize) {
			m_spinLock.UnLock();
			return;
		}
	}

	if (_size <= m_maxSize - m_rCur) {
		memcpy(m_buffer + m_rCur, _data, _size);

		m_rCur += _size;
	} else {
		uint32_t frontSize = m_maxSize - m_rCur;
		uint32_t backSize = _size - frontSize;

		memcpy(m_buffer + m_rCur, _data, frontSize);
		memcpy(m_buffer, _data + frontSize, backSize);

		m_rCur = backSize;
	}

	m_useSize += _size;

	m_spinLock.UnLock();
}

void LpBuffer::Pop(char* _data, uint32_t _size) {
	m_spinLock.Lock();

	if (_data == nullptr || _size <= 0 || _size > m_maxSize || _size > m_useSize) {
		m_spinLock.UnLock();
		return;
	}

	if (_size <= m_maxSize - m_lCur) {
		memcpy(_data, m_buffer + m_lCur, _size);

		m_lCur += _size;
	} else {
		uint32_t frontSize = m_maxSize - m_lCur;
		uint32_t backSize = _size - frontSize;

		memcpy(_data, m_buffer + m_lCur, frontSize);
		memcpy(_data + frontSize, m_buffer, backSize);

		m_lCur = backSize;
	}

	m_useSize -= _size;

	m_spinLock.UnLock();
}

bool LpBuffer::Expand() {
	int expandSize = m_maxSize * 2;

	char* tmp = (char*)realloc(m_buffer, sizeof(char) * expandSize);
	if (tmp == nullptr) {
		return false;
	} else {
		m_buffer = tmp;
	}

	if (m_lCur > m_rCur) {
		memcpy(m_buffer + m_maxSize, m_buffer, m_rCur);
		m_rCur = m_lCur + m_useSize;
	}

	m_maxSize = expandSize;
	return true;
}

char* LpBuffer::GetBuffer() {
	m_spinLock.Lock();

	if (m_rCur >= m_maxSize) {
		// error!
	}

	m_spinLock.UnLock();

	return &(m_buffer[m_rCur]);
}

uint32_t LpBuffer::GetUseSize() {
	m_spinLock.Lock();

	uint32_t size = m_useSize;

	m_spinLock.UnLock();

	return size;
}

uint32_t LpBuffer::GetAvailableSize() {
	m_spinLock.Lock();

	uint32_t size = m_maxSize - m_useSize;

	m_spinLock.UnLock();

	return size;
}

uint32_t LpBuffer::GetBufferMaxSize() {
	m_spinLock.Lock();

	uint32_t size = m_maxSize;

	m_spinLock.UnLock();

	return size;
}

uint32_t LpBuffer::GetOffset() {
	m_spinLock.Lock();

	uint32_t size = m_rCur;

	m_spinLock.UnLock();

	return size;
}