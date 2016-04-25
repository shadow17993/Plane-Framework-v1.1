#include "Appearance.h"
#include "Transform.h"
#include "ParticleModel.h"

#pragma once

class Cube
{
public:
	Cube(Transform* transform, ParticleModel* particleModel, Appearance* appearance);
	~Cube();

	void Update(float t);
	void Draw(ID3D11DeviceContext * pImmediateContext);

	Transform * GetTransform() const { return _transform; }
	ParticleModel * GetParticleModel() const { return _particleModel; }
	Appearance * GetAppearance() const { return _appearance; }

private:
	Transform* _transform;
	ParticleModel* _particleModel;
	Appearance* _appearance;
};

