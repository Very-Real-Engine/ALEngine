#pragma once

#include "Core/MouseCodes.h"
#include "Events/Event.h"

namespace ale
{
/**
 * @class MouseMovedEvent
 * @brief 마우스가 이동했을 때 발생하는 이벤트.
 */
class MouseMovedEvent : public Event
{
  public:
	/**
	 * @brief MouseMovedEvent 생성자.
	 * @param x 마우스의 X 좌표.
	 * @param y 마우스의 Y 좌표.
	 */
	MouseMovedEvent(const float x, const float y) : m_MouseX(x), m_MouseY(y)
	{
	}

	/**
	 * @brief 마우스의 X 좌표를 반환합니다.
	 * @return float 현재 마우스의 X 좌표.
	 */
	float getX() const
	{
		return m_MouseX;
	}

	/**
	 * @brief 마우스의 Y 좌표를 반환합니다.
	 * @return float 현재 마우스의 Y 좌표.
	 */
	float getY() const
	{
		return m_MouseY;
	}

	/**
	 * @brief 이벤트 정보를 문자열로 변환합니다.
	 * @return std::string "MouseMovedEvent: x, y" 형식의 문자열.
	 */
	std::string toString() const override
	{
		std::stringstream ss;
		ss << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
		return ss.str();
	}

	EVENT_CLASS_TYPE(MOUSE_MOVE);
	EVENT_CLASS_CATEGORY(EVENT_CATEGORY_MOUSE | EVENT_CATEGORY_INPUT);

  private:
	float m_MouseX, m_MouseY;
};

/**
 * @class MouseScrolledEvent
 * @brief 마우스가 스크롤될 때 발생하는 이벤트.
 */
class MouseScrolledEvent : public Event
{
  public:
	/**
	 * @brief MouseScrolledEvent 생성자.
	 * @param xOffset X축 스크롤 오프셋.
	 * @param yOffset Y축 스크롤 오프셋.
	 */
	MouseScrolledEvent(const float xOffset, const float yOffset) : m_XOffset(xOffset), m_YOffset(yOffset)
	{
	}

	/**
	 * @brief 마우스 X축 스크롤 오프셋을 반환합니다.
	 * @return float X축 스크롤 값.
	 */
	float getXOffset() const
	{
		return m_XOffset;
	}

	/**
	 * @brief 마우스 Y축 스크롤 오프셋을 반환합니다.
	 * @return float Y축 스크롤 값.
	 */
	float getYOffset() const
	{
		return m_YOffset;
	}

	/**
	 * @brief 이벤트 정보를 문자열로 변환합니다.
	 * @return std::string "MouseScrolledEvent: xOffset, yOffset" 형식의 문자열.
	 */
	std::string toString() const override
	{
		std::stringstream ss;
		ss << "MouseScrolledEvent: " << m_XOffset << ", " << m_YOffset;
		return ss.str();
	}

	EVENT_CLASS_TYPE(MOUSE_SCROLLED);
	EVENT_CLASS_CATEGORY(EVENT_CATEGORY_MOUSE | EVENT_CATEGORY_INPUT);

  private:
	float m_XOffset, m_YOffset;
};

/**
 * @class MouseButtonEvent
 * @brief 마우스 버튼과 관련된 이벤트의 기본 클래스.
 */
class MouseButtonEvent : public Event
{
  public:
	/**
	 * @brief 눌린 마우스 버튼을 반환합니다.
	 * @return MouseCode 마우스 버튼 코드.
	 */
	MouseCode getMouseButton() const
	{
		return m_Button;
	}

	EVENT_CLASS_CATEGORY(EVENT_CATEGORY_MOUSE | EVENT_CATEGORY_INPUT | EVENT_CATEGORY_MOUSE_BUTTON);

  protected:
	/**
	 * @brief MouseButtonEvent 생성자.
	 * @param button 이벤트가 발생한 마우스 버튼 코드.
	 */
	MouseButtonEvent(const MouseCode button) : m_Button(button)
	{
	}

	MouseCode m_Button;
};

/**
 * @class MouseButtonPressedEvent
 * @brief 마우스 버튼이 눌렸을 때 발생하는 이벤트.
 */
class MouseButtonPressedEvent : public MouseButtonEvent
{
  public:
	/**
	 * @brief MouseButtonPressedEvent 생성자.
	 * @param button 눌린 마우스 버튼 코드.
	 */
	MouseButtonPressedEvent(const MouseCode button) : MouseButtonEvent(button)
	{
	}

	/**
	 * @brief 이벤트 정보를 문자열로 변환합니다.
	 * @return std::string "MouseButtonPressedEvent: button" 형식의 문자열.
	 */
	std::string toString() const override
	{
		std::stringstream ss;
		ss << "MouseButtonPressedEvent: " << m_Button;
		return ss.str();
	}

	EVENT_CLASS_TYPE(MOUSE_BUTTON_PRESSED);
};

/**
 * @class MouseButtonReleasedEvent
 * @brief 마우스 버튼이 떼어졌을 때 발생하는 이벤트.
 */
class MouseButtonReleasedEvent : public MouseButtonEvent
{
  public:
	/**
	 * @brief MouseButtonReleasedEvent 생성자.
	 * @param button 떼어진 마우스 버튼 코드.
	 */
	MouseButtonReleasedEvent(const MouseCode button) : MouseButtonEvent(button)
	{
	}

	/**
	 * @brief 이벤트 정보를 문자열로 변환합니다.
	 * @return std::string "MouseButtonReleasedEvent: button" 형식의 문자열.
	 */
	std::string toString() const override
	{
		std::stringstream ss;
		ss << "MouseButtonReleasedEvent: " << m_Button;
		return ss.str();
	}

	EVENT_CLASS_TYPE(MOUSE_BUTTON_RELEASED);
};

} // namespace ale
