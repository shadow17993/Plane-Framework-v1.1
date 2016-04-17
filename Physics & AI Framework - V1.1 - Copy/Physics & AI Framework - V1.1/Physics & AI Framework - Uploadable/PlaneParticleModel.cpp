#include "PlaneParticleModel.h"


PlaneParticleModel::PlaneParticleModel(Transform* transform, float mass) : ParticleModel(transform, mass)
{
	_thrust = { 0.0f, 0.0f, 0.0f };
	_engineSpeed = 0.0f;
	_wheelRadius = 10.0f;

	_engineSpeedLimit = 2.0f;

	_planeVelocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
}

PlaneParticleModel::~PlaneParticleModel()
{
}

void PlaneParticleModel::AddEngineSpeed(float engineSpeed)
{
	_thrust.z += engineSpeed;
	TruncateSpeed();
}

void PlaneParticleModel::TruncateSpeed()
{
	if (_thrust.z > _engineSpeedLimit)
	{
		_thrust.z = _engineSpeedLimit;
	}
}

void PlaneParticleModel::CalculateVelocity()
{
	_planeVelocity.x = _planeDirection.x * _thrust.x;
	_planeVelocity.y = _planeDirection.y * _thrust.y;
	_planeVelocity.z = _planeDirection.z * _thrust.z;
}

void PlaneParticleModel::Update(float t)
{
	//CalculateThrust(t);
	CalculateVelocity();

	// Update Particle Model
	ParticleModel::Update(t);
}