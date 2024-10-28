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

constexpr int32 g_defaultMargin = 4;

namespace UIColors
{
	inline Color VeryDarkGray = Color("#1A1A1A");
	inline Color DarkGray = Color("#242424");
	inline Color MidDarkGray = Color("#2F2F2F");
	inline Color MidGray = Color("#383838");
	inline Color LightGray = Color("#E1E1E1");

	inline Color Black = Color("#0F0F0F");
	inline Color White = Color("#FFFFFF");

	inline Color Red = Color("#ED4C2C");
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

#define GENERATE_SUPER(n) using Super = n;

class Widget
{
protected:
	/** Properties **/
	ELayoutMode m_layoutMode = ELayoutMode::Horizontal;
	EResizeMode m_horizontalResizeMode = EResizeMode::Expanding;
	EResizeMode m_verticalResizeMode = EResizeMode::Expanding;
	bool		m_consumeInput = false;

	Widget*				 m_parent = nullptr;
	std::vector<Widget*> m_children;
	std::string			 m_objectName;

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

	std::string getObjectName() const { return m_objectName; }
	void		setObjectName(const std::string& name) { m_objectName = name; }

	recti getGeometry() const { return m_geometry; }
	int32 getWidth() const { return m_geometry.width; }
	int32 getHeight() const { return m_geometry.height; }
	recti getRenderGeometry() const { return m_renderGeometry; }
	vec2i getSize() const { return vec2i(m_geometry.width, m_geometry.height); }
	vec2i getPosition() const { return vec2i(m_geometry.x, m_geometry.y); }

	void setPosition(const vec2i& pos)
	{
		m_geometry.x = pos.x;
		m_geometry.y = pos.y;
	}
	void setPosition(int32 x, int32 y)
	{
		m_geometry.x = x;
		m_geometry.y = y;
	}

	void setMargin(vec4i margin) { m_margin = margin; }
	void setMargin(int32 x, int32 y, int32 z, int32 w) { m_margin = { x, y, z, w }; }
	void setMargin(int32 value) { m_margin = { value, value, value, value }; }

	vec4i getMargin() const { return m_margin; }

	vec2i getFixedSize() const { return m_fixedSize; }
	int32 getFixedWidth() const { return m_fixedSize.x; }
	int32 getFixedHeight() const { return m_fixedSize.y; }
	void  setFixedWidth(int32 width) { m_fixedSize.x = width; }
	void  setFixedHeight(int32 height) { m_fixedSize.y = height; }
	void  setFixedSize(vec2i size) { m_fixedSize = size; }

	virtual void paint(Painter* painter) {}
	virtual void update(MouseData& mouse)
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
	virtual void setWidth(int32 width) { m_geometry.width = width; }
	virtual void setHeight(int32 height) { m_geometry.height = height; }
	virtual void recompute()
	{
		// Set the render geometry to the base geometry
		m_renderGeometry = m_geometry;
	}

	virtual void onHoverBegin() { LOG_DEBUG("{} Hover begin", m_objectName) }
	virtual void onHoverEnd() { LOG_DEBUG("{} Hover end", m_objectName) }

	virtual void onClickBegin() { LOG_DEBUG("{} Click begin", m_objectName) }
	virtual void onClickEnd() { LOG_DEBUG("{} Click end", m_objectName) }

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
	GENERATE_SUPER(Widget)
};

class Spacer : public Widget
{
	GENERATE_SUPER(Widget)
};

class Panel : public Widget
{
	GENERATE_SUPER(Widget)
public:
	virtual void paint(Painter* painter) override
	{
		// Draw the filled panel
		painter->drawRectFilled(m_geometry, UIColors::DarkGray);
		painter->drawRect(m_geometry, UIColors::VeryDarkGray);
	}
};

class Label : public Widget
{
	GENERATE_SUPER(Widget)
protected:
	std::string m_text;
	Color		m_textColor = Color::white();
	int32		m_defaultFixedHeight = 25;
	int32		m_defaultMargin = 5;

public:
	Label(const std::string& text = "")
	{
		setMargin(m_defaultMargin);
		setFixedHeight(m_defaultFixedHeight);
		m_text = text;
	}

	virtual void paint(Painter* painter) override
	{
		painter->drawRectFilled(m_geometry, UIColors::DarkGray);
		painter->drawRect(m_geometry, UIColors::VeryDarkGray);
		vec2i textPos = m_geometry.min();
		int32 textWidth = m_text.size() * g_glyphTextureWidth;
		textPos.x += (m_geometry.width / 2) - (textWidth / 2);
		textPos.y += (m_geometry.height / 2) - (g_glyphTextureHeight / 2);
		painter->drawText(textPos, m_text);
	}

