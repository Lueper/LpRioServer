#pragma once

#include "LpIOContext.h"

class LpSocketChannel : public std::enable_shared_from_this<LpSocketChannel> {
public:
	LpSocketChannel() = default;
	virtual ~LpSocketChannel() = default;

	virtual HANDLE GetHandle() = 0;
	virtual void ProcessIO(LpIOContext* ioContext, uint32_t bytes = 0) = 0;
private:
};