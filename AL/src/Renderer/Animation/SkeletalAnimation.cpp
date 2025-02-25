#include "Core/Timestep.h"
#include "Renderer/Animation/SkeletalAnimation.h"

namespace ale
{

SkeletalAnimation::SkeletalAnimation(std::string const& name) : m_Name(name), m_Repeat(false) {}

void SkeletalAnimation::start() { m_CurrentKeyFrameTime = m_FirstKeyFrameTime; }

void SkeletalAnimation::stop() { m_CurrentKeyFrameTime = m_LastKeyFrameTime + 1.0f; }

bool SkeletalAnimation::isRunning() const { return (m_Repeat || (m_CurrentKeyFrameTime <= m_LastKeyFrameTime)); }

bool SkeletalAnimation::willExpire(const Timestep& timestep) const
{
	return (!m_Repeat && ((m_CurrentKeyFrameTime + timestep) > m_LastKeyFrameTime));
}

void SkeletalAnimation::update(const Timestep& timestep, Armature::Skeleton& skeleton)
{
	if (!isRunning())
	{
		// AL_INFO("SkeletalAnimation::update(): {0} Animation expired", m_Name);
		return;
	}
	m_CurrentKeyFrameTime += timestep;

	if (m_Repeat && (m_CurrentKeyFrameTime > m_LastKeyFrameTime))
	{
		m_CurrentKeyFrameTime = m_FirstKeyFrameTime;
	}
	for (auto& channel : m_Channels)
	{
		auto& sampler = m_Samplers[channel.m_samplerIndex];
		int boneIndex = skeleton.m_NodeNameToBoneIndex[channel.m_NodeName];
		auto& bone = skeleton.m_Bones[boneIndex]; // the joint to be animated
		for (size_t i = 0; i < sampler.m_Timestamps.size() - 1; i++)
		{
			if ((m_CurrentKeyFrameTime >= sampler.m_Timestamps[i]) &&
				(m_CurrentKeyFrameTime <= sampler.m_Timestamps[i + 1]))
			{
				switch (sampler.m_Interpolation)
				{
					case EInterpolationMethod::LINEAR:
					{
						float a = (m_CurrentKeyFrameTime - sampler.m_Timestamps[i]) /
									(sampler.m_Timestamps[i + 1] - sampler.m_Timestamps[i]);
						switch (channel.m_Path)
						{
							case EPath::TRANSLATION:
							{
								bone.m_DeformedNodeTranslation =
									glm::mix(sampler.m_TRSoutputValuesToBeInterpolated[i],
											 sampler.m_TRSoutputValuesToBeInterpolated[i + 1], a);
								break;
							}
							case EPath::ROTATION:
							{
								glm::quat quaternion1;
								quaternion1.x = sampler.m_TRSoutputValuesToBeInterpolated[i].x;
								quaternion1.y = sampler.m_TRSoutputValuesToBeInterpolated[i].y;
								quaternion1.z = sampler.m_TRSoutputValuesToBeInterpolated[i].z;
								quaternion1.w = sampler.m_TRSoutputValuesToBeInterpolated[i].w;

								glm::quat quaternion2;
								quaternion2.x = sampler.m_TRSoutputValuesToBeInterpolated[i + 1].x;
								quaternion2.y = sampler.m_TRSoutputValuesToBeInterpolated[i + 1].y;
								quaternion2.z = sampler.m_TRSoutputValuesToBeInterpolated[i + 1].z;
								quaternion2.w = sampler.m_TRSoutputValuesToBeInterpolated[i + 1].w;

								bone.m_DeformedNodeRotation = glm::normalize(glm::slerp(quaternion1, quaternion2, a));
								break;
							}
							case EPath::SCALE:
							{
								bone.m_DeformedNodeScale =
									glm::mix(sampler.m_TRSoutputValuesToBeInterpolated[i],
											 sampler.m_TRSoutputValuesToBeInterpolated[i + 1], a);
								break;
							}
							default:
								std::cout << "path not found\n";
						}
						break;
					}
					case EInterpolationMethod::STEP:
					{
						switch (channel.m_Path)
						{
							case EPath::TRANSLATION:
							{
								bone.m_DeformedNodeTranslation =
									glm::vec3(sampler.m_TRSoutputValuesToBeInterpolated[i]);
								break;
							}
							case EPath::ROTATION:
							{
								bone.m_DeformedNodeRotation.x = sampler.m_TRSoutputValuesToBeInterpolated[i].x;
								bone.m_DeformedNodeRotation.y = sampler.m_TRSoutputValuesToBeInterpolated[i].y;
								bone.m_DeformedNodeRotation.z = sampler.m_TRSoutputValuesToBeInterpolated[i].z;
								bone.m_DeformedNodeRotation.w = sampler.m_TRSoutputValuesToBeInterpolated[i].w;
								break;
							}
							case EPath::SCALE:
							{
								bone.m_DeformedNodeScale = glm::vec3(sampler.m_TRSoutputValuesToBeInterpolated[i]);
								break;
							}
							default:
								std::cout << "path not found\n";
						}
						break;
					}
					case EInterpolationMethod::CUBICSPLINE:
					{
						std::cout << "SkeletalAnimation::Update(...): interploation method CUBICSPLINE not supported\n";
						break;
					}
					default:
						std::cout << "SkeletalAnimation::Update(...): interploation method not supported\n";
						break;
				}
			}
		}
	}
}

void SkeletalAnimation::uploadData(const SAData& data, bool repeat)
{
	m_Repeat = repeat;
	m_FirstKeyFrameTime = data.m_FirstKeyFrameTime;
	m_LastKeyFrameTime = data.m_LastKeyFrameTime;
	m_CurrentKeyFrameTime = data.m_CurrentKeyFrameTime;
}

struct SAData SkeletalAnimation::getData() const
{
	struct SAData data{};
	// data.m_Repeat = m_Repeat;
	data.m_FirstKeyFrameTime = m_FirstKeyFrameTime;
	data.m_LastKeyFrameTime = m_LastKeyFrameTime;
	data.m_CurrentKeyFrameTime = m_CurrentKeyFrameTime;

	return data;
}
}