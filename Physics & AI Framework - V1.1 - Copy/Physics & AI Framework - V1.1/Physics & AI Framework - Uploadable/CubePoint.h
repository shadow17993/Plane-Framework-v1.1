#pragma once
#include "Transform.h"
#include "Appearance.h"
#include "ParticleModel.h"
#include "Cube.h"

class CubePoint
{
public:
	CubePoint(Appearance* appearance);
	~CubePoint();

	void Update(float t);
	void Draw(ID3D11DeviceContext * pImmediateContext);

	vector<Cube*> getCubes() { return _cubes; }

private:
	vector<Cube*> _cubes;
	Transform* _transform;
	Appearance* _particleApp;
};

