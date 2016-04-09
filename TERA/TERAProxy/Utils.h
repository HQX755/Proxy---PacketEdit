#ifndef __UTILS__H_
#define __UTILS__H_

#pragma once

#ifdef _WIN32
#define _BYTE(x, y)		*(uint8_t*)		(reinterpret_cast<uint32_t*>(x + y))
#define _WORD(x, y)		*(uint16_t*)	(reinterpret_cast<uint32_t*>(x + y))
#define _DWORD(x, y)	*(uint32_t*)	(reinterpret_cast<uint32_t*>(x + y))
#else
#define _BYTE(x, y)		*(uint8_t*)		(reinterpret_cast<uint64_t*>(x + y))
#define _WORD(x, y)		*(uint16_t*)	(reinterpret_cast<uint64_t*>(x + y))
#define _DWORD(x, y)	*(uint32_t*)	(reinterpret_cast<uint64_t*>(x + y))
#endif

template<typename T>
inline void ChangeVirtualTable(T* _1, T* _2)
{
	_DWORD(_1, 0) = _DWORD(_2, 0);
}

template<typename T>
inline T* CopyHeap(T* data, size_t size)
{
	uint8_t* copy = new uint8_t[size];
	memcpy(copy, data, size);
	return copy;
}

#ifdef _DEBUG
#define DebugPrint printf
#else
#define DebugPrint(x, ...)
#endif
#endif