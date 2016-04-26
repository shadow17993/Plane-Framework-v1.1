#pragma once

#include "PlaneParticleModel.h"
#include "GameObject.h"

class Plane
{
public:
	Plane(GameObject* _planeBody);
	~Plane();

	virtual void Update(float t);
	void Draw(ID3D11DeviceContext* _pImmediateContext);

	// Plane Input
	void Input();

	// Get Plane Body and Wheels
	GameObject* GetPlaneBody() const { return planeBody; };

	// Get/Set Plane Position
	XMFLOAT3 GetPlanePosition() const { return planePos; };
	void SetPlanePosition(XMFLOAT3 _planePos) { planePos = _planePos; };
	void SetPlanePosition(float x, float y, float z) { planePos.x = x, planePos.y = y, planePos.z = z; };

	// Get/Calculate Plane Forward Vector
	XMFLOAT3 GetForwardVector() const { return planeForwardVector; };
	void CalculateForwardVector();

	// Get/Set Plane Rotation
	XMFLOAT3 GetPlaneRotation() const { return planeRotation; };

	// Get/Set Plane Rotation Speed
//	float GetPlaneRotationSpeed() const { return planeRotationSpeed; };
//	void SetPlaneRotationSpeed(float _planeRotationSpeed) { planeRotationSpeed = planeRotationSpeed; };


private:
	// Plane Object
	GameObject* planeBody;

	// Plane Properties
	XMFLOAT3 planePos;
	XMFLOAT3 planeForwardVector;

	XMFLOAT3 planeRotation;
	float planeRotationSpeed;

	float engineSpeedAdd;
	XMFLOAT3 floorPos = { 0.0f, 0.0f, 0.0f };
};