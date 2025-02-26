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
			isOn = false;
        }

		void onUpdate()
		{

		}
    }
}
