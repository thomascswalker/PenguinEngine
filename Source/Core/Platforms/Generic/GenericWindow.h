#pragma once

#include <string>

#include "Core/Types.h"
#include "Math/Color.h"
#include "Renderer/UI/Widget.h"

enum EWindowType
{
	Normal,
	Tooltip,
	Popup
};

struct WindowDescription
{
	int32 width;
	int32 height;
	int32 x;
	int32 y;

	std::string title;
	EWindowType type;
	Color		backgroundColor;
};

class GenericWindow
{
protected:
	WindowDescription			  m_description;
	std::shared_ptr<Canvas> m_canvas;

public:
	GenericWindow() { m_canvas = std::make_shared<Canvas>(); }
	virtual void resize(int32 width, int32 height) = 0;
	virtual void show() = 0;
	virtual void hide() = 0;
	virtual void paint() = 0;
	virtual void clear() = 0;

	std::shared_ptr<Canvas> getCanvas() const { return m_canvas; }

	int32 getWidth() const { return m_description.width; }
	int32 getHeight() const { return m_description.height; }
	Color getBackgroundColor() const { return m_description.backgroundColor; }
};