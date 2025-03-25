using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ALEngine
{
    public abstract class Component
    {
        public Entity Entity { get; internal set; }
    }

    public class TransformComponent : Component
    {
        public Vector3 Translation
        {
            get
            {
                InternalCalls.TransformComponent_getPosition(Entity.ID, out Vector3 translation);
                return translation;
            }
            set
            {
                InternalCalls.TransformComponent_setPosition(Entity.ID, ref value);
            }
        }

        public Vector3 Rotation
        {
            get
            {
                InternalCalls.TransformComponent_getRotation(Entity.ID, out Vector3 rotation);
                return rotation;
            }
            set
            {
                InternalCalls.TransformComponent_setRotation(Entity.ID, ref value);
            }
        }
    }

    public class RigidbodyComponent : Component
    {
        public Vector3 Position
        {
            get
            {
                InternalCalls.RigidbodyComponent_getPosition(Entity.ID, out Vector3 position);
                return position;
            }
            set
            {
                InternalCalls.RigidbodyComponent_setPosition(Entity.ID, ref value);
            }
        }

        public Quaternion Rotation
        {
            get
            {
                InternalCalls.RigidbodyComponent_getRotation(Entity.ID, out Quaternion rotation);
                return rotation;
            }
            set
            {
                InternalCalls.RigidbodyComponent_setRotation(Entity.ID, ref value);
            }
        }
        public void addForce(Vector3 force)
        {
            InternalCalls.RigidbodyComponent_addForce(Entity.ID, ref force);
        }

        public int getTouchNum()
        {
            return InternalCalls.RigidbodyComponent_getTouchedNum(Entity.ID);
        }

        // public 
    }

    public class ScriptComponent : Component
    {
        public bool getFieldValue(string fieldName)
        {
            InternalCalls.ScriptComponent_getField(Entity.ID, fieldName, out bool ret);
            return ret;
        }

        public void setFieldValue(string fieldName, bool boolean)
        {
            InternalCalls.ScriptComponent_setField(Entity.ID, fieldName, ref boolean);
        }

        public void activate()
        {
            InternalCalls.ScriptComponent_activate(Entity.ID);
        }

        public void deactivate()
        {
            InternalCalls.ScriptComponent_deactivate(Entity.ID);
        }
    }

    public class SkeletalAnimatorComponent : Component
    {
        public string[] getAnimations()
        {
            InternalCalls.Animator_getAnimations(Entity.ID, out string[] outAnimations);
            return outAnimations;
        }

        public void run(int index)
        {
            InternalCalls.Animator_runAnimation(Entity.ID, index);
        }

        public void setRepeat(bool repeat, int index)
        {
            InternalCalls.Animator_setRepeat(Entity.ID, ref repeat, index);
        }

        public void onInverse()
        {
            InternalCalls.Animator_onInverse(Entity.ID);
        }

        public void offInverse()
        {
            InternalCalls.Animator_offInverse(Entity.ID);
        }
    }

    public class BoxCollider : Component
    {
        public bool isTriggered(out string targetEntityName)
        {
            return InternalCalls.BoxCollider_IsTriggered(Entity.ID, out targetEntityName);
        }
    }
}
