#pragma once

#include "Events/Event.h"

namespace ale
{
/**
 * @class WindowResizeEvent
 * @brief 이벤트 클래스: 윈도우 크기 변경 시 발생하는 이벤트.
 */
class WindowResizeEvent : public Event
{
  public:
	/**
	 * @brief WindowResizeEvent 생성자.
	 * @param width 변경된 윈도우의 너비.
	 * @param height 변경된 윈도우의 높이.
	 */
	WindowResizeEvent(unsigned int width, unsigned int height) : m_Width(width), m_Height(height)
	{
	}

	/**
	 * @brief 윈도우의 너비를 반환합니다.
	 * @return 현재 윈도우의 너비.
	 */
	unsigned int getWidth() const
	{
		return m_Width;
	}

	/**
	 * @brief 윈도우의 높이를 반환합니다.
	 * @return 현재 윈도우의 높이.
	 */
	unsigned int getHeight() const
	{
		return m_Height;
	}

	/**
	 * @brief 이벤트 정보를 문자열로 변환합니다.
	 * @return "WindowResizeEvent: width, height" 형식의 문자열.
	 */
	std::string toString() const override
	{
		std::stringstream ss;
		ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
		return ss.str();
	}

	EVENT_CLASS_TYPE(WINDOW_RESIZE);
	EVENT_CLASS_CATEGORY(EVENT_CATEGORY_APP);

  private:
	unsigned int m_Width, m_Height;
};

/**
 * @class WindowCloseEvent
 * @brief 이벤트 클래스: 윈도우가 닫힐 때 발생하는 이벤트.
 */
class WindowCloseEvent : public Event
{
  public:
	/// @brief WindowCloseEvent 생성자.
	WindowCloseEvent() = default;

	EVENT_CLASS_TYPE(WINDOW_CLOSE);
	EVENT_CLASS_CATEGORY(EVENT_CATEGORY_APP);
};

/**
 * @class AppTickEvent
 * @brief 이벤트 클래스: 애플리케이션 틱(프레임 갱신) 시 발생하는 이벤트.
 */
class AppTickEvent : public Event
{
  public:
	/// @brief AppTickEvent 생성자.
	AppTickEvent() = default;

	EVENT_CLASS_TYPE(APP_TICK);
	EVENT_CLASS_CATEGORY(EVENT_CATEGORY_APP);
};

/**
 * @class AppUpdateEvent
 * @brief 이벤트 클래스: 애플리케이션이 업데이트될 때 발생하는 이벤트.
 */
class AppUpdateEvent : public Event
{
  public:
	/// @brief AppUpdateEvent 생성자.
	AppUpdateEvent() = default;

	EVENT_CLASS_TYPE(APP_UPDATE);
	EVENT_CLASS_CATEGORY(EVENT_CATEGORY_APP);
};

/**
 * @class AppRenderEvent
 * @brief 이벤트 클래스: 애플리케이션이 렌더링될 때 발생하는 이벤트.
 */
class AppRenderEvent : public Event
{
  public:
	/// @brief AppRenderEvent 생성자.
	AppRenderEvent() = default;

	EVENT_CLASS_TYPE(APP_RENDER);
	EVENT_CLASS_CATEGORY(EVENT_CATEGORY_APP);
};

} // namespace ale
