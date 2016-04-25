#include "Cube.h"



Cube::Cube(Transform* transform, ParticleModel* particleModel, Appearance* appearance) : _transform(transform), _particleModel(particleModel), _appearance(appearance)
{
}


Cube::~Cube()
{
}

void Cube::Update(float t)
{
	_transform->Update(t);
	_particleModel->UpdateCube(t);
}

void Cube::Draw(ID3D11DeviceContext * pImmediateContext)
{
	_appearance->Draw(pImmediateContext);
}