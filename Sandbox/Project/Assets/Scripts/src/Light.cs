using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using ALEngine;

namespace Sandbox
{
    public class Light : Entity
    {
		private ScriptComponent m_TargetScript;
		private ScriptComponent m_Script;

		private Entity m_Target;

		private bool m_IsChanged;

		public string targetEntity = "Button";
		public string targetField = "isOn";
		public bool isOn;
		void onCreate()
		{
			Console.WriteLine($"Light.OnCreate - {ID}");
			m_IsChanged = false;
			isOn = false;

			if (!System.String.IsNullOrEmpty(targetEntity))
			{
				m_Target = findEntityByName(targetEntity);
				m_TargetScript = m_Target.getComponent<ScriptComponent>();
			}
			m_Script = getComponent<ScriptComponent>();
			m_Script.deactivate();
		}

		void onUpdate(float ts)
		{
			if (m_TargetScript != null)
			{
				bool result = m_TargetScript.getFieldValue(targetField);
				if (result != isOn)
				{
					isOn = result;
					m_IsChanged = true;
				}
			}

			if (m_IsChanged)
			{
				if (isOn)
					m_Script.activate();
				else
					m_Script.deactivate();
				m_IsChanged = false;
			}
		}
	}
}
