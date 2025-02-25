#include "Renderer/Animation/Skeleton.h"

namespace ale
{
namespace Armature
{
void Skeleton::update()
{
	int16_t numberOfBones = static_cast<int16_t>(m_Bones.size());

	if (!m_isAnimated)
	{
		for (int16_t boneIndex = 0; boneIndex < numberOfBones; ++boneIndex)
			m_ShaderData.m_FinalBonesMatrices[boneIndex] = alglm::mat4(1.0f);
	}
	else
	{
		for (int16_t boneIndex = 0; boneIndex < numberOfBones; ++boneIndex)
		{
			m_ShaderData.m_FinalBonesMatrices[boneIndex] = m_Bones[boneIndex].getDeformedBindMatrix();
		}

		updateBone(ROOT_JOINT);

		for (int16_t boneIndex = 0; boneIndex < numberOfBones; ++boneIndex)
		{
			m_ShaderData.m_FinalBonesMatrices[boneIndex] =
				m_ShaderData.m_FinalBonesMatrices[boneIndex] * m_Bones[boneIndex].m_InverseBindMatrix;
		}
	}
}

void Skeleton::updateBone(int16_t boneIndex)
{
	auto &currentBone = m_Bones[boneIndex];

	int16_t parentBone = currentBone.m_ParentBone;
	if (parentBone != Armature::NO_PARENT)
	{
		m_ShaderData.m_FinalBonesMatrices[boneIndex] =
			m_ShaderData.m_FinalBonesMatrices[parentBone] * m_ShaderData.m_FinalBonesMatrices[boneIndex];
	}

	size_t numberOfChildren = currentBone.m_Children.size();
	for (size_t childIndex = 0; childIndex < numberOfChildren; ++childIndex)
	{
		int childBone = currentBone.m_Children[childIndex];
		updateBone(childBone);
	}
}
}; // namespace Armature
} // namespace ale