#pragma once

#include "Core/Base.h"
#include <sstream>

namespace ale
{
/**
 * @enum EEventType
 * @brief 다양한 이벤트 타입을 정의하는 열거형.
 */
enum class EEventType
{
	NONE = 0,
	WINDOW_CLOSE,
	WINDOW_RESIZE,
	WINDOW_FOCUS,
	WINDOW_LOST_FOCUS,
	WINDOW_MOVED, // window
	APP_TICK,
	APP_UPDATE,
	APP_RENDER, // app
	KEY_PRESSED,
	KEY_RELEASED,
	KEY_TYPED, // key
	MOUSE_BUTTON_PRESSED,
	MOUSE_BUTTON_RELEASED,
	MOUSE_MOVE,
	MOUSE_SCROLLED // mouse
};

/**
 * @enum EEventCategory
 * @brief 이벤트의 카테고리를 정의하는 열거형.
 */
enum EEventCategory
{
	NONE = 0,
	EVENT_CATEGORY_APP = BIT(0),
	EVENT_CATEGORY_INPUT = BIT(1),
	EVENT_CATEGORY_KEYBOARD = BIT(2),
	EVENT_CATEGORY_MOUSE = BIT(3),
	EVENT_CATEGORY_MOUSE_BUTTON = BIT(4)
};

// instance 없이 type get
// #은 매크로에서 토큰을 문자열로 변환하는 데 사용됨
/**
 * @def EVENT_CLASS_TYPE(type)
 * @brief 이벤트 타입을 정의하는 매크로.
 * @param type 이벤트 타입 (EEventType 열거형 값).
 */
#define EVENT_CLASS_TYPE(type)                                                                                         \
	static EEventType getStaticType()                                                                                  \
	{                                                                                                                  \
		return EEventType::type;                                                                                       \
	}                                                                                                                  \
	virtual EEventType getEventType() const override                                                                   \
	{                                                                                                                  \
		return getStaticType();                                                                                        \
	}                                                                                                                  \
	virtual const char *getName() const override                                                                       \
	{                                                                                                                  \
		return #type;                                                                                                  \
	}

/**
 * @def EVENT_CLASS_CATEGORY(category)
 * @brief 이벤트 카테고리를 정의하는 매크로.
 * @param category 이벤트 카테고리 (EEventCategory 열거형 값).
 */
#define EVENT_CLASS_CATEGORY(category)                                                                                 \
	virtual int getCategoryFlags() const override                                                                      \
	{                                                                                                                  \
		return category;                                                                                               \
	}

/**
 * @class Event
 * @brief 모든 이벤트의 기본 클래스.
 */
class Event
{
	friend class EventDispatcher;

  public:
	/// @brief 가상 소멸자.
	virtual ~Event() = default;

	/**
	 * @brief 이벤트 타입을 반환합니다.
	 * @return EEventType 이벤트 타입.
	 */
	virtual EEventType getEventType() const = 0;

	/**
	 * @brief 이벤트 이름을 반환합니다.
	 * @return const char* 이벤트 이름.
	 */
	virtual const char *getName() const = 0;

	/**
	 * @brief 이벤트의 카테고리 플래그를 반환합니다.
	 * @return int 이벤트 카테고리 플래그.
	 */
	virtual int getCategoryFlags() const = 0;

	/**
	 * @brief 이벤트 정보를 문자열로 변환합니다.
	 * @return std::string 이벤트의 문자열 표현.
	 */
	virtual std::string toString() const
	{
		return getName();
	}

	/**
	 * @brief 특정 카테고리에 속하는지 확인합니다.
	 * @param category 확인할 이벤트 카테고리.
	 * @return true 해당 카테고리에 속하는 경우.
	 * @return false 해당 카테고리에 속하지 않는 경우.
	 */
	inline bool isInCategory(EEventCategory category)
	{
		return getCategoryFlags() & category;
	}

	bool m_Handled = false;
};

/**
 * @class EventDispatcher
 * @brief 발생한 이벤트를 처리하는 클래스.
 */
class EventDispatcher
{
  public:
	/**
	 * @brief EventDispatcher 생성자.
	 * @param event 처리할 이벤트.
	 */
	EventDispatcher(Event &event) : m_Event(event)
	{
	}

	/**
	 * @brief 특정 이벤트 타입에 대해 핸들러 함수를 실행합니다.
	 * @tparam T 이벤트 타입.
	 * @tparam F 이벤트 핸들러 함수의 타입.
	 * @param func 이벤트 핸들러 함수.
	 * @return true 이벤트가 처리된 경우.
	 * @return false 이벤트가 처리되지 않은 경우.
	 */
	template <typename T, typename F> bool dispatch(const F &func)
	{
		// F는 컴파일러에 의해 추론
		if (m_Event.getEventType() == T::getStaticType())
		{
			m_Event.m_Handled |= func(static_cast<T &>(m_Event));
			return true;
		}
		return false;
	}

  private:
	Event &m_Event;
};

/**
 * @brief 이벤트를 출력 스트림에 출력합니다.
 * @param os 출력 스트림.
 * @param e 출력할 이벤트.
 * @return std::ostream& 출력 스트림.
 */
inline std::ostream &operator<<(std::ostream &os, const Event &e)
{
	return os << e.toString();
}

} // namespace ale

