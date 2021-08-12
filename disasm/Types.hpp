#pragma once

// CommonTypes.h

#include <vector>

#ifndef _PPSSPP_COMMON_TYPES_H_
#define _PPSSPP_COMMON_TYPES_H_

#if defined(_MSC_VER)

typedef unsigned __int8 u8;
typedef unsigned __int16 u16;
typedef unsigned __int32 u32;
typedef unsigned __int64 u64;

typedef signed __int8 s8;
typedef signed __int16 s16;
typedef signed __int32 s32;
typedef signed __int64 s64;

#else

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long long s64;

#endif // _WIN32

#endif

typedef u32 Opcode;
typedef std::vector<u32> Addresses;
typedef u32 refts;