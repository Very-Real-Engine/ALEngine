#include "Renderer/Animation/SkeletalAnimations.h"

namespace ale
{
SkeletalAnimations::SkeletalAnimations() : m_CurrentAnimation(nullptr), m_FrameCounter(1) {}

void SkeletalAnimations::push(std::shared_ptr<SkeletalAnimation> const& animation)
{
	if (animation)
	{
		m_Animations[animation->getName()] = animation;
		m_AnimationsVector.push_back(animation);
		m_NameToIndex[animation->getName()] = static_cast<int>(m_AnimationsVector.size() - 1);
	}
}

// by name
SkeletalAnimation& SkeletalAnimations::operator[](std::string const& animation) { return *m_Animations[animation]; }

// by index
SkeletalAnimation& SkeletalAnimations::operator[](uint16_t index) { return *m_AnimationsVector[index]; }

void SkeletalAnimations::start(std::string const& animation)
{
	SkeletalAnimation* currentAnimation = m_Animations[animation].get();
	if (currentAnimation)
	{
		m_CurrentAnimation = currentAnimation;
		m_CurrentAnimation->start();
	}
}
float SkeletalAnimations::getCurrentTime()
{
	if (m_CurrentAnimation)
	{
		return m_CurrentAnimation->getCurrentTime();
	}
	else
	{
		return 0.0f;
	}
}

uint16_t SkeletalAnimations::getCurrentFrame() { return m_FrameCounter; }

std::string SkeletalAnimations::getName()
{
	if (m_CurrentAnimation)
	{
		return m_CurrentAnimation->getName();
	}
	else
	{
		return std::string("");
	}
}

float SkeletalAnimations::getDuration(std::string const& animation) { return m_Animations[animation]->getDuration(); }

void SkeletalAnimations::start(size_t index)
{
	if (!(index < m_AnimationsVector.size()))
	{
		AL_CORE_INFO("invalid animation index: {0}", index);
		return;
	}
	SkeletalAnimation* currentAnimation = m_AnimationsVector[index].get();
	if (currentAnimation)
	{
		m_CurrentAnimation = currentAnimation;
		m_CurrentAnimation->start();
	}
}

void SkeletalAnimations::stop()
{
	if (m_CurrentAnimation)
	{
		m_CurrentAnimation->stop();
	}
}

void SkeletalAnimations::setRepeat(bool repeat)
{
	if (m_CurrentAnimation)
	{
		m_CurrentAnimation->setRepeat(repeat);
	}
}

void SkeletalAnimations::setRepeatAll(bool repeat)
{
	for (auto& animation : m_AnimationsVector)
	{
		animation->setRepeat(repeat);
	}
}

bool SkeletalAnimations::isRunning() const
{
	if (m_CurrentAnimation)
	{
		return m_CurrentAnimation->isRunning();
	}
	else
	{
		return false;
	}
}

bool SkeletalAnimations::willExpire(const Timestep& timestep) const
{
	if (m_CurrentAnimation)
	{
		return m_CurrentAnimation->willExpire(timestep);
	}
	else
	{
		return false;
	}
}

void SkeletalAnimations::uploadData(SkeletalAnimation* animation, uint32_t frameCounter)
{
	m_CurrentAnimation = animation;
	m_FrameCounter = frameCounter;
}

void SkeletalAnimations::update(const Timestep& timestep, Armature::Skeleton& skeleton, uint16_t frameCounter)
{
	// if (m_FrameCounter != frameCounter)
	// {
		m_FrameCounter = frameCounter;

		if (m_CurrentAnimation)
		{
			m_CurrentAnimation->update(timestep, skeleton);
		}
	// }
}

// range-based for loop auxiliary functions
SkeletalAnimations::Iterator SkeletalAnimations::begin() { return Iterator(&(*m_AnimationsVector.begin())); }
SkeletalAnimations::Iterator SkeletalAnimations::end() { return Iterator(&(*m_AnimationsVector.end())); }

// iterator functions
SkeletalAnimations::Iterator::Iterator(pSkeletalAnimation* pointer) // constructor
{
	m_Pointer = pointer;
}
SkeletalAnimations::Iterator& SkeletalAnimations::Iterator::operator++() // pre increment
{
	++m_Pointer;
	return *this;
}
bool SkeletalAnimations::Iterator::operator!=(const Iterator& rightHandSide) // compare
{
	return m_Pointer != rightHandSide.m_Pointer;
}
SkeletalAnimation& SkeletalAnimations::Iterator::operator*() // dereference
{
	return *(*m_Pointer /*shared_ptr*/);
}

int SkeletalAnimations::getIndex(std::string const& animation)
{
	bool found = false;
	for (auto& element : m_AnimationsVector)
	{
		if (element->getName() == animation)
		{
			found = true;
			break;
		}
	}
	if (found)
	{
		return m_NameToIndex[animation];
	}
	else
	{
		return -1;
	}
}
}