///=================================================================================================
// file:	InputAssembler.cpp
//
// summary:	Implements the input assembler class
///=================================================================================================

#include "InputAssembler.h"
//==================================================
//
//--------------	--------------------------------
//|   Params   |	|		  Description          |
//--------------	--------------------------------
//==================================================
InputAssembler::InputAssembler()
{
	g_VertexBuffer = NULL;
	g_InputLayout = NULL;

	m_PixelShader = NULL;
	m_VertexShader = NULL;
	m_BlobVS = NULL;
	m_BlobPS = NULL;
	m_BlobError = NULL;
}

//==================================================
//
//--------------	--------------------------------
//|   Params   |	|		  Description          |
//--------------	--------------------------------
//==================================================
InputAssembler::~InputAssembler() {}

//==================================================
//
//--------------	--------------------------------
//|   Params   |	|		  Description          |
//--------------	--------------------------------
//==================================================
void InputAssembler::Init(ID3D11Device* device, ID3D11DeviceContext* context)
{
	_device = device;
	_context = context;

	// Deferred Shader goes here
	pDeferredMRT = new Effect();
	pDeferredMRT->Init(device, L"DefVertexShader.hlsl", L"DefPixelShader.hlsl");
	pDeferredMRT->SetShaderResources(L"BOX.DDS");
	pDeferredMRT->SetShaderResources(L"BoxNormalAlpha.DDS");
	pDeferredMRT->SetShaderResources(L"BoxRoughness.DDS");

	// Fullscreen Quad to backbuffer
	pRTBackbuffer = new Effect();
	pRTBackbuffer->Init(device, L"VertexShader.hlsl", L"PixelShader.hlsl");

	// Primer Mesh
	_model = new Model();
	_model->Init(L"box.txt", L"BoxMaterials.txt");

	// TODO: Load defaults to init this object. There will be other methods that I can pass
	// Geometry data to load and automaticly create the vertex buffer from that data
	BuildVertexBuffer();
	CreateInputLayout(device);

	// Device Context setup
	UINT stride = sizeof(VertexTypeDef); // I wasted an hour on you.......
	UINT offset = 0;

	context->IASetInputLayout(g_InputLayout);
	context->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(g_IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


}

/* Query Methods */

//==================================================
//
//--------------	--------------------------------
//|   Params   |	|		  Description          |
//--------------	--------------------------------
//==================================================
ID3D11InputLayout* InputAssembler::GetInputLayout()
{
	return g_InputLayout;
}

//==================================================
//
//--------------	--------------------------------
//|   Params   |	|		  Description          |
//--------------	--------------------------------
//==================================================
ID3D11Buffer* InputAssembler::GetVertexBuffer()
{
	return g_VertexBuffer;
}

/* HElper Methods */

//==================================================
//
//--------------	--------------------------------
//|   Params   |	|		  Description          |
//--------------	--------------------------------
//==================================================
void InputAssembler::BuildVertexBuffer()
{	
	//====================================
	// ALERT ALERT ALERT ALERT ALERT ALERT
	//====================================
	// Index buffer not updating! Figure this out! Sould be similar to VERTEXBUFFER!!!!
	//
	//===================================================================================
	
	// Grab vertex data
	std::vector<VertexTypeDef> vertices = _model->GetVertexArray();

	BatchGeometry();

	if (_vertexBufferCreated)
	{
		D3D11_MAPPED_SUBRESOURCE resource;
		ZeroMemory(&resource, sizeof(resource));

		// Lock Vertex Buffer
		_context->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);

		// Copy
		memcpy(resource.pData, &vertices[0], sizeof(VertexTypeDef) * vertices.size());

		// Unmap
		_context->Unmap(g_VertexBuffer, 0);

	}
	else
	{
		/* Now Create VertexBuffer!
		Step 1: Bind Vertex information to D3D11_SUBRESOURCE_DATA
		Step 2: Fill out buffer description
		Step 3: Bind Subresource data to device (copy from sys mem to video mem)
		*/

		// Bind Subresource data to device
		D3D11_SUBRESOURCE_DATA initData = { 0 };
		ZeroMemory(&initData, sizeof(initData));
		initData.pSysMem = &vertices[0];

		// Fill out buffer description
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = sizeof(VertexTypeDef) * vertices.size();
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bd.MiscFlags = 0;
		bd.StructureByteStride = 0;

		// Bind Subresource data to device
		_device->CreateBuffer(&bd, &initData, &g_VertexBuffer);

		_vertexBufferCreated = true;


		UINT stride = sizeof(VertexTypeDef); // I wasted an hour on you.......
		UINT offset = 0;
		_context->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

#pragma region INDEX BUFFER TEST
		/* INDEX BUFFER */
		vector<unsigned short> indices = _model->GetIndexArray();

#pragma endregion

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		ZeroMemory(&indexBufferData, sizeof(indexBufferData));
		indexBufferData.pSysMem = &indices[0];
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;

		CD3D11_BUFFER_DESC indexBufferDesc;
		ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
		indexBufferDesc.ByteWidth = sizeof(indices) * indices.size();
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		// This call allocates a device resource for the index buffer and copies
		// in the data.
		_device->CreateBuffer(
			&indexBufferDesc,
			&indexBufferData,
			&g_IndexBuffer
			);
	}
}

