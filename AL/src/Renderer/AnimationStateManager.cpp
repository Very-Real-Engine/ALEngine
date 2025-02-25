#include "Core/Timestep.h"
#include "Renderer/AnimationStateManager.h"

namespace ale
{

AnimationStateManager::AnimationStateManager() :
	inTransition(false),
	isTransitionFinish(false)
{
}

void AnimationStateManager::update(const Timestep& timestep)
{
	if (inTransition)
	{
		transitionTime += timestep;
		if (transitionTime >= transitionDuration)
			finishTransition();
	}
	else
		timeSinceLastTransition += timestep;

	processRequests();
	processTransitions();
}

void AnimationStateManager::processRequests()
{
	while (!m_RequestQueue.empty())
	{
		AnimationStateChangeRequest req = m_RequestQueue.front();
		m_RequestQueue.pop();

		for (auto& t : m_Transitions)
		{
			if ((t.fromState == "All" || t.fromState == currentState.stateName) &&
				t.toState == req.targetState)
			{
				if ((!t.invertCondition && t.condition && t.condition()) ||
					(t.invertCondition && t.condition && !t.condition()))
				{
					startTransition(t);
					break;
				}
			}
		}
	}
}

void AnimationStateManager::processTransitions()
{
	if (!currentState.interruptible && inTransition) return;

	for (auto& t : m_Transitions)
	{
		if ((t.fromState == "All" || t.fromState == currentState.stateName) &&
			t.toState != currentState.stateName)
		{
			if ((!t.invertCondition && t.condition && t.condition()) ||
				(t.invertCondition && t.condition && !t.condition()))
			{
				startTransition(t);
				break;
			}
		}
	}
}

void AnimationStateManager::startTransition(const AnimationStateTransition& t)
{
	if (currentState.stateName == t.toState) return;

	inTransition = true;
	isTransitionFinish = false;

	timeSinceLastTransition = 0.0f;
	transitionTime = 0.0f;
	transitionDuration = t.blendTime;

	prevState = currentState;
	currentState = *getState(t.toState);

	// AL_INFO("[AnimationStateManager] Transition:" + prevState.stateName + "->" + currentState.stateName);
}

void AnimationStateManager::finishTransition()
{
	inTransition = false;
	isTransitionFinish = true;

	transitionTime = 0.0f;
	transitionDuration = 0.0f;

	// AL_INFO("[AnimationStateManager] Transition Finished:" + prevState.stateName + "->" + currentState.stateName);
}

AnimationState* AnimationStateManager::getState(const std::string& stateName)
{
	auto it = m_States.find(stateName);

	if (it != m_States.end())
		return &it->second;
	return nullptr;
}

void AnimationStateManager::deleteTransition(const AnimationStateTransition& t)
{
	for (auto it = m_Transitions.begin(); it != m_Transitions.end(); ++it)
	{
		if (*it == t)
		{
			m_Transitions.erase(it);
			break;
		}
	}
}

bool AnimationStateManager::hasNoTransitionFor(float seconds) const
{
	return (timeSinceLastTransition >= seconds);
}

void AnimationStateManager::setStates(std::unordered_map<std::string, AnimationState>& states)
{
	m_States = states;
}

void AnimationStateManager::setTransitions(std::vector<AnimationStateTransition>& transitions)
{
	m_Transitions = transitions;
}

std::vector<AnimationStateTransition>& AnimationStateManager::getTransitions()
{
	return m_Transitions;
}

std::unordered_map<std::string, AnimationState>& AnimationStateManager::getStates()
{
	return m_States;
}

AnimationState* AnimationStateManager::getStateFromAnimName(const std::string& animationName)
{
	for (auto& it : m_States)
	{
		if (it.second.animationName == animationName)
			return &(it.second);
	}
	return nullptr;
}

}; //namespace ale