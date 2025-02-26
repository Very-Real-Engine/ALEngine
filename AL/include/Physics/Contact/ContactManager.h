#pragma once

#include "Physics/BroadPhase.h"
#include "Physics/Contact/Contact.h"

namespace ale
{
/**
 * @file
 * @brief 충돌(contact) 관리 기능을 담당하는 ContactManager 클래스 정의.
 */

/**
 * @class ContactManager
 * @brief 물리 엔진의 충돌을 관리하는 클래스.
 * @details ContactManager는 BroadPhase 충돌 탐지를 수행하고, 새로운 충돌을 찾아 Contact 리스트를 갱신합니다.
 */
class ContactManager
{
  public:
	/**
	 * @brief ContactManager 생성자.
	 * @details 충돌 목록을 초기화합니다.
	 */
	ContactManager();

	/**
	 * @brief 두 개체 간의 충돌을 추가합니다.
	 * @param proxyUserDataA 첫 번째 개체의 프록시 데이터.
	 * @param proxyUserDataB 두 번째 개체의 프록시 데이터.
	 */
	void addPair(void *proxyUserDataA, void *proxyUserDataB);

	/**
	 * @brief 새로운 충돌을 찾습니다.
	 * @details BroadPhase를 통해 현재 존재하는 충돌을 감지하고 Contact 목록을 갱신합니다.
	 */
	void findNewContacts();

	/**
	 * @brief 기존 충돌과 동일한 충돌인지 확인합니다.
	 * @param link 확인할 ContactLink.
	 * @param fixtureA 첫 번째 개체의 Fixture.
	 * @param fixtureB 두 번째 개체의 Fixture.
	 * @param indexA 첫 번째 개체의 인덱스.
	 * @param indexB 두 번째 개체의 인덱스.
	 * @return 동일한 충돌이면 true, 아니면 false.
	 */
	bool isSameContact(ContactLink *link, Fixture *fixtureA, Fixture *fixtureB, int32_t indexA, int32_t indexB);

	/**
	 * @brief 현재 존재하는 충돌을 처리합니다.
	 * @details 모든 활성화된 충돌을 검사하고 필요한 물리 연산을 수행합니다.
	 */
	void collide();

	BroadPhase m_broadPhase;
	Contact *m_contactList;
	int32_t m_contactCount;
};
} // namespace ale