#pragma once

#include "Core/KeyCodes.h"
#include "Events/Event.h"

namespace ale
{
/**
 * @class KeyEvent
 * @brief 키보드 이벤트의 기본 클래스.
 */
class KeyEvent : public Event
{
  public:
	/**
	 * @brief 키 코드 값을 반환합니다.
	 * @return KeyCode 키보드 키 코드.
	 */
	KeyCode getKeyCode()
	{
		return m_KeyCode;
	}

	EVENT_CLASS_CATEGORY(EVENT_CATEGORY_KEYBOARD | EVENT_CATEGORY_INPUT)
  protected:
	/**
	 * @brief KeyEvent 생성자.
	 * @param keycode 이벤트가 발생한 키 코드.
	 */
	KeyEvent(const KeyCode keycode) : m_KeyCode(keycode)
	{
	}

	KeyCode m_KeyCode;
};

/**
 * @class KeyPressedEvent
 * @brief 키가 눌렸을 때 발생하는 이벤트.
 */
class KeyPressedEvent : public KeyEvent
{
  public:
	/**
	 * @brief KeyPressedEvent 생성자.
	 * @param keycode 눌린 키의 키 코드.
	 * @param isRepeat 키 반복 여부 (디폴트: false).
	 */
	KeyPressedEvent(const KeyCode keycode, bool isRepeat = false) : KeyEvent(keycode), m_IsRepeat(isRepeat)
	{
	}

	/**
	 * @brief 키가 반복 입력인지 확인합니다.
	 * @return true 키가 반복 입력된 경우.
	 * @return false 키가 한 번만 입력된 경우.
	 */
	bool isRepeat()
	{
		return m_IsRepeat;
	}

	/**
	 * @brief 이벤트 정보를 문자열로 변환합니다.
	 * @return std::string "KeyPressedEvent: keycode (repeat = isRepeat)" 형식의 문자열.
	 */
	std::string toString() const override
	{
		std::stringstream ss;
		ss << "KeyPressedEvent: " << m_KeyCode << " (repeat = " << m_IsRepeat << ")";
		return ss.str();
	}

	EVENT_CLASS_TYPE(KEY_PRESSED)

  protected:
	bool m_IsRepeat;
};

/**
 * @class KeyReleasedEvent
 * @brief 키가 떼어졌을 때 발생하는 이벤트.
 */
class KeyReleasedEvent : public KeyEvent
{
  public:
	/**
	 * @brief KeyReleasedEvent 생성자.
	 * @param keycode 떼어진 키의 키 코드.
	 */
	KeyReleasedEvent(const KeyCode keycode) : KeyEvent(keycode)
	{
	}

	/**
	 * @brief 이벤트 정보를 문자열로 변환합니다.
	 * @return std::string "KeyReleasedEvent: keycode" 형식의 문자열.
	 */
	std::string toString() const override
	{
		std::stringstream ss;
		ss << "KeyReleasedEvent: " << m_KeyCode;
		return ss.str();
	}

	EVENT_CLASS_TYPE(KEY_RELEASED)
};

/**
 * @class KeyTypedEvent
 * @brief 키가 입력되었을 때 발생하는 이벤트 (텍스트 입력을 위한 이벤트).
 */
class KeyTypedEvent : public KeyEvent
{
  public:
	/**
	 * @brief KeyTypedEvent 생성자.
	 * @param keycode 입력된 키의 키 코드.
	 */
	KeyTypedEvent(const KeyCode keycode) : KeyEvent(keycode)
	{
	}

	/**
	 * @brief 이벤트 정보를 문자열로 변환합니다.
	 * @return std::string "KeyTypedEvent: keycode" 형식의 문자열.
	 */
	std::string toString() const override
	{
		std::stringstream ss;
		ss << "KeyTypedEvent: " << m_KeyCode;
		return ss.str();
	}

	EVENT_CLASS_TYPE(KEY_TYPED)
};

} // namespace ale
