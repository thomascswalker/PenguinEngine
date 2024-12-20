﻿// ReSharper disable CppInconsistentNaming
#pragma once

#include <map>

#include "Core/Bitmask.h"
#include "Math/Vector.h"
#include "Engine/Delegate.h"
#include "Input/Mouse.h"

class IInputHandler;
class Win32InputHandler;
class MacOSInputHandler;
class LinuxInputHandler;

enum class EMouseButtonType : uint8
{
	Invalid,
	Left,
	Right,
	Middle
};

enum class EKey : uint8
{
	A,
	B,
	C,
	D,
	E,
	F,
	G,
	H,
	I,
	J,
	K,
	L,
	M,
	N,
	O,
	P,
	Q,
	R,
	S,
	T,
	U,
	V,
	W,
	X,
	Y,
	Z,
	One,
	Two,
	Three,
	Four,
	Five,
	Six,
	Seven,
	Eight,
	Nine,
	Zero,
	F1,
	F2,
	F3,
	F4,
	F5,
	F6,
	F7,
	F8,
	F9,
	F10,
	F11,
	F12,
	Escape,
	Spacebar,
	Enter,
	Ctrl,
	Shift,
	Alt,
	CapsLock,
	Backspace,

	// Meta
	Count
};

enum class EModifierKey : uint8
{
	None = 0x00,
	Shift = 0x01,
	Ctrl = 0x02,
	Alt = 0x04
};

DEFINE_BITMASK_OPERATORS(EModifierKey)

enum class EMenuAction : uint8
{
	// File
	LoadModel,
	LoadTexture,
	Quit,

	// Display
	Wireframe,
	Shaded,
	Depth,
	Normals,
	VertexNormals,
};

DECLARE_MULTICAST_DELEGATE(OnMouseMoved, MouseData&);
DECLARE_MULTICAST_DELEGATE(OnMouseLeftDown, MouseData&);
DECLARE_MULTICAST_DELEGATE(OnMouseLeftUp, MouseData&);
DECLARE_MULTICAST_DELEGATE(OnMouseRightDown, MouseData&);
DECLARE_MULTICAST_DELEGATE(OnMouseRightUp, MouseData&);
DECLARE_MULTICAST_DELEGATE(OnMouseMiddleDown, MouseData&);
DECLARE_MULTICAST_DELEGATE(OnMouseMiddleUp, MouseData&);
DECLARE_MULTICAST_DELEGATE(OnMouseMiddleScrolled, MouseData&);

DECLARE_MULTICAST_DELEGATE(OnKeyPressed, EKey);

DECLARE_MULTICAST_DELEGATE(OnMenuActionPressed, EMenuAction);

class IInputHandler
{
protected:
	vec2f m_clickPosition;
	vec2f m_currentCursorPosition;
	vec2f m_previousCursorPosition;
	vec2f m_deltaCursorPosition;

	std::map<EKey, bool> m_keyStateMap;
	EModifierKey		 m_modifierKeys = EModifierKey::None;

	IInputHandler();
	~IInputHandler() = default;

public:
	// Events
	OnMouseMoved		  m_onMouseMoved;
	OnMouseLeftDown		  m_onMouseLeftDown;
	OnMouseRightDown	  m_onMouseRightDown;
	OnMouseMiddleDown	  m_onMouseMiddleDown;
	OnMouseLeftUp		  m_onMouseLeftUp;
	OnMouseRightUp		  m_onMouseRightUp;
	OnMouseMiddleUp		  m_onMouseMiddleUp;
	OnMouseMiddleScrolled m_onMouseMiddleScrolled;
	OnKeyPressed		  m_keyPressed;
	OnMenuActionPressed	  m_menuActionPressed;

	// Mouse
	virtual bool onMouseDown(EMouseButtonType buttonType, const vec2f& cursorPosition);
	virtual bool onMouseUp(EMouseButtonType buttonType, const vec2f& cursorPosition);
	virtual bool onMouseWheel(float delta);
	virtual bool onMouseMoved(const vec2f& cursorPosition);
	virtual bool isMouseDown(EMouseButtonType buttonType) const;
	virtual bool isAnyMouseDown() const;

	virtual vec2f getClickPosition() const;
	virtual vec2f getCurrentCursorPosition() const;
	virtual vec2f getPreviousCursorPosition() const;
	virtual vec2f getDeltaCursorPosition() const;
	virtual void  resetDeltaCursorPosition();

	// Keys
	virtual std::vector<EKey> getKeysDown() const;
	virtual bool			  onKeyDown(EKey keyCode, int32 keyFlags, bool isRepeat);
	virtual bool			  onKeyUp(EKey keyCode, int32 keyFlags, bool isRepeat);
	virtual bool			  isKeyDown(EKey keyCode) const;
	virtual void			  consumeKey(const EKey keyCode);
	virtual bool			  isAltDown() const;
	virtual bool			  isShiftDown() const;
	virtual bool			  isCtrlDown() const;
};
