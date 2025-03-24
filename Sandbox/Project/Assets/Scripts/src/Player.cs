using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using ALEngine;

namespace Sandbox
{
	[System.Runtime.InteropServices.StructLayout(System.Runtime.InteropServices.LayoutKind.Sequential)]
    public class Player : Entity
    {
        private RigidbodyComponent m_Rigidbody;
        private TransformComponent m_Transform;

        private float MouseSensitivity = 0.01f;
        private float Speed = 2.0f;
        private float RotationSpeed = 1.0f;

        void onCreate()
        {

            m_Rigidbody = getComponent<RigidbodyComponent>();
            m_Transform = getComponent<TransformComponent>();

            Console.WriteLine($"Player.OnCreate - {ID}");
        }

        void onUpdate(float ts)
        {
            Vector3 velocity = Vector3.Zero;

            // 현재 회전(쿼터니언) 값을 가져옵니다.
            Quaternion currentRotation = m_Rigidbody.Rotation;

            // 이동 입력 (카메라의 로컬 좌표 기준)
            if (Input.isKeyDown(KeyCode.W))
                velocity.Z = -1.0f; // 전진: 기본적으로 -Z가 전방이라고 가정
            else if (Input.isKeyDown(KeyCode.S))
                velocity.Z = 1.0f;  // 후진

            if (Input.isKeyDown(KeyCode.A))
                velocity.X = 1.0f; // 좌측 이동
            else if (Input.isKeyDown(KeyCode.D))
                velocity.X = -1.0f;  // 우측 이동

            // Q키와 E키 입력에 따른 회전량 계산
            // Q: 왼쪽으로 회전 (Y축 양의 방향)
            if (Input.isKeyDown(KeyCode.Q))
            {
                // Y축 기준으로 회전 (라디안 단위)
                Quaternion deltaRotation = Quaternion.FromAxisAngle(new Vector3(0, 1, 0), RotationSpeed * ts);
                currentRotation = deltaRotation * currentRotation;
            }
            // E: 오른쪽으로 회전 (Y축 음의 방향)
            else if (Input.isKeyDown(KeyCode.E))
            {
                Quaternion deltaRotation = Quaternion.FromAxisAngle(new Vector3(0, 1, 0), -RotationSpeed * ts);
                currentRotation = deltaRotation * currentRotation;
            }

            // 업데이트된 회전값을 transform에 적용합니다.
            m_Rigidbody.Rotation = currentRotation;

            // 카메라 로컬 좌표계의 forward, right 벡터 계산
            Vector3 forward = Vector3Extensions.Transform(new Vector3(0, 0, -1), currentRotation);
            Vector3 right   = Vector3Extensions.Transform(new Vector3(1, 0, 0), currentRotation);

            // 카메라 기준 이동 벡터: forward * 전진/후진 + right * 좌/우
            Vector3 moveDir = forward * velocity.Z + right * velocity.X;

            // 현재 위치에 이동 적용
            Vector3 position = m_Rigidbody.Position;
            position += moveDir * Speed * ts;
            m_Rigidbody.Position = position;
        }

        bool isMoveForward()
        {
            if (Input.isKeyDown(KeyCode.W))
                return true;
            return false;
        }

        bool isMoveBackward()
        {
            if (Input.isKeyDown(KeyCode.S))
                return true;
            return false;
        }

        bool isMoveRight()
        {
            if (Input.isKeyDown(KeyCode.D))
                return true;
            return false;
        }

        bool isMoveLeft()
        {
            if (Input.isKeyDown(KeyCode.A))
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
