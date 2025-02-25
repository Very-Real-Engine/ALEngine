#ifndef SKELETALANIMATIONS_H
#define SKELETALANIMATIONS_H

#include <map>
#include "Renderer/Animation/SkeletalAnimation.h"

namespace ale
{
/** @brief SkeletalAnimations(Set): SkeletalAnimation(Clip) 관리 클래스
 * @details 여러개의 애니메이션 동작을 관리함.
*/
class SkeletalAnimations
{
	public:
		using pSkeletalAnimation = std::shared_ptr<SkeletalAnimation>;

		/** @brief Iterator 구조체. */
		struct Iterator
		{
			/** @brief 생성자.
			 * @param pointer 포인터.
			 */
			Iterator(pSkeletalAnimation* pointer);

			/** @brief 전위 증가 연산자.
			 * @return Iterator 참조.
			 */
			Iterator& operator++();

			/** @brief 비교 연산자.
			 * @param rightHandSide 비교 대상.
			 * @return 같지 않으면 true.
			 */
			bool operator!=(const Iterator& rightHandSide);

			/** @brief 역참조 연산자.
			 * @return SkeletalAnimation 참조.
			 */
			SkeletalAnimation& operator*();

		private:
			pSkeletalAnimation* m_Pointer; /**< 내부 포인터. */
		};

	public:
		/** @brief 시작 Iterator 반환.
		 * @return 시작 Iterator.
		 */
		Iterator begin();

		/** @brief 끝 Iterator 반환.
		 * @return 끝 Iterator.
		 */
		Iterator end();

		/** @brief 이름으로 SkeletalAnimation 반환.
		 * @param animation 애니메이션 이름.
		 * @return SkeletalAnimation 참조.
		 */
		SkeletalAnimation& operator[](std::string const& animation);

		/** @brief 인덱스로 SkeletalAnimation 반환.
		 * @param index 애니메이션 인덱스.
		 * @return SkeletalAnimation 참조.
		 */
		SkeletalAnimation& operator[](uint16_t index);

	public:
		/** @brief 생성자. */
		SkeletalAnimations();

		/** @brief 애니메이션 개수 반환.
		 * @return 애니메이션 개수.
		 */
		size_t size() const { return m_Animations.size(); }

		/** @brief 애니메이션 추가.
		 * @param animation 추가할 애니메이션.
		 */
		void push(std::shared_ptr<SkeletalAnimation> const& animation);

		/** @brief 이름으로 애니메이션 시작.
		 * @param animation 애니메이션 이름.
		 */
		void start(std::string const& animation);

		/** @brief 인덱스로 애니메이션 시작.
		 * @param index 애니메이션 인덱스.
		 */
		void start(size_t index);

		/** @brief 기본 애니메이션 시작. */
		void start() { start(0); };

		/** @brief 애니메이션 정지. */
		void stop();

		/** @brief 반복 설정.
		 * @param repeat 반복 여부.
		 */
		void setRepeat(bool repeat);

		/** @brief 전체 반복 설정.
		 * @param repeat 반복 여부.
		 */
		void setRepeatAll(bool repeat);

		/** @brief 애니메이션 실행 상태 반환.
		 * @return 실행 중이면 true.
		 */
		bool isRunning() const;

		/** @brief 애니메이션 만료 여부 반환.
		 * @param timestep 검사할 시간.
		 * @return 만료되면 true.
		 */
		bool willExpire(const Timestep& timestep) const;

		/** @brief 이름으로 애니메이션 지속 시간 반환.
		 * @param animation 애니메이션 이름.
		 * @return 애니메이션 지속 시간.
		 */
		float getDuration(std::string const& animation);

		/** @brief 현재 애니메이션 시간 반환.
		 * @return 현재 애니메이션 시간.
		 */
		float getCurrentTime();

		/** @brief 현재 프레임 반환.
		 * @return 현재 프레임.
		 */
		uint16_t getCurrentFrame();

		/** @brief 현재 애니메이션 이름 반환.
		 * @return 애니메이션 이름.
		 */
		std::string getName();

		/** @brief 데이터 업로드.
		 * @param animation 애니메이션 포인터.
		 * @param frameCounter 프레임 카운터.
		 */
		void uploadData(SkeletalAnimation* animation, uint32_t frameCounter);

		/** @brief 애니메이션 업데이트.
		 * @param timestep Timestep 객체.
		 * @param skeleton 스켈레톤 객체.
		 * @param frameCounter 프레임 카운터.
		 */
		void update(const Timestep& timestep, Armature::Skeleton& skeleton, uint16_t frameCounter);

		/** @brief 이름으로 인덱스 반환.
		 * @param animation 애니메이션 이름.
		 * @return 애니메이션 인덱스.
		 */
		int getIndex(std::string const& animation);

	private:
		std::map<std::string, std::shared_ptr<SkeletalAnimation>> m_Animations; /**< 애니메이션 맵. */
		std::vector<std::shared_ptr<SkeletalAnimation>> m_AnimationsVector;       /**< 애니메이션 벡터. */
		SkeletalAnimation* m_CurrentAnimation; /**< 현재 애니메이션 포인터. */
		uint32_t m_FrameCounter;               /**< 프레임 카운터. */
		std::map<std::string, int> m_NameToIndex; /**< 이름과 인덱스 매핑. */
};
}

#endif