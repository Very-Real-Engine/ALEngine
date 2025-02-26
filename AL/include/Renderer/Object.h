#pragma once

/**
 * @file Object.h
 * @brief 객체 클래스
 */

#include "Core/Base.h"
#include "Renderer/Common.h"
#include "Renderer/Model.h"
#include "Renderer/Texture.h"

namespace ale
{
/**
 * @class Object
 * @brief 객체 클래스
 */
class Object
{
  public:
	/**
	 * @brief 객체 생성
	 * @param name 이름
	 * @param model 모델
	 * @param texture 텍스쳐
	 * @param position 위치
	 * @param rotation 회전
	 * @param scale 크기
	 * @return std::unique_ptr<Object> 객체
	 */
	static std::unique_ptr<Object> createObject(std::string name, std::shared_ptr<Model> model,
												std::shared_ptr<Texture> texture, alglm::vec3 position,
												alglm::vec3 rotation, alglm::vec3 scale);
	/**
	 * @brief 객체 소멸
	 */
	~Object()
	{
	}

	/**
	 * @brief 객체 그리기
	 * @param commandBuffer 명령 버퍼
	 */
	void draw(VkCommandBuffer commandBuffer);

	/**
	 * @brief 모델 행렬 반환
	 * @return alglm::mat4 모델 행렬
	 */
	alglm::mat4 getModelMatrix();
	/**
	 * @brief 텍스쳐 반환
	 * @return const std::shared_ptr<Texture> & 텍스쳐
	 */
	const std::shared_ptr<Texture> &getTexture()
	{
		return m_texture;
	}
	/**
	 * @brief 이름 반환
	 * @return const std::string & 이름
	 */
	const std::string &getName()
	{
		return m_name;
	}
	/**
	 * @brief 위치 반환
	 * @return alglm::vec3 & 위치
	 */
	alglm::vec3 &getPosition()
	{
		return m_position;
	}
	/**
	 * @brief 회전 반환
	 * @return alglm::vec3 & 회전
	 */
	alglm::vec3 &getRotation()
	{
		return m_rotation;
	}
	/**
	 * @brief 크기 반환
	 * @return alglm::vec3 & 크기
	 */
	alglm::vec3 &getScale()
	{
		return m_scale;
	}
	/**
	 * @brief 위치 설정
	 * @param position 위치
	 */
	void setPosition(alglm::vec3 position)
	{
		m_position = position;
	}
	/**
	 * @brief 회전 설정
	 * @param rotation 회전
	 */
	void setRotation(alglm::vec3 rotation)
	{
		m_rotation = rotation;
	}
	/**
	 * @brief 크기 설정
	 * @param scale 크기
	 */
	void setScale(alglm::vec3 scale)
	{
		m_scale = scale;
	}

  private:
	Object()
	{
	}

	std::shared_ptr<Model> m_model;
	std::shared_ptr<Texture> m_texture;
	alglm::vec3 m_position;
	alglm::vec3 m_rotation;
	alglm::vec3 m_scale;
	std::string m_name;

	/**
	 * @brief 객체 초기화
	 * @param name 이름
	 * @param model 모델
	 * @param texture 텍스쳐
	 * @param position 위치
	 * @param rotation 회전
	 * @param scale 크기
	 */
	void initObject(std::string name, std::shared_ptr<Model> model, std::shared_ptr<Texture> texture,
					alglm::vec3 position, alglm::vec3 rotation, alglm::vec3 scale);
};
} // namespace ale
