#pragma once

#include <string>

namespace ale
{
/**
 * @class FileDialogs
 * @brief 파일 열기 및 저장 대화 상자를 제공하는 유틸리티 클래스.
 */
class FileDialogs
{
  public:
	/**
	 * @brief 파일 열기 대화 상자를 표시하고 선택한 파일 경로를 반환합니다.
	 * @param filter 파일 필터(예: "Image Files (*.png;*.jpg)\0*.png;*.jpg\0").
	 * @return std::string 선택한 파일의 경로. 사용자가 취소하면 빈 문자열 반환.
	 */
	static std::string openFile(const char *filter);

	/**
	 * @brief 파일 저장 대화 상자를 표시하고 선택한 파일 경로를 반환합니다.
	 * @param filter 파일 필터(예: "Text Files (*.txt)\0*.txt\0").
	 * @return std::string 선택한 파일의 경로. 사용자가 취소하면 빈 문자열 반환.
	 */
	static std::string saveFile(const char *filter);
};
} // namespace ale