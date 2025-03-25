using System;
using System.Runtime.CompilerServices;

namespace ALEngine
{
	public static class InternalCalls
	{
		// internal
		// 멤버가 같은 어셈블리에서만 접근 가능.

		// MethodImplAttribute
		// .NET 런타임에 메서드 구현 세부 정보를 제공하는 특성.

		// MethodImplOptions.InternalCall
		// 해당 메서드가 런타임에 네이티브 구현임을 나타냄.

		#region Entity
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Entity_hasComponent(ulong entityID, Type componentType);
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static ulong Entity_findEntityByName(string name);
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static ulong[] Entity_findEntitiesByComponent(ulong entityID, Type componentType);
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static object getScriptInstance(ulong entityID);
		#endregion

		#region TransformComponent
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_getPosition(ulong entityID, out Vector3 translation);
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_setPosition(ulong entityID, ref Vector3 translation);
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_getRotation(ulong entityID, out Vector3 rotation);
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_setRotation(ulong entityID, ref Vector3 rotation);
		#endregion

		#region RigidbodyComponent
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void RigidbodyComponent_getPosition(ulong entityID, out Vector3 position);
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void RigidbodyComponent_setPosition(ulong entityID, ref Vector3 position);
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void RigidbodyComponent_getRotation(ulong entityID, out Quaternion rotation);
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void RigidbodyComponent_setRotation(ulong entityID, ref Quaternion rotation);
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void RigidbodyComponent_addForce(ulong entityID, ref Vector3 force);
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static int RigidbodyComponent_getTouchedNum(ulong entityID);
		#endregion

		#region ScriptComponent
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void ScriptComponent_getField(ulong entityID, string fieldName, out bool ret);
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void ScriptComponent_setField(ulong entityID, string fieldName, ref bool boolean);
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void ScriptComponent_activate(ulong entityID);
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void ScriptComponent_deactivate(ulong entityID);
		#endregion

		#region Animator
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Animator_getAnimations(ulong entityID, out string[] outAnimations);
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Animator_runAnimation(ulong entityID, int index);
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Animator_setRepeat(ulong entityID, ref bool repeat, int index);
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Animator_onInverse(ulong entityID);
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Animator_offInverse(ulong entityID);
		#endregion

		#region BoxCollider
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool BoxCollider_IsTriggered(ulong entityID, out string targetEntityName);
		#endregion

		#region Input
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Input_isKeyDown(KeyCode keycode);
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Input_isMouseLeftPressed();
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Input_isMouseRightPressed();
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Input_getMousePos(out Vector2 pos);
		#endregion
	}
}