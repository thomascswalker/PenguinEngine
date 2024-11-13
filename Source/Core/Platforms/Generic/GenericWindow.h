#pragma once

#include "Core/Types.h"
#include <string>

struct WindowDescription
{
	int32 width;
	int32 height;
	int32 x;
	int32 y;

	std::string title;
};

class GenericWindow
{
protected:
	WindowDescription m_description;

public:
	GenericWindow(){}
	virtual void resize(int32 widht, int32 height) = 0;
	virtual void show() = 0;
	virtual void paint() = 0;
};