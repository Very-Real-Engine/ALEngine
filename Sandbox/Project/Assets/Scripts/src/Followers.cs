using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using ALEngine;

namespace Sandbox
{

	[System.Runtime.InteropServices.StructLayout(System.Runtime.InteropServices.LayoutKind.Sequential)]
	public class Follower : Entity
	{
		private RigidbodyComponent m_Rigidbody;
		private RigidbodyComponent m_PlayerRigidbody;
		private bool fKeyWasDown = false;

		public string PlayerTag = "Player";
		public string TargetField = "isOn";
		public float Xoffset;
		public float Yoffset;
		public float Zoffset;

		void onCreate()
		{
			Console.WriteLine($"Follower.OnCreate - {ID}");

			m_Rigidbody = getComponent<RigidbodyComponent>();
			if (!System.String.IsNullOrEmpty(PlayerTag))
			{
				m_PlayerRigidbody = findEntityByName(PlayerTag).getComponent<RigidbodyComponent>();
			}
		}

		void onUpdate(float ts)
		{
			// follow target
			if (!System.String.IsNullOrEmpty(PlayerTag) && findEntityByName(PlayerTag) != null)
			{
				if (m_PlayerRigidbody == null)
					m_PlayerRigidbody = findEntityByName(PlayerTag).getComponent<RigidbodyComponent>();

				Vector3 playerPosition = m_PlayerRigidbody.Position;

				playerPosition.X += Xoffset;
				playerPosition.Y += Yoffset;
				playerPosition.Z += Zoffset;

				m_Rigidbody.Position = playerPosition;

				Quaternion playerRotation = m_PlayerRigidbody.Rotation;

				m_Rigidbody.Rotation = playerRotation;
			}
		}

		void isCollide()
		{
			// 현재 프레임의 F키 상태를 먼저 확인
			bool isFDown = Input.isKeyDown(KeyCode.F);

			// 한 번의 키 입력으로 토글할지 결정 (Edge detection: 지금 눌렸고, 이전에는 안 눌렸을 때)
			bool shouldToggle = isFDown && !fKeyWasDown;

			// 업데이트 후, fKeyWasDown을 갱신 (다음 프레임을 위해)
			fKeyWasDown = isFDown;

			// 만약 이번 프레임에 키가 처음 눌렸다면 한 번만 토글
			if (shouldToggle)
			{
				// 조건에 맞는 엔티티 처리
				if (m_Rigidbody.getTouchNum() > 0 && !string.IsNullOrEmpty(TargetField))
				{
					Entity[] entities = findEntitiesByComponent<RigidbodyComponent>();

					foreach (var entity in entities)
					{
						// 만약 엔티티가 조건을 만족한다면
						if (entity.getComponent<RigidbodyComponent>().getTouchNum() > 0 && entity.hasComponent<ScriptComponent>())
						{
							ScriptComponent targetScript = entity.getComponent<ScriptComponent>();
							bool currentValue = targetScript.getFieldValue(TargetField);
							
							// 출력 및 토글 처리
							Console.WriteLine($"previous : {currentValue}");
							targetScript.setFieldValue(TargetField, !currentValue);
						}
					}
				}
			}
		}
	}
}
