#pragma once

/**
 * @file AL.h
 * @brief 애플리케이션의 핵심 기능, 입력 처리, UI, ECS 관련 헤더를 포함하는 통합 헤더 파일.
 */

// App
#include "Core/App.h"
#include "Core/Assert.h"
#include "Core/Layer.h"
#include "Core/Log.h"

// Input
#include "Core/Input.h"
#include "Core/KeyCodes.h"
#include "Core/MouseCodes.h"

// ImGui
#include "ImGui/ImGuiLayer.h"

// ECS
#include "Scene/Component.h"
#include "Scene/Entity.h"
#include "Scene/Scene.h"
#include "Scene/ScriptableEntity.h"
