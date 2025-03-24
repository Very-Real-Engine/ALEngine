using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using ALEngine;

namespace Sandbox
{
	[System.Runtime.InteropServices.StructLayout(System.Runtime.InteropServices.LayoutKind.Sequential)]
	public class PlayerCamera : Entity
	{
		private TransformComponent m_Transform;
		private RigidbodyComponent m_PlayerTransform;

		public string PlayerTag = "Player";
		public float MouseSensitivity = 0.01f;
		public float Xoffset = 0.0f;
		public float Yoffset = 1.65f;
		public float Zoffset = -5.0f;

		void onCreate()
		{
			Console.WriteLine($"PlayerCamera.OnCreate - {ID}");

			m_Transform = getComponent<TransformComponent>();
			if (!System.String.IsNullOrEmpty(PlayerTag))
			{
				m_PlayerTransform = findEntityByName(PlayerTag).getComponent<RigidbodyComponent>();

				Quaternion rotationQuaternion = m_PlayerTransform.Rotation;
				Vector3 rotation = rotationQuaternion.Rotate(Vector3.Zero);

				rotation.X += 3.14f;
				m_Transform.Rotation = rotation;
			}
		}

		void onUpdate(float ts)
		{
			if (m_PlayerTransform != null)
			{
				Vector3 playerTranslation = m_PlayerTransform.Position;

				playerTranslation.X += Xoffset;
				playerTranslation.Y += Yoffset;
				playerTranslation.Z += Zoffset;

				m_Transform.Translation = playerTranslation;
			}
			else
			{
				Console.WriteLine("PlayerTransform is null");
				PlayerTag = "Player";
				m_PlayerTransform = findEntityByName(PlayerTag).getComponent<RigidbodyComponent>();
				Console.WriteLine("Done");
			}
		}
	}
}
