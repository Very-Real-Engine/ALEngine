#pragma once

#include <cstdint>

namespace ale
{
const int32_t STACK_SIZE = 10 * 1024 * 1024; // 100k
const int32_t MAX_STACK_ENTRY_SIZE = 32;

struct StackEntry
{
	char *data;
	int32_t size;
};

class StackAllocator
{
  public:
	StackAllocator();
	~StackAllocator();

	void *allocateStack(int32_t size);
	void freeStack();

  private:
	alignas(16) char m_data[STACK_SIZE];
	int32_t m_index;

	StackEntry m_entries[MAX_STACK_ENTRY_SIZE];
	int32_t m_entryCount;
};
} // namespace ale