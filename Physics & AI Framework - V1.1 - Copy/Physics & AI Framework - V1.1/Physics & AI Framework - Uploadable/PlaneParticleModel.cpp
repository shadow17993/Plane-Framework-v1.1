#include "PlaneParticleModel.h"


PlaneParticleModel::PlaneParticleModel(Transform* transform, float mass) : ParticleModel(transform, mass)
{
	_thrust = 0.0f;
	_wingLift = 0.0f;
	_yawForce = 0.0f;

	_engineSpeed = 0.0f;

	_engineSpeedLimit = 3.0f;

	_planeVelocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
}

PlaneParticleModel::~PlaneParticleModel()
{
}

void PlaneParticleModel::AddEngineSpeed(float engineSpeed)
{
	_thrust += engineSpeed;
	TruncateSpeed();
}

void PlaneParticleModel::TruncateSpeed()
{
	if (_thrust > _engineSpeedLimit)
	{
		_thrust = _engineSpeedLimit;
	}
}

void PlaneParticleModel::CalculateVelocity()
{
	_planeVelocity.x = _planeDirection.x * ((_yawForce * 2) * _thrust);
	_planeVelocity.y = _planeDirection.y * (sin(_wingLift) * _thrust);
	_planeVelocity.z = _planeDirection.z * _thrust;
}

void PlaneParticleModel::Update(float t)
{
	//CalculateThrust(t);
	CalculateVelocity();

	// Update Particle Model
	ParticleModel::Update(t);
}