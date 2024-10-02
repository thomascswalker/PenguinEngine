#pragma once

#include <cstdint>
#include <numbers>
#include <intrin.h>

#include "Core/Types.h"

/** SSE **/

#if defined(_INCLUDED_MM2) || defined(_M_X64)

#define PENG_SSE

struct vecmu32
{
	union
	{
		uint32 u[4];
		__m128 v;
	};
};

/** 8-bit mask **/

constexpr auto g_maskX = 0b11101111;
constexpr auto g_maskY = 0b11011111;
constexpr auto g_maskZ = 0b10111111;
constexpr auto g_maskW = 0b01111111;

/** 32-bit mask **/

constexpr vecmu32 g_mask1 = {UINT32_MAX, 0, 0, 0};
constexpr vecmu32 g_mask2 = {UINT32_MAX, UINT32_MAX, 0, 0};
constexpr vecmu32 g_mask3 = {UINT32_MAX, UINT32_MAX, UINT32_MAX, 0};
constexpr vecmu32 g_mask4 = {UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX};

#endif

/** Constants **/

#define NOMINMAX

constexpr float g_pi     = std::numbers::pi_v<float>;
constexpr float g_2Pi    = 6.283185307f;
constexpr float g_invPi  = std::numbers::inv_pi_v<float>;
constexpr float g_inv2Pi = 0.159154943f;
constexpr float g_piDiv2 = 1.570796327f;
constexpr float g_piDiv4 = 0.785398163f;
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

#define DEPTH_1 1
#define DEPTH_2 2
#define DEPTH_4 4
#define DEPTH_8 8
#define DEPTH_16 16
#define DEPTH_32 32

// Buffer
template <int32 N>
struct TChannel;
using ColorChannel = TChannel<4>;
using DataChannel  = TChannel<1>;

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
	Clockwise,        // Clockwise
	CounterClockwise, // Counter-clockwise
	CoLinear          // Co-linear, in a line
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

struct ViewData;
