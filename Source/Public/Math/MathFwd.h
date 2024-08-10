#pragma once

#define PI 3.14159265358979323846f
#define EPSILON FLT_EPSILON
#define SINGULARITY_THRESHOLD 0.4999995f
#define DEG_TO_RAD 0.0174533f
#define RAD_TO_DEG 57.2958f
#define MIN_Z 0.001f
#define MAX_Z 100000.0f
#define ZERO_PRECISION = 0.0f
#define VERY_SMALL_NUMBER FLT_EPSILON
#define SMALL_NUMBER 0.0001f
#define LARGE_NUMBER 10000.0f
#define VERY_LARGE_NUMBER 10000000.0f

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
using ColorChannel = TChannel<4>;
using DataChannel = TChannel<1>;

// Rect
template <typename T>
struct TRect;
using rectf = TRect<float>;
using rectd = TRect<double>;
using recti = TRect<int32>;

// Lines
template <typename T>
struct TLine;
using linef = TLine<float>;
using linei = TLine<int32>;

template <typename T>
struct TLine3d;
using line3d = TLine3d<float>;

// Vector
template <typename T>
struct TVector2;
using vec2i = TVector2<int32>;
using vec2f = TVector2<float>;
using vec2d = TVector2<double>;

template <typename T>
struct TVector3;
using vec3i = TVector3<int32>;
using vec3f = TVector3<float>;
using vec3d = TVector3<double>;

template <typename T>
struct TVector4;
using vec4i = TVector4<int32>;
using vec4f = TVector4<float>;
using vec4d = TVector4<double>;

template <typename T>
struct TTriangle;
using trianglef = TTriangle<float>;
using triangled = TTriangle<double>;

// Matrix
template <typename T>
struct TMatrix;
using mat4f = TMatrix<float>;

template <typename T>
struct TPerspectiveMatrix;
using mat4f_persp = TPerspectiveMatrix<float>;

template <typename T>
struct TLookAtMatrix;
using mat4f_lookat = TLookAtMatrix<float>;

template <typename T>
struct TRotationMatrix;
using mat4f_rot = TRotationMatrix<float>;

template <typename T>
struct TTranslationMatrix;
using mat4f_trans = TTranslationMatrix<float>;

template <typename T>
struct TRotationTranslationMatrix;
using mat4f_rottrans = TRotationTranslationMatrix<float>;

// Rotator
template <typename T>
struct TRotator;
using rotf = TRotator<float>;
using rotd = TRotator<double>;

// Quaternion
template <typename T>
struct TQuat;
using quatf = TQuat<float>;
using quatd = TQuat<double>;

// Transform
template <typename T>
struct TTransform;
using transf = TTransform<float>;
using transd = TTransform<double>;

// Plane
template <typename T>
struct TPlane;
using planef = TPlane<float>;

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
