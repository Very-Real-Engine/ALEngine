#pragma once

#include <cstdint>

namespace ale
{

/**
 * @file
 * @brief 스택 기반 메모리 할당을 관리하는 StackAllocator 클래스 정의.
 */

const int32_t STACK_SIZE = 10 * 1024 * 1024; /**< 스택 크기 (10MB) */
const int32_t MAX_STACK_ENTRY_SIZE = 32;	 /**< 최대 스택 항목 개수 */

/**
 * @struct StackEntry
 * @brief 개별 스택 항목을 나타내는 구조체.
 */
struct StackEntry
{
	char *data;	  /**< 할당된 데이터의 시작 주소 */
	int32_t size; /**< 데이터 크기 (바이트 단위) */
};

/**
 * @class StackAllocator
 * @brief 스택 기반 메모리 할당을 수행하는 클래스.
 */
class StackAllocator
{
  public:
	/**
	 * @brief 생성자.
	 */
	StackAllocator();
	/**
	 * @brief 소멸자.
	 */
	~StackAllocator();

	/**
	 * @brief 지정된 크기의 메모리를 스택에서 할당합니다.
	 * @param size 할당할 메모리 크기 (바이트 단위).
	 * @return 할당된 메모리의 포인터.
	 */
	void *allocateStack(int32_t size);

	/**
	 * @brief 스택에서 최근 할당된 메모리를 해제합니다.
	 */
	void freeStack();

  private:
	alignas(16) char m_data[STACK_SIZE];
	int32_t m_index;

	StackEntry m_entries[MAX_STACK_ENTRY_SIZE]; /**< 스택 할당 항목 목록 */
	int32_t m_entryCount;						/**< 현재 스택 항목 개수 */
};
} // namespace ale