using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using ALEngine;

namespace Sandbox
{
    public class ShootObject : Entity
    {
        private TransformComponent m_Transform;
        private RigidbodyComponent m_Rigidbody;

        public float force;

        void onCreate()
        {
            Console.WriteLine($"ShootObject.OnCreate - {ID}");

            m_Transform = getComponent<TransformComponent>();
            m_Rigidbody = getComponent<RigidbodyComponent>();
        }

        void onUpdate(float ts)
        {
            Vector3 velocity = Vector3.Zero;

            if (Input.isKeyDown(KeyCode.Right))
            {
                velocity.Z = -1.0f;
            }
            else if (Input.isKeyDown(KeyCode.Left))
            {
                velocity.Z = 1.0f;
            }

            if (Input.isKeyDown(KeyCode.Up))
            {
                velocity.X = -1.0f;
            }
            else if (Input.isKeyDown(KeyCode.Down))
            {
                velocity.X = 1.0f;
            }
			else if (Input.isKeyDown(KeyCode.E))
            {
                velocity.Y = 1.0f;
            }
			else if (Input.isKeyDown(KeyCode.Q))
            {
                velocity.Y = -1.0f;
            }

            velocity *= force;
            m_Rigidbody.addForce(velocity.XYZ);
        }

        bool isMoveForward()
        {
            if (Input.isKeyDown(KeyCode.Right))
                return true;
            return false;
        }

        bool isMoveBackward()
        {
            if (Input.isKeyDown(KeyCode.Left))
                return true;
            return false;
        }

        bool isMoveRight()
        {
            if (Input.isKeyDown(KeyCode.Down))
                return true;
            return false;
        }

        bool isMoveLeft()
        {
            if (Input.isKeyDown(KeyCode.Up))
                return true;
            return false;
        }

        bool isMoved()
        {
            if (isMoveForward() || isMoveBackward() || isMoveLeft() || isMoveRight())
                return true;
            return false;
        }
    }
}
