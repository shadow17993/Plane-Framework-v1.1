#include "Application.h"
#include "CubePoint.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

bool Application::HandleKeyboard(MSG msg)
{
	XMFLOAT3 cameraPosition = _camera->GetPosition();

	switch (msg.wParam)
	{
	case VK_UP:
		_cameraOrbitRadius = max(_cameraOrbitRadiusMin, _cameraOrbitRadius - (_cameraSpeed * 0.2f));
		return true;
		break;

	case VK_DOWN:
		_cameraOrbitRadius = min(_cameraOrbitRadiusMax, _cameraOrbitRadius + (_cameraSpeed * 0.2f));
		return true;
		break;

	case VK_RIGHT:
		_cameraOrbitAngleXZ -= _cameraSpeed;
		return true;
		break;

	case VK_LEFT:
		_cameraOrbitAngleXZ += _cameraSpeed;
		return true;
		break;
	}

	return false;
}

Application::Application()
{
	_hInst = nullptr;
	_hWnd = nullptr;
	_driverType = D3D_DRIVER_TYPE_NULL;
	_featureLevel = D3D_FEATURE_LEVEL_11_0;
	_pd3dDevice = nullptr;
	_pImmediateContext = nullptr;
	_pSwapChain = nullptr;
	_pRenderTargetView = nullptr;
	_pVertexShader = nullptr;
	_pPixelShader = nullptr;
	_pVertexLayout = nullptr;
	_pVertexBuffer = nullptr;
	_pIndexBuffer = nullptr;
	_pConstantBuffer = nullptr;

	DSLessEqual = nullptr;
	RSCullNone = nullptr;
}

Application::~Application()
{
	Cleanup();
}

// ----------------- Initialisation ------------------ //

HRESULT Application::Initialise(HINSTANCE hInstance, int nCmdShow)
{
    if (FAILED(InitWindow(hInstance, nCmdShow)))
	{
        return E_FAIL;
	}

    RECT rc;
    GetClientRect(_hWnd, &rc);
    _WindowWidth = rc.right - rc.left;
    _WindowHeight = rc.bottom - rc.top;

    if (FAILED(InitDevice()))
    {
        Cleanup();

        return E_FAIL;
    }

	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\stone.dds", nullptr, &_pTextureRV);
	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\CFAPlaneTexture.dds", nullptr, &_pAIPlaneTex);
	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\Hercules_COLOR.dds", nullptr, &_pPlaneTex);
	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\carTex.dds", nullptr, &_pCarTex);

	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\track01.dds", nullptr, &_pRaceTrackTex);
	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\grass.dds", nullptr, &_pGrassTex);
	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\mountain.dds", nullptr, &_pMountainTex);
	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\floor.dds", nullptr, &_pStoneTex);
	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\skyBox3.dds", nullptr, &_pSkyTex);

	// Setup Camera
	XMFLOAT3 eye = XMFLOAT3(0.0f, 2.0f, -1.0f);
	XMFLOAT3 at = XMFLOAT3(0.0f, 2.0f, 0.0f);
	XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);

	_camera = new Camera(eye, at, up, (float)_renderWidth, (float)_renderHeight, 0.01f, 100.0f);

	InitObjects();

	return S_OK;
}

