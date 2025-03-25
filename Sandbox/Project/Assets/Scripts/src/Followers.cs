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
		private TransformComponent m_PlayerRigidbody;
		private bool fKeyWasDown = false;

		public string PlayerTag = "SceneCamera";
		public string TargetField = "isOn";
		public float Xoffset = 0.0f;
		public float Yoffset = 0.0f;
		public float Zoffset = 0.0f;

		void onCreate()
		{
			Console.WriteLine($"Follower.OnCreate - {ID}");

			m_Rigidbody = getComponent<RigidbodyComponent>();
			if (!System.String.IsNullOrEmpty(PlayerTag))
			{
				m_PlayerRigidbody = findEntityByName(PlayerTag).getComponent<TransformComponent>();
			}
		}

		void onUpdate(float ts)
		{
			// follow target
			if (!System.String.IsNullOrEmpty(PlayerTag) && findEntityByName(PlayerTag) != null)
			{
				if (m_PlayerRigidbody == null)
					m_PlayerRigidbody = findEntityByName(PlayerTag).getComponent<TransformComponent>();

				Vector3 playerPosition = m_PlayerRigidbody.Translation;

				playerPosition.X += Xoffset;
				playerPosition.Y += Yoffset;
				playerPosition.Z += Zoffset;

				m_Rigidbody.Position = playerPosition;

				Quaternion playerRotation = Quaternion.FromEulerAngles(m_PlayerRigidbody.Rotation);

				m_Rigidbody.Rotation = playerRotation;
			}
		}
	}
}
