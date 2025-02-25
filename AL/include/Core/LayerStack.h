#pragma once

/**
 * @file LayerStack.h
 * @brief 애플리케이션 레이어 관리 클래스
 *
 * 이 파일은 `LayerStack` 클래스를 정의하며, 레이어의 추가, 제거, 순회 기능을 제공합니다.
 */

#include "Core/Base.h"
#include "Core/Layer.h"

#include <vector>

namespace ale
{
/**
 * @class LayerStack
 * @brief 레이어 관리 스택
 *
 * `LayerStack` 클래스는 애플리케이션에서 사용되는 레이어를 저장하고 관리하는 역할을 합니다.
 * 일반적인 레이어는 스택에 추가되며, 오버레이(overlay) 레이어는 최상위에 추가됩니다.
 */
class LayerStack
{
  public:
	/**
     * @brief 기본 생성자
     *
     * 빈 레이어 스택을 생성합니다.
     */
	LayerStack() = default;

	/**
     * @brief 소멸자
     *
     * 스택에 남아 있는 모든 레이어를 정리합니다.
     */
	~LayerStack();

	/**
     * @brief 일반 레이어를 스택에 추가합니다.
     * 
     * @param layer 추가할 레이어 객체
     */
	void pushLayer(Layer *layer);

	/**
     * @brief 오버레이 레이어를 스택에 추가합니다.
     * 
     * @param overlay 추가할 오버레이 객체
     */
	void pushOverlay(Layer *overlay);
	
	/**
     * @brief 일반 레이어를 스택에서 제거합니다.
     * 
     * @param layer 제거할 레이어 객체
     */
	void popLayer(Layer *layer);

	/**
     * @brief 오버레이 레이어를 스택에서 제거합니다.
     * 
     * @param overlay 제거할 오버레이 객체
     */
	void popOverlay(Layer *overlay);

	/**
     * @brief 모든 레이어를 삭제합니다.
     */
	void onDetach();

	/**
     * @brief 레이어 스택의 begin iterator를 반환합니다.
     * @return `m_Layers`의 begin iterator
     */
	std::vector<Layer *>::iterator begin()
	{
		return m_Layers.begin();
	}

	/**
     * @brief 레이어 스택의 end iterator를 반환합니다.
     * @return `m_Layers`의 end iterator
     */
	std::vector<Layer *>::iterator end()
	{
		return m_Layers.end();
	}

	/**
     * @brief 레이어 스택의 reverse_iterator를 반환합니다.
     * @return `m_Layers`의 reverse_iterator
     */
	std::vector<Layer *>::reverse_iterator rbegin()
	{
		return m_Layers.rbegin();
	}

	/**
     * @brief 레이어 스택의 end reverse_iterator를 반환합니다.
     * @return `m_Layers`의 end reverse_iterator
     */
	std::vector<Layer *>::reverse_iterator rend()
	{
		return m_Layers.rend();
	}

	/**
     * @brief 레이어 스택의 const_iterator를 반환합니다.
     * @return `m_Layers`의 const_iterator
     */
	std::vector<Layer *>::const_iterator begin() const
	{
		return m_Layers.begin();
	}

	/**
     * @brief 레이어 스택의 end const_iterator를 반환합니다.
     * @return `m_Layers`의 end const_iterator
     */
	std::vector<Layer *>::const_iterator end() const
	{
		return m_Layers.end();
	}

	/**
     * @brief 레이어 스택의 const_reverse_iterator를 반환합니다.
     * @return `m_Layers`의 const_reverse_iterator
     */
	std::vector<Layer *>::const_reverse_iterator rbegin() const
	{
		return m_Layers.rbegin();
	}

	/**
     * @brief 레이어 스택의 const_reverse_iterator를 반환합니다.
     * @return `m_Layers`의 const_reverse_iterator
     */
	std::vector<Layer *>::const_reverse_iterator rend() const
	{
		return m_Layers.rend();
	}

  private:
  	/**< 레이어 스택을 저장하는 컨테이너 */
	std::vector<Layer *> m_Layers;
	/**< 일반 레이어가 삽입될 위치 인덱스 */
	uint32_t m_LayerInsertIndex = 0;
};

} // namespace ale
