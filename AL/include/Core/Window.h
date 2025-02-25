#pragma once

/**
 * @file Window.h
 * @brief 플랫폼 독립적인 윈도우 인터페이스 정의
 *
 * 이 파일은 애플리케이션의 창(Window)을 추상화하는 `Window` 클래스를 정의합니다.
 * 플랫폼에 따라 다르게 구현될 수 있으며, 창의 속성, 이벤트 처리 설정 등을 지원합니다.
 */

#include "ALpch.h"
#include "Core/Base.h"
#include "Events/Event.h"
#include <GLFW/glfw3.h>

namespace ale
{
/**
 * @struct WindowProps
 * @brief 창의 속성을 정의하는 구조체
 *
 * 창의 제목(title), 너비(width), 높이(height)을 설정할 수 있습니다.
 */
struct WindowProps
{
	std::string title;
	uint32_t width;
	uint32_t height;

	/**
     * @brief `WindowProps` 구조체의 생성자
     *
     * @param title 창 제목 (기본값: "ALEngine")
     * @param width 창 너비 (기본값: `WINDOW_WIDTH`)
     * @param height 창 높이 (기본값: `WINDOW_HEIGHT`)
     */
	WindowProps(const std::string &title = "ALEngine", unsigned int width = WINDOW_WIDTH,
				unsigned int height = WINDOW_HEIGHT)
		: title(title), width(width), height(height)
	{
	}
};

/**
 * @class Window
 * @brief 애플리케이션 창을 추상화하는 인터페이스
 *
 * `Window` 클래스는 창의 생성, 업데이트, 속성 설정 및 이벤트 처리를 위한 순수 가상 함수(pure virtual functions)를 제공합니다.
 * 플랫폼에 따라 다르게 구현될 수 있습니다.
 */
class Window
{
  public:
  	/**
     * @brief 이벤트 콜백 함수 타입 정의
     *
     * 이 타입을 사용하여 이벤트 핸들러를 설정할 수 있습니다.
     */
	using EventCallbackFn = std::function<void(Event &)>;

	/**
     * @brief 가상 소멸자
     *
     * 윈도우 객체의 소멸을 처리합니다.
     */
	virtual ~Window() = default;

	/**
     * @brief 창을 갱신합니다.
     *
     * 이 함수는 매 프레임마다 호출되며, 창의 내용을 갱신하는 역할을 합니다.
     */
	virtual void onUpdate() = 0;

	/**
     * @brief 창의 너비를 반환합니다.
     *
     * @return 창 너비 (`uint32_t`)
     */
	virtual uint32_t getWidth() const = 0;

	/**
     * @brief 창의 높이를 반환합니다.
     *
     * @return 창 높이 (`uint32_t`)
     */
	virtual uint32_t getHeight() const = 0;

	/**
     * @brief GLFW의 네이티브 창 포인터를 반환합니다.
     *
     * @return GLFW 창 객체 (`GLFWwindow *`)
     */
	virtual GLFWwindow *getNativeWindow() const = 0;

	/**
     * @brief 창의 이벤트 콜백을 설정합니다.
     *
     * @param callback 이벤트를 처리할 콜백 함수
     */
	virtual void setEventCallback(const EventCallbackFn &callback) = 0;

	/**
     * @brief VSync(수직 동기화)를 설정합니다.
     *
     * @param enabled VSync 활성화 여부 (`true` = 활성화, `false` = 비활성화)
     */
	virtual void setVSync(bool enabled) = 0;

	/**
     * @brief VSync(수직 동기화) 활성화 여부를 반환합니다.
     *
     * @return VSync 활성화 여부 (`true` = 활성화, `false` = 비활성화)
     */
	virtual bool isVSync() const = 0;

	/**
     * @brief 윈도우 객체를 생성합니다.
     *
     * 이 함수는 `Window`의 플랫폼별 구현체를 생성하여 반환합니다.
     *
     * @param props 창 속성을 지정하는 `WindowProps` 객체 (기본값: `WindowProps()`)
     * @return 생성된 `Window` 객체의 포인터
     */
	static Window *create(const WindowProps &props = WindowProps());
};
} // namespace ale
