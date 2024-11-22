#pragma once

#include <utility>

#include "Core/Core.h"
#include "Core/Macros.h"
#include "Engine/Delegate.h"
#include "Input/Mouse.h"
#include "Math/Rect.h"
#include "Painter.h"
#include "Renderer/Texture.h"

class Widget;
class Layout;
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

enum EResizeMode
{
	Fixed,
	Expanding,
};

class Widget
{
protected:
	/** Properties **/
	EResizeMode m_horizontalResizeMode = Expanding;
	EResizeMode m_verticalResizeMode = Expanding;
	bool		m_consumeInput = false;

	Widget*				 m_parent = nullptr;
	std::vector<Widget*> m_children{};
	Layout*				 m_layout;
	std::string			 m_objectName{};

	/** Geometry **/

	recti m_geometry{};
	vec2i m_fixedSize{};
	recti m_renderGeometry{};
	vec4i m_margin{ 0, 0, 0, 0 };

	/** State **/

	bool m_hovered = false;
	bool m_clicked = false;

public:
	Widget();
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
	virtual void update(MouseData* mouse);
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

	void				 addChild(Widget* w);
	void				 addChild(std::shared_ptr<Widget> w);
	std::vector<Widget*> getChildren(bool recursive = false);

	void	setLayout(Layout* layout);
	Layout* getLayout() { return m_layout; }
	virtual void layoutChildren() = 0;

	virtual void		setHorizontalResizeMode(EResizeMode mode) { m_horizontalResizeMode = mode; }
	virtual EResizeMode getHorizontalResizeMode() const { return m_horizontalResizeMode; }

	virtual void		setVerticalResizeMode(EResizeMode mode) { m_verticalResizeMode = mode; }
	virtual EResizeMode getVerticalResizeMode() const { return m_verticalResizeMode; }
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

	virtual void layoutChildren() override
	{

	}
};

class Label : public Panel
{
	GENERATE_SUPER(Widget)
protected:
	std::string m_text;
	Color		m_textColor = Color::white();
	int32		m_fontSize = 11;
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
		// textPos.y += (m_geometry.height);
		painter->drawText(textPos, m_text);
	}

	virtual void setText(const std::string& text) { m_text = text; }
	virtual void setTextColor(const Color& color) { m_textColor = color; }
	virtual void setFontSize(const int32 fontSize) { m_fontSize = fontSize; }
};

DECLARE_MULTICAST_DELEGATE(OnClicked);

class Button : public Label
{
	GENERATE_SUPER(Label)

public:
	// Emitted when the button is clicked.
	OnClicked onClicked;

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
		//// Draw the button border
		painter->drawRect(m_geometry, UIColors::VeryDarkGray);
		// Draw text
		painter->setFontColor(m_textColor);
		painter->setFontSize(m_fontSize);

		// Compute the full text width in order to center the text
		int32 textWidth = 0;
		for (auto& c : m_text)
		{
			textWidth += painter->getCharacter(c)->size.x;
		}

		// Center alignment
		vec2i textPos = m_geometry.min();
		textPos.x += (m_geometry.width / 2) - (textWidth / 2);
		textPos.y += (m_geometry.height / 2) + (painter->getFontSize() / 2);

		painter->drawText(textPos, m_text);
	}

	virtual void update(MouseData* mouse) override
	{
		Super::update(mouse);

		// If input has been consumed, skip this widget updating
		if (mouse->inputConsumed)
		{
			return;
		}
		// Click
		bool newClicked = mouse->leftDown && m_hovered;
		if (newClicked && !m_clicked)
		{
			onClickBegin();
		}
		if (!newClicked && m_clicked)
		{
			onClickEnd();
			// Set input to consumed
			mouse->inputConsumed = m_consumeInput;
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
		onClicked.broadcast();
	}
};

class ViewportWidget : public Widget
{
	GENERATE_SUPER(Widget)

public:
	virtual void paint(Painter* painter) override { painter->fill(Color::black()); }
};

class LayoutItem
{
public:
	LayoutItem() = default;
	virtual Widget* getWidget() = 0;
	virtual Layout* getLayout() = 0;
};

class WidgetItem : public LayoutItem
{
	Widget* widget;
	Layout* layout;

public:
	WidgetItem(Widget* w, Layout* l) : widget(w), layout(l) {}
	Widget* getWidget() override { return widget; }
	Layout* getLayout() override { return widget->getLayout(); }
};

class Layout : public LayoutItem
{
	Widget*					 m_parent = nullptr;
	std::vector<LayoutItem*> m_items;
	EOrientation			 m_orientation = Horizontal;

public:
	Layout(Widget* parent = nullptr)
	{
		if (parent == nullptr)
		{
			return;
		}
		parent->setLayout(this);
		m_parent = parent;
	}

	Widget* getParent() const { return m_parent; }
	void	setParent(Widget* w) { m_parent = w; }
	Widget* getWidget() override { return nullptr; }
	Layout* getLayout() override { return this; }

	EOrientation getOrientation() const { return m_orientation; }
	void		 setOrientation(EOrientation o) { m_orientation = o; }

	template <typename T>
	void addWidget(T* w)
	{
		// Create a WidgetItem wrapper for this widget
		m_items.emplace_back(new WidgetItem(w, this));
	}

	std::vector<Widget*> getWidgets();
};

namespace LayoutEngine
{
	void  updateWidgets(Widget* w, MouseData* mouse);
	recti layoutWidget(Widget* parent, const recti& viewport);
	void  layoutAllWidgets(Widget* w, const recti& viewport);
}; // namespace LayoutEngine