using System;
using System.Runtime.CompilerServices;

namespace ALEngine
{
    public class Entity
    {
        protected Entity() { ID = 0; }

        internal Entity(ulong id)
        {
            ID = id;
        }

        public readonly ulong ID;

        public Vector3 Position
        {
            get
            {
                InternalCalls.TransformComponent_getPosition(ID, out Vector3 result);
                return result;
            }
            set
            {
                InternalCalls.TransformComponent_setPosition(ID, ref value);
            }
        }

        public bool hasComponent<T>() where T : Component, new()
        {
            Type componentType = typeof(T);
            return InternalCalls.Entity_hasComponent(ID, componentType);
        }

        public T getComponent<T>() where T : Component, new()
        {
            if (!hasComponent<T>())
                return null;

            T component = new T() { Entity = this };
            return component;
        }

        public Entity findEntityByName(string name)
        {
            ulong entityID = InternalCalls.Entity_findEntityByName(name);
            if (entityID == 0)
                return null;

            return new Entity(entityID);
        }

        public Entity[] findEntitiesByComponent<T>() where T : Component, new()
        {
            ulong[] uuids = InternalCalls.Entity_findEntitiesByComponent(ID, typeof(T));

            // 반환된 UUID 배열을 기반으로 Entity 배열을 구성합니다.
            Entity[] entities = new Entity[uuids.Length];
            for (int i = 0; i < uuids.Length; i++)
            {
                entities[i] = new Entity(uuids[i]);
            }

            return entities;
        }

        public T As<T>() where T : Entity, new()
        {
            object instance = InternalCalls.getScriptInstance(ID);
            return instance as T;
        }

    }

}
