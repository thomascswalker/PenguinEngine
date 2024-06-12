#pragma once

#define P_PI 3.14159265358979323846f
#define P_EPSILON FLT_EPSILON
#define P_SINGULARITY_THRESHOLD 0.4999995f
#define P_DEG_TO_RAD 0.0174533f
#define P_RAD_TO_DEG 57.2958f
#define P_MIN_Z 0.001f
#define P_MAX_Z 100000.0f
#define P_ZERO_PRECISION = 0.0f
#define P_VERY_SMALL_NUMBER FLT_EPSILON
#define P_SMALL_NUMBER 0.0001f
#define P_LARGE_NUMBER 10000.0f
#define P_VERY_LARGE_NUMBER 10000000.0f

// Basic types
using int8 = char;
using int16 = short;
using int32 = int;
using int64 = long long;

using uint8 = unsigned char;
using uint16 = unsigned short;
using uint32 = unsigned int;
using uint64 = unsigned long long;

// Buffer
template <int32 N>
struct TChannel;
using PColorChannel = TChannel<4>;
using PDataChannel = TChannel<1>;

// Rect
template <typename T>
struct TRect;
using FRect = TRect<float>;
using DRect = TRect<double>;
using IRect = TRect<int32>;

// Lines
template <typename T>
struct TLine;
using FLine = TLine<float>;

template <typename T>
struct TLine3d;
using FLine3d = TLine3d<float>;

// Vector
template <typename T>
struct TVector2;
using IVector2 = TVector2<int32>;
using FVector2 = TVector2<float>;
using DVector2 = TVector2<double>;

template <typename T>
struct TVector3;
using IVector3 = TVector3<int32>;
using FVector3 = TVector3<float>;
using DVector3 = TVector3<double>;

template <typename T>
struct TVector4;
using IVector4 = TVector4<int32>;
using FVector4 = TVector4<float>;
using DVector4 = TVector4<double>;

template <typename T>
struct TTriangle;
using FTriangle = TTriangle<float>;
using DTriangle = TTriangle<double>;

// Matrix
template <typename T>
struct TMatrix;
using FMatrix = TMatrix<float>;

template <typename T>
struct TPerspectiveMatrix;
using FPerspectiveMatrix = TPerspectiveMatrix<float>;

template <typename T>
struct TLookAtMatrix;
using FLookAtMatrix = TLookAtMatrix<float>;

template <typename T>
struct TRotationMatrix;
using FRotationMatrix = TRotationMatrix<float>;

template <typename T>
struct TTranslationMatrix;
using FTranslationMatrix = TTranslationMatrix<float>;

template <typename T>
struct TRotationTranslationMatrix;
using FRotationTranslationMatrix = TRotationTranslationMatrix<float>;

// Rotator
template <typename T>
struct TRotator;
using FRotator = TRotator<float>;
using DRotator = TRotator<double>;

// Quaternion
template <typename T>
struct TQuat;
using FQuat = TQuat<float>;
using DQuat = TQuat<double>;

// Transform
template <typename T>
struct TTransform;
using FTransform = TTransform<float>;
using DTransform = TTransform<double>;

// Plane
template <typename T>
struct TPlane;
using FPlane = TPlane<float>;

// Enums
enum class EWindingOrder : uint8
{
    CW,
    // Clockwise
    CCW,
    // Counter-clockwise
    CL // Co-linear, in a line
};

// TODO: Swap Up and forward directions
enum class EAxis : uint8
{
    X,
    // Right
    Y,
    // Up
    Z // Foward
};

enum class EChannelType : uint8
{
    Data,
    Color
};

// Cameras

struct PViewData;
