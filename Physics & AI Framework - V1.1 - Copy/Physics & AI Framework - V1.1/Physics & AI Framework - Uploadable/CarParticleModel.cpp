#include "CarParticleModel.h"


CarParticleModel::CarParticleModel(Transform* transform, float mass) : ParticleModel(transform, mass)
{
	_gearRatio = 10;
	_wheelSpeed = 1.0f;
	_engineSpeed = 0.0f;
	_wheelRadius = 10.0f;

	_engineSpeedLimit = 0.3f;

	_carVelocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
}

CarParticleModel::~CarParticleModel()
{
}

void CarParticleModel::AddEngineSpeed(float engineSpeed)
{
	_engineSpeed += engineSpeed;
	TruncateSpeed();
}

void CarParticleModel::CalculateWheelSpeed()
{
	TruncateSpeed();

	_wheelSpeed = _engineSpeed / _gearRatio;
}

void CarParticleModel::TruncateSpeed()
{
	if (_engineSpeed > _engineSpeedLimit)
	{
		_engineSpeed = _engineSpeedLimit;
	}
}

void CarParticleModel::CalculateThrust(float t)
{
	_thrust = ((_wheelSpeed * 60) * 2 * XM_PI * _wheelRadius) / t;
}

void CarParticleModel::CalculateVelocity()
{
	_carVelocity.x = _carDirection.x * _thrust;
	_carVelocity.y = _carDirection.y * _thrust;
	_carVelocity.z = _carDirection.z * _thrust;
}

void CarParticleModel::Update(float t)
{
	CalculateWheelSpeed();
	CalculateThrust(t);
	CalculateVelocity();

	// Update Particle Model
	ParticleModel::Update(t);
}