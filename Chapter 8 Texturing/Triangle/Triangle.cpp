//***************************************************************************************
// CrateDemo.cpp by Frank Luna (C) 2011 All Rights Reserved.
//
// Demonstrates texturing a box.
//
// Controls:
//		Hold the left mouse button down and move the mouse to rotate.
//      Hold the right mouse button down to zoom in and out.
//
//***************************************************************************************
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <fstream>

#include "d3dApp.h"
#include "d3dx11Effect.h"
#include "GeometryGenerator.h"
#include "MathHelper.h"
#include "LightHelper.h"
#include "Effects.h"
//#include "Vertex.h"

class TriangleApp : public D3DApp
{
public:
	TriangleApp(HINSTANCE hInstance);
	~TriangleApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BuildGeometryBuffers();
	bool InitShader(const wchar_t* vsFilename, const wchar_t* psFilename);

private:
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
	};

	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

private:
	ID3D11Buffer * mTriangleVB;
	ID3D11Buffer* mTriangleIB;

	ID3D11Texture2D* mTexture;
	ID3D11ShaderResourceView* mTextureView;

	XMMATRIX mProj;
	XMMATRIX mWorld;
	XMMATRIX mOrthoMatrix;
	XMMATRIX mView;

	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* mLayout;
	ID3D11Buffer* mMatrixBuffer;
	ID3D11SamplerState* m_sampleState;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	TriangleApp theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}


TriangleApp::TriangleApp(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
	//// 创建投影矩阵
	//mProj = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);

	//// 这里世界矩阵是一个单位矩阵
	//mWorld = XMMatrixIdentity();

	//// 创建一个正交投影矩阵
	//mOrthoMatrix = XMMatrixOrthographicLH((float)mClientWidth, (float)mClientHeight, 1.0f, 1000.0f);
}

TriangleApp::~TriangleApp()
{
	DestroyWindow(mhMainWnd);
	mhMainWnd = NULL;
}

bool TriangleApp::Init()
{
	if (!D3DApp::Init())
		return false;

	BuildGeometryBuffers();
	CreateTargaTextureFromFile(md3dDevice, md3dImmediateContext, L"Textures/stone01.tga", (ID3D11Resource**)&mTexture, &mTextureView, 0);
	
	InitShader(L"Shaders/texture.vs", L"Shaders/texture.ps");
	
	return true;
}

void TriangleApp::OnResize()
{
	D3DApp::OnResize();

	// 创建投影矩阵
	//mProj = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	mProj = XMMatrixIdentity();
}
void TriangleApp::UpdateScene(float dt)
{
	// Convert Spherical to Cartesian coordinates.
	float x = 2 * sinf(dt * 0.01);
	float z = 0;
	float y = 2 * cosf(dt * 0.01);

	mWorld = XMMatrixIdentity();
	mView = XMMatrixIdentity();
	// Build the view matrix.
	/*XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	mView = XMMatrixLookAtLH(pos, target, up);*/
	return;
}

void TriangleApp::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Black));// Clear the back buffer.
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);// Clear the depth buffer.
	
	// 创建了输入布局对象之后，它不会自动绑定到设备上，我们必须调用下面的语句来实现绑定。
	md3dImmediateContext->IASetInputLayout(mLayout);
	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(VertexType);
	UINT offset = 0;
	// 在创建顶点缓冲区后，我们必须把它绑定到设备的输入槽上，只有这样才能将顶点送入管线，这一工作使用如下方法完成。
	// 1．StartSlot：顶点缓冲区所要绑定的起始输入槽。一共有16个输入槽，索引依次为0到15。
	// 2．NumBuffers：顶点缓冲区所要绑定的输入槽的数量，如果起始输入槽为索引k，我们绑定了n个缓冲，那么缓冲将绑定在索引为Ik，Ik + 1……Ik + n - 1的输入槽上。
	// 3．ppVertexBuffers：指向顶点缓冲区数组的第一个元素的指针。
	// 4．pStrides：指向步长数组的第一个元素的指针（该数组的每个元素对应一个顶点缓冲区，也就是，第i个步长对应于第i个顶点缓冲区）。这个步长是指顶点缓冲区中的元素的字节长度。
	// 5．pOffsets：指向偏移数组的第一个元素的指针（该数组的每个元素对应一个顶点缓冲区，也就是，第i个偏移量对应于第i个顶点缓冲区）。这个偏移量是指从顶点缓冲区的起始位置开始，到输入装配阶段将要开始读取数据的位置之间的字节长度。当希望跳过顶点缓冲区前面的一部分数据时，可以使用该参数
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mTriangleVB, &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	md3dImmediateContext->IASetIndexBuffer(mTriangleIB, DXGI_FORMAT_R32_UINT, 0);

	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;

	// Lock the constant buffer so it can be written to.
	HR(md3dImmediateContext->Map(mMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	
	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	dataPtr->world      = mWorld;
	dataPtr->view       = mView;
	dataPtr->projection = mProj;

	// Unlock the constant buffer.
	md3dImmediateContext->Unmap(mMatrixBuffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	// Finanly set the constant buffer in the vertex shader with the updated values.
	md3dImmediateContext->VSSetConstantBuffers(0, 1, &mMatrixBuffer);

	// Set shader texture resource in the pixel shader.
	md3dImmediateContext->PSSetShaderResources(0, 1, &mTextureView);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	md3dImmediateContext->VSSetShader(m_vertexShader, NULL, 0);
	md3dImmediateContext->PSSetShader(m_pixelShader, NULL, 0);

	// Set the sampler state in the pixel shader.
	md3dImmediateContext->PSSetSamplers(0, 1, &m_sampleState);

	// Render the triangle.
	md3dImmediateContext->DrawIndexed(3, 0, 0);

	HR(mSwapChain->Present(1, 0));

	return;
}

void TriangleApp::OnMouseDown(WPARAM btnState, int x, int y)
{

}

void TriangleApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	
}

void TriangleApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	
}

