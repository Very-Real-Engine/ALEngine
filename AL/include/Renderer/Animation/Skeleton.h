#ifndef SKELETON_H
#define SKELETON_H

#include "Core/Base.h"
#include "Renderer/Animation/Bones.h"
#include "Renderer/Common.h"
#include "alpch.h"
#include <iostream>
#include <unordered_map>

namespace ale
{
	namespace Armature
	{
		/** @brief 부모가 없는 경우를 나타내는 상수 */
		static constexpr int NO_PARENT = -1;
		/** @brief 루트 조인트 인덱스를 나타내는 상수 */
		static constexpr int ROOT_JOINT = 0;

		/** @brief 셰이더 데이터 구조체 */
		struct ShaderData
		{
			std::vector<glm::mat4> m_FinalBonesMatrices; /**< 최종 본 행렬 벡터 */
		};

		// TODO : 최적화 -> 동적/정적 데이터 분리

		/** @brief 본(Bone) 구조체 */
		struct Bone
		{
			std::string m_Name;              /**< 본의 이름 */
			glm::mat4 m_InverseBindMatrix;   /**< 역 바인드 행렬 */
			glm::vec3 m_DeformedNodeTranslation{0.0f};  /**< 변형된 노드의 위치 */
			glm::quat m_DeformedNodeRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);  /**< 변형된 노드의 회전 */
			glm::vec3 m_DeformedNodeScale{1.0f};        /**< 변형된 노드의 스케일 */

			/** @brief 변형된 바인드 행렬 계산.
			 * @return 계산된 변형 바인드 행렬.
			 */
			glm::mat4 getDeformedBindMatrix()
			{
				return	glm::translate(glm::mat4(1.0f), m_DeformedNodeTranslation) * /**< 위치 변환 */
						glm::mat4(m_DeformedNodeRotation) * /**< 회전 변환 */
						glm::scale(glm::mat4(1.0f), m_DeformedNodeScale); /**< 스케일 변환 */
			}

			int m_ParentBone;              /**< 부모 본의 인덱스 */
			std::vector<int> m_Children;   /**< 자식 본들의 인덱스 목록 */
		};

		/** @brief 스켈레톤 구조체 */
		struct Skeleton
		{
			/** @brief 스켈레톤 전체 업데이트. */
			void update();

			/** @brief 특정 본 업데이트.
			 * @param boneIndex 업데이트할 본의 인덱스.
			 */
			void updateBone(int16_t boneIndex);

			std::string m_Name;                                  /**< 스켈레톤 이름 */
			std::vector<Bone> m_Bones;                           /**< 본(Bone)들의 벡터 */
			std::unordered_map<std::string, int> m_NodeNameToBoneIndex; /**< 노드 이름과 본 인덱스 매핑 */
			ShaderData m_ShaderData;                             /**< 셰이더에 전달할 본 행렬 데이터 */
			bool m_isAnimated = true;                            /**< 애니메이션 활성화 여부 */
		};
	};
}
#endif