#pragma once

#include "Core/Buffer.h"

#include <filesystem>

namespace ale
{
/**
 * @brief 파일을 읽기 위해 사용하는 클래스
 * @details FileSystem 객체를 생성하지 않고 사용 가능.
 * 
 */

class FileSystem
{
  public:
	static SBuffer readFileBinary(const std::filesystem::path &filepath);
};
} // namespace ale