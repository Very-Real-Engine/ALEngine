#pragma once

#include "Memory/BlockAllocator.h"
#include "Memory/StackAllocator.h"

namespace ale
{
/**
 * @file
 * @brief 물리 엔진에서 메모리 할당을 관리하는 PhysicsAllocator 클래스 정의.
 */

/**
 * @class PhysicsAllocator
 * @brief 물리 엔진에서 메모리 할당을 담당하는 클래스.
 * @details PhysicsAllocator는 BlockAllocator와 StackAllocator를 통해 동적 메모리 관리를 수행합니다.
 */
class PhysicsAllocator
{
  public:
	/**
	 * @brief 기본 생성자.
	 */
	PhysicsAllocator() = default;
	/**
	 * @brief 기본 소멸자.
	 */
	~PhysicsAllocator() = default;

	static BlockAllocator m_blockAllocator;
	static StackAllocator m_stackAllocator;
};

} // namespace ale