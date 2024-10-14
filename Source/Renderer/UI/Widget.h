#pragma once

#include <utility>

#include "Math/Rect.h"
#include "Renderer/Texture.h"
#include "Engine/Mouse.h"

class Widget;
class Button;

class Widget
{
protected:
	Texture m_data;
	rectf	m_geometry{};
	bool	m_hovered = false;
	bool	m_clicked = false;

public:
	rectf getGeometry() const { return m_geometry; }
	vec2f getSize() const
	{
		return vec2f(m_geometry.width, m_geometry.height);
	}
	vec2f getPosition() const
	{
		return vec2f(m_geometry.x, m_geometry.y);
	}

	uint8*	 getData() { return m_data.getData(); }
	Texture* getTexture() { return &m_data; }

	virtual void paint() {}
	virtual void update(const MouseData& mouse)
	{
		// Hover
		bool newHovered = m_geometry.contains(mouse.position);
		if (newHovered && !m_hovered)
		{
			onHoverBegin();
		}
		if (!newHovered && m_hovered)
		{
			onHoverEnd();
		}
		m_hovered = newHovered;
		onHover();

		// Click
		bool newClicked = mouse.leftDown && m_hovered;
		if (newClicked && !m_clicked)
		{
			onClickBegin();
		}
		if (!newClicked && m_clicked)
		{
			onClickEnd();
		}
		m_clicked = newClicked;
	}
	virtual void reposition(const vec2i& position)
	{
		m_geometry.x = position.x;
		m_geometry.y = position.y;
	}
	virtual void resize(const vec2i& size)
	{
		m_geometry.width = size.x;
		m_geometry.height = size.y;
		m_data.resize(size);
	}

	virtual void onHover() {}
	virtual void onHoverBegin() { LOG_INFO("Hover begin") }
	virtual void onHoverEnd() { LOG_INFO("Hover end") }

	virtual void onClickBegin() { LOG_INFO("Click begin") }
	virtual void onClickEnd() { LOG_INFO("Click end") }
};

class Button : public Widget
{
	virtual void paint() override
	{
		// Fill with 50% gray
		auto  size = m_data.getDataSize();
		auto  mem = m_data.getData();
		uint8 color = m_hovered ? 200 : 128;
		color = m_clicked ? 50 : color;
		std::fill(mem, mem + size, color);
	}
};

namespace WidgetManager
{
	inline std::vector<Widget*> g_widgets;

	template <typename T>
	T* constructWidget()
	{
		T* newWidget = new T();
		g_widgets.emplace_back(std::move(newWidget));
		return dynamic_cast<T*>(g_widgets.back());
	}

	inline void updateWidgets(const MouseData& mouse)
	{
		for (Widget* w : g_widgets)
		{
			w->update(mouse);
		}
	}
} // namespace WidgetManager
