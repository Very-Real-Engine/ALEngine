#ifndef SKELETALANIMATIONS_H
#define SKELETALANIMATIONS_H

#include <map>
#include "Renderer/Animation/SkeletalAnimation.h"

namespace ale
{
class SkeletalAnimations
{
	public:
		using pSkeletalAnimation = std::shared_ptr<SkeletalAnimation>;

		struct Iterator
		{
			Iterator(pSkeletalAnimation* pointer);
			Iterator& operator++();
			bool operator!=(const Iterator& rightHandSide);
			SkeletalAnimation& operator*();

		private:
			pSkeletalAnimation* m_Pointer;
		};

	public:
		Iterator begin();
		Iterator end();
		SkeletalAnimation& operator[](std::string const& animation);
		SkeletalAnimation& operator[](uint16_t index);

	public:
		SkeletalAnimations();

		size_t size() const { return m_Animations.size(); }
		void push(std::shared_ptr<SkeletalAnimation> const& animation);

		void start(std::string const& animation); // by name
		void start(size_t index);                 // by index
		void start() { start(0); };               // start animation 0
		void stop();
		void setRepeat(bool repeat);
		void setRepeatAll(bool repeat);
		bool isRunning() const;
		bool willExpire(const Timestep& timestep) const;
		float getDuration(std::string const& animation);
		float getCurrentTime();
		uint16_t getCurrentFrame();
		std::string getName();
		void uploadData(SkeletalAnimation* animation, uint32_t frameCounter);
		void update(const Timestep& timestep, Armature::Skeleton& skeleton, uint16_t frameCounter);
		int getIndex(std::string const& animation);

	private:
		std::map<std::string, std::shared_ptr<SkeletalAnimation>> m_Animations;
		std::vector<std::shared_ptr<SkeletalAnimation>> m_AnimationsVector;
		SkeletalAnimation* m_CurrentAnimation;
		uint32_t m_FrameCounter;
		std::map<std::string, int> m_NameToIndex;
};
}
#endif