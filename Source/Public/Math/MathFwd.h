﻿#pragma once

#define NOMINMAX

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
struct rect_t;
using rectf = rect_t<float>;
using rectd = rect_t<double>;
using recti = rect_t<int32>;

// Lines
template <typename T>
struct line_t;
using linef = line_t<float>;
using linei = line_t<int32>;

template <typename T>
struct line3d_t;
using line3d = line3d_t<float>;

// Vector
template <typename T>
struct vec2_t;
using vec2i = vec2_t<int32>;
using vec2f = vec2_t<float>;
using vec2d = vec2_t<double>;

template <typename T>
struct vec3_t;
using vec3i = vec3_t<int32>;
using vec3f = vec3_t<float>;
using vec3d = vec3_t<double>;

template <typename T>
struct vec4_t;
using vec4i = vec4_t<int32>;
using vec4f = vec4_t<float>;
using vec4d = vec4_t<double>;

template <typename T>
struct triangle_t;
using trianglef = triangle_t<float>;
using triangled = triangle_t<double>;

// Matrix
template <typename T>
struct mat4_t;
using mat4f = mat4_t<float>;

template <typename T>
struct mat4_persp_t;
using mat4f_persp = mat4_persp_t<float>;

template <typename T>
struct mat4_lookat_t;
using mat4f_lookat = mat4_lookat_t<float>;

template <typename T>
struct mat4_rot_t;
using mat4f_rot = mat4_rot_t<float>;

template <typename T>
struct mat4_trans_t;
using mat4f_trans = mat4_trans_t<float>;

template <typename T>
struct mat4_rottrans_t;
using mat4f_rottrans = mat4_rottrans_t<float>;

// Rotator
template <typename T>
struct rot_t;
using rotf = rot_t<float>;
using rotd = rot_t<double>;

// Quaternion
template <typename T>
struct quat_t;
using quatf = quat_t<float>;
using quatd = quat_t<double>;

// Transform
template <typename T>
struct trans_t;
using transf = trans_t<float>;
using transd = trans_t<double>;

// Plane
template <typename T>
struct plane_t;
using planef = plane_t<float>;

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
