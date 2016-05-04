#include "ParticleSystem.h"


ParticleSystem::ParticleSystem(Transform* transform, XMFLOAT3 offsetPos,  Appearance* particleApp) : _transform(transform), _offsetPos(offsetPos),  _particleApp(particleApp)
{
	if (transform->getParent() != nullptr)
	{
		_transform->SetPosition(_transform->getParent()->GetPosition().x + _offsetPos.x, 
								_transform->getParent()->GetPosition().y + _offsetPos.y, 
								_transform->getParent()->GetPosition().z + _offsetPos.z);
	}
}


ParticleSystem::~ParticleSystem()
{
}

void ParticleSystem::Update(float dt)
{
	if (_transform->getParent() != nullptr)
	{
		_transform->SetPosition(_transform->getParent()->GetPosition().x + _offsetPos.x,
			_transform->getParent()->GetPosition().y + _offsetPos.y,
			_transform->getParent()->GetPosition().z + _offsetPos.z);
	}
	_transform->Update(dt);
	
	if (particles.size() != 200)
	{
		Transform* t = new Transform(nullptr, XMFLOAT3{ _transform->GetPosition().x, _transform->GetPosition().y + 10, _transform->GetPosition().z }, { 0.0f, 0.0f, 0.0f }, { 0.5f, 0.5f, 0.5f });
		ParticleModel* pm = new ParticleModel
		(
			t,
			{ -2.0f + static_cast <float> (rand()) / static_cast <float> (RAND_MAX / (2.0 - -2.0)), 0.0f, -2.0f + static_cast <float> (rand()) / static_cast <float> (RAND_MAX / (2.0 - -2.0)) },
			1.0f, 0.0f
		);
			
		particles.push_back(new Particle(t, pm, _particleApp));
	}

	for (int i = 0; i < particles.size(); i++)
	{
		particles.at(i)->Update(dt);

		// Erases particle if energy is 0;
		if (particles.at(i)->getEnergy() == 0)
		{
			particles.at(i)->GetTransform()->SetPosition({ _transform->GetPosition().x, _transform->GetPosition().y + 10, _transform->GetPosition().z });
			particles.at(i)->GetParticleModel()->SetVelocity({ -2.0f + static_cast <float> (rand()) / static_cast <float> (RAND_MAX / (2.0 - -2.0)), 0.0f, -2.0f + static_cast <float> (rand()) / static_cast <float> (RAND_MAX / (2.0 - -2.0)) });
			particles.at(i)->setEnergy(rand() % 300);
		}
	}
}

void ParticleSystem::Draw(ID3D11DeviceContext * pImmediateContext)
{
	/*
	for (auto particle : particles)
	{
		particle->Draw(pImmediateContext);
	}*/
}