void Application::InitObjects()
{
	// Setup the scene's light
	basicLight.AmbientLight = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	basicLight.DiffuseLight = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	basicLight.SpecularLight = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	basicLight.SpecularPower = 20.0f;
	basicLight.LightVecW = XMFLOAT3(0.0f, 1.0f, -1.0f);

	// SkyBox Material
	Material skyBoxMat;
	skyBoxMat.ambient = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	skyBoxMat.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	skyBoxMat.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	skyBoxMat.specularPower = 10.0f;

	// Shiny Material
	shinyMaterial.ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	shinyMaterial.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	shinyMaterial.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	shinyMaterial.specularPower = 10.0f;

	// No Specular Material
	noSpecMaterial.ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	noSpecMaterial.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	noSpecMaterial.specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	noSpecMaterial.specularPower = 0.0f;

	// Appearance, Transform, ParticleModel
	Appearance* appearance;
	Transform* transform;
	ParticleModel* particleModel;

	// Ground Plane Initialisation
	Geometry groundGeometry = OBJLoader::Load("Objects/groundPlane.obj", _pd3dDevice);

	appearance = new Appearance(groundGeometry, shinyMaterial);
	appearance->SetTextureRV(_pGrassTex);

	transform = new Transform();
	transform->SetPosition(0.0f, -1.0f, 0.0f);
	transform->SetScale(0.5f, 0.5f, 0.5f);
	transform->SetRotation(XMConvertToRadians(0.0f), 0.0f, 0.0f);

	particleModel = new PlaneParticleModel(transform, 1.0f);

	groundPlane = new GameObject("Ground Plane", appearance, transform, particleModel, false);

	// Mountains Initialisation
	Geometry mountainGeometry = OBJLoader::Load("Objects/mountain.obj", _pd3dDevice);

	appearance = new Appearance(mountainGeometry, shinyMaterial);
	appearance->SetTextureRV(_pMountainTex);

	transform = new Transform();
	transform->SetPosition(0.0f, 0.0f, 0.0f);
	transform->SetScale(0.2f, 0.2f, 0.2f);
	transform->SetRotation(XMConvertToRadians(0.0f), 0.0f, 0.0f);

	particleModel = new PlaneParticleModel(transform, 1.0f);

	mountain = new GameObject("Mountain", appearance, transform, particleModel, false);

	// Race Track Initialisaton
	Geometry raceTrackGeometry = OBJLoader::Load("Objects/raceTrack.obj", _pd3dDevice);

	appearance = new Appearance(raceTrackGeometry, shinyMaterial);
	appearance->SetTextureRV(_pRaceTrackTex);

	transform = new Transform();
	transform->SetPosition(0.0f, 0.5f, 0.0f);
	transform->SetScale(1.0f, 1.0f, 1.0f);
	transform->SetRotation(XMConvertToRadians(0.0f), 0.0f, 0.0f);

	particleModel = new PlaneParticleModel(transform, 1.0f);

	raceTrack = new GameObject("Race Track", appearance, transform, particleModel, false);


	// Sky Box Initialisation
	Geometry skyBoxGeometry = OBJLoader::Load("Objects/sphere.obj", _pd3dDevice);

	appearance = new Appearance(skyBoxGeometry, skyBoxMat);
	appearance->SetTextureRV(_pSkyTex);

	transform = new Transform();
	transform->SetPosition(0.0f, 0.0f, 0.0f);
	transform->SetScale(-10000.0f, -10000.0f, -10000.0f);
	transform->SetRotation(XMConvertToRadians(0.0f), 0.0f, 0.0f);

	particleModel = new PlaneParticleModel(transform, 1000.0f);

	skyBox = new GameObject("Sky Box", appearance, transform, particleModel, false);


	// Init Plane collection Objects
	InitPlaneObjects();

	XMFLOAT3 planePos = myPlane->GetPlaneBody()->GetTransform()->GetPosition();


	// Sphere Initialisation
	sphereGeometry = OBJLoader::Load("Objects/sphere.obj", _pd3dDevice);

	Appearance* sphereAppearance = new Appearance(sphereGeometry, shinyMaterial);
	appearance->SetTextureRV(_pTextureRV);

	transform = new Transform();
	transform->SetPosition(planePos.x + 100, 30.0f, planePos.z + 50.0f);
	transform->SetScale(10.0f, 10.0f, 10.0f);
	transform->SetRotation(0.0f, 0.0f, 0.0f);

	particleModel = new ParticleModel(transform, XMFLOAT3{0.0f, 0.0f, 0.0f}, 1.0f, 8.0f);

	sphere = new GameObject("Sphere", sphereAppearance, transform, particleModel, true);



	// Particle System Initialisation
	Transform * psTransform = new Transform(myPlane->GetPlaneBody()->GetTransform(), planePos);

	sphereAppearance = new Appearance(sphereGeometry, shinyMaterial);
	appearance->SetTextureRV(_pStoneTex);

	ParticleSystem*_ps = new ParticleSystem(psTransform, { 8.0f, 0.0f, 10.0f }, sphereAppearance);

	particleSystems.push_back(_ps);

	psTransform = new Transform(myPlane->GetPlaneBody()->GetTransform(), planePos);
	_ps = new ParticleSystem(psTransform, { -8.0f, 0.0f, 10.0f }, sphereAppearance);

	particleSystems.push_back(_ps);


	// Cube Point Initialisation
	Geometry cubeGeometry = OBJLoader::Load("Objects/cube.obj", _pd3dDevice);

	Transform * cubeTransform = new Transform(myPlane->GetPlaneBody()->GetTransform(), planePos);

	Appearance* cubeAppearance = new Appearance(cubeGeometry, shinyMaterial);
	cubeAppearance->SetTextureRV(_pStoneTex);

	_cp = new CubePoint(cubeAppearance);


	// Camera
	myPlane->CalculateForwardVector();
	XMFLOAT3 planeDirection = myPlane->GetForwardVector();

	camNum = 2;

	eyeX = -10.0f;
	eyeY = 4.0f;
	eyeZ = -8.5f;

	// Main Camera
	XMFLOAT3 Eye = XMFLOAT3(eyeX, eyeY, eyeZ);
	XMFLOAT3 At = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 Up = XMFLOAT3(0.0f, 1.0f, 0.0f);

	cameraMain = new Camera(Eye, At, Up, (float)_WindowWidth, (float)_WindowHeight, 0.01f, 10000.0f);
	cameraMain->CalculateViewProjection();

	// Top View Camera
	Eye = XMFLOAT3(planePos.x + 0.01f, planePos.y + 15.0f, planePos.z + 0.01f);
	At = XMFLOAT3(planePos.x, planePos.y, planePos.z);
	Up = XMFLOAT3(0.0f, 1.0f, 0.0f);

	cameraTopView = new Camera(Eye, At, Up, (float)_WindowWidth, (float)_WindowHeight, 0.01f, 10000.0f);
	cameraTopView->CalculateViewProjection();

	// Plane Main Camera
	Eye = XMFLOAT3(-cameraPlanePos.x, -cameraPlanePos.y + 10.0f, -cameraPlanePos.z);
	At = XMFLOAT3(planeDirection.x, planeDirection.y, planeDirection.z);
	Up = XMFLOAT3(0.0f, 1.0f, 0.0f);

	cameraPlaneMain = new Camera(Eye, At, Up, (float)_WindowWidth, (float)_WindowHeight, 0.01f, 10000.0f);
	cameraPlaneMain->CalculateViewProjection();

	// Plane Alternative Camera
	Eye = XMFLOAT3(cameraPlanePos.x, cameraPlanePos.y + 10.0f, cameraPlanePos.z);
	At = XMFLOAT3(planeDirection.x, planeDirection.y, planeDirection.z);
	Up = XMFLOAT3(0.0f, 1.0f, 0.0f);

	cameraPlaneAlternate = new Camera(Eye, At, Up, (float)_WindowWidth, (float)_WindowHeight, 0.01f, 10000.0f);
	cameraPlaneAlternate->CalculateViewProjection();

	// Plane Perspective Camera calculation
	Eye = XMFLOAT3(planePos.x + 10.0f, planePos.y + 10.0f, planePos.z + 10.0f);
	At = XMFLOAT3(planePos.x, planePos.y, planePos.z);
	Up = XMFLOAT3(0.0f, 1.0f, 0.0f);

	cameraPlanePerspective = new Camera(Eye, At, Up, (float)_WindowWidth, (float)_WindowHeight, 0.01f, 5000.0f);
	cameraPlanePerspective->CalculateViewProjection();
}

