using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using ALEngine;

namespace Sandbox
{
    public class Button : Entity
    {
        private TransformComponent m_Transform;
        private RigidbodyComponent m_Rigidbody;

		public bool isOn = false;

        void onCreate()
        {
            m_Rigidbody = getComponent<RigidbodyComponent>();

            Console.WriteLine($"Button.OnCreate - {ID}");
        }

		void onUpdate(float ts)
		{
            if (m_Rigidbody.getTouchNum() > 0 && Input.isKeyDown(KeyCode.F))
                isOn = true;
            else
                isOn = false;
		}
    }
}
