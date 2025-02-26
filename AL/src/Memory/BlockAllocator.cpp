#include "alpch.h"

#include "Memory/BlockAllocator.h"

namespace ale
{
int32_t BlockAllocator::s_blockSizes[BLOCK_SIZE_COUNT] = {
	16,	  // 0
	32,	  // 1
	64,	  // 2
	96,	  // 3
	128,  // 4
	160,  // 5
	192,  // 6
	224,  // 7
	256,  // 8
	320,  // 9
	384,  // 10
	448,  // 11
	512,  // 12
	1024, // 13
	2048, // 14
	4096, // 15
};

uint8_t BlockAllocator::s_blockSizeLookup[MAX_BLOCK_SIZE + 1];
bool BlockAllocator::s_blockSizeLookupInitialized;

BlockAllocator::BlockAllocator()
{
	m_chunkSpace = CHUNK_ARRAY_INCREMENT;
	m_chunkCount = 0;
	m_chunks = (Chunk *)_aligned_malloc(m_chunkSpace * sizeof(Chunk), 16);

	memset(m_chunks, 0, m_chunkSpace * sizeof(Chunk));
	memset(m_availableBlocks, 0, sizeof(m_availableBlocks));

	// 블록 크기 조회 배열 초기화
	if (s_blockSizeLookupInitialized == false)
	{
		int8_t j = 0;
		for (int32_t i = 1; i <= MAX_BLOCK_SIZE; ++i)
		{
			if (j >= BLOCK_SIZE_COUNT)
			{
				AL_CORE_ERROR("BLOCK_SIZE_COUNT: {0}", BLOCK_SIZE_COUNT);
				throw std::runtime_error("failed to initialize blockSizeLookup");
			}

			if (i <= s_blockSizes[j])
			{
				s_blockSizeLookup[i] = static_cast<uint8_t>(j);
			}
			else
			{
				++j;
				s_blockSizeLookup[i] = static_cast<uint8_t>(j);
			}
		}
		
		s_blockSizeLookupInitialized = true;
	}
}

BlockAllocator::~BlockAllocator()
{
	// 모든 청크의 블록 해제
	for (int32_t i = 0; i < m_chunkCount; ++i)
	{
		free(m_chunks[i].blocks);
	}

	// 청크 리스트 해제
	free(m_chunks);
}

void *BlockAllocator::allocateBlock(int32_t size)
{
	if (size <= 0)
	{
		return nullptr;
	}

	if (size > MAX_BLOCK_SIZE)
	{
		AL_CORE_ERROR("Requested block size: {0}", size);
		throw std::runtime_error("try too large memory allocated");
	}

	int32_t index = s_blockSizeLookup[size];

	if (m_availableBlocks[index] != nullptr)
	{
		// 청크 내부에 할당된 블록이 존재하는 경우 해당 블록 return
		Block *block = m_availableBlocks[index];
		m_availableBlocks[index] = block->next;
		return block;
	}
	else
	{
		// 할당된 블록이 없는 경우 블록 새로 할당
		if (m_chunkCount == m_chunkSpace)
		{
			// 청크가 꽉찬 경우 크기 증가
			Chunk *oldChunks = m_chunks;
			m_chunkSpace += CHUNK_ARRAY_INCREMENT;
			m_chunks = (Chunk *)_aligned_malloc(m_chunkSpace * sizeof(Chunk), 16);
			memcpy(m_chunks, oldChunks, m_chunkCount * sizeof(Chunk));
			free(oldChunks);
		}

		// 새로운 청크 생성
		Chunk *chunk = m_chunks + m_chunkCount;
		chunk->blocks = (Block *)_aligned_malloc(CHUNK_SIZE, 16);

		// 청크 내부 블록들 생성
		int32_t blockSize = s_blockSizes[index];
		chunk->blockSize = blockSize;
		int32_t blockCount = CHUNK_SIZE / blockSize;

		Block *block = chunk->blocks;
		for (int32_t i = 1; i < blockCount; ++i)
		{
			Block *next = (Block *)((int8_t *)chunk->blocks + blockSize * i);
			block->next = next;
			block = next;
		}
		block->next = nullptr;

		m_availableBlocks[index] = chunk->blocks->next;
		++m_chunkCount;

		return chunk->blocks;
	}
}

void BlockAllocator::freeBlock(void *pointer, int32_t size)
{
	if (size <= 0)
	{
		return;
	}

	if (size > MAX_BLOCK_SIZE)
	{
		return;
	}
	int32_t index = s_blockSizeLookup[size];

	// free한 pointer를 다시 avilableBlock에 편입
	Block *block = (Block *)pointer;
	block->next = m_availableBlocks[index];
	m_availableBlocks[index] = block;
}
} // namespace ale