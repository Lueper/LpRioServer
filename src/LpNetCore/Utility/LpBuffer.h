#pragma once

#include "LpSpinLock.h"

class LpBuffer {
public:
	LpBuffer(uint32_t _size);
	~LpBuffer();

	void Clear();
	void Push(char* _data, uint32_t _size);
	void Pop(char* _data, uint32_t _size);
	bool Expand();

	char* GetBuffer();
	uint32_t GetUseSize();
	uint32_t GetAvailableSize();
	uint32_t GetBufferMaxSize();
	uint32_t GetOffset();

private:
	LpSpinLock m_spinLock;

	uint32_t m_maxSize;
	uint32_t m_useSize;
	uint32_t m_lCur;
	uint32_t m_rCur;
	char* m_buffer;
	int m_expandCount;
};