void TriangleApp::BuildGeometryBuffers()
{
	VertexType vertices[3] = {
		{ XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3( 0.0f,  1.0f, 0.0f), XMFLOAT2(0.5f, 0.0f) },
		{ XMFLOAT3( 1.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
	};

	// 1．填写一个D3D11_BUFFER_DESC结构体，描述我们所要创建的缓冲区
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(VertexType) * 3;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	// 2．填写一个D3D11_SUBRESOURCE_DATA结构体，为缓冲区指定初始化数据
	D3D11_SUBRESOURCE_DATA vertexData;
	vertexData.pSysMem = vertices;//包含初始化数据的系统内存数组的指针。当缓冲区可以存储n个顶点时，对应的初始化数组也应至少包含n个顶点，从而使整个缓冲区得到初始化

	// 3．调用ID3D11Device::CreateBuffer方法来创建缓冲区
	HR(md3dDevice->CreateBuffer(&vbd, &vertexData, &mTriangleVB));

	UINT indices[] = {
		0, 1, 2
	};

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * 3;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	HR(md3dDevice->CreateBuffer(&ibd, &indexData, &mTriangleIB));
}


bool TriangleApp::InitShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage = NULL;
	ID3D10Blob* vertexShaderBuffer = NULL;
	ID3D10Blob* pixelShaderBuffer = NULL;

	
	D3D11_SAMPLER_DESC samplerDesc;

	// Compile the vertex shader code.
	HR(D3DCompileFromFile(vsFilename, NULL, NULL, "TextureVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,	&vertexShaderBuffer, &errorMessage));

	// Compile the pixel shader code.
	HR(D3DCompileFromFile(psFilename, NULL, NULL, "TexturePixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage));

	// Create the vertex shader from the buffer.
	HR(md3dDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader));

	// Create the pixel shader from the buffer.
	HR(md3dDevice->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader));

	// D3D11_INPUT_ELEMENT_DESC 数组中的每个元素描述了顶点结构体的一个分量
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,   0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	// 指定了输入布局描述之后，我们就可以使用ID3D11Device::CreateInputLayout方法获取一个表示输入布局的ID3D11InputLayout接口的指针
	// 1．pInputElementDescs：一个用于描述顶点结构体的D3D11_INPUT_ELEMENT_DESC数组。
	// 2．NumElements：D3D11_INPUT_ELEMENT_DESC 数组的元素数量。
	// 3．pShaderBytecodeWithInputSignature：指向顶点着色器参数的字节码的指针。
	// 4．BytecodeLength：顶点着色器参数的字节码长度，单位为字节。
	// 5．ppInputLayout：返回创建后的ID3D11InputLayout指针。
	HR(md3dDevice->CreateInputLayout(layout, sizeof(layout) / sizeof(layout[0]), vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &mLayout));
	
	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	ReleaseCOM(vertexShaderBuffer);
	ReleaseCOM(pixelShaderBuffer);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	D3D11_BUFFER_DESC MatrixBufferDesc;
	MatrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	MatrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	MatrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	MatrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	MatrixBufferDesc.MiscFlags = 0;
	MatrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	HR(md3dDevice->CreateBuffer(&MatrixBufferDesc, NULL, &mMatrixBuffer));

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.初始化阶段就创建采样状态
	HR(md3dDevice->CreateSamplerState(&samplerDesc, &m_sampleState));

	return true;
}

