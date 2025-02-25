#include "Scene/SceneCamera.h"
#include "Alpch.h"

namespace ale
{

void SceneCamera::updateSceneCamera(alglm::vec3 &pos, alglm::vec3 &rot)
{
	setPosition(pos);
	setRotation(rot);
	updateViewMatrix();
}

} // namespace ale