void Application::InitPlaneObjects()
{
	Appearance* appearance;
	Transform* transform;
	ParticleModel* particleModel;

	// Plane Body
	Geometry planeGeometry = OBJLoader::Load("Objects/Plane Objects/Hercules.obj", _pd3dDevice);

	//XMFLOAT3 planePos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 planePos = XMFLOAT3(-415.415f, 0.0f, 35.63f);

	appearance = new Appearance(planeGeometry, shinyMaterial);
	appearance->SetTextureRV(_pPlaneTex);

	transform = new Transform();
	transform->SetScale(1.0f, 1.0f, 1.0f);
	transform->SetPosition(planePos);
	transform->SetRotation(0.0f, XMConvertToRadians(180.0f), 0.0f);

	particleModel = new PlaneParticleModel(transform, 1.0f);
	particleModel->SetCollisionRadius(10.0f);

	GameObject* planeBody = new GameObject("Plane", appearance, transform, particleModel, false);

	myPlane = new Plane(planeBody);
}


HRESULT Application::InitShadersAndInputLayout()
{
	HRESULT hr;

    // Compile the vertex shader
    ID3DBlob* pVSBlob = nullptr;
    hr = CompileShaderFromFile(L"DX11 Framework.fx", "VS", "vs_4_0", &pVSBlob);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
                   L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

	// Create the vertex shader
	hr = _pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &_pVertexShader);

	if (FAILED(hr))
	{	
		pVSBlob->Release();
        return hr;
	}

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
    hr = CompileShaderFromFile(L"DX11 Framework.fx", "PS", "ps_4_0", &pPSBlob);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
                   L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

	// Create the pixel shader
	hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &_pPixelShader);
	pPSBlob->Release();

    if (FAILED(hr))
        return hr;
	
    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT numElements = ARRAYSIZE(layout);

    // Create the input layout
	hr = _pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
                                        pVSBlob->GetBufferSize(), &_pVertexLayout);
	pVSBlob->Release();

	if (FAILED(hr))
        return hr;

    // Set the input layout
    _pImmediateContext->IASetInputLayout(_pVertexLayout);

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = _pd3dDevice->CreateSamplerState(&sampDesc, &_pSamplerLinear);

	return hr;
}

