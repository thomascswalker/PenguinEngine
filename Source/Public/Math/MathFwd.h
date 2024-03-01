#pragma once

// Basic types
typedef char int8;
typedef short int16;
typedef int int32;
typedef long long int64;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;

// Rect
template <typename T> struct TRect;
typedef TRect<int> PRectI;
typedef TRect<float> PRectF;
typedef TRect<double> PRectD;

// Vector
template <typename T>
struct TVector2;
template <typename T>
struct TVector3;
template <typename T>
struct TVector4;

typedef TVector2<float> PVector2;
typedef TVector2<double> PVector2d;
typedef TVector3<float> PVector3;
typedef TVector3<double> PVector3d;
typedef TVector4<float> PVector4;
typedef TVector4<double> PVector4d;

template <typename T>
struct TBarycentric;
typedef TBarycentric<float> PBarycentric;

// Rotator
template <typename T>
struct TRotator;
typedef TRotator<float> PRotator;

// Quaternion
template <typename T>
struct TQuat;
typedef TQuat<float> PQuat;
