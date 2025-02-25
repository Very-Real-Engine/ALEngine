#pragma once

#include "imgui/imgui.h"

namespace ale::UI
{
/**
 * @struct ScopedStyleColor
 * @brief ImGui 스타일 색상을 일시적으로 변경하는 유틸리티 구조체.
 *        생성 시 지정한 색상을 적용하고, 소멸 시 자동으로 원래 스타일로 복원함.
 */
struct ScopedStyleColor
{
	/**
	 * @brief 기본 생성자.
	 */
	ScopedStyleColor() = default;

	/**
	 * @brief ImGui 스타일 색상을 설정하는 생성자 (ImVec4 색상 사용).
	 * @param idx 변경할 스타일 색상의 인덱스 (ImGuiCol).
	 * @param color 적용할 색상 (ImVec4).
	 * @param predicate 스타일 변경을 수행할지 여부 (기본값: true).
	 */
	ScopedStyleColor(ImGuiCol idx, ImVec4 color, bool predicate = true) : m_Set(predicate)
	{
		if (predicate)
			ImGui::PushStyleColor(idx, color);
	}

	/**
	 * @brief ImGui 스타일 색상을 설정하는 생성자 (ImU32 색상 사용).
	 * @param idx 변경할 스타일 색상의 인덱스 (ImGuiCol).
	 * @param color 적용할 색상 (ImU32).
	 * @param predicate 스타일 변경을 수행할지 여부 (기본값: true).
	 */
	ScopedStyleColor(ImGuiCol idx, ImU32 color, bool predicate = true) : m_Set(predicate)
	{
		if (predicate)
			ImGui::PushStyleColor(idx, color);
	}

	/**
	 * @brief 소멸자. 설정된 스타일 색상을 자동으로 복원합니다.
	 */
	~ScopedStyleColor()
	{
		if (m_Set)
			ImGui::PopStyleColor();
	}

  private:
	bool m_Set = false;
};
} // namespace ale::UI