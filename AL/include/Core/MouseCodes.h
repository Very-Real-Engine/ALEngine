#pragma once

/**
 * @file MouseCode.h
 * @brief 마우스 버튼 코드를 정의하는 헤더 파일
 *
 * 이 파일은 마우스 버튼 입력을 나타내는 `MouseCode` 타입과
 * `Mouse` 네임스페이스 내에서 버튼 코드를 정의하는 열거형을 제공합니다.
 */

namespace ale
{
/**
 * @typedef MouseCode
 * @brief 마우스 버튼을 표현하는 타입
 *
 * 마우스 입력을 나타내는 16비트 정수형(`uint16_t`) 타입을 `MouseCode`로 정의합니다.
 */
using MouseCode = uint16_t;

namespace Mouse
{
/**
 * @enum Mouse
 * @brief 마우스 버튼 코드 열거형
 *
 * 이 열거형은 마우스에서 사용되는 다양한 버튼 코드 값을 정의합니다.
 * 값은 `GLFW`의 마우스 버튼 코드와 일치합니다.
 */
enum : MouseCode
{
	// From glfw3.h
	Button0 = 0,
	Button1 = 1,
	Button2 = 2,
	Button3 = 3,
	Button4 = 4,
	Button5 = 5,
	Button6 = 6,
	Button7 = 7,

	ButtonLast = Button7,
	ButtonLeft = Button0,
	ButtonRight = Button1,
	ButtonMiddle = Button2
};
} // namespace Mouse
} // namespace ale
