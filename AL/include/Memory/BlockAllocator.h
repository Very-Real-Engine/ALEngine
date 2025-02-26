#pragma once

#include <cstdint>

namespace ale
{

/**
 * @file
 * @brief 블록 메모리 할당을 관리하는 BlockAllocator 클래스 정의.
 */

const int32_t CHUNK_SIZE = 1024 * 1024;	   /**< 청크 크기 (1MB) */
const int32_t MAX_BLOCK_SIZE = 4096;	   /**< 최대 블록 크기 */
const int32_t BLOCK_SIZE_COUNT = 16;	   /**< 블록 크기 개수 */
const int32_t CHUNK_ARRAY_INCREMENT = 256; /**< 청크 배열 증가 단위 */

/**
 * @struct Block
 * @brief 블록 단위의 메모리 관리 구조체.
 */
struct Block
{
	Block *next; /**< 다음 블록을 가리키는 포인터 */
};

/**
 * @struct Chunk
 * @brief 특정 크기의 블록을 보관하는 구조체.
 */
struct Chunk
{
	Block *blocks;	   /**< 블록 메모리 포인터 */
	int32_t blockSize; /**< 블록 크기 */
};

/**
 * @class BlockAllocator
 * @brief 블록 기반 메모리 할당을 관리하는 클래스.
 */
class BlockAllocator
{
  public:
	/**
	 * @brief 생성자.
	 */
	BlockAllocator();

	/**
	 * @brief 소멸자.
	 */
	~BlockAllocator();

	/**
	 * @brief 블록을 할당합니다.
	 * @param size 할당할 블록의 크기 (바이트 단위).
	 * @return 할당된 블록의 포인터.
	 */
	void *allocateBlock(int32_t size);

	/**
	 * @brief 블록을 해제합니다.
	 * @param pointer 해제할 블록의 포인터.
	 * @param size 해제할 블록의 크기 (바이트 단위).
	 */
	void freeBlock(void *pointer, int32_t size);

  private:
	Chunk *m_chunks;		/**< 전체 청크 메모리 */
	int32_t m_chunkCount;	/**< 사용 중인 청크 수 */
	int32_t m_chunkSpace;	/**< 전체 청크 공간 */

	Block *m_availableBlocks[BLOCK_SIZE_COUNT];	/**< 사용 가능한 블록 리스트 */

	static int32_t s_blockSizes[BLOCK_SIZE_COUNT];			/**< 블록 크기 배열 */
	static uint8_t s_blockSizeLookup[MAX_BLOCK_SIZE + 1];	/**< 블록 크기 조회 테이블 */
	static bool s_blockSizeLookupInitialized;				/**< 블록 크기 조회 테이블 초기화 여부 */
};
} // namespace ale
