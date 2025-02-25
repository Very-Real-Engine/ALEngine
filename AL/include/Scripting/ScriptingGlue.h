#pragma once

namespace ale
{
/**
 * @class ScriptGlue
 * @brief C# 스크립트와 C++ 엔진을 연결하는 클래스.
 *        엔진의 컴포넌트 및 함수를 C#에서 사용할 수 있도록 등록하는 역할을 수행.
 */
class ScriptGlue
{
  public:
	/// @brief 엔진의 컴포넌트를 C#에서 사용할 수 있도록 등록합니다.
	static void registerComponents();

	/// @brief 엔진의 특정 함수를 C#에서 호출할 수 있도록 등록합니다.
	static void registerFunctions();
};
} // namespace ale