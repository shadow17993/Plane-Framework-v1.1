#pragma once

#include <directxmath.h>
#include <d3d11_1.h>

#include "Transform.h"

using namespace DirectX;
using namespace std;

class ParticleModel
{
public:
	ParticleModel(Transform* transform, float mass);
	ParticleModel(Transform* transform, float mass, float radius);
	ParticleModel(Transform* transform, XMFLOAT3 velocity, float mass, float radius);
	ParticleModel(Transform* transform, XMFLOAT3 velocity, XMFLOAT3 acceleration);
	~ParticleModel();

	// --------------- Movement ----------------

	void Move(float amountX, float amountY, float amountZ);
	void MoveConstVel(float t);
	void MoveConstAcc(float t);
	
	// --------------- Rotation ----------------

	void SpinConstVel(float t);
	void SpinConstAcc(float t);

	// -------------- Drag Force ---------------- //

	void DragForce();
	void DragLamFlow();
	void MotionInFluid(float t);
	void TurbulantFlow();

	// --------------- Sliding ------------------ //

	void SlidingMotion();
	void SlidingForce(float theta, float frCoef);

	// ------------ Collision Check ------------- //

	void BaseCollisionCheck(XMFLOAT3 floorPos);
	bool CollisionCheck(XMFLOAT3 object2Pos, float object2Radius);
	void ResolveCollision(ParticleModel* object2);
	void ResolveSphereCollision(ParticleModel* object2);

	// --------------- Updates ------------------ //

	void UpdateNetForce();
	void UpdateAccel();
	void UpdateState();
	virtual void Update(float t);
	void UpdateSphere(float t);
	void UpdateParticle(float t);

	// --------------- Get/Set ------------------ //

	// Get/Set Collision Radius
	float GetCollisionRadius() const { return _collisionRadius; }
	void SetCollisionRadius(float collisionRadius) { _collisionRadius = collisionRadius; }

	// Get/Set Mass
	float GetMass() const { return _mass; };
	void SetMass(float mass) { _mass = mass; }

	// Get/Set Velocity
	XMFLOAT3 GetVelocity() const { return _velocity; }
	void SetVelocity(XMFLOAT3 velocity) { _velocity = velocity; }

	void setSpinAccel(XMFLOAT3 acceleration) { _spinAccel = acceleration; }
	void setSpinAccel(float x, float y, float z) { _spinAccel = { x, y, z }; }
	XMFLOAT3 getSpinAccel() { return _spinAccel; }

	void setSpinVel(XMFLOAT3 velocity) { _spinVelocity = velocity; }
	void setSpinVel(float x, float y, float z) { _spinVelocity = { x, y, z }; }
	XMFLOAT3 getSpinVel() { return _spinVelocity; }

	void SetTheta(float theta) { _theta = theta; }

protected:
	Transform* _transform;

private:
	// Movement
	XMFLOAT3 _acceleration;
	XMFLOAT3 _velocity;

	XMFLOAT3 _constAccel;
	

	bool _useConstAcc;

	// Rotation
	XMFLOAT3 _spinAccel;
	XMFLOAT3 _spinVelocity;

	// Forces
	XMFLOAT3 _netForce;
	float _forceMag;

	// Drag Force
	XMFLOAT3 _dragForce;
	float _dragFactor;
	bool _laminar;

	// Sliding
	XMFLOAT3 _slidingForce;
	bool _slidingOn;

	// Collision Check
	float _collisionRadius;

	// Other
	static const float _gravity;
	float _mass;
	float _weight;

	float _theta;
};