//--------------------------------------------------------------------------------------
// File: D3D11Render.h
//
// This application displays a triangle rendered using Direct3D 11. (TEST BRANCH WORKS!)
//
// Concepts:
//     Creating a device and swap chain
//     Compiling and creating shaders
//     Setting up the render target and viewport
//     Setting up a simple vertex buffer and its input layout
//     Clearing the render target
//     Drawing a triangle
//     Presenting the rendered frame to the display
//
// Author: Robert House 2014
//--------------------------------------------------------------------------------------

#pragma once

#include <Windows.h>
#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
#include <dxgi.h>
#pragma comment(lib, "dxgi.lib")
#include <DirectXColors.h>
#include <vector>

/* LOCAL COMPONENTS */
#include "InputAssembler.h"
#include "RenderTarget.h"

using namespace DirectX;
using namespace std;

// Vertex Shader CBuffer
struct VS_CONSTANT_BUFFER_DATA
{
	XMFLOAT4X4 world;
	XMFLOAT4X4 view;
	XMFLOAT4X4 proj;
	XMFLOAT4X4 texture;
};

class D3D11Render
{
private:
	static const int				NUMTARGETS = 3;
	bool							_isWindowed;
	bool							_vsync;
	float							_screenDepth;
	float							_screenNear;
	float							_aspectRatio;
	int								_screenWidth;
	int								_screenHeight;
	int								_refreashRateNum = 60;
	int								_refreashRateDen = 1;
	D3D_FEATURE_LEVEL				_featureLevel = D3D_FEATURE_LEVEL_11_0;
	unsigned int					count = 0; // Can delete when I have my timer finished

	HWND							_hwnd;
	ID3D11Device*					_device;
	vector <IDXGIAdapter*>			_adapters;
	ID3D11DeviceContext*			_immContext;
	IDXGISwapChain*					_swapChain;
	ID3D11RenderTargetView*			_renderTargetView;
	D3D11_VIEWPORT					_viewPort;
	ID3D11Buffer*					_constantBuffer;
	VS_CONSTANT_BUFFER_DATA			_constantBufferData;
	InputAssembler* ia;

	// RenderTarget
	RenderTarget *gBuffer;


	/* Function Prototypes */
public:
	D3D11Render();				// Constructor
	~D3D11Render();				// Destructor

	bool Init(float screenWidth, float screenHeight, HWND handle, bool vsync, bool window, 
		float depth, float snear);
	void Draw();
	bool ShutDown();
	void Update();

private:
	HRESULT DeviceSetup();
	HRESULT BackBufferSetup();
	void ConstantBufferSetup();

	/* Set Methods */
public:
	void SetCameraData(DirectX::XMFLOAT4X4 viewMatrix, DirectX::XMFLOAT4X4 projMatrix);


	/* Test Components */
public:
	
	void TestComponents();
	//vector <IDXGIAdapter*> D3D11Render::EnumerateAdapters();
	//void D3D11Render::GetDisplayModes();
	void CreateGBuffer();
	void RenderToTexture(ID3D11Device *device);

};