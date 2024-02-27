#pragma once

typedef char int8;
typedef short int16;
typedef int int32;
typedef long long int64;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;

#define CHECK_FP_TYPE(T) if (!std::is_floating_point<T>()) { throw std::runtime_error("Type is not floating point."); }