#include "Widget.h"

Widget::Widget()
{
	m_layout = new Layout(this);
}

void Widget::update(MouseData* mouse)
{
	// Hover
	bool newHovered = m_geometry.contains(mouse->position.toType<int32>());
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

void Widget::addChild(Widget* w)
{
	CHECK(m_layout);
	m_layout->addWidget(w);
}

void Widget::addChild(std::shared_ptr<Widget> w)
{
	CHECK(m_layout);
	addChild(w.get());
}

std::vector<Widget*> Widget::getChildren(bool recursive)
{
	std::vector<Widget*> widgets;
	if (m_layout == nullptr)
	{
		return widgets;
	}
	if (recursive)
	{
		for (auto w : m_layout->getWidgets())
		{
			auto children = w->getChildren(recursive);
			for (auto child : children)
			{
				widgets.emplace_back(child);
			}
		}
	}
	return m_layout->getWidgets();
}

void Widget::setLayout(Layout* layout)
{
	m_layout = layout;
	if (m_layout->getParent() != this)
	{
		m_layout->setParent(this);
	}
}

std::vector<Widget*> Layout::getWidgets()
{
	std::vector<Widget*> widgets;

	// Iterate items in this layout
	for (LayoutItem* item : m_items)
	{
		// Only look at WidgetItems
		if (WidgetItem* widgetItem = dynamic_cast<WidgetItem*>(item))
		{
			// Check the widget is valid
			if (Widget* widget = widgetItem->getWidget())
			{
				// Add to out widget list
				widgets.emplace_back(widget);
			}
		}
	}
	return widgets;
}

void LayoutEngine::updateWidgets(Widget* w, MouseData* mouse)
{
	w->update(mouse);
	for (auto child : w->getChildren())
	{
		updateWidgets(child, mouse);
	}
}

recti LayoutEngine::layoutWidget(Widget* parent, const recti& viewport)
{

	EResizeMode h = parent->getHorizontalResizeMode();
	EResizeMode v = parent->getVerticalResizeMode();

	// Resize _this_ widget
	switch (h)
	{
		case Fixed:
			{
				parent->setWidth(std::clamp(parent->getFixedWidth(), 0, viewport.width));
				break;
			}
		case Expanding:
			{
				parent->setWidth(viewport.width);
				break;
			}
	}
	switch (v)
	{
		case Fixed:
			{
				parent->setHeight(std::clamp(parent->getFixedHeight(), 0, viewport.height));
				break;
			}
		case Expanding:
			{
				parent->setHeight(viewport.height);
				break;
			}
	}

	// Get the layout within this widget
	Layout* layout = parent->getLayout();
	if (layout != nullptr && parent->getChildren().size() > 0)
	{
		int32 childCount = parent->getChildren().size();
		int32 divisor = childCount <= 1 ? 1 : childCount;

		auto		 children = parent->getChildren();
		EOrientation orientation = layout->getOrientation();
		for (int32 i = 0; i < childCount; i++)
		{
			auto child = children[i];

			// Root child position starts at the parent's position
			vec2i childPosition = parent->getPosition();

			// Compute size
			int32 childWidth = (child->getHorizontalResizeMode() == Expanding) // If the child horizontally expands
				? orientation == Horizontal									   // And the orientation of the parent is horizontal
					? parent->getWidth() / divisor								   // Divide parent width by divisor to get child width
					: parent->getWidth()											   // Otherwise just use the entire parent width
				: child->getFixedWidth();									   // Otherwise use fixed width
			int32 childHeight = (child->getVerticalResizeMode() == Expanding)  // If the child vertically expands
				? orientation == Vertical									   // And the orientation of the parent is vertical
					? parent->getHeight() / divisor								   // Divide parent height by divisor to get child height
					: parent->getHeight()										   // Otherwise just use the entire parent height
				: child->getFixedHeight();									   // Otherwise use fixed height

			// Set child width/height based on the computed width/height above
			child->setWidth(childWidth);
			child->setHeight(childHeight);

			// Compute position
			childPosition.x = orientation == Horizontal ? childPosition.x + (childWidth * i) : childPosition.x;
			childPosition.y = orientation == Vertical ? childPosition.y + (childHeight * i) : childPosition.y;
			child->setPosition(childPosition);
			child->recompute();

			if (child->getLayout() != nullptr)
			{
				vec2i childSize(childWidth, childHeight);
				recti available = recti{ childPosition, childPosition + childSize };
				layoutWidget(child, available);
			}
		}
	}

	return parent->getGeometry();
}

void LayoutEngine::layoutAllWidgets(Widget* w, const recti& viewport)
{
	auto widgets = w->getChildren(true);
	if (widgets.size() == 0)
	{
		return;
	}
	layoutWidget(w, viewport);
}
