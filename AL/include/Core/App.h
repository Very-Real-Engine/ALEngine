#pragma once

#include "Core/Base.h"
#include "Core/LayerStack.h"
#include "Core/Timestep.h"
#include "Core/Window.h"

#include "ImGui/ImGuiLayer.h"

#include "Events/AppEvent.h"
#include "Events/Event.h"

#include "Renderer/Common.h"
#include "Renderer/Renderer.h"

namespace ale
{
struct ApplicationCommandLineArgs
{
	int count = 0;
	char **args = nullptr;

	const char *operator[](int index) const
	{
		return args[index];
	}
};

struct ApplicationSpecification
{
	std::string m_Name = "ALEngine";
	std::string m_WorkingDirectory;
	ApplicationCommandLineArgs m_CommandLineArgs;
};

/**
 * @brief Window, Renderer, Layer를 관리하는 App 클래스.
 * @details 게임 엔진의 생명주기를 관리하는 클래스. init -> run -> delete 순서로 돌아간다.
 * 
 */

class App
{
  public:
	/**
	 * @brief App 생성자
	 * 
	 * @param spec 
	 */
	App(const ApplicationSpecification &spec);
	
	/**
	 * @brief App 소멸자
	 * 
	 */
	virtual ~App();

	/**
	 * @brief app 실행
	 * @details 게임 엔진의 main loop. 정의한 layer를 순회하며 update 한다.
	 * 
	 */
	void run();

	/**
	 * @brief App event dispatch
	 * @details WindowCloseEvent, WindowResizeEvent dispatch, layer들의 onEvent 실행
	 * 
	 * @param e Event
	 */
	void onEvent(Event &e);
	
	/// @brief App 관리 자원 정리 
	void cleanup();

	/// @brief close App
	void close();

	/**
	 * @brief LayerStack pushLayer
	 * 
	 * @param layer 
	 */
	void pushLayer(Layer *layer);

	/**
	 * @brief LayerStack pushOverlay
	 * 
	 * @param layer 
	 */
	void pushOverlay(Layer *layer);

	/// @brief Window getter
	Window &getWindow()
	{
		return *m_Window;
	}

	/// @brief Renderer getter
	Renderer &getRenderer()
	{
		return *m_Renderer;
	}

	/// @brief Specification getter
	const ApplicationSpecification &getSpecification() const
	{
		return m_Spec;
	}

	/**
	 * @brief Get the App object
	 * 
	 * @return App& 
	 */
	static App &get();

  private:

	/**
	 * @brief Window close
	 * 
	 */
	bool onWindowClose(WindowCloseEvent &e);
	
	/**
	 * @brief ESC 눌렀을 때, Window close
	 * 
	 */
	bool onWindowClose(KeyPressedEvent &e);

	/**
	 * @brief Window resize
	 * 
	 */
	bool onWindowResize(WindowResizeEvent &e);

	/// @brief App이 관리하는 Window
	std::unique_ptr<Window> m_Window;
	/// @brief App이 관리하는 Renderer
	std::unique_ptr<Renderer> m_Renderer;
	/// @brief App이 관리하는 ImGuiLayer
	ImGuiLayer *m_ImGuiLayer;
	/// @brief Layer를 담고 있는 자료구조
	LayerStack m_LayerStack;
	/// @brief App 정보
	ApplicationSpecification m_Spec;
	/// @brief 마지막 프레임 시간
	Chrono::TimePoint m_LastFrameTime;
	/// @brief App 실행 여부
	bool m_Running = true;
	/// @brief 창 최소화 여부
	bool m_Minimized = false;

  private:
	/// @brief App static 객체
	static App *s_Instance;
};

// To be defined in CLIENT
App *createApp(ApplicationCommandLineArgs args);

} // namespace ale