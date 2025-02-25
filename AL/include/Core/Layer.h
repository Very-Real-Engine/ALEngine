#pragma once

/**
 * @file Layer.h
 * @brief 애플리케이션 레이어를 정의하는 클래스
 *
 * 이 파일은 애플리케이션의 레이어(layer)를 나타내는 `Layer` 클래스를 정의합니다.
 * 각 레이어는 독립적으로 업데이트되며, 이벤트를 처리하고 UI를 렌더링할 수 있습니다.
 */

#include "Core/Base.h"
#include "Core/Timestep.h"

#include "Events/Event.h"

namespace ale
{
/**
 * @class Layer
 * @brief 애플리케이션의 레이어를 표현하는 클래스
 *
 * `Layer` 클래스는 애플리케이션의 특정 레이어를 나타냅니다.
 * 이 클래스는 서브클래싱하여 특정 레이어 동작을 구현할 수 있습니다.
 */
class Layer
{
  public:
	/**
     * @brief 레이어 객체를 생성합니다.
     * 
     * @param name 레이어의 이름 (기본값: "Layer")
     */
	Layer(const std::string &name = "Layer");
	
	/**
     * @brief 가상 소멸자
     *
     * 기본적으로 `default` 처리되며, 레이어 객체가 소멸될 때 자동으로 호출됩니다.
     */
	virtual ~Layer() = default;

	/**
     * @brief 레이어가 애플리케이션에 추가될 때 호출됩니다.
     *
     * 이 함수는 서브클래스에서 오버라이드하여 초기화 작업을 수행할 수 있습니다.
     */
	virtual void onAttach();

	/**
     * @brief 레이어가 애플리케이션에서 제거될 때 호출됩니다.
     *
     * 이 함수는 서브클래스에서 오버라이드하여 정리 작업을 수행할 수 있습니다.
     */
	virtual void onDetach();

	/**
     * @brief 레이어의 업데이트를 수행합니다.
     * 
     * @param ts 현재 프레임의 시간 간격 (`Timestep` 객체)
     *
     * 매 프레임마다 호출되며, 서브클래스에서 오버라이드하여 특정 업데이트 로직을 추가할 수 있습니다.
     */
	virtual void onUpdate(Timestep ts);

	/**
     * @brief ImGui UI를 렌더링합니다.
     *
     * ImGui 요소를 그릴 때 사용되며, 서브클래스에서 오버라이드하여 구현할 수 있습니다.
     */
	virtual void onImGuiRender();

	/**
     * @brief 이벤트를 처리합니다.
     * 
     * @param event 처리할 이벤트 객체 (`Event` 타입)
     *
     * 애플리케이션의 이벤트를 처리하는 함수로, 서브클래스에서 오버라이드하여 특정 이벤트 로직을 구현할 수 있습니다.
     *//**
     * @brief 이벤트를 처리합니다.
     * 
     * @param event 처리할 이벤트 객체 (`Event` 타입)
     *
     * 애플리케이션의 이벤트를 처리하는 함수로, 서브클래스에서 오버라이드하여 특정 이벤트 로직을 구현할 수 있습니다.
     */
	virtual void onEvent(Event &event);

	/**
     * @brief 레이어의 이름을 반환합니다.
     * 
     * @return 레이어 이름 (`std::string` 참조 반환)
     */
	const std::string &getName() const
	{
		return m_DebugName;
	}

  private:
  	/**< 레이어의 디버그용 이름 */
	std::string m_DebugName;
};
} // namespace ale
