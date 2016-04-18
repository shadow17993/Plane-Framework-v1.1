#pragma once

#include "ParticleModel.h"

class PlaneParticleModel : public ParticleModel
{
public:
	PlaneParticleModel(Transform* transform, float mass);
	~PlaneParticleModel();

	void CalculateVelocity();

	void TruncateSpeed();

	void Update(float t);


	// --------------- Get/Set ------------------ //

	// Get/Set Car Direction
	XMFLOAT3 GetPlaneDirection() const { return _planeDirection; }
	void SetPlaneDirection(XMFLOAT3 planeDirection) { _planeDirection = planeDirection; }

	// Get/Set Car Velocity
	XMFLOAT3 GetPlaneVelocity() const { return _planeVelocity; }
	void SetEngineSpeed(XMFLOAT3 planeVelocity) { _planeVelocity = planeVelocity; }

	// Get/Set/Add Engine Speed
	float GetEngineSpeed() const { return _engineSpeed; }
	void SetEngineSpeed(float engineSpeed) { _engineSpeed = engineSpeed; }
	void AddEngineSpeed(float engineSpeed);

	// Get/Set/Add Engine Speed Limit
	float GetEngineSpeedLimit() const { return _engineSpeedLimit; }
	void SetEngineSpeedLimit(float engineSpeedLimit) { _engineSpeedLimit = engineSpeedLimit; }

	// Get/Set Thrust along Z Axis
	float GetThrust() const { return _thrust; }
	void SetThrust(float thrust) { _thrust = thrust; }

	// Get/Set Wing Lift along Y Axis
	float GetWingLift() const { return _wingLift; }
	void SetWingLift(float wingLift) { _wingLift = wingLift; }

	// Get/Set Yaw Force along X Axis
	float GetYawForce() const { return _yawForce; }
	void SetYawForce(float yawForce) { _yawForce = yawForce; }

private:

	float _engineSpeed;
	float _engineSpeedLimit;

	float _thrust;
	float _wingLift;
	float _yawForce;

	XMFLOAT3 _planeVelocity;
	XMFLOAT3 _planeDirection;
};