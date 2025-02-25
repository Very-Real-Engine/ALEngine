#ifndef SACOMPONENT_H
#define SACOMPONENT_H

#include "Core/Base.h"
#include "Core/Timestep.h"
#include "Renderer/Common.h"
#include "Renderer/Model.h"
#include "Renderer/Animation/SkeletalAnimations.h"
#include "Renderer/AnimationStateManager.h"

namespace ale
{

class SAComponent
{
public:
	#define NON_CURRENT_ANIMATION_STRING "NONE"
	#define NON_CURRENT_ANIMATION_FLOAT -1.0f
	#define NON_CURRENT_ANIMATION_INT -1
	#define NON_CURRENT_ANIMATION_BOOL false

	using Bones = std::vector<Armature::Bone>;

	SAComponent();
	SAComponent(std::shared_ptr<Model>& model);
	void initStateManager();
	void start(std::string const& animation);
	void start(size_t index);
	void start();
	void stop();
	void setModel(std::shared_ptr<Model>& model);
	void setRepeat(bool repeat, int index = -1);
	void setRepeatAll(bool repeat);
	void setRepeatAll(const std::vector<bool> repeats) { m_Repeats = repeats; };
	void setCurrentRepeat(bool repeat);
	void setData(uint16_t currentFrame, const SAData& data, unsigned int index = 0);
	void setSpeedFactor(float factor);
	void setCurrentFrame(uint32_t currentFrame) { m_FrameCounter = currentFrame; };
	void setCurrentAnimation(SkeletalAnimation* animation);
	void updateAnimation(const Timestep& timestep, uint32_t currentFrame);
	bool isRunning() const;
	bool willExpire(const Timestep& timestep) const;
	int getCurrentAnimationIndex();
	bool getRepeat(int index = -1);
	float getDuration();
	float getCurrentTime();
	float getSpeedFactor() { return m_SpeedFactor; };
	Bones blendBones(Bones& to, Bones& from, float blendFactor);
	struct SAData getData(unsigned int index = 0) const;
	uint16_t getCurrentFrame() { return m_FrameCounter; };
	std::string getCurrentAnimationName();
	std::vector<bool> getRepeatAll() { return m_Repeats; };
	std::vector<glm::mat4>& getCurrentPose() { return m_CurrentPose; };
	std::shared_ptr<SkeletalAnimations> getAnimations() { return m_Animations; };
	std::shared_ptr<AnimationStateManager> getStateManager() { return m_StateManager; };

public:

private:
	void init(SkeletalAnimation* animation);
	void blendUpdate(const Timestep& timestep, Armature::Skeleton& skeleton, uint32_t currentFrame);
	int getAnimIndex(const std::string& name);
	bool getAnimRepeat(SkeletalAnimation* animation);

private:
	SkeletalAnimation* m_CurrentAnimation;
	std::shared_ptr<AnimationStateManager> m_StateManager;
	std::shared_ptr<SkeletalAnimations> m_Animations;
	std::shared_ptr<Armature::Skeleton> m_Skeleton;
	std::shared_ptr<Model> m_Model;

	// save
	float	m_SpeedFactor;
	std::vector<bool> m_Repeats;

	// non-save
	uint32_t m_FrameCounter;
	Bones m_CapturedPose;
	std::vector<glm::mat4> m_CurrentPose;

	struct SAData m_Data[2];
};
} //namespace ale

#endif