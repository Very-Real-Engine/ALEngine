#include "Renderer/Animation/SkeletalAnimation.h"
#include "Core/Timestep.h"

namespace ale
{

SkeletalAnimation::SkeletalAnimation(std::string const& name):
	m_Name(name),
	m_Repeat(false),
	m_Inverse(false)
{}


void SkeletalAnimation::start()
{
	m_CurrentKeyFrameTime = m_FirstKeyFrameTime;
	if (m_Inverse)
		m_CurrentKeyFrameTime = m_LastKeyFrameTime;
}

void SkeletalAnimation::stop()
{
	m_CurrentKeyFrameTime = m_LastKeyFrameTime + 1.0f;
	if (m_Inverse)
		m_CurrentKeyFrameTime = m_FirstKeyFrameTime;
}

bool SkeletalAnimation::isRunning() const
{
	if (m_Inverse)
		return (m_Repeat || m_CurrentKeyFrameTime >= m_FirstKeyFrameTime);
	else
		return (m_Repeat || (m_CurrentKeyFrameTime <= m_LastKeyFrameTime));
}

bool SkeletalAnimation::willExpire(const Timestep &timestep) const
{
	if (m_Inverse)
		return (!m_Repeat && ((m_CurrentKeyFrameTime - timestep) < m_FirstKeyFrameTime));
	else
		return (!m_Repeat && ((m_CurrentKeyFrameTime + timestep) > m_LastKeyFrameTime));
}

void SkeletalAnimation::update(const Timestep &timestep, Armature::Skeleton &skeleton)
{
	if (!isRunning())
	{
		// AL_INFO("SkeletalAnimation::update(): {0} Animation expired", m_Name);
		return;
	}

	if (m_Inverse)
		m_CurrentKeyFrameTime -= timestep;
	else
		m_CurrentKeyFrameTime += timestep;

	if (m_Repeat && (m_CurrentKeyFrameTime > m_LastKeyFrameTime))
	{
		if (m_Inverse)
			m_CurrentKeyFrameTime = m_LastKeyFrameTime;
		else
			m_CurrentKeyFrameTime = m_FirstKeyFrameTime;
	}
	for (auto &channel : m_Channels)
	{
		auto &sampler = m_Samplers[channel.m_samplerIndex];
		int boneIndex = skeleton.m_NodeNameToBoneIndex[channel.m_NodeName];
		auto &bone = skeleton.m_Bones[boneIndex]; // the joint to be animated
		for (size_t i = 0; i < sampler.m_Timestamps.size() - 1; i++)
		{
			if ((m_CurrentKeyFrameTime >= sampler.m_Timestamps[i]) &&
				(m_CurrentKeyFrameTime <= sampler.m_Timestamps[i + 1]))
			{
				switch (sampler.m_Interpolation)
				{
				case EInterpolationMethod::LINEAR: {
					float a = (m_CurrentKeyFrameTime - sampler.m_Timestamps[i]) /
							  (sampler.m_Timestamps[i + 1] - sampler.m_Timestamps[i]);
					switch (channel.m_Path)
					{
					case EPath::TRANSLATION: {
						bone.m_DeformedNodeTranslation =
							alglm::mix(sampler.m_TRSoutputValuesToBeInterpolated[i],
									   sampler.m_TRSoutputValuesToBeInterpolated[i + 1], a);
						break;
					}
					case EPath::ROTATION: {
						alglm::quat quaternion1;
						quaternion1.x = sampler.m_TRSoutputValuesToBeInterpolated[i].x;
						quaternion1.y = sampler.m_TRSoutputValuesToBeInterpolated[i].y;
						quaternion1.z = sampler.m_TRSoutputValuesToBeInterpolated[i].z;
						quaternion1.w = sampler.m_TRSoutputValuesToBeInterpolated[i].w;

						alglm::quat quaternion2;
						quaternion2.x = sampler.m_TRSoutputValuesToBeInterpolated[i + 1].x;
						quaternion2.y = sampler.m_TRSoutputValuesToBeInterpolated[i + 1].y;
						quaternion2.z = sampler.m_TRSoutputValuesToBeInterpolated[i + 1].z;
						quaternion2.w = sampler.m_TRSoutputValuesToBeInterpolated[i + 1].w;

						bone.m_DeformedNodeRotation = alglm::normalize(alglm::slerp(quaternion1, quaternion2, a));
						break;
					}
					case EPath::SCALE: {
						bone.m_DeformedNodeScale = alglm::mix(sampler.m_TRSoutputValuesToBeInterpolated[i],
															  sampler.m_TRSoutputValuesToBeInterpolated[i + 1], a);
						break;
					}
					default:
						std::cout << "path not found\n";
					}
					break;
				}
				case EInterpolationMethod::STEP: {
					switch (channel.m_Path)
					{
					case EPath::TRANSLATION: {
						bone.m_DeformedNodeTranslation = alglm::vec3(sampler.m_TRSoutputValuesToBeInterpolated[i]);
						break;
					}
					case EPath::ROTATION: {
						bone.m_DeformedNodeRotation.x = sampler.m_TRSoutputValuesToBeInterpolated[i].x;
						bone.m_DeformedNodeRotation.y = sampler.m_TRSoutputValuesToBeInterpolated[i].y;
						bone.m_DeformedNodeRotation.z = sampler.m_TRSoutputValuesToBeInterpolated[i].z;
						bone.m_DeformedNodeRotation.w = sampler.m_TRSoutputValuesToBeInterpolated[i].w;
						break;
					}
					case EPath::SCALE: {
						bone.m_DeformedNodeScale = alglm::vec3(sampler.m_TRSoutputValuesToBeInterpolated[i]);
						break;
					}
					default:
						std::cout << "path not found\n";
					}
					break;
				}
				case EInterpolationMethod::CUBICSPLINE: {
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

void SkeletalAnimation::uploadData(const SAData &data, bool repeat)
{
	m_Repeat = repeat;
	m_FirstKeyFrameTime = data.m_FirstKeyFrameTime;
	m_LastKeyFrameTime = data.m_LastKeyFrameTime;
	m_CurrentKeyFrameTime = data.m_CurrentKeyFrameTime;
	m_Inverse = data.m_Inverse;
}

void SkeletalAnimation::flush()
{
	m_FirstKeyFrameTime = 0;
	m_LastKeyFrameTime = 0;
	m_CurrentKeyFrameTime = 0;
	m_Inverse = false;
}

struct SAData SkeletalAnimation::getData() const
{
	struct SAData data{};
	// data.m_Repeat = m_Repeat;
	data.m_FirstKeyFrameTime = m_FirstKeyFrameTime;
	data.m_LastKeyFrameTime = m_LastKeyFrameTime;
	data.m_CurrentKeyFrameTime = m_CurrentKeyFrameTime;
	data.m_Inverse = m_Inverse;

	return data;
}
} // namespace ale