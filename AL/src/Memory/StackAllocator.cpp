#include "alpch.h"

#include "Memory/StackAllocator.h"

namespace ale
{
StackAllocator::StackAllocator() : m_index(0), m_entryCount(0) {};

StackAllocator::~StackAllocator()
{
	while (m_entryCount >= 0)
	{
		StackEntry *entry = m_entries + m_entryCount;

		--m_entryCount;
	}
}

void *StackAllocator::allocateStack(int32_t size)
{
	if (size % 16 != 0)
	{
		size = (size / 16 + 1) * 16;
	}
	
	if (m_entryCount == MAX_STACK_ENTRY_SIZE)
	{
		AL_CORE_ERROR("Requested size: {0}", size);
		throw std::runtime_error("stack overflow");
	}

	StackEntry *entry = m_entries + m_entryCount;
	entry->size = size;
	if (m_index + size > STACK_SIZE)
	{
		AL_CORE_ERROR("Requested size: {0}", size);
		throw std::runtime_error("stack overflow");
	}
	else
	{
		entry->data = m_data + m_index;
		m_index += size;
	}

	++m_entryCount;

	return entry->data;
}

void StackAllocator::freeStack()
{
	if (m_entryCount == 0)
	{
		return;
	}

	StackEntry *entry = m_entries + m_entryCount - 1;

	m_index -= entry->size;

	--m_entryCount;
}
} // namespace ale
