#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include "Math.h"
#include "vec2f.h"
#include "vec3.h"

enum button_type { pressed, hovered, normal };

const button_type Normal = button_type::normal;
const button_type Pressed = button_type::pressed;
const button_type Hovered = button_type::hovered;

class LineShape : public sf::Shape
{
public:

	explicit LineShape(const sf::Vector2f& point1, const sf::Vector2f& point2, const sf::Color c = sf::Color::Red) :
		m_direction(point2 - point1)
	{
		setPosition(point1);
		setThickness(2.f);
		setFillColor(c);
	}

	void setThickness(float thickness)
	{
		m_thickness = thickness;
		update();
	}

	float getThickness() const
	{
		return m_thickness;
	}

	float getLength() const
	{
		return std::sqrt(m_direction.x*m_direction.x + m_direction.y*m_direction.y);
	}

	/*const sf::Color& getFillColor() const
	{
		return shape_color;
	}*/

	virtual std::size_t getPointCount() const
	{
		return 4; // fixed, but could be an attribute of the class if needed
	}

	virtual sf::Vector2f getPoint(std::size_t index) const
	{
		sf::Vector2f unitDirection = m_direction / getLength();
		sf::Vector2f unitPerpendicular(-unitDirection.y, unitDirection.x);

		sf::Vector2f offset = (m_thickness / 2.f)*unitPerpendicular;

		switch (index)
		{
			default:
			case 0: return offset;
			case 1: return (m_direction + offset);
			case 2: return (m_direction - offset);
			case 3: return (-offset);
		}
	}

private:

	sf::Vector2f m_direction; ///< Direction of the line
	float m_thickness;    ///< Thickness of the line
	sf::Color shape_color;
};

class Button
{
public:

	Button(	const sf::Vector2f size, const sf::Vector2f pos,
			const sf::Color c, const sf::Color outlc,
			const float thick, sf::Text txt)
	{
		sf::Color rc = c;
		sf::Color routc = outlc;

		normal.setSize(size);
		normal.setFillColor(rc);
		normal.setOutlineColor(routc);
		normal.setOutlineThickness(thick);
		normal.setPosition(pos);

		rc.r -= 15.f, rc.g -= 15.f, rc.b -= 15.f;
		routc.r -= 15.f, routc.g -= 15.f, routc.b -= 15.f;

		hovered.setSize(size);
		hovered.setFillColor(rc);
		hovered.setOutlineColor(routc);
		hovered.setOutlineThickness(thick);
		hovered.setPosition(pos);

		rc.r -= 15.f, rc.g -= 15.f, rc.b -= 15.f;
		routc.r -= 15.f, routc.g -= 15.f, routc.b -= 15.f;

		pressed.setSize(size);
		pressed.setFillColor(rc);
		pressed.setOutlineColor(routc);
		pressed.setOutlineThickness(thick);
		pressed.setPosition(pos);

		curr = normal;

		//load resources
		text = txt;
	}

	virtual void onHover() { bBeingHovered = true; setShape(Hovered); }
	virtual void onNormalize() { bBeingHovered = false, bBeingPressed = false; setShape(Normal); }
	virtual void onPress() { bBeingPressed = true; setShape(Pressed); }

	bool mouseIsHovering(sf::Vector2i m_pos)
	{
		sf::Vector2i lpos = m_pos;
		sf::Vector2f mouse_pos((float)lpos.x, (float)lpos.y);

		return	mouse_pos.x >= curr.getPosition().x &&
			mouse_pos.x <= curr.getPosition().x + curr.getSize().x &&
			mouse_pos.y >= curr.getPosition().y &&
			mouse_pos.y <= curr.getPosition().y + curr.getSize().y;
	}

	void setShape(button_type type)
	{
		switch (type)
		{
		case Pressed:
			curr = pressed;
			break;
		case Hovered:
			curr = hovered;
			break;
		case Normal:
			curr = normal;
			break;
		}
		//curr.setPosition(pos);
	}

	void tick(sf::Vector2i m_pos)
	{
		if (mouseIsHovering(m_pos))
		{
			if (!bBeingPressed)
				onHover();
		}
		else
			onNormalize();
	}

public:
	sf::Text text;
	sf::RectangleShape curr;
	sf::RectangleShape normal;
	sf::RectangleShape hovered;
	sf::RectangleShape pressed;
	bool bBeingHovered = false;
	bool bBeingPressed = false;
};

class ToggleButton : public Button
{
public:
	ToggleButton(const sf::Vector2f size, const sf::Vector2f pos,
				 const sf::Color c, const sf::Color outlc,
				 const float thick, sf::Text txt, bool *v) :
				 Button(size, pos, c, outlc, thick, txt),  val(v)
	{
		msg = txt.getString();
		text.setString("Hide " + msg);
	}
	virtual void onPress() override
	{ 
		Button::onPress(); 
		*val = !*val;
		if(*val == true)
			text.setString("Hide " + msg);
		else
			text.setString("Show " + msg);
	}

public:
	std::string msg;
	bool *val;
};

class IncreaseButton : public Button
{
public:
	IncreaseButton(const sf::Vector2f size, const sf::Vector2f pos,
		const sf::Color c, const sf::Color outlc,
		const float thick, sf::Text txt, int *v) :
		Button(size, pos, c, outlc, thick, txt), val(v)
	{
		text.setString("+");
	}

	virtual void onPress() override
	{
		Button::onPress();
		*val += 1;
	}

public:
	int *val;
};

class DecreaseButton : public Button
{
public:
	DecreaseButton(const sf::Vector2f size, const sf::Vector2f pos,
		const sf::Color c, const sf::Color outlc,
		const float thick, sf::Text txt, int *v) :
		Button(size, pos, c, outlc, thick, txt), val(v)
	{
		text.setString("-");
	}

	virtual void onPress() override
	{
		Button::onPress();
		*val -= 1;
	}

public:
	int *val;
};