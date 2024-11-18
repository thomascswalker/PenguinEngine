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

std::vector<Widget*> Widget::getChildren()
{
	if (m_layout == nullptr)
	{
		return std::vector<Widget*>();
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