HRESULT Application::InitVertexBuffer()
{
	HRESULT hr;

    // Create vertex buffer
    SimpleVertex vertices[] =
    {
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },

		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
    };

    D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * 24;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices;

    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pVertexBuffer);

    if (FAILED(hr))
        return hr;

	// Create vertex buffer
	SimpleVertex planeVertices[] =
	{
		{ XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 5.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(5.0f, 5.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(5.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
	};

	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = planeVertices;

	hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pPlaneVertexBuffer);

	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT Application::InitIndexBuffer()
{
	HRESULT hr;

    // Create index buffer
    WORD indices[] =
    {
		3, 1, 0,
		2, 1, 3,

		6, 4, 5,
		7, 4, 6,

		11, 9, 8,
		10, 9, 11,

		14, 12, 13,
		15, 12, 14,

		19, 17, 16,
		18, 17, 19,

		22, 20, 21,
		23, 20, 22
    };

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * 36;     
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = indices;
    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pIndexBuffer);

    if (FAILED(hr))
        return hr;

	// Create plane index buffer
	WORD planeIndices[] =
	{
		0, 3, 1,
		3, 2, 1,
	};

	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * 6;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = planeIndices;
	hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pPlaneIndexBuffer);

	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT Application::InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW );
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"TutorialWindowClass";
    wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
    if (!RegisterClassEx(&wcex))
        return E_FAIL;

    // Create window
    _hInst = hInstance;
    RECT rc = {0, 0, 960, 540};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    _hWnd = CreateWindow(L"TutorialWindowClass", L"FGGC Semester 2 Framework", WS_OVERLAPPEDWINDOW,
                         CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
                         nullptr);
    if (!_hWnd)
		return E_FAIL;

    ShowWindow(_hWnd, nCmdShow);

    return S_OK;
}

HRESULT Application::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

    if (FAILED(hr))
    {
        if (pErrorBlob != nullptr)
            OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());

        if (pErrorBlob) pErrorBlob->Release();

        return hr;
    }

    if (pErrorBlob) pErrorBlob->Release();

    return S_OK;
}

HRESULT Application::InitDevice()
{
    HRESULT hr = S_OK;

    UINT createDeviceFlags = 0;

#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };

    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	UINT sampleCount = 4;

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = _renderWidth;
    sd.BufferDesc.Height = _renderHeight;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = _hWnd;
	sd.SampleDesc.Count = sampleCount;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
        _driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain(nullptr, _driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                                           D3D11_SDK_VERSION, &sd, &_pSwapChain, &_pd3dDevice, &_featureLevel, &_pImmediateContext);
        if (SUCCEEDED(hr))
            break;
    }

    if (FAILED(hr))
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = _pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    if (FAILED(hr))
        return hr;

    hr = _pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &_pRenderTargetView);
    pBackBuffer->Release();

    if (FAILED(hr))
        return hr;

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)_renderWidth;
    vp.Height = (FLOAT)_renderHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    _pImmediateContext->RSSetViewports(1, &vp);

	InitShadersAndInputLayout();

	InitVertexBuffer();
	InitIndexBuffer();

    // Set primitive topology
    _pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Create the constant buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
    hr = _pd3dDevice->CreateBuffer(&bd, nullptr, &_pConstantBuffer);

    if (FAILED(hr))
        return hr;

	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = _renderWidth;
	depthStencilDesc.Height = _renderHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = sampleCount;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	_pd3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, &_depthStencilBuffer);
	_pd3dDevice->CreateDepthStencilView(_depthStencilBuffer, nullptr, &_depthStencilView);

	_pImmediateContext->OMSetRenderTargets(1, &_pRenderTargetView, _depthStencilView);

	// Rasterizer
	D3D11_RASTERIZER_DESC cmdesc;

	ZeroMemory(&cmdesc, sizeof(D3D11_RASTERIZER_DESC));
	cmdesc.FillMode = D3D11_FILL_SOLID;
	cmdesc.CullMode = D3D11_CULL_NONE;
	hr = _pd3dDevice->CreateRasterizerState(&cmdesc, &RSCullNone);

	D3D11_DEPTH_STENCIL_DESC dssDesc;
	ZeroMemory(&dssDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	dssDesc.DepthEnable = true;
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dssDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	_pd3dDevice->CreateDepthStencilState(&dssDesc, &DSLessEqual);

	ZeroMemory(&cmdesc, sizeof(D3D11_RASTERIZER_DESC));

	cmdesc.FillMode = D3D11_FILL_SOLID;
	cmdesc.CullMode = D3D11_CULL_BACK;

	cmdesc.FrontCounterClockwise = true;
	hr = _pd3dDevice->CreateRasterizerState(&cmdesc, &CCWcullMode);

	cmdesc.FrontCounterClockwise = false;
	hr = _pd3dDevice->CreateRasterizerState(&cmdesc, &CWcullMode);

    return S_OK;
}

