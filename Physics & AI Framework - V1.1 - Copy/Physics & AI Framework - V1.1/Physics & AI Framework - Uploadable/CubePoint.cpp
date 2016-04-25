#include "CubePoint.h"



CubePoint::CubePoint( Appearance* appearance) : _particleApp(appearance)
{
}


CubePoint::~CubePoint()
{
}

void CubePoint::Update(float t)
{
	if (_cubes.size() != 6)
	{
		Transform* t = new Transform(
			nullptr, 
			XMFLOAT3{ -1000.0f + static_cast <float> (rand()) / static_cast <float> (RAND_MAX / (1000.0 - -1000.0)), 100.0f, -1000.0f + static_cast <float> (rand()) / static_cast <float> (RAND_MAX / (1000.0 - -1000.0)) }, { 0.0f, 0.0f, 0.0f }, { 10.0f, 10.0f, 10.0f });
		ParticleModel* pm = new ParticleModel(t, { 0.0f, 0.5f, 0.0f }, { 0.0f, 0.0f, 0.0f }, 20.0f);

		_cubes.push_back(new Cube(t, pm, _particleApp));
	}

	for (int i = 0; i < _cubes.size(); i++)
	{
		_cubes.at(i)->Update(t);

		// Erases particle if energy is 0;
		if (_cubes.at(i)->GetParticleModel()->getSpinVel().y > 2.0f)
		{
			_cubes.erase(_cubes.begin() + i);
		}
	}
}

void CubePoint::Draw(ID3D11DeviceContext * pImmediateContext)
{
	for (auto c : getCubes())
	{
		c->Draw(pImmediateContext);
	}
}

