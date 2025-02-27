#include "Renderer/SAComponent.h"

namespace ale
{

SAComponent::SAComponent()
{
	init();
}

SAComponent::SAComponent(std::shared_ptr<Model>& model)
{
	init();

	m_Model = model;
	if (m_Model->m_SkeletalAnimations)
	{
		m_Skeleton = model->getSkeleton();
		m_Animations = model->getAnimations();

		initKeyFrame();

		m_Repeats.resize(m_Animations->size());
		for (size_t i = 0; i < m_Animations->size(); ++i)
			m_Repeats[i] = false;

		//init state-animation
		if (m_StateManager->getStates().size() == 0)
			initStateManager();

		if (m_Animations->size() > 0)
		{
			start(0);
		}
	}
}

void SAComponent::init()
{
	m_CurrentAnimation = nullptr;
	m_StateManager = std::make_shared<AnimationStateManager>();
}

void SAComponent::initKeyFrame()
{
	if (m_Animations)
	{
		size_t numberOfAnimations = m_Animations->size();
		m_Data.resize(numberOfAnimations);

		for (unsigned int i = 0; i < numberOfAnimations; ++i)
		{
			m_Data[i].m_CurrentKeyFrameTime = 0.0f;
			m_Data[i].m_FirstKeyFrameTime = 0.0f;
			m_Data[i].m_LastKeyFrameTime = 0.0f;
			m_Data[i].m_Inverse = false;
		}
	}
}

void SAComponent::initAnimation(SkeletalAnimation* animation)
{
	int animIndex = getAnimIndex(animation->getName());

	m_CurrentAnimation = animation;
	m_Data[animIndex] = animation->getData();
	m_FrameCounter = m_Animations->getCurrentFrame();

	AnimationState* state = m_StateManager->getStateFromAnimName(animation->getName());
	if (state)
	{
		m_StateManager->currentState = *state;
	}
}

void SAComponent::initStateManager()
{
	m_StateManager->addState({"All", "All", false, false, 0.5f});
	for (size_t animationIndex = 0; animationIndex < m_Animations->size(); ++animationIndex)
	{
		std::string name = (*m_Animations)[animationIndex].getName();
		m_StateManager->addState({name, name, false, false, 0.5f});
	}
}

void SAComponent::setModel(std::shared_ptr<Model> &model)
{
	m_Model = model;
	if (m_Model->m_SkeletalAnimations)
	{
		m_Skeleton = model->getSkeleton();
		m_Animations = model->getAnimations();

		initKeyFrame();

		m_Repeats.resize(m_Animations->size());
		for (size_t i = 0; i < m_Animations->size(); ++i)
			m_Repeats[i] = false;

		if (m_StateManager->getStates().size() == 0)
			initStateManager();

		if (m_Animations->size() > 0)
		{
			start(0);
		}
	}
}

void SAComponent::start(std::string const &animation)
{
	m_Animations->start(animation);
	initAnimation(&(*m_Animations)[animation]);
}

void SAComponent::start(size_t index)
{
	m_Animations->start(index);
	initAnimation(&(*m_Animations)[index]);
}

void SAComponent::start()
{
	if (m_CurrentAnimation)
		start(m_CurrentAnimation->getName());
	else
		start(0);
}

void SAComponent::stop()
{
	if (m_CurrentAnimation)
		m_CurrentAnimation->stop();
}

void SAComponent::setRepeat(bool repeat, int index)
{
	if (m_CurrentAnimation && index == -1)
	{
		int index = m_Animations->getIndex(m_CurrentAnimation->getName());
		if (index != -1)
			m_Repeats[index] = repeat;
	}
	else
	{
		m_Repeats[index] = repeat;
	}
}

void SAComponent::setRepeatAll(bool repeat)
{
	for (size_t i = 0; i < m_Repeats.size(); ++i)
		m_Repeats[i] = repeat;
}

void SAComponent::setCurrentRepeat(bool repeat)
{
	if (m_CurrentAnimation)
	{
		m_Repeats[getAnimIndex(m_CurrentAnimation->getName())] = repeat;
	}
}

void SAComponent::setInverse(bool inverse, int index)
{
	m_Data[index].m_Inverse = inverse;
}

void SAComponent::setCurrentAnimation(SkeletalAnimation *animation)
{
	if (animation)
		start(animation->getName());
}

void SAComponent::updateAnimation(const Timestep& timestep, uint32_t currentFrame)
{
	if (m_StateManager->inTransition) // BLENDING-ANIMATION (2)
	{
		if (m_StateManager->currentState.animationName != m_CurrentAnimation->getName())
		{
			auto &prevAnim = (*m_Animations)[m_StateManager->prevState.animationName];
			// if (!getAnimRepeat(&prevAnim))
				// prevAnim.start();
			setData(m_FrameCounter, prevAnim.getData(), getAnimIndex(prevAnim.getName()));
			
			m_CurrentAnimation = &(*m_Animations)[m_StateManager->currentState.animationName];
			if (!getAnimRepeat(m_CurrentAnimation))
				m_CurrentAnimation->start();
			setData(m_FrameCounter, m_CurrentAnimation->getData(), getAnimIndex(m_CurrentAnimation->getName()));
		}
		blendUpdate(timestep, *m_Skeleton, currentFrame);
	}
	else if (m_StateManager->isTransitionFinish) // AFTER BLENDING CHANGED TO CURRENT-ANIMATION
	{
		m_StateManager->isTransitionFinish = false;
		m_CurrentAnimation = &(*m_Animations)[m_StateManager->currentState.animationName];
	}
	else if (m_CurrentAnimation && m_Model->m_SkeletalAnimations) // SINGLE-ANIMATION
	{
		int animIndex = getAnimIndex(m_CurrentAnimation->getName());
		if (animIndex == -1) return ;

		m_CurrentAnimation->uploadData(this->getData(animIndex),
			m_Repeats[animIndex]);
		m_Animations->uploadData(m_CurrentAnimation, m_FrameCounter);
		m_Animations->update(timestep, *m_Skeleton, currentFrame);
		m_Skeleton->update();

		m_CurrentPose = m_Skeleton->m_ShaderData.m_FinalBonesMatrices;
		flush();
		this->setData(m_Animations->getCurrentFrame() ,m_CurrentAnimation->getData(), animIndex);
	}
	m_StateManager->update(timestep);
}

void SAComponent::updateAnimationWithoutTransition(const Timestep& timestep)
{
	if (m_CurrentAnimation && m_Model->m_SkeletalAnimations)
	{
		int animIndex = getAnimIndex(m_CurrentAnimation->getName());
		if (animIndex == -1) return;
		m_CurrentAnimation->uploadData(this->getData(animIndex),
			m_Repeats[animIndex]);
		m_Animations->uploadData(m_CurrentAnimation, m_FrameCounter);
		m_Animations->update(timestep, *m_Skeleton, 0); // disable currentframe
		m_Skeleton->update();
		
		m_CurrentPose = m_Skeleton->m_ShaderData.m_FinalBonesMatrices;
		flush();
		this->setData(m_Animations->getCurrentFrame() ,m_CurrentAnimation->getData(), animIndex);
	}
}

void SAComponent::blendUpdate(
	const Timestep& timestep,
	Armature::Skeleton& skeleton,
	uint32_t currentFrame
)
{
	auto& animA = (*m_Animations)[m_StateManager->prevState.animationName];
	auto& animB = (*m_Animations)[m_StateManager->currentState.animationName];

	int animAIndex = getAnimIndex(animA.getName());
	int animBIndex = getAnimIndex(animB.getName());

	if (animAIndex == -1 || animBIndex == -1) return;

	animA.uploadData(getData(animAIndex), getAnimRepeat(&animA));
	animB.uploadData(getData(animBIndex), getAnimRepeat(&animB));

	float blendFactor = std::min(m_StateManager->transitionTime / m_StateManager->transitionDuration, 1.0f);
	Bones poseFrom, poseTo;

	if (animA.isRunning())
	{
		m_Animations->uploadData(&animA, m_FrameCounter);
		m_Animations->update(timestep, *m_Skeleton, currentFrame);
		poseFrom = m_Skeleton->m_Bones;
		this->setData(m_FrameCounter, animA.getData(), animAIndex); // prevState 애니메이션이 기존 애니메이션이므로 기존 애니메이션의 키프레임 데이터를 유지
	}
	else
	{
		if (m_StateManager->transitionTime == 0.0f)
		{
			m_Animations->uploadData(&animA, m_FrameCounter);
			m_Animations->update(timestep, *m_Skeleton, currentFrame);
			m_CapturedPose = m_Skeleton->m_Bones;
			this->setData(m_FrameCounter, animA.getData(), animAIndex); // prevState 애니메이션이 기존 애니메이션이므로 기존 애니메이션의 키프레임 데이터를 유지
		}
		poseFrom = m_CapturedPose;
	}

	m_Animations->uploadData(&animB, m_FrameCounter);
	m_Animations->update(timestep, *m_Skeleton, currentFrame);
	poseTo = m_Skeleton->m_Bones;
	this->setData(currentFrame, animB.getData(), animBIndex);

	m_Skeleton->m_Bones = blendBones(poseTo, poseFrom, blendFactor);
	m_Skeleton->update();

	m_CurrentPose = m_Skeleton->m_ShaderData.m_FinalBonesMatrices;
	flush();
}

SAComponent::Bones SAComponent::blendBones(Bones &to, Bones &from, float blendFactor)
{
	if (to.size() != from.size())
	{
		return to;
	}

	size_t numberOfBones = to.size();

	for (size_t boneIndex = 0; boneIndex < numberOfBones; ++boneIndex)
	{
		to[boneIndex].m_DeformedNodeTranslation =
			alglm::mix(from[boneIndex].m_DeformedNodeTranslation, to[boneIndex].m_DeformedNodeTranslation, blendFactor);
		to[boneIndex].m_DeformedNodeRotation = alglm::normalize(
			alglm::slerp(from[boneIndex].m_DeformedNodeRotation, to[boneIndex].m_DeformedNodeRotation, blendFactor));
		to[boneIndex].m_DeformedNodeScale =
			alglm::mix(from[boneIndex].m_DeformedNodeScale, to[boneIndex].m_DeformedNodeScale, blendFactor);
	}

	return to;
}

struct SAData SAComponent::getData(unsigned int index) const
{
	return m_Data[index];
}

void SAComponent::setData(uint16_t currentFrame, const SAData &data, unsigned int index)
{
	m_FrameCounter = currentFrame; //??

	m_Data[index].m_FirstKeyFrameTime = data.m_FirstKeyFrameTime;
	m_Data[index].m_LastKeyFrameTime = data.m_LastKeyFrameTime;
	m_Data[index].m_CurrentKeyFrameTime = data.m_CurrentKeyFrameTime;
	m_Data[index].m_Inverse = data.m_Inverse;
}

void SAComponent::setDataAll(std::vector<SAData> datas)
{
	m_Data = datas;
}

int SAComponent::getAnimIndex(const std::string &name)
{
	return m_Animations->getIndex(name);
}

bool SAComponent::getAnimRepeat(SkeletalAnimation *animation)
{
	return m_Repeats[getAnimIndex(animation->getName())];
}

bool SAComponent::getRepeat(int index)
{
	if (index == -1)
		return getAnimRepeat(m_CurrentAnimation);
	return getAnimRepeat(&(*m_Animations)[index]);
}

float SAComponent::getDuration()
{
	if (m_CurrentAnimation)
		return m_CurrentAnimation->getDuration();
	return NON_CURRENT_ANIMATION_FLOAT;
}

float SAComponent::getCurrentTime()
{
	if (m_CurrentAnimation)
		return m_CurrentAnimation->getCurrentTime();
	return NON_CURRENT_ANIMATION_FLOAT;
}

std::string SAComponent::getCurrentAnimationName()
{
	if (m_CurrentAnimation)
		return m_CurrentAnimation->getName();
	return NON_CURRENT_ANIMATION_STRING;
}

bool SAComponent::isRunning() const
{
	if (m_CurrentAnimation)
		return m_CurrentAnimation->isRunning();
	return NON_CURRENT_ANIMATION_BOOL;
}

bool SAComponent::willExpire(const Timestep &timestep) const
{
	if (m_CurrentAnimation)
		return m_CurrentAnimation->willExpire(timestep);
	return NON_CURRENT_ANIMATION_BOOL;
}

int SAComponent::getCurrentAnimationIndex()
{
	if (m_CurrentAnimation)
		return getAnimIndex(m_CurrentAnimation->getName());
	return NON_CURRENT_ANIMATION_INT;
}

bool SAComponent::getInverse(int index)
{
	return m_Data[index].m_Inverse;
}

void SAComponent::flush()
{
	auto& bones = m_Skeleton->m_ShaderData.m_FinalBonesMatrices;
	for (auto b : bones)
		b = alglm::mat4(1.0f);
}

} //namespace ale