// -------------- Cleanup -------------- //

void Application::Cleanup()
{
    if (_pImmediateContext) _pImmediateContext->ClearState();
	if (_pSamplerLinear) _pSamplerLinear->Release();

	if (_pTextureRV) _pTextureRV->Release();

	if (_pGrassTex) _pGrassTex->Release();

    if (_pConstantBuffer) _pConstantBuffer->Release();

    if (_pVertexBuffer) _pVertexBuffer->Release();
    if (_pIndexBuffer) _pIndexBuffer->Release();
	if (_pPlaneVertexBuffer) _pPlaneVertexBuffer->Release();
	if (_pPlaneIndexBuffer) _pPlaneIndexBuffer->Release();

    if (_pVertexLayout) _pVertexLayout->Release();
    if (_pVertexShader) _pVertexShader->Release();
    if (_pPixelShader) _pPixelShader->Release();
    if (_pRenderTargetView) _pRenderTargetView->Release();
    if (_pSwapChain) _pSwapChain->Release();
    if (_pImmediateContext) _pImmediateContext->Release();
    if (_pd3dDevice) _pd3dDevice->Release();
	if (_depthStencilView) _depthStencilView->Release();
	if (_depthStencilBuffer) _depthStencilBuffer->Release();

	if (DSLessEqual) DSLessEqual->Release();
	if (RSCullNone) RSCullNone->Release();

	if (CCWcullMode) CCWcullMode->Release();
//	if (CWcullMode) CWcullMode->Release();

	if (_camera)
	{
		delete _camera;
		_camera = nullptr;
	}

	/*for (auto gameObject : _cubes)
	{
		if (gameObject)
		{
			delete gameObject;
			gameObject = nullptr;
		}
	}*/
}

// --------------- Input --------------- //