//==================================================
//
//--------------	--------------------------------
//|   Params   |	|		  Description          |
//--------------	--------------------------------
//==================================================
void InputAssembler::CreateInputLayout(ID3D11Device* device)
{
	/* Create Input Layout
	Step 1: Fill out element description (must match vertex type
	Step 2: Bind Input Layout to device

	NOTE: Why do I have to supply the Vertex Shader to the inputlayout?
	*/

	// Fill out Input Layout element description
	// SHOULD change this depending on the vertex type used
	D3D11_INPUT_ELEMENT_DESC elements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
		D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12,
		D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20,
		D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32,
		D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44,
		D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numElements = _countof(elements);

	// Bind Input Layout to device
	// NOTE: THIS WILL CAUSE PROBLEMS LATER
	ID3D10Blob *VS = pDeferredMRT->SendVSBlob();
	device->CreateInputLayout(elements, numElements, VS->GetBufferPointer(), VS->GetBufferSize(),
		&g_InputLayout);
}


//==================================================
//
//--------------	--------------------------------
//|   Params   |	|		  Description          |
//--------------	--------------------------------
//==================================================
void InputAssembler::Load()
{
}

//==========================================
// OLD SHADER MEMBERS
//==========================================
void InputAssembler::BindShaders()
{	
	pRTBackbuffer->BindEffect();
}

void InputAssembler::BindDeferredShaders()
{
	pDeferredMRT->BindEffect();
}
void InputAssembler::SetDeferredResource(ID3D11ShaderResourceView *srv)
{
	pRTBackbuffer->SetShaderResources(srv);
}

void InputAssembler::BatchGeometry()
{
	// Clear buffers
	_vertices.clear();
	_indices.clear();
	_offsetBuffer.clear();
	_cBufferPerObject.clear();

	// Local vars
	Model *model;
	EntityDrawable *entity;
	unsigned int offset;

	//As long as the draw queue is not empty
	while (!_drawQueue.empty())
	{
		// Pop first object to draw
		entity = _drawQueue.front();
		_drawQueue.pop();

		// Grab Mesh
		model = entity->getModel();

		// Get offset value from the model
		offset = model->getNumIndex();

		// Push vertices into the vertex buffer
		for (int i = 0; i < model->getNumVerts(); i++)
		{
			_vertices.push_back(model->GetVertexArray()[i]);
		}

		// Load indices into IndexBuffer
		for (int i = 0; i < offset; i++)
		{
			_indices.push_back(model->GetIndexArray()[i]);
		}

		// Add offset to buffer
		_offsetBuffer.push_back(offset);

		// Load Cbuffer for this object
		_cBufferPerObject.push_back(entity->getWorldMatrix());

		// Add stats here later
		// Triangles this frame
		// Vertices this frame
		//
	}
}

bool InputAssembler::AddToDrawQueue(EntityDrawable *entity)
{
	_drawQueue.push(entity);

	return true;
}

void InputAssembler::LoadMaterials(vector<string> materialList)
{
	pDeferredMRT->ClearResources();
	string s;

	for (int i = 0; i < materialList.size(); i++)
	{
		s = materialList[i];

		std::wstring stemp = std::wstring(s.begin(), s.end());
		LPCWSTR sw = stemp.c_str();
		pDeferredMRT->SetShaderResources(sw);
	}
}