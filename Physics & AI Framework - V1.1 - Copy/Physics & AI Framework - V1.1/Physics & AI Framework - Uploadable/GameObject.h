#pragma once

#include <directxmath.h>
#include <d3d11_1.h>
#include <string>
#include <vector>

#include "Appearance.h"
#include "ParticleModel.h"
#include "ParticleSystem.h"

using namespace DirectX;
using namespace std;

class GameObject
{
public:
	GameObject(string type, Appearance* appearance, Transform* transform, ParticleModel* particleModel, bool isMoving);
	~GameObject();

	string GetType() const { return _type; };

	Appearance* GetAppearance() const { return _appearance; };
	Transform* GetTransform() const { return _transform; };
	ParticleModel* GetParticleModel() { return _particleModel; };

	void Update(float t);
	void Draw(ID3D11DeviceContext* pImmediateContext);

	void AddChild(ParticleSystem* child) { children.push_back(child); child->getTransform()->SetParent(_transform); };
	vector < ParticleSystem* > GetChildren() { return children; };

private:
	string _type;

	Appearance* _appearance;
	Transform* _transform;
	ParticleModel* _particleModel;

	bool _isMoving;
	vector< ParticleSystem* > children;
};