void Application::CameraInput()
{
	// If Camera 1 then Check Free Camera input
	if (camNum == 1)
	{
		float cameraMoveSpeed = 0.1f;

		// Zoom Out
		if (GetAsyncKeyState(VK_DOWN))
		{
			eyeX -= cameraMoveSpeed;
		}

		// Zoom In
		if (GetAsyncKeyState(VK_UP))
		{
			eyeX += cameraMoveSpeed; // or call camera setX function appropriately
		}

		// Pan Up
		if (GetAsyncKeyState(0x54))
		{
			eyeY += cameraMoveSpeed;
		}

		// Pan Down
		if (GetAsyncKeyState(0x47))
		{
			eyeY -= cameraMoveSpeed;
		}

		// Pan Left
		if (GetAsyncKeyState(0x46))
		{
			eyeZ += cameraMoveSpeed;
		}
		// Pan right
		if (GetAsyncKeyState(0x48))
		{
			eyeZ -= cameraMoveSpeed;
		}
	}

	// If Camera 1, 2 or 3 then Check Plane Input
	if (camNum == 1 || camNum == 2 || camNum == 3)
	{
		myPlane->Input();
	}

	// Camera Input Check
	if (GetAsyncKeyState('1'))
	{
		camNum = 1;
	}
	else if (GetAsyncKeyState('2'))
	{
		camNum = 2;
	}
	else if (GetAsyncKeyState('3'))
	{
		camNum = 3;
	}
	else if (GetAsyncKeyState('4'))
	{
		camNum = 4;
	}
	else if (GetAsyncKeyState('5'))
	{
		camNum = 5;
	}

	XMFLOAT3 Eye;
	XMFLOAT3 At;

	XMFLOAT3 planePos = myPlane->GetPlanePosition();
	XMFLOAT3 planeDirection = myPlane->GetForwardVector();

	// Check Camera Number
	if (camNum == 1)
	{
		Eye = XMFLOAT3(eyeX, eyeY, eyeZ);
		At = XMFLOAT3(eyeX + 1.0f, eyeY, eyeZ);

		cameraMain->SetEye(Eye);
		cameraMain->SetAt(At);
		cameraMain->CalculateViewProjection();

		cameraCurrent = cameraMain;
	}
	if (camNum == 1 || camNum == 2)
	{
		cameraCurrent = cameraPlaneMain;

		cameraPlanePos.x = planePos.x;
		cameraPlanePos.y = planePos.y;
		cameraPlanePos.z = planePos.z;

		Eye = XMFLOAT3(cameraPlanePos.x - (planeDirection.x * 15.0f), cameraPlanePos.y + 10.0f, cameraPlanePos.z - (planeDirection.z * 15.0f));
		At = XMFLOAT3(planePos.x + (planeDirection.x * 10.0f), planePos.y + (planeDirection.y * 10.0f), planePos.z + (planeDirection.z * 10.0f));

		cameraPlaneMain->SetEye(Eye);
		cameraPlaneMain->SetAt(At);
		cameraPlaneMain->CalculateViewProjection();
	}
	else if (camNum == 3)
	{
		cameraCurrent = cameraPlaneAlternate;

		cameraPlanePos.x = planePos.x - planeDirection.x;
		cameraPlanePos.y = planePos.y - planeDirection.y;
		cameraPlanePos.z = planePos.z - planeDirection.z;

		Eye = XMFLOAT3(cameraPlanePos.x + (planeDirection.x * 15.0f), cameraPlanePos.y + 10.0f, cameraPlanePos.z + (planeDirection.z * 15.0f));
		At = XMFLOAT3(cameraPlanePos.x - (planeDirection.x * 15.0f), cameraPlanePos.y - 4.0f, cameraPlanePos.z - (planeDirection.z * 15.0f));

		cameraPlaneAlternate->SetEye(Eye);
		cameraPlaneAlternate->SetAt(At);
		cameraPlaneAlternate->CalculateViewProjection();
	}
	else if (camNum == 4)
	{
		cameraCurrent = cameraTopView;

		Eye = XMFLOAT3(planePos.x + 0.01f, planePos.y + 15.0f, planePos.z + 0.01f);
		At = XMFLOAT3(planePos.x, planePos.y, planePos.z);

		cameraTopView->SetEye(Eye);
		cameraTopView->SetAt(At);
		cameraTopView->CalculateViewProjection();
	}
	else if (camNum == 5)
	{
		cameraCurrent = cameraPlanePerspective;

		Eye = XMFLOAT3(planePos.x + 10.0f, planePos.y + 10.0f, planePos.z + 10.0f);
		At = XMFLOAT3(planePos.x, planePos.y, planePos.z);

		cameraPlanePerspective->SetEye(Eye);
		cameraPlanePerspective->SetAt(At);
		cameraPlanePerspective->CalculateViewProjection();
	}
}

// -------------- Updates ------------- //

void Application::PlaneUpdate(float t)
{
	CameraInput();

	// Plane Body Updates
	myPlane->Update(t);
	
}

void Application::Update(float t)
{
	// Plane Object Updates
	PlaneUpdate(t);

	if (myPlane->GetPlaneBody()->GetParticleModel()->CollisionCheck(sphere->GetTransform()->GetPosition(), sphere->GetParticleModel()->GetCollisionRadius()))
	{
		myPlane->GetPlaneBody()->GetParticleModel()->ResolveSphereCollision(sphere->GetParticleModel());
	}

	// Update Ground Plane
	groundPlane->Update(t);

	// Update Mountain
	mountain->Update(t);

	// Update Race Track
	raceTrack->Update(t);

	// Sky Box Update
	//skyBox->GetTransform()->SetPosition(object2Pos);
	skyBox->Update(t);

	//Sphere Update
	sphere->Update(t);

	//sphere->GetTransform()->SetPosition(sphere->GetTransform()->GetPrevPosition().x, sphere->GetTransform()->GetPrevPosition().y, sphere->GetTransform()->GetPrevPosition().z);
	sphere->GetParticleModel()->BaseCollisionCheck(XMFLOAT3{ 0.0f, 0.0f, 0.0f });
	
	for (auto ps : particleSystems)
	{
		ps->Update(t);
	}

	_cp->Update(t);
}

// --------------- Draw --------------- //

