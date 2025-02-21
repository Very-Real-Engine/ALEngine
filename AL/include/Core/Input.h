#pragma once

/**
 * @file Input.h
 * @brief 입력 처리를 담당하는 클래스
 *
 * 이 클래스는 키보드 및 마우스 입력을 확인하는 정적(static) 메서드를 제공합니다.
 */

#include "Core/Base.h"
#include "Core/KeyCodes.h"
#include "Core/MouseCodes.h"
#include <glm/glm.hpp>

namespace ale
{

/**
 * @class Input
 * @brief 사용자 입력을 처리하는 클래스
 *
 * `Input` 클래스는 키보드 및 마우스 입력을 확인할 수 있는 정적 메서드를 제공합니다.
 */

class Input
{
  public:
  	/**
     * @brief 특정 키가 현재 눌려 있는지 확인합니다.
     * 
     * @param key 확인할 키 코드
     * @return 키가 눌려 있으면 `true`, 그렇지 않으면 `false`
     */
	static bool isKeyPressed(KeyCode key);

	/**
     * @brief 특정 마우스 버튼이 현재 눌려 있는지 확인합니다.
     * 
     * @param button 확인할 마우스 버튼 코드
     * @return 버튼이 눌려 있으면 `true`, 그렇지 않으면 `false`
     */
	static bool isMouseButtonPressed(MouseCode button);
	
	/**
     * @brief 현재 마우스 커서의 위치를 가져옵니다.
     * 
     * @return 마우스 위치 (x, y) 좌표를 포함하는 `glm::vec2` 객체
     */
	static glm::vec2 getMousePosition();
	
	/**
     * @brief 현재 마우스 커서의 X 좌표를 가져옵니다.
     * 
     * @return 마우스 커서의 X 좌표 값
     */
	static float getMouseX();

	/**
     * @brief 현재 마우스 커서의 Y 좌표를 가져옵니다.
     * 
     * @return 마우스 커서의 Y 좌표 값
     */
	static float getMouseY();
};
} // namespace ale
