#include "Plane.h"
#include <iostream>

Plane::Plane(GameObject* _planeBody, vector < GameObject* > _planeWheels)
{
	planeBody = _planeBody;
	planeWheels = _planeWheels;

	planePos = planeBody->GetTransform()->GetPosition();

	planeRotation = {0.0f, 0.0f, 0.0f};
	planeRotationSpeed = 0.001f;

	engineSpeedAdd = 0.01f;
}

Plane::~Plane()
{

}

void Plane::Input()
{
	PlaneParticleModel* planeBodyModel = (PlaneParticleModel*)planeBody->GetParticleModel();
	float engineSpeed = planeBodyModel->GetEngineSpeed();

	planePos = planeBody->GetTransform()->GetPosition();

	if (GetAsyncKeyState('B'))
	{
		planePos = XMFLOAT3(planePos.x, 20.0f, planePos.z);
		planeBody->GetTransform()->SetPosition(planePos);
	}

	if (GetAsyncKeyState('Z'))
	{
		// Set Car Engine Speed
		planeBodyModel->AddEngineSpeed((engineSpeedAdd));
	}
	else if (GetAsyncKeyState('C'))
	{
		if (engineSpeed > 0)
		{
			planeBodyModel->AddEngineSpeed(-engineSpeedAdd * 2.0f);
		}
		else
		{
			planeBodyModel->AddEngineSpeed((-engineSpeedAdd / 1.5f));
		}
	}
	else
	{
		float engineSpeed = planeBodyModel->GetEngineSpeed();

		if (engineSpeed > 0)
		{
			planeBodyModel->AddEngineSpeed(-engineSpeedAdd);

			if (engineSpeed < 0.0008f && engineSpeed != 0.0f)
			{
				planeBodyModel->SetEngineSpeed(0.0f);
			}
		}
		else if (engineSpeed < 0)
		{
			planeBodyModel->AddEngineSpeed(engineSpeedAdd);
		}
	}

	// Car Rotation Check
	if (GetAsyncKeyState('A'))
	{
		planeRotation.z += 0.001f;
		
		planeBodyModel->SetThrust(XMFLOAT3{
			planeBodyModel->GetThrust().x + planeRotation.z,
			planeBodyModel->GetThrust().y,
			planeBodyModel->GetThrust().z
		});
	}
	else if (GetAsyncKeyState('D'))
	{
		planeRotation.z -= 0.001f;

		planeBodyModel->SetThrust(XMFLOAT3{
			planeBodyModel->GetThrust().x + planeRotation.z,
			planeBodyModel->GetThrust().y,
			planeBodyModel->GetThrust().z
		});
	}
	else
	{
		if (planeRotation.z < 0)
		{
			planeRotation.z += 0.02f;
		}
		else if (planeRotation.z > 0)
		{

			planeRotation.z -= 0.02f;
		}

		if (
			planeRotation.z < 0.01f && planeRotation.z > -0.01f)
		{

			planeRotation.z = 0;
		}
	}


	if (GetAsyncKeyState('W'))
	{
		planeRotation.x += 0.001f;

		planeBodyModel->SetThrust(XMFLOAT3{
			planeBodyModel->GetThrust().x,
			planeBodyModel->GetThrust().y + planeRotation.x,
			planeBodyModel->GetThrust().z
		});
	}
	else if (GetAsyncKeyState('S'))
	{
		planeRotation.x -= 0.001f;

		planeBodyModel->SetThrust(XMFLOAT3{
			planeBodyModel->GetThrust().x,
			planeBodyModel->GetThrust().y + planeRotation.x,
			planeBodyModel->GetThrust().z
		});
	}
	else
	{
		if (planeRotation.x < 0)
		{
			planeRotation.x += 0.02f;
		}
		else if (planeRotation.x > 0)
		{

			planeRotation.x -= 0.02f;
		}

		if (
			planeRotation.x < 0.01f && planeRotation.x > -0.01f)
		{

			planeRotation.x = 0;
		}
	}
}

void Plane::CalculateForwardVector()
{
	planeBody->GetTransform()->GetRotation();

	planeForwardVector.x = sin((planeRotation.y / 17.425f) * (XM_PI / 180.0f));
	planeForwardVector.y = 0.0f;
	planeForwardVector.z = cos((planeRotation.x / 17.425f) * (XM_PI / 180.0f));

	float planeDirectionMag = sqrt((planeForwardVector.x * planeForwardVector.x) + (planeForwardVector.y * planeForwardVector.y) + (planeForwardVector.z * planeForwardVector.z));
	planeForwardVector = XMFLOAT3((planeForwardVector.x / planeDirectionMag), (planeForwardVector.y / planeDirectionMag), (planeForwardVector.z / planeDirectionMag));

	PlaneParticleModel* planeBodyModel = (PlaneParticleModel*)planeBody->GetParticleModel();
	planeBodyModel->SetPlaneDirection(planeForwardVector);

	// Reset Car Rotation if over 360 or 0 degrees
	if (planeRotation.x <= -6247.0f || planeRotation.x >= 6247.0f)
	{
		planeRotation.x = 0.0f;
	}

	if (planeRotation.y <= -6247.0f || planeRotation.y >= 6247.0f)
	{
		planeRotation.y = 0.0f;
	}

	if (planeRotation.z <= -6247.0f || planeRotation.z >= 6247.0f)
	{
		planeRotation.z = 0.0f;
	}
}

void Plane::Update(float t)
{
	CalculateForwardVector();

	PlaneParticleModel* planeBodyModel = (PlaneParticleModel*)planeBody->GetParticleModel();
	float engineSpeed = planeBodyModel->GetEngineSpeed();

	// Check Plane type --- you will want to add an AI controlled plane
	string type = planeBody->GetType();

	if (type == "Plane")
	{
		XMFLOAT3 velTemp = planeBody->GetParticleModel()->GetVelocity();
		XMFLOAT3 carVelTemp = planeBodyModel->GetPlaneVelocity();

		velTemp.x += carVelTemp.x;
		velTemp.y += carVelTemp.y;
		velTemp.z += carVelTemp.z;

		planeBodyModel->SetVelocity(velTemp);
	}
	else
	{
		planePos = planeBody->GetTransform()->GetPosition();
		XMFLOAT3 velTemp = planeBody->GetParticleModel()->GetVelocity();

		// AI positioning
		XMFLOAT3 planeVelocity = planeBodyModel->GetPlaneVelocity();
		planePos = XMFLOAT3((planePos.x + planeVelocity.x), (planePos.y + planeVelocity.y), (planePos.z + planeVelocity.z));
		planeBody->GetTransform()->SetPosition(planePos);

	}


	// Rotate Car in consideration to the Wheels Rotation
	if (engineSpeed > 0)
	{
		planeRotation.z += 100 * (engineSpeed * 20);
	}
	else if (engineSpeed < 0)
	{
		engineSpeed *= -1;
		planeRotation.z -= 100 * (engineSpeed * 20);
	}

	if (floorPos.y >= planePos.y)
	{
		planeBody->GetTransform()->SetPosition(planeBody->GetTransform()->GetPosition().x, 0.0f, planeBody->GetTransform()->GetPosition().z);
	}

	planeBody->GetTransform()->SetRotation(planeRotation.x, XMConvertToRadians(180.0f) + planeRotation.y * planeRotationSpeed, planeRotation.z);

	// Update Transform
	planeBody->Update(t);

	// Update Particle Model
	planeBody->GetParticleModel()->Update(t);
}

void Plane::Draw(ID3D11DeviceContext* _pImmediateContext)
{
	planeBody->Draw(_pImmediateContext);
}
