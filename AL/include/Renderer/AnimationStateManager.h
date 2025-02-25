#ifndef ANIMATIONSTATEMANAGER_H
#define ANIMATIONSTATEMANAGER_H

#include <queue>
#include "Core/Base.h"
#include "Renderer/Common.h"
#include "Renderer/Model.h"

namespace ale
{

/** @brief 애니메이션 상태 구조체. */
struct AnimationState
{
	std::string stateName;      /**< 상태 이름 */
	std::string animationName;  /**< 애니메이션 이름 */
	bool        looping;        /**< 루핑 여부 */
	bool        interruptible;  /**< 인터럽트 가능 여부 */
	float       defaultBlendTime; /**< 기본 블렌드 시간 */
};

/** @brief 애니메이션 상태 전환 구조체. */
struct AnimationStateTransition
{
	std::string fromState;      /**< 시작 상태 */
	std::string toState;        /**< 목표 상태 */
	std::string conditionName;  /**< 조건 이름 */
	std::function<bool()> condition; /**< 조건 함수 */
	float       blendTime;      /**< 블렌드 시간 */
	bool        invertCondition = false; /**< 조건 반전 여부 */

	/** @brief 연산자 ==.
	 * @param others 비교 대상.
	 * @return 같으면 true.
	 */
	bool operator==(const AnimationStateTransition& others)
	{
		if (fromState == others.fromState &&
			toState == others.toState &&
			conditionName == others.conditionName
		)
			return true;
		return false;
	}
};

/** @brief 애니메이션 상태 변경 요청 구조체. */
struct AnimationStateChangeRequest
{
	std::string targetState; /**< 목표 상태 (AnimationState::stateName) */
};

/** @brief 애니메이션 상태 관리 클래스. */
class AnimationStateManager
{
public:
	AnimationState currentState;         /**< 현재 상태 */
	AnimationState prevState;            /**< 이전 상태 */
	bool           inTransition;         /**< 전환 중 여부 */
	bool           isTransitionFinish;   /**< 전환 완료 여부 */
	float          transitionTime = 0.0f;  /**< 전환 시간 */
	float          transitionDuration = 0.0f; /**< 전환 지속 시간 */
	float          timeSinceLastTransition = 0.0f; /**< 마지막 전환 이후 시간 */

public:
	/** @brief 생성자. */
	AnimationStateManager();

	/** @brief 상태 반환.
	 * @param stateName 상태 이름.
	 * @return 상태 포인터.
	 */
	AnimationState* getState(const std::string& stateName);

	/** @brief 애니메이션 이름으로 상태 반환.
	 * @param animationName 애니메이션 이름.
	 * @return 상태 포인터.
	 */
	AnimationState* getStateFromAnimName(const std::string& animationName);

	/** @brief 초기화. */
	void init();

	/** @brief 상태 추가.
	 * @param s 상태 객체.
	 */
	void addState(const AnimationState& s) { m_States[s.stateName] = s; }

	/** @brief 전환 추가.
	 * @param t 전환 객체.
	 */
	void addTransition(const AnimationStateTransition& t) { m_Transitions.push_back(t); }

	/** @brief 상태 변경 요청 추가.
	 * @param target 목표 상태.
	 */
	void pushStateChangeRequest(const std::string& target) { m_RequestQueue.push({ target }); }

	/** @brief 전환 삭제.
	 * @param t 삭제할 전환 객체.
	 */
	void deleteTransition(const AnimationStateTransition& t);

	/** @brief 상태 설정.
	 * @param states 상태 매핑.
	 */
	void setStates(std::unordered_map<std::string, AnimationState>& states);

	/** @brief 전환 설정.
	 * @param transitions 전환 벡터.
	 */
	void setTransitions(std::vector<AnimationStateTransition>& transitions);

	/** @brief 애니메이션 상태관리 루프 업데이트.
	 * @param timestep Timestep 객체.
	 */
	void update(const Timestep& timestep);

	/** @brief 전환 없음 여부 확인.
	 * @param seconds 시간.
	 * @return 전환이 없으면 true.
	 */
	bool hasNoTransitionFor(float seconds) const;

	/** @brief 전환 벡터 반환.
	 * @return 전환 벡터.
	 */
	std::vector<AnimationStateTransition>& getTransitions();

	/** @brief 상태 매핑 반환.
	 * @return 상태 매핑.
	 */
	std::unordered_map<std::string, AnimationState>& getStates();

private:
	/** @brief 요청 처리. */
	void processRequests();

	/** @brief 전환 처리. */
	void processTransitions();

	/** @brief 전환 시작.
	 * @param t 전환 객체.
	 */
	void startTransition(const AnimationStateTransition& t);

	/** @brief 전환 완료. */
	void finishTransition();

private:
	std::unordered_map<std::string, AnimationState>	m_States;         /**< 상태 매핑 */
	std::vector<AnimationStateTransition>			m_Transitions;    /**< 전환 벡터 */
	std::queue<AnimationStateChangeRequest>			m_RequestQueue;   /**< 상태 변경 요청 큐 */
};

} // namespace ale

#endif