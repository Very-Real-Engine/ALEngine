#pragma once

/**
 * @file alpch.h
 * @brief 프로젝트에서 사용되는 미리 컴파일된 헤더(precompiled header).
 *        자주 사용되는 헤더 파일을 포함하여 컴파일 시간을 단축합니다.
 */

// precompiled header
#include <algorithm>
#include <fstream>
#include <iostream>
#include <utility>
#include <vector>

#include <filesystem>
#include <functional>
#include <memory>
#include <sstream>
#include <string>

#include "Core/Log.h"
#include "Debug/Instrumentor.h"

// vulkan
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <optional>
#include <set>
#include <stdexcept>

// glm
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
