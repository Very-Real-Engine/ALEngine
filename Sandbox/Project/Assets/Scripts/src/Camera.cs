using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using ALEngine;

namespace Sandbox
{
    public class Camera : Entity
    {
        private TransformComponent m_Transform;

        public float MouseSensitivity = 0.01f;
        public float Speed;

        void onCreate()
        {
            Console.WriteLine($"Camera.OnCreate - {ID}");

            m_Transform = getComponent<TransformComponent>();
        }

        void onUpdate(float ts)
        {
            Input.UpdateMousePosition();
            // 우클릭 시 카메라 이동 멈춤
            if (!Input.isMouseRightPressed())
            {

                // 마우스 델타 값 가져오기
                Vector2 mouseDelta = Input.GetMouseDelta();
                
                // 마우스 감도 조절 상수
                
                // 현재 회전값 업데이트 (Y: yaw, X: pitch)
                Vector3 rotation = m_Transform.Rotation;
                rotation.Y -= mouseDelta.Y * MouseSensitivity;
                rotation.X -= mouseDelta.X * MouseSensitivity;
                
                // pitch 제한 (예: -90도 ~ 90도, 단위: 라디안)
                m_Transform.Rotation = rotation;
            }

            Vector3 velocity = Vector3.Zero;

            // 이동 입력 (카메라의 로컬 좌표 기준)
            if (Input.isKeyDown(KeyCode.W))
                velocity.Z = -1.0f; // 전진: 기본적으로 -Z가 전방이라고 가정
            else if (Input.isKeyDown(KeyCode.S))
                velocity.Z = 1.0f;  // 후진

            if (Input.isKeyDown(KeyCode.A))
                velocity.X = -1.0f; // 좌측 이동
            else if (Input.isKeyDown(KeyCode.D))
                velocity.X = 1.0f;  // 우측 이동

            // 카메라의 회전 정보를 쿼터니언으로 변환 (Quaternion.FromEulerAngles 유틸리티 함수 필요)
            Quaternion camRotation = Quaternion.FromEulerAngles(m_Transform.Rotation);

            // 카메라 로컬 좌표계의 forward, right 벡터 계산
            // (좌표계에 따라 forward 벡터가 (0, 0, -1)일 수 있습니다)
            Vector3 forward = Vector3Extensions.Transform(new Vector3(0, 0, 1), camRotation);
            Vector3 right   = Vector3Extensions.Transform(new Vector3(1, 0, 0), camRotation);

            // 카메라 기준 이동 벡터: forward * 전진/후진 + right * 좌/우
            Vector3 moveDir = forward * velocity.Z + right * velocity.X;

            // 현재 위치에 이동 적용
            Vector3 translation = m_Transform.Translation;
            translation += moveDir * Speed * ts;
            m_Transform.Translation = translation;
        }
    }

}
