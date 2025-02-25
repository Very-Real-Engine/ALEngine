#pragma once

#include "Core/Window.h"
#include <GLFW/glfw3.h>

namespace ale
{
/**
 * @class WindowsWindow
 * @brief GLFW를 사용하여 구현된 Windows 플랫폼용 창(Window) 클래스.
 */
class WindowsWindow : public Window
{
  public:
	/**
	 * @brief WindowsWindow 생성자.
	 * @param props 창의 속성을 정의하는 WindowProps 구조체.
	 */
	WindowsWindow(const WindowProps &props);

	/// @brief WindowsWindow 소멸자.
	virtual ~WindowsWindow();

	/// @brief 창을 업데이트하는 함수 (렌더링 루프에서 호출됨).
	void onUpdate() override;

	/**
	 * @brief 창의 너비를 반환합니다.
	 * @return uint32_t 현재 창의 너비.
	 */
	inline uint32_t getWidth() const override
	{
		return m_Data.width;
	}

	/**
	 * @brief 창의 높이를 반환합니다.
	 * @return uint32_t 현재 창의 높이.
	 */
	inline uint32_t getHeight() const override
	{
		return m_Data.height;
	}

	/**
	 * @brief 이벤트 콜백 함수를 설정합니다.
	 * @param callback 이벤트 콜백 함수.
	 */
	inline void setEventCallback(const EventCallbackFn &callback) override
	{
		m_Data.eventCallback = callback;
	}

	/**
	 * @brief GLFW의 기본 창 객체를 반환합니다.
	 * @return GLFWwindow* GLFW 창 객체 포인터.
	 */
	inline GLFWwindow *getNativeWindow() const override
	{
		return m_Window;
	}

	/**
	 * @brief VSync(수직 동기화) 설정.
	 * @param enabled true이면 VSync 활성화, false이면 비활성화.
	 */
	void setVSync(bool enabled) override;
	
	/**
	 * @brief VSync(수직 동기화) 상태를 반환합니다.
	 * @return true VSync가 활성화된 경우.
	 * @return false VSync가 비활성화된 경우.
	 */
	bool isVSync() const override;

  private:
	/**
	 * @brief 창을 초기화하는 내부 함수.
	 * @param props 창의 속성을 정의하는 WindowProps 구조체.
	 */
	virtual void init(const WindowProps &props);
	
	/// @brief 창을 종료하는 내부 함수.
	virtual void shutDown();

  private:
	GLFWwindow *m_Window;

	/**
	 * @struct WindowData
	 * @brief 창의 속성을 저장하는 구조체.
	 */
	struct WindowData
	{
		std::string title;
		uint32_t width, height;
		bool vSync;

		EventCallbackFn eventCallback;
	};

	WindowData m_Data;
};
} // namespace ale
