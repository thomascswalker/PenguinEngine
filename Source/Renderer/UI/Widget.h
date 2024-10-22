#pragma once

#include <utility>

#include "Math/Rect.h"
#include "Renderer/Texture.h"
#include "Input/Mouse.h"
#include "Engine/Delegate.h"
#include "Painter.h"

class Widget;
class Button;
class Panel;

namespace UIColors
{
	inline Color VeryDarkGray = Color("#1A1A1A");
	inline Color DarkGray = Color("#242424");
	inline Color MidDarkGray = Color("#2F2F2F");
	inline Color MidGray = Color("#383838");
	inline Color LightGray = Color("#E1E1E1");

	inline Color Black = Color("#0F0F0F");
	inline Color White = Color("#FFFFFF");

	inline Color Red = Color("#B92D10");
	inline Color Green = Color("#84BD49");
	inline Color Blue = Color("#2E84D8");
} // namespace UIColors

enum class ELayoutMode
{
	Horizontal,
	Vertical
};

enum class EResizeMode
{
	Fixed,
	Expanding
};

class Widget
{
protected:
	/** Properties **/
	ELayoutMode m_layoutMode = ELayoutMode::Horizontal;
	EResizeMode m_horizontalResizeMode = EResizeMode::Expanding;
	EResizeMode m_verticalResizeMode = EResizeMode::Expanding;

	Widget*				 m_parent = nullptr;
	std::vector<Widget*> m_children;

	/** Geometry **/

	recti m_geometry{};
	vec2i m_fixedSize{};
	recti m_renderGeometry{};
	vec4i m_margin{ 0, 0, 0, 0 };

	/** State **/

	bool m_hovered = false;
	bool m_clicked = false;

	Widget() {}

public:
	void	setParent(Widget* w) { m_parent = w; }
	Widget* getParent() { return m_parent; }

	recti getGeometry() const { return m_geometry; }
	recti getRenderGeometry() const { return m_renderGeometry; }
	vec2i getSize() const
	{
		return vec2i(m_geometry.width, m_geometry.height);
	}
	vec2i getPosition() const
	{
		return vec2i(m_geometry.x, m_geometry.y);
	}

	void setMargin(vec4i margin)
	{
		m_margin = margin;
	}
	void setMargin(int32 x, int32 y, int32 z, int32 w)
	{
		m_margin = { x, y, z, w };
	}
	vec4i getMargin() const { return m_margin; }

	vec2i getFixedSize() const { return m_fixedSize; }
	int32 getFixedWidth() const { return m_fixedSize.x; }
	int32 getFixedHeight() const { return m_fixedSize.y; }
	void  setFixedWidth(int32 width)
	{
		m_fixedSize.x = width;
	}
	void setFixedHeight(int32 height)
	{
		m_fixedSize.y = height;
	}
	void setFixedSize(vec2i size)
	{
		m_fixedSize = size;
	}

	virtual void paint(Painter* painter) {}
	virtual void update(const MouseData& mouse)
	{
		// Hover
		bool newHovered = m_renderGeometry.contains(mouse.position);
		if (newHovered && !m_hovered)
		{
			onHoverBegin();
		}
		if (!newHovered && m_hovered)
		{
			onHoverEnd();
		}
		m_hovered = newHovered;

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
	}
	virtual void setWidth(int32 width)
	{
		m_geometry.width = width;
	}
	virtual void setHeight(int32 height)
	{
		m_geometry.height = height;
	}
	virtual void recompute()
	{
		m_renderGeometry = m_geometry;

		m_renderGeometry.x += m_margin.x;
		m_renderGeometry.y += m_margin.y;

		switch (m_horizontalResizeMode)
		{
			case EResizeMode::Fixed:
				m_renderGeometry.width = m_fixedSize.x;
				break;
			case EResizeMode::Expanding:
				m_renderGeometry.width -= (m_margin.z * 2);
				break;
		}

		switch (m_verticalResizeMode)
		{
			case EResizeMode::Fixed:
				m_renderGeometry.height = m_fixedSize.y;
				break;
			case EResizeMode::Expanding:
				m_renderGeometry.height -= (m_margin.w * 2);
				break;
		}
	}

	virtual void onHoverBegin() { LOG_INFO("Hover begin") }
	virtual void onHoverEnd() { LOG_INFO("Hover end") }

