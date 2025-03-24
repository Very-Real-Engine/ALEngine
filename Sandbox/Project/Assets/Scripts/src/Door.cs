using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using ALEngine;

namespace Sandbox
{
	public class Door : Entity
	{
		private RigidbodyComponent m_Rigidbody;

		private SkeletalAnimatorComponent m_Animator;

		public bool isOpen = false;
		private bool fkeyWasDown = false;

        void onCreate()
        {
            m_Rigidbody = getComponent<RigidbodyComponent>();
			m_Animator = getComponent<SkeletalAnimatorComponent>();

			m_Animator.onInverse();

			Console.WriteLine($"Door.OnCreate - {ID}");
		}

		void onUpdate(float ts)
		{
			bool isFDown = Input.isKeyDown(KeyCode.F);

			bool shouldToggle = isFDown && !fkeyWasDown;

			fkeyWasDown = isFDown;

			if (shouldToggle)
			{
				isOpen = !isOpen;

				if (!isOpen)
					m_Animator.onInverse();
				else
					m_Animator.offInverse();
			}
			
		}
	}
}
