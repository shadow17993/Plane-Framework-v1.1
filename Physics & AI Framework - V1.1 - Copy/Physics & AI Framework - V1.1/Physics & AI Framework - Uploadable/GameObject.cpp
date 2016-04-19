#include "GameObject.h"

GameObject::GameObject(string type, Appearance* appearance, Transform* transform, ParticleModel* particleModel, bool isMoving)
{
	_type = type;

	_appearance = appearance;
	_particleModel = particleModel;
	_transform = transform;
	_isMoving = isMoving;
}

GameObject::~GameObject()
{
}

void GameObject::Update(float t)
{
	_transform->Update(t);

	if (_isMoving)
	{
		_particleModel->Update(t);
	}
}

void GameObject::Draw(ID3D11DeviceContext* pImmediateContext)
{
	_appearance->Draw(pImmediateContext);

	/*if (!children.empty())
	{
		for each(GameObject* child in children)
		{
			child->Draw(pImmediateContext);
		}
	}*/
}