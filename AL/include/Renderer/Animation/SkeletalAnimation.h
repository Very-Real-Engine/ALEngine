#ifndef SKELETALANIMATION_H
#define SKELETALANIMATION_H

#include "Renderer/Animation/Skeleton.h"

namespace ale
{

struct SAData
{
	float m_FirstKeyFrameTime;
	float m_LastKeyFrameTime;
	float m_CurrentKeyFrameTime;
};

class Timestep;
class SkeletalAnimation
{
  public:
	enum class EPath
	{
		TRANSLATION,
		ROTATION,
		SCALE
	};

	enum class EInterpolationMethod
	{
		LINEAR,
		STEP,
		CUBICSPLINE
	};

	struct Channel
	{
		size_t m_samplerIndex;
		std::string m_NodeName;
		EPath m_Path;
	};

	struct Sampler
	{
		EInterpolationMethod m_Interpolation;
		std::vector<float> m_Timestamps;
		std::vector<alglm::vec4> m_TRSoutputValuesToBeInterpolated;
	};

  public:
	SkeletalAnimation(std::string const &name);

	void start();
	void stop();
	bool isRunning() const;
	bool willExpire(const Timestep &timestep) const;
	std::string const &getName() const
	{
		return m_Name;
	}
	void setRepeat(bool repeat)
	{
		m_Repeat = repeat;
	}
	void uploadData(const SAData &data, bool repeat);
	void update(const Timestep &timestep, Armature::Skeleton &skeleton);
	float getDuration() const
	{
		return m_LastKeyFrameTime - m_FirstKeyFrameTime;
	}
	float getCurrentTime() const
	{
		return m_CurrentKeyFrameTime - m_FirstKeyFrameTime;
	}
	struct SAData getData() const;

	std::vector<SkeletalAnimation::Sampler> m_Samplers;
	std::vector<SkeletalAnimation::Channel> m_Channels;

	void setFirstKeyFrameTime(float firstKeyFrameTime)
	{
		m_FirstKeyFrameTime = firstKeyFrameTime;
	}
	void setLastKeyFrameTime(float lastKeyFrameTime)
	{
		m_LastKeyFrameTime = lastKeyFrameTime;
	}

  private:
	std::string m_Name;
	bool m_Repeat;

	float m_FirstKeyFrameTime;
	float m_LastKeyFrameTime;
	float m_CurrentKeyFrameTime = 0.0f;
};
} // namespace ale
#endif