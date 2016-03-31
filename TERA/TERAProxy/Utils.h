#ifndef __UTILS__H_
#define __UTILS__H_

#pragma once

#ifdef _WIN32
#define _BYTE(x, y)		*(uint8_t*)		(reinterpret_cast<uint32_t*>(x) + y)
#define _WORD(x, y)		*(uint16_t*)	(reinterpret_cast<uint32_t*>(x) + y)
#define _DWORD(x, y)	*(uint32_t*)	(reinterpret_cast<uint32_t*>(x) + y)
#else
#define _BYTE(x, y)		*(uint8_t*)		(reinterpret_cast<uint64_t*>(x) + y)
#define _WORD(x, y)		*(uint16_t*)	(reinterpret_cast<uint64_t*>(x) + y)
#define _DWORD(x, y)	*(uint32_t*)	(reinterpret_cast<uint64_t*>(x) + y)
#endif

#ifdef _DEBUG
#define DebugPrint printf
#else
#define DebugPrint(x, ...)
#endif
#endif