	virtual void setText(const std::string& text) { m_text = text; }

	virtual void setTextColor(const Color& color) { m_textColor = color; }
};

DECLARE_MULTICAST_DELEGATE(OnClicked);

class Button : public Label
{
	GENERATE_SUPER(Label)
public:
	OnClicked m_onClicked;

	Button(const std::string& text = "") : Label(text) {}

	virtual void paint(Painter* painter) override
	{
		// Fill with 50% gray
		Color color = m_hovered ? UIColors::MidGray : UIColors::MidDarkGray;
		if (m_clicked)
		{
			color = UIColors::Blue;
		}
		// Draw the filled button
		painter->drawRectFilled(m_geometry, color);
		// Draw the button border
		painter->drawRect(m_geometry, UIColors::VeryDarkGray);
		// Draw text
		painter->setFontColor(m_textColor);
		// Compute text position
		vec2i textPos = m_geometry.min();
		int32 textWidth = m_text.size() * g_glyphTextureWidth;
		textPos.x += (m_geometry.width / 2) - (textWidth / 2);
		textPos.y += (m_geometry.height / 2) - (g_glyphTextureHeight / 2);
		painter->drawText(textPos, m_text);
	}

	virtual void update(MouseData& mouse) override
	{
		Super::update(mouse);

		// If input has been consumed, skip this widget updating
		if (mouse.inputConsumed)
		{
			return;
		}
		// Click
		bool newClicked = mouse.leftDown && m_hovered;
		if (newClicked && !m_clicked)
		{
			onClickBegin();
		}
		if (!newClicked && m_clicked)
		{
			onClickEnd();
			// Set input to consumed
			mouse.inputConsumed = m_consumeInput;
		}
		m_clicked = newClicked;
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

	template <typename T> T* constructWidget(const std::string& name = "")
	{
		T* newWidget = new T();
		g_widgets.emplace_back(std::move(newWidget));
		g_widgets.back()->setObjectName(name);
		return dynamic_cast<T*>(g_widgets.back());
	}

	inline void updateWidgets(MouseData& mouse)
	{
		for (Widget* w : g_widgets)
		{
			w->update(mouse);
		}
	}

	inline recti layoutWidget(Widget* w, const vec2i& available, const recti& viewport)
	{
		ELayoutMode layoutMode = w->getLayoutMode();
		EResizeMode h = w->getHorizontalResizeMode();
		EResizeMode v = w->getVerticalResizeMode();

		switch (h)
		{
			case EResizeMode::Fixed:
			{
				w->setWidth(std::clamp(0, w->getFixedWidth(), available.x));
				break;
			}
			case EResizeMode::Expanding:
			{
				w->setWidth(available.x);
				break;
			}
		}
		// Vertical resize
		switch (v)
		{
			case EResizeMode::Fixed:
			{
				w->setHeight(std::clamp(0, w->getFixedHeight(), available.y));
				break;
			}
			case EResizeMode::Expanding:
			{
				w->setHeight(available.y);
				break;
			}
		}

		vec2i childPosition = w->getPosition();
		int32 childCount = w->getChildren().size();
		int32 divisor = childCount <= 1 ? 1 : childCount - 1;

		for (auto child : w->getChildren())
		{
			int32 childWidth = (child->getHorizontalResizeMode() == EResizeMode::Expanding) ? layoutMode == ELayoutMode::Horizontal ? w->getWidth() / divisor // Expanding + Horizontal
																																	: w->getWidth()			  // Expanding
																							: child->getFixedWidth();										  // Fixed
			int32 childHeight = (child->getVerticalResizeMode() == EResizeMode::Expanding) ? layoutMode == ELayoutMode::Vertical ? w->getHeight() / divisor	  // Expanding + Vertical
																																 : w->getHeight()			  // Expanding
																						   : child->getFixedHeight();										  // Vertical

			childPosition.x = childPosition.x >= viewport.width ? childPosition.x - viewport.width : childPosition.x;
			childPosition.y = childPosition.y >= viewport.height ? childPosition.y - viewport.height : childPosition.y;
			child->setPosition(childPosition);

			layoutWidget(child, vec2i{ childWidth, childHeight }, viewport);

			if (h == EResizeMode::Expanding)
			{
				childPosition.x += childWidth;
			}
			if (v == EResizeMode::Expanding)
			{
				childPosition.y += childHeight;
			}
		}

		// Compute render geometry
		w->recompute();

		return w->getGeometry();
	}
} // namespace WidgetManager