	virtual void onClickBegin() { LOG_INFO("Click begin") }
	virtual void onClickEnd() { LOG_INFO("Click end") }

	virtual void		setLayoutMode(ELayoutMode mode) { m_layoutMode = mode; }
	virtual ELayoutMode getLayoutMode() const { return m_layoutMode; }

	virtual void		setHorizontalResizeMode(EResizeMode mode) { m_horizontalResizeMode = mode; }
	virtual EResizeMode getHorizontalResizeMode() const { return m_horizontalResizeMode; }

	virtual void		setVerticalResizeMode(EResizeMode mode) { m_verticalResizeMode = mode; }
	virtual EResizeMode getVerticalResizeMode() const { return m_verticalResizeMode; }

	virtual void addChild(Widget* w)
	{
		m_children.emplace_back(w);
		w->setParent(this);
	}
	virtual std::vector<Widget*>& getChildren() { return m_children; }
};

class Canvas : public Widget
{
};

class Spacer : public Widget
{
};

class Panel : public Widget
{
public:
	virtual void paint(Painter* painter) override
	{
		// Draw the filled panel
		painter->drawRectFilled(m_renderGeometry, UIColors::DarkGray);
	}
};

DECLARE_MULTICAST_DELEGATE(OnClicked);

class Button : public Widget
{
public:
	OnClicked m_onClicked;

	Button()
	{
		setMargin(8, 8, 8, 8);
	}

	virtual void paint(Painter* painter) override
	{
		// Fill with 50% gray
		Color color = m_hovered ? UIColors::MidGray : UIColors::MidDarkGray;
		if (m_clicked)
		{
			color = UIColors::Blue;
		}
		// Draw the filled button
		painter->drawRectFilled(m_renderGeometry, color);
		// Draw the button border
		painter->drawRect(m_renderGeometry, UIColors::VeryDarkGray);
	}

	virtual void onClickEnd() override
	{
		// Only broadcast that we've clicked the button if we're still hovering on
		// the button.
		if (!m_hovered)
		{
			return;
		}
		m_onClicked.broadcast();
	}
};

namespace WidgetManager
{
	inline std::vector<Widget*> g_widgets;
	inline Widget*				g_rootWidget = nullptr;

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

	inline void layoutWidgets(Widget* w, recti bounds)
	{
		ELayoutMode			  mode = w->getLayoutMode();
		std::vector<Widget*>& children = w->getChildren();
		int32				  childCount = w->getChildren().size();

		int32 horizontalSize = childCount != 0 ? bounds.width / childCount : bounds.width;
		int32 verticalSize = childCount != 0 ? bounds.height / childCount : bounds.height;

		int32 halfWidth = bounds.width / 2;
		int32 halfHeight = bounds.height / 2;

		// Move and resize the children
		int32 totalWidth = 0;
		int32 totalHeight = 0;
		for (int32 i = 0; i < childCount; i++)
		{
			Widget* child = children[i];
			vec4i	margin = child->getMargin();
			switch (mode)
			{
				case ELayoutMode::Horizontal:
				{
					switch (child->getHorizontalResizeMode())
					{
						case EResizeMode::Expanding:
							child->reposition(vec2i(horizontalSize * i, 0));
							child->resize(vec2i(horizontalSize, bounds.height));
							break;
						case EResizeMode::Fixed:
							int32 width = child->getFixedWidth();
							child->reposition(vec2i(totalWidth, 0));
							totalWidth += width;
							child->resize(vec2i(width, bounds.height));
							break;
					}

					break;
				}
				case ELayoutMode::Vertical:
				{
					switch (child->getVerticalResizeMode())
					{
						case EResizeMode::Expanding:
							child->reposition(vec2i(0, verticalSize * i));
							child->resize(vec2i(bounds.width, verticalSize));
							break;
						case EResizeMode::Fixed:
							int32 height = child->getFixedHeight();
							child->reposition(vec2i(0, totalHeight * i));
							totalHeight += height;
							child->resize(vec2i(bounds.width, height));
							break;
					}
					break;
				}
				default:
					return;
			}
			child->recompute();

			// Layout of children of this widget, if present
			if (child->getChildren().size() != 0)
			{
				layoutWidgets(child, child->getRenderGeometry());
			}
		}
	}
} // namespace WidgetManager
