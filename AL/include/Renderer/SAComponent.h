#ifndef SACOMPONENT_H
#define SACOMPONENT_H

#include "Core/Base.h"
#include "Core/Timestep.h"
#include "Renderer/Animation/SkeletalAnimations.h"
#include "Renderer/AnimationStateManager.h"
#include "Renderer/Common.h"
#include "Renderer/Model.h"

namespace ale
{

/** @brief 엔티티 애니메이션 인스턴스 관리 클래스 */
class SAComponent
{
  public:
#define NON_CURRENT_ANIMATION_STRING "NONE"
#define NON_CURRENT_ANIMATION_FLOAT -1.0f
#define NON_CURRENT_ANIMATION_INT -1
#define NON_CURRENT_ANIMATION_BOOL false

	using Bones = std::vector<Armature::Bone>;

	/** @brief 생성자. */
	SAComponent();
	/** @brief 생성자.
	 * @param model 모델 참조.
	 */
	SAComponent(std::shared_ptr<Model>& model);
	/** @brief 초기화. */
	void init();
	/** @brief 애니메이션 초기화.
	 * @param animation SkeletalAnimation 포인터.
	 */
	void initAnimation(SkeletalAnimation* animation);
	/** @brief 키 프레임 초기화. */
	void initKeyFrame();
	/** @brief 상태 관리자 초기화. */
	void initStateManager();
	/** @brief 애니메이션 시작.
	 * @param animation 애니메이션 이름.
	 */
	void start(std::string const& animation);
	/** @brief 애니메이션 시작.
	 * @param index 애니메이션 인덱스.
	 */
	void start(size_t index);
	/** @brief 기본 애니메이션 시작. */
	void start();
	/** @brief 애니메이션 정지. */
	void stop();
	/** @brief 모델 설정.
	 * @param model 모델 포인터.
	 */
	void setModel(std::shared_ptr<Model>& model);
	/** @brief 반복 설정.
	 * @param repeat 반복 여부.
	 * @param index 애니메이션 인덱스 (기본값 -1).
	 */
	void setRepeat(bool repeat, int index = -1);
	/** @brief 전체 반복 설정.
	 * @param repeat 반복 여부.
	 */
	void setRepeatAll(bool repeat);
	/** @brief 전체 반복 설정.
	 * @param repeats 반복 벡터.
	 */
	void setRepeatAll(const std::vector<bool> repeats) { m_Repeats = repeats; };
	/** @brief 현재 반복 설정.
	 * @param repeat 반복 여부.
	 */
	void setCurrentRepeat(bool repeat);
	/** @brief 데이터 설정.
	 * @param currentFrame 현재 프레임.
	 * @param data SAData 구조체.
	 * @param index 인덱스 (기본값 0).
	 */
	void setData(uint16_t currentFrame, const SAData& data, unsigned int index = 0);
	/** @brief 전체 데이터 설정.
	 * @param datas SAData 벡터.
	 */
	void setDataAll(std::vector<SAData> datas);
	/** @brief 속도 계수 설정.
	 * @param factor 속도 계수.
	 */
	void setSpeedFactor(float factor);
	/** @brief 현재 프레임 설정.
	 * @param currentFrame 프레임 값.
	 */
	void setCurrentFrame(uint32_t currentFrame) { m_FrameCounter = currentFrame; };
	/** @brief 현재 애니메이션 설정.
	 * @param animation SkeletalAnimation 포인터.
	 */
	void setCurrentAnimation(SkeletalAnimation* animation);
	/** @brief 애니메이션 업데이트.
	 * @param timestep Timestep 객체.
	 * @param currentFrame 현재 프레임.
	 */
	void updateAnimation(const Timestep& timestep, uint32_t currentFrame);
	/** @brief 전환 없이 애니메이션 업데이트.
	 * @param timestep Timestep 객체.
	 */
	void updateAnimationWithoutTransition(const Timestep& timestep);
	/** @brief 실행 상태 확인.
	 * @return 실행 중이면 true.
	 */
	bool isRunning() const;
	/** @brief 만료 여부 확인.
	 * @param timestep Timestep 객체.
	 * @return 만료되면 true.
	 */
	bool willExpire(const Timestep& timestep) const;
	/** @brief 현재 애니메이션 인덱스 반환.
	 * @return 애니메이션 인덱스.
	 */
	int getCurrentAnimationIndex();
	/** @brief 반복 여부 반환.
	 * @param index 애니메이션 인덱스 (기본값 -1).
	 * @return 반복 여부.
	 */
	bool getRepeat(int index = -1);
	/** @brief 애니메이션 지속 시간 반환.
	 * @return 지속 시간.
	 */
	float getDuration();
	/** @brief 현재 애니메이션 시간 반환.
	 * @return 현재 시간.
	 */
	float getCurrentTime();
	/** @brief 본 블렌드.
	 * @param to 대상 본 벡터.
	 * @param from 원본 본 벡터.
	 * @param blendFactor 블렌드 계수.
	 * @return 블렌드된 본 벡터.
	 */
	Bones blendBones(Bones& to, Bones& from, float blendFactor);
	/** @brief SAData 반환.
	 * @param index 인덱스 (기본값 0).
	 * @return SAData 구조체.
	 */
	SAData getData(unsigned int index = 0) const;
	/** @brief 현재 프레임 반환.
	 * @return 현재 프레임.
	 */
	uint16_t getCurrentFrame() { return m_FrameCounter; };
	/** @brief 현재 애니메이션 이름 반환.
	 * @return 애니메이션 이름.
	 */
	std::string getCurrentAnimationName();
	/** @brief 전체 반복 벡터 반환.
	 * @return 반복 벡터.
	 */
	std::vector<bool> getRepeatAll() { return m_Repeats; };
	/** @brief 현재 포즈 반환.
	 * @return 현재 포즈 행렬 벡터.
	 */
	std::vector<alglm::mat4>& getCurrentPose() { return m_CurrentPose; };
	/** @brief SAData 벡터 반환.
	 * @return SAData 벡터.
	 */
	std::vector<SAData> getDatas() { return m_Data; };
	/** @brief SkeletalAnimations 반환.
	 * @return SkeletalAnimations 포인터.
	 */
	std::shared_ptr<SkeletalAnimations> getAnimations() { return m_Animations; };
	/** @brief AnimationStateManager 반환.
	 * @return AnimationStateManager 포인터.
	 */
	std::shared_ptr<AnimationStateManager> getStateManager() { return m_StateManager; };

public:

private:
	/** @brief 애니메이션 블렌드 업데이트.
	 * @details AnimationStateManager에서 현재 트랜지션에 있는 두 가지 애니메이션 연산진행.
	 * @param timestep Timestep 객체.
	 * @param skeleton 스켈레톤 객체.
	 * @param currentFrame 현재 프레임.
	 */
	void blendUpdate(const Timestep& timestep, Armature::Skeleton& skeleton, uint32_t currentFrame);
	/** @brief m_Skeleton->m_ShaderData.m_FinalJointMatrices 크기 유지하면서 값 초기화(항등행렬). */
	void flush();
	/** @brief 애니메이션 인덱스 반환.
	 * @param name 애니메이션 이름.
	 * @return 애니메이션 인덱스.
	 */
	int getAnimIndex(const std::string& name);
	/** @brief 애니메이션 반복 여부 반환.
	 * @param animation SkeletalAnimation 포인터.
	 * @return 반복 여부.
	 */
	bool getAnimRepeat(SkeletalAnimation* animation);

private:
	SkeletalAnimation* m_CurrentAnimation;                                   /**< 현재 애니메이션 */
	std::shared_ptr<AnimationStateManager> m_StateManager;                   /**< 상태 관리자 */
	std::shared_ptr<SkeletalAnimations> m_Animations;                        /**< 애니메이션들 */
	std::shared_ptr<Armature::Skeleton> m_Skeleton;                          /**< 스켈레톤 */
	std::shared_ptr<Model> m_Model;                                          /**< 모델 */
	std::vector<bool> m_Repeats;                                             /**< 각 애니메이션 별 반복 벡터 */
	uint32_t m_FrameCounter;                                                 /**< 프레임 카운터 */
	Bones m_CapturedPose;                                                    /**< 캡처된 포즈 */
	std::vector<alglm::mat4> m_CurrentPose;                                  /**< 현재 포즈 행렬 벡터 (UBO)*/
	std::vector<SAData> m_Data;                                              /**< 각 애니메이션 별 키프레임 데이터(SAData) 벡터 */
};

} // namespace ale

#endif