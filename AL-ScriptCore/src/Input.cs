namespace ALEngine
{
	// 이전 프레임의 마우스 위치
	public class Input
	{		
		private static Vector2 s_LastMousePosition = Vector2.Zero;
		// 현재 프레임의 마우스 위치
		private static Vector2 s_CurrentMousePosition = Vector2.Zero;
		// 계산된 마우스 이동량(델타)
		private static Vector2 s_MouseDelta = Vector2.Zero;

		/// <summary>
		/// 매 프레임 엔진에서 호출되어 현재 마우스 위치를 업데이트 합니다.
		/// 예를 들어, 플랫폼별 API로부터 마우스 좌표를 얻은 후 호출합니다.
		/// </summary>
		public static void UpdateMousePosition()
		{
			Vector2 currentPosition = getMousePos();

            if (!isMouseRightPressed())
            {
                s_LastMousePosition = currentPosition;
                s_CurrentMousePosition = currentPosition;
                s_MouseDelta = Vector2.Zero;
            }
            else
            {
                s_LastMousePosition = s_CurrentMousePosition;
                s_CurrentMousePosition = currentPosition;
                s_MouseDelta = s_CurrentMousePosition - s_LastMousePosition;
            }
		}

		/// <summary>
		/// 지난 프레임 대비 마우스 이동량(델타)를 반환합니다.
		/// </summary>
		public static Vector2 GetMouseDelta()
		{
			return s_MouseDelta;
		}

		public static bool isMouseLeftPressed()
		{
			return InternalCalls.Input_isMouseLeftPressed();
		}

		public static bool isMouseRightPressed()
		{
			return InternalCalls.Input_isMouseRightPressed();
		}

		public static bool isKeyDown(KeyCode keycode)
		{
			return InternalCalls.Input_isKeyDown(keycode);
		}

		public static Vector2 getMousePos()
		{
			InternalCalls.Input_getMousePos(out Vector2 pos);
			return pos;
		}

	}
}
