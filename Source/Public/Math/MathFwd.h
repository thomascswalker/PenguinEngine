﻿#pragma once

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
template <typename T>
struct TRect;
typedef TRect<float> FRect;
typedef TRect<double> DRect;
typedef TRect<int32> IRect;

// Vector
template <typename T>
struct TVector2;
typedef TVector2<int32> IVector2;
typedef TVector2<float> FVector2;
typedef TVector2<double> DVector2;

template <typename T>
struct TVector3;
typedef TVector3<float> FVector3;
typedef TVector3<double> DVector3;

template <typename T>
struct TVector4;
typedef TVector4<float> FVector4;
typedef TVector4<double> DVector4;

template <typename T>
struct TBarycentric;
typedef TBarycentric<float> FBarycentric;
typedef TBarycentric<double> DBarycentric;

// Matrix
template <typename T>
struct TMatrix;
typedef TMatrix<float> FMatrix;

template <typename T>
struct TPerspectiveMatrix;
typedef TPerspectiveMatrix<float> FPerspectiveMatrix;

template <typename T>
struct TReversedZPerspectiveMatrix;
typedef TReversedZPerspectiveMatrix<float> FReversedZPerspectiveMatrix;

template <typename T>
struct TLookAtMatrix;
typedef TLookAtMatrix<float> FLookAtMatrix;

template <typename T>
struct TInverseRotationMatrix;
typedef TInverseRotationMatrix<float> FInverseRotationMatrix;

template <typename T>
struct TTranslationMatrix;
typedef TTranslationMatrix<float> FTranslationMatrix;

// Rotator
template <typename T>
struct TRotator;
typedef TRotator<float> FRotator;
typedef TRotator<double> DRotator;

// Quaternion
template <typename T>
struct TQuat;
typedef TQuat<float> FQuat;
typedef TQuat<double> DQuat;

// Transform
template <typename T>
struct TTransform;
typedef TTransform<float> FTransform;
typedef TTransform<double> DTransform;

// Plane
template <typename T>
struct TPlane;
typedef TPlane<float> FPlane;
