#ifndef SKELETALANIMATION_H
#define SKELETALANIMATION_H

#include "Renderer/Animation/Skeleton.h"

namespace ale
{

/**
 * @brief 애니메이션 키 프레임 데이터를 저장하는 구조체.
 *
 * 첫 번째, 마지막, 현재 키 프레임 시간을 저장함.
 */
struct SAData
{
	float m_FirstKeyFrameTime;  /**< 첫 키 프레임 시간 */
	float m_LastKeyFrameTime;   /**< 마지막 키 프레임 시간 */
	float m_CurrentKeyFrameTime;/**< 현재 키 프레임 시간 */
	bool m_Inverse;
};

class Timestep;

/**
 * @brief 스켈레탈 애니메이션 처리를 위한 클래스.
 *
 * 애니메이션의 시작, 정지, 업데이트, 데이터 업로드 기능을 제공함.
 */
class SkeletalAnimation
{
	public:
		/**
		 * @brief 애니메이션 변환 경로 열거형.
		 *
		 * 위치, 회전, 스케일 변환 경로를 나타냄.
		 */
		enum class EPath
		{
			TRANSLATION,  /**< 위치 변환 */
			ROTATION,     /**< 회전 변환 */
			SCALE         /**< 스케일 변환 */
		};

		/**
		 * @brief 보간 방법 열거형.
		 *
		 * 키 프레임 간 보간 방법을 나타냄.
		 */
		enum class EInterpolationMethod
		{
			LINEAR,      /**< 선형 보간 */
			STEP,        /**< 스텝 보간 */
			CUBICSPLINE  /**< 큐빅 스플라인 보간 */
		};
		
		/**
		 * @brief 애니메이션 채널 구조체.
		 *
		 * 노드에 대한 애니메이션 적용 정보를 저장함.
		 */
		struct Channel
		{
			size_t		m_samplerIndex; /**< 관련 샘플러 인덱스 */
			std::string	m_NodeName;     /**< 노드 이름 */
			EPath		m_Path;         /**< 변환 경로 */
		};

		/**
		 * @brief 애니메이션 샘플러 구조체.
		 *
		 * 보간 방법, 타임스탬프, 변환 값을 저장함.
		 */
		struct Sampler
		{
			EInterpolationMethod	m_Interpolation; /**< 보간 방법 */
			std::vector<float>		m_Timestamps;    /**< 키 프레임 타임스탬프 */
			std::vector<alglm::vec4>	m_TRSoutputValuesToBeInterpolated; /**< 변환 값들 */
		};

	public:
		/**
		 * @brief 생성자.
		 *
		 * 애니메이션 이름을 초기화함.
		 *
		 * @param name 애니메이션 이름
		 */
		SkeletalAnimation(std::string const& name);

		/**
		 * @brief 애니메이션 시작.
		 */
		void start();

		/**
		 * @brief 애니메이션 정지.
		 */
		void stop();

		/**
		 * @brief 애니메이션 실행 상태 확인.
		 *
		 * @return 실행 중이면 true, 아니면 false
		 */
		bool isRunning() const;

		/**
		 * @brief 애니메이션 만료 여부 확인.
		 *
		 * @param timestep 시간 간격 정보
		 * @return 만료되면 true, 아니면 false
		 */
		bool willExpire(const Timestep& timestep) const;

		/**
		 * @brief 애니메이션 이름 반환.
		 *
		 * @return 애니메이션 이름
		 */
		std::string const& getName() const { return m_Name; }

		/**
		 * @brief 애니메이션 반복 설정.
		 *
		 * @param repeat 반복 여부 (true면 반복)
		 */
		void setRepeat(bool repeat) { m_Repeat = repeat; }

		/**
		 * @brief 애니메이션 데이터 업로드.
		 *
		 * SAData 데이터를 업로드하며 반복 여부에 따라 처리함.
		 *
		 * @param data 업로드할 데이터
		 * @param repeat 반복 여부
		 */
		void uploadData(const SAData& data, bool repeat);

		/**
		 * @brief 애니메이션 업데이트.
		 *
		 * 시간 간격에 따라 애니메이션 상태를 갱신하고 스켈레톤에 적용함.
		 *
		 * @param timestep 시간 간격 정보
		 * @param skeleton 업데이트할 스켈레톤 객체
		 */
		void update(const Timestep& timestep, Armature::Skeleton& skeleton);

		/**
		 * @brief 애니메이션 총 지속 시간 반환.
		 *
		 * 첫 키 프레임과 마지막 키 프레임 시간의 차이를 반환함.
		 *
		 * @return 애니메이션 지속 시간
		 */
		float getDuration() const { return m_LastKeyFrameTime - m_FirstKeyFrameTime; }

		/**
		 * @brief 현재 애니메이션 시간 반환.
		 *
		 * 현재 시간과 첫 키 프레임 시간의 차이를 반환함.
		 *
		 * @return 현재 애니메이션 시간
		 */
		float getCurrentTime() const { return m_CurrentKeyFrameTime - m_FirstKeyFrameTime; }

		/**
		 * @brief 현재 애니메이션 데이터 반환.
		 *
		 * 현재 상태를 기반으로 SAData 구조체를 생성하여 반환함.
		 *
		 * @return 현재 애니메이션 데이터
		 */
		struct SAData getData() const;

		/** @brief 애니메이션 샘플러들을 저장하는 벡터. */
		std::vector<SkeletalAnimation::Sampler> m_Samplers;
		/** @brief 애니메이션 채널들을 저장하는 벡터. */
		std::vector<SkeletalAnimation::Channel> m_Channels;

		/**
		 * @brief 첫 키 프레임 시간 설정.
		 *
		 * @param firstKeyFrameTime 첫 키 프레임 시간
		 */
		void setFirstKeyFrameTime(float firstKeyFrameTime) { m_FirstKeyFrameTime = firstKeyFrameTime; }

		/**
		 * @brief 마지막 키 프레임 시간 설정.
		 *
		 * @param lastKeyFrameTime 마지막 키 프레임 시간
		 */
		void setLastKeyFrameTime(float lastKeyFrameTime) { m_LastKeyFrameTime = lastKeyFrameTime; }

	private:
		std::string m_Name;              /**< 애니메이션 이름 */
		bool m_Inverse;                  /**< 역재생 */
		bool m_Repeat;                   /**< 반복 실행 여부 */
		float m_FirstKeyFrameTime;       /**< 첫 키 프레임 시간 */
		float m_LastKeyFrameTime;        /**< 마지막 키 프레임 시간 */
		float m_CurrentKeyFrameTime = 0.0f;/**< 현재 키 프레임 시간 */
};

} // namespace ale
#endif