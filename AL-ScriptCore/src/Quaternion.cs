using System;

namespace ALEngine
{
    public struct Quaternion
    {
        public float X;
        public float Y;
        public float Z;
        public float W;

        public Quaternion(float x, float y, float z, float w)
        {
            W = w;
            X = x;
            Y = y;
            Z = z;
        }

        // 축(axis)과 각도(angle, 라디안)를 받아 Quaternion 생성
        public static Quaternion FromAxisAngle(Vector3 axis, float angle)
        {
            float halfAngle = angle * 0.5f;
            float sinHalf = (float)Math.Sin(halfAngle);
            float cosHalf = (float)Math.Cos(halfAngle);
            return new Quaternion(
                axis.X * sinHalf,
                axis.Y * sinHalf,
                axis.Z * sinHalf,
                cosHalf);
        }

        // Euler 각도 (라디안 단위, pitch=X, yaw=Y, roll=Z)를 Quaternion으로 변환합니다.
        public static Quaternion FromEulerAngles(Vector3 euler)
        {
            float halfPitch = euler.X * 0.5f;
            float halfYaw   = euler.Y * 0.5f;
            float halfRoll  = euler.Z * 0.5f;

            float sinPitch = (float)Math.Sin(halfPitch);
            float cosPitch = (float)Math.Cos(halfPitch);
            float sinYaw   = (float)Math.Sin(halfYaw);
            float cosYaw   = (float)Math.Cos(halfYaw);
            float sinRoll  = (float)Math.Sin(halfRoll);
            float cosRoll  = (float)Math.Cos(halfRoll);

            Quaternion q;
            q.W = cosYaw * cosPitch * cosRoll + sinYaw * sinPitch * sinRoll;
            q.X = sinYaw * cosPitch * cosRoll - cosYaw * sinPitch * sinRoll;
            q.Y = cosYaw * sinPitch * cosRoll + sinYaw * cosPitch * sinRoll;
            q.Z = cosYaw * cosPitch * sinRoll - sinYaw * sinPitch * cosRoll;
            return q;
        }

        // 두 Quaternion의 곱. 순서에 주의하세요.
        public static Quaternion operator *(Quaternion a, Quaternion b)
        {
            return new Quaternion(
                a.W * b.X + a.X * b.W + a.Y * b.Z - a.Z * b.Y,
                a.W * b.Y - a.X * b.Z + a.Y * b.W + a.Z * b.X,
                a.W * b.Z + a.X * b.Y - a.Y * b.X + a.Z * b.W,
                a.W * b.W - a.X * b.X - a.Y * b.Y - a.Z * b.Z);
        }

        // 단위 Quaternion의 경우 역수는 켤레와 동일합니다.
        public Quaternion Inverse()
        {
            return new Quaternion(-X, -Y, -Z, W);
        }

        // Quaternion을 이용하여 벡터를 회전시킵니다.
        // v' = q * v * q⁻¹, 여기서 v는 w=0인 Quaternion으로 취급됩니다.
        public Vector3 Rotate(Vector3 v)
        {
            Quaternion qv = new Quaternion(v.X, v.Y, v.Z, 0);
            Quaternion inv = Inverse();
            Quaternion result = this * qv * inv;
            return new Vector3(result.X, result.Y, result.Z);
        }
    }

    // Vector3에 Quaternion 회전을 적용하는 확장 메서드입니다.
    public static class Vector3Extensions
    {
        public static Vector3 Transform(this Vector3 v, Quaternion rotation)
        {
            return rotation.Rotate(v);
        }
    }
} //namespace ALEngine