void Application::Draw()
{
    // Clear buffers
	float ClearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f }; // red,green,blue,alpha
    _pImmediateContext->ClearRenderTargetView(_pRenderTargetView, ClearColor);
	_pImmediateContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // Setup buffers and render scene
	_pImmediateContext->IASetInputLayout(_pVertexLayout);

	_pImmediateContext->VSSetShader(_pVertexShader, nullptr, 0);
	_pImmediateContext->PSSetShader(_pPixelShader, nullptr, 0);

	_pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->PSSetSamplers(0, 1, &_pSamplerLinear);


	// Setup View and Projection
	XMMATRIX view;
	XMMATRIX projection;

	if (camNum == 1)
	{
		XMFLOAT4X4 cameraView = cameraMain->GetView();
		view = XMLoadFloat4x4(&cameraView);

		XMFLOAT4X4 cameraProjection = cameraMain->GetProjection();
		projection = XMLoadFloat4x4(&cameraProjection);
	}
	else
	{
		XMFLOAT4X4 cameraView = cameraCurrent->GetView();
		view = XMLoadFloat4x4(&cameraView);

		XMFLOAT4X4 cameraProjection = cameraCurrent->GetProjection();
		projection = XMLoadFloat4x4(&cameraProjection);
	}

	// Render Objects
	ConstantBuffer cb;

	cb.View = XMMatrixTranspose(view);
	cb.Projection = XMMatrixTranspose(projection);
	
	cb.light = basicLight;
	cb.EyePosW = _camera->GetPosition();

	Material material;

	// --------------- Draw Sky Box ---------------- //

	material = skyBox->GetAppearance()->GetMaterial();

	// Copy material to shader
	cb.surface.AmbientMtrl = material.ambient;
	cb.surface.DiffuseMtrl = material.diffuse;
	cb.surface.SpecularMtrl = material.specular;

	// Set world matrix
	cb.World = XMMatrixTranspose(skyBox->GetTransform()->GetWorldMatrix());

	// Set texture
	if (skyBox->GetAppearance()->HasTexture())
	{
		ID3D11ShaderResourceView* textureRV = skyBox->GetAppearance()->GetTextureRV();
		_pImmediateContext->PSSetShaderResources(0, 1, &textureRV);
		cb.HasTexture = 1.0f;
	}
	else
	{
		cb.HasTexture = 0.0f;
	}

	// Update constant buffer
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

	// Draw object
	skyBox->Draw(_pImmediateContext);

	// ------------- Draw Ground Plane ------------- //

	material = groundPlane->GetAppearance()->GetMaterial();

	// Copy material to shader
	cb.surface.AmbientMtrl = material.ambient;
	cb.surface.DiffuseMtrl = material.diffuse;
	cb.surface.SpecularMtrl = material.specular;

	// Set world matrix
	cb.World = XMMatrixTranspose(groundPlane->GetTransform()->GetWorldMatrix());

	// Set texture
	if (groundPlane->GetAppearance()->HasTexture())
	{
		ID3D11ShaderResourceView* textureRV = groundPlane->GetAppearance()->GetTextureRV();
		_pImmediateContext->PSSetShaderResources(0, 1, &textureRV);
		cb.HasTexture = 1.0f;
	}
	else
	{
		cb.HasTexture = 0.0f;
	}

	// Update constant buffer
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

	// Draw object
	groundPlane->Draw(_pImmediateContext);

	// --------------- Draw Mountain --------------- //

	material = mountain->GetAppearance()->GetMaterial();

	// Copy material to shader
	cb.surface.AmbientMtrl = material.ambient;
	cb.surface.DiffuseMtrl = material.diffuse;
	cb.surface.SpecularMtrl = material.specular;

	// Set world matrix
	cb.World = XMMatrixTranspose(mountain->GetTransform()->GetWorldMatrix());

	// Set texture
	if (mountain->GetAppearance()->HasTexture())
	{
		ID3D11ShaderResourceView* textureRV = mountain->GetAppearance()->GetTextureRV();
		_pImmediateContext->PSSetShaderResources(0, 1, &textureRV);
		cb.HasTexture = 1.0f;
	}
	else
	{
		cb.HasTexture = 0.0f;
	}

	// Update constant buffer
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

	// Draw object
	mountain->Draw(_pImmediateContext);


	// -------------- Draw Race Track -------------- //

	material = raceTrack->GetAppearance()->GetMaterial();

	// Copy material to shader
	cb.surface.AmbientMtrl = material.ambient;
	cb.surface.DiffuseMtrl = material.diffuse;
	cb.surface.SpecularMtrl = material.specular;

	// Set world matrix
	cb.World = XMMatrixTranspose(raceTrack->GetTransform()->GetWorldMatrix());

	// Set texture
	if (raceTrack->GetAppearance()->HasTexture())
	{
		ID3D11ShaderResourceView* textureRV = raceTrack->GetAppearance()->GetTextureRV();
		_pImmediateContext->PSSetShaderResources(0, 1, &textureRV);
		cb.HasTexture = 1.0f;
	}
	else
	{
		cb.HasTexture = 0.0f;
	}

	// Update constant buffer
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

	// Draw object
	raceTrack->Draw(_pImmediateContext);


	
	// ------------- Draw Plane Body ------------- //

	GameObject* planeBody = myPlane->GetPlaneBody();
	material = planeBody->GetAppearance()->GetMaterial();

	// Copy material to shader
	cb.surface.AmbientMtrl = material.ambient;
	cb.surface.DiffuseMtrl = material.diffuse;
	cb.surface.SpecularMtrl = material.specular;

	// Set world matrix
	cb.World = XMMatrixTranspose(planeBody->GetTransform()->GetWorldMatrix());

	// Set plane texture
	if (planeBody->GetAppearance()->HasTexture())
	{
		ID3D11ShaderResourceView* textureRV = planeBody->GetAppearance()->GetTextureRV();
		_pImmediateContext->PSSetShaderResources(0, 1, &textureRV);
		cb.HasTexture = 1.0f;
	}
	else
	{
		cb.HasTexture = 0.0f;
	}

	// Update constant buffer
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

	// Draw object
	planeBody->Draw(_pImmediateContext);


	// --------------- Draw Sphere ---------------- //

	material = sphere->GetAppearance()->GetMaterial();

	// Copy material to shader
	cb.surface.AmbientMtrl = material.ambient;
	cb.surface.DiffuseMtrl = material.diffuse;
	cb.surface.SpecularMtrl = material.specular;

	// Set world matrix
	cb.World = XMMatrixTranspose(sphere->GetTransform()->GetWorldMatrix());

	// Set plane texture
	if (sphere->GetAppearance()->HasTexture())
	{
		ID3D11ShaderResourceView* textureRV = sphere->GetAppearance()->GetTextureRV();
		_pImmediateContext->PSSetShaderResources(0, 1, &textureRV);
		cb.HasTexture = 1.0f;
	}
	else
	{
		cb.HasTexture = 0.0f;
	}

	// Update constant buffer
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

	// Draw object
	sphere->Draw(_pImmediateContext);


	// --------------- Draw Particles ---------------- //
	for (auto ps : particleSystems)
	{
		for (auto particles : ps->getParticles())
		{
			// Get render material
			Material material = particles->GetAppearance()->GetMaterial();

			// Copy material to shader
			cb.surface.AmbientMtrl = material.ambient;
			cb.surface.DiffuseMtrl = material.diffuse;
			cb.surface.SpecularMtrl = material.specular;

			// Set world matrix
			cb.World = XMMatrixTranspose(particles->GetTransform()->GetWorldMatrix());

			// Set texture
			if (particles->GetAppearance()->HasTexture())
			{
				ID3D11ShaderResourceView * textureRV = particles->GetAppearance()->GetTextureRV();
				_pImmediateContext->PSSetShaderResources(0, 1, &textureRV);
				cb.HasTexture = 1.0f;
			}
			else
			{
				cb.HasTexture = 0.0f;
			}

			// Update constant buffer
			_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

			// Draw object
			particles->Draw(_pImmediateContext);
		}
	}

	for (auto cubes : _cp->getCubes())
	{
		// Get render material
		Material material = cubes->GetAppearance()->GetMaterial();

		// Copy material to shader
		cb.surface.AmbientMtrl = material.ambient;
		cb.surface.DiffuseMtrl = material.diffuse;
		cb.surface.SpecularMtrl = material.specular;

		// Set world matrix
		cb.World = XMMatrixTranspose(cubes->GetTransform()->GetWorldMatrix());

		// Set texture
		if (cubes->GetAppearance()->HasTexture())
		{
			ID3D11ShaderResourceView * textureRV = cubes->GetAppearance()->GetTextureRV();
			_pImmediateContext->PSSetShaderResources(0, 1, &textureRV);
			cb.HasTexture = 1.0f;
		}
		else
		{
			cb.HasTexture = 0.0f;
		}

		// Update constant buffer
		_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

		// Draw object
		cubes->Draw(_pImmediateContext);
	}
	


    // Present our back buffer to our front buffer
    _pSwapChain->Present(0, 0);
}