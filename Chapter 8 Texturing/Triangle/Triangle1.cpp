// Triangle.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
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
	void InitBuffers();
	bool LoadTarga(const char* filename, int& height, int& width);
	bool LoadTexture(const char* filename);
	bool InitShader(WCHAR* vsFilename, WCHAR* psFilename);

private:
	struct VertexType
	{
		XMFLOAT2 position;
		XMFLOAT2 texture;
	};

	struct TargaHeader
	{
		unsigned char data1[12];
		unsigned short width;
		unsigned short height;
		unsigned char bpp;
		unsigned char data2;
	};

	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

private:
	unsigned char* m_targaData;

	XMMATRIX mProj;
	XMMATRIX mWorldMatrix;
	XMMATRIX mOrthoMatrix;

	ID3D11Buffer* mTriangleVB;
	ID3D11Buffer* mTriangleIB;

	ID3D11Texture2D* mTexture;
	ID3D11ShaderResourceView* mTextureView;

	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;
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
	// 创建投影矩阵
	mProj = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);

	// 这里世界矩阵是一个单位矩阵
	mWorldMatrix = XMMatrixIdentity();

	// 创建一个正交投影矩阵
	mOrthoMatrix = XMMatrixOrthographicLH((float)mClientWidth, (float)mClientHeight, 1.0f, 1000.0f);
}

TriangleApp::~TriangleApp()
{
	;
}

bool TriangleApp::Init()
{
	if (!D3DApp::Init())
		return false;

	InitBuffers();
	LoadTexture("D:\\MyWork\\From-liangdefeng\\DirectX\\dx11\\dx11s2tut05\\dx11s2tut05_src\\data\\stone01.tga");

	return true;
}

void TriangleApp::OnResize()
{
	D3DApp::OnResize();

	// 创建投影矩阵
	mProj = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
}

void TriangleApp::UpdateScene(float dt)
{
	return;
}

void TriangleApp::DrawScene()
{
	//float black[] = { 0, 0, 0, 1 };
	//md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, black);// Clear the back buffer.
	//md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);// Clear the depth buffer.
	//																						   // Set the vertex input layout.
	//md3dImmediateContext->IASetInputLayout(m_layout);


	//XMFLOAT3 up, position, lookAt;
	//XMVECTOR upVector, positionVector, lookAtVector;
	//float yaw, pitch, roll;
	//XMMATRIX rotationMatrix;


	//// Setup the vector that points upwards.
	//up.x = 0.0f;
	//up.y = 1.0f;
	//up.z = 0.0f;

	//// Load it into a XMVECTOR structure.
	//upVector = XMLoadFloat3(&up);

	//// Setup the position of the camera in the world.
	//position.x = m_positionX;
	//position.y = m_positionY;
	//position.z = m_positionZ;

	//// Load it into a XMVECTOR structure.
	//positionVector = XMLoadFloat3(&position);

	//// Setup where the camera is looking by default.
	//lookAt.x = 0.0f;
	//lookAt.y = 0.0f;
	//lookAt.z = 1.0f;

	//// Load it into a XMVECTOR structure.
	//lookAtVector = XMLoadFloat3(&lookAt);

	//// Set the yaw (Y axis), pitch (X axis), and roll (Z axis) rotations in radians.
	//pitch = m_rotationX * 0.0174532925f;
	//yaw = m_rotationY * 0.0174532925f;
	//roll = m_rotationZ * 0.0174532925f;

	//// Create the rotation matrix from the yaw, pitch, and roll values.
	//rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	//// Transform the lookAt and up vector by the rotation matrix so the view is correctly rotated at the origin.
	//lookAtVector = XMVector3TransformCoord(lookAtVector, rotationMatrix);
	//upVector = XMVector3TransformCoord(upVector, rotationMatrix);

	//// Translate the rotated camera position to the location of the viewer.
	//lookAtVector = XMVectorAdd(positionVector, lookAtVector);

	//// Finally create the view matrix from the three updated vectors.
	//m_viewMatrix = XMMatrixLookAtLH(positionVector, lookAtVector, upVector);

	//unsigned int stride;
	//unsigned int offset;


	//// Set vertex buffer stride and offset.
	//stride = sizeof(VertexType);
	//offset = 0;

	//// Set the vertex buffer to active in the input assembler so it can be rendered.
	//deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	//// Set the index buffer to active in the input assembler so it can be rendered.
	//deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	//deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//HRESULT result;
	//D3D11_MAPPED_SUBRESOURCE mappedResource;
	//MatrixBufferType* dataPtr;
	//unsigned int bufferNumber;


	//// Transpose the matrices to prepare them for the shader.
	//worldMatrix = XMMatrixTranspose(worldMatrix);
	//viewMatrix = XMMatrixTranspose(viewMatrix);
	//projectionMatrix = XMMatrixTranspose(projectionMatrix);

	//// Lock the constant buffer so it can be written to.
	//result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	//if (FAILED(result))
	//{
	//	return false;
	//}

	//// Get a pointer to the data in the constant buffer.
	//dataPtr = (MatrixBufferType*)mappedResource.pData;

	//// Copy the matrices into the constant buffer.
	//dataPtr->world = worldMatrix;
	//dataPtr->view = viewMatrix;
	//dataPtr->projection = projectionMatrix;

	//// Unlock the constant buffer.
	//deviceContext->Unmap(m_matrixBuffer, 0);

	//// Set the position of the constant buffer in the vertex shader.
	//bufferNumber = 0;

	//// Finanly set the constant buffer in the vertex shader with the updated values.
	//deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

	//// Set shader texture resource in the pixel shader.
	//deviceContext->PSSetShaderResources(0, 1, &texture);

	//

	//// Set the vertex and pixel shaders that will be used to render this triangle.
	//deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	//deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	//// Set the sampler state in the pixel shader.
	//deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	//// Render the triangle.
	//deviceContext->DrawIndexed(indexCount, 0, 0);

	HR(mSwapChain->Present(0, 0));
	
	return;
}


void TriangleApp::OnMouseDown(WPARAM btnState, int x, int y)
{

}

void TriangleApp::OnMouseUp(WPARAM btnState, int x, int y){}
void TriangleApp::OnMouseMove(WPARAM btnState, int x, int y){}

void TriangleApp::InitBuffers()
{
	VertexType vertices[3] = {
		{ XMFLOAT2( -1.0f, -1.0f ), XMFLOAT2( 0.0f, 1.0f ) },
		{ XMFLOAT2(  0.0f,  1.0f ), XMFLOAT2( 0.5f, 0.0f ) },
		{ XMFLOAT2(  1.0f, -1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
		//{ { 1.0f, -1.0f },{ 1.0f, 1.0f } }
	};

	// 1．填写一个D3D11_BUFFER_DESC结构体，描述我们所要创建的缓冲区
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = sizeof(VertexType) * 3;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;

	// 2．填写一个D3D11_SUBRESOURCE_DATA结构体，为缓冲区指定初始化数据
	D3D11_SUBRESOURCE_DATA vertexData;
	vertexData.pSysMem = vertices;//包含初始化数据的系统内存数组的指针。当缓冲区可以存储n个顶点时，对应的初始化数组也应至少包含n个顶点，从而使整个缓冲区得到初始化
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// 3．调用ID3D11Device::CreateBuffer方法来创建缓冲区
	md3dDevice->CreateBuffer(&vbd, &vertexData, &mTriangleVB);

	UINT indices[] = {
		0, 1, 2
	};

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = sizeof(UINT) * 3;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	
	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	md3dDevice->CreateBuffer(&ibd, &indexData, &mTriangleIB);
}

bool TriangleApp::LoadTexture(const char* filename)
{
	bool result;
	int height, width;
	D3D11_TEXTURE2D_DESC textureDesc;
	HRESULT hResult;
	unsigned int rowPitch;
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;


	// Load the targa image data into memory.
	result = LoadTarga(filename, height, width);
	if (!result)
	{
		return false;
	}

	// Setup the description of the texture.
	textureDesc.Height = height;
	textureDesc.Width = width;
	textureDesc.MipLevels = 0;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	// Create the empty texture.
	hResult = md3dDevice->CreateTexture2D(&textureDesc, NULL, &mTexture);
	if (FAILED(hResult))
	{
		return false;
	}

	// Set the row pitch of the targa image data.
	rowPitch = (width * 4) * sizeof(unsigned char);

	// Copy the targa image data into the texture.
	md3dImmediateContext->UpdateSubresource(mTexture, 0, NULL, m_targaData, rowPitch, 0);

	// Setup the shader resource view description.
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;

	// Create the shader resource view for the texture.
	hResult = md3dDevice->CreateShaderResourceView(mTexture, &srvDesc, &mTextureView);
	if (FAILED(hResult))
	{
		return false;
	}

	// Generate mipmaps for this texture.
	md3dImmediateContext->GenerateMips(mTextureView);

	// Release the targa image data now that the image data has been loaded into the texture.
	delete[] m_targaData;
	m_targaData = 0;

	return true;
}

bool TriangleApp::LoadTarga(const char* filename, int& height, int& width)
{
	int error, bpp, imageSize, index, i, j, k;
	FILE* filePtr;
	unsigned int count;
	TargaHeader targaFileHeader;
	unsigned char* targaImage;


	// Open the targa file for reading in binary.
	error = fopen_s(&filePtr, filename, "rb");
	if (error != 0)
	{
		return false;
	}

	// Read in the file header.
	count = (unsigned int)fread(&targaFileHeader, sizeof(TargaHeader), 1, filePtr);
	if (count != 1)
	{
		return false;
	}

	// Get the important information from the header.
	height = (int)targaFileHeader.height;
	width = (int)targaFileHeader.width;
	bpp = (int)targaFileHeader.bpp;

	// Check that it is 32 bit and not 24 bit.
	if (bpp != 32)
	{
		return false;
	}

	// Calculate the size of the 32 bit image data.
	imageSize = width * height * 4;

	// Allocate memory for the targa image data.
	targaImage = new unsigned char[imageSize];
	if (!targaImage)
	{
		return false;
	}

	// Read in the targa image data.
	count = (unsigned int)fread(targaImage, 1, imageSize, filePtr);
	if (count != imageSize)
	{
		return false;
	}

	// Close the file.
	error = fclose(filePtr);
	if (error != 0)
	{
		return false;
	}

	// Allocate memory for the targa destination data.
	m_targaData = new unsigned char[imageSize];
	if (!m_targaData)
	{
		return false;
	}

	// Initialize the index into the targa destination data array.
	index = 0;

	// Initialize the index into the targa image data.
	k = (width * height * 4) - (width * 4);

	// Now copy the targa image data into the targa destination array in the correct order since the targa format is stored upside down.
	for (j = 0; j < height; j++)
	{
		for (i = 0; i < width; i++)
		{
			m_targaData[index + 0] = targaImage[k + 2];  // Red.
			m_targaData[index + 1] = targaImage[k + 1];  // Green.
			m_targaData[index + 2] = targaImage[k + 0];  // Blue
			m_targaData[index + 3] = targaImage[k + 3];  // Alpha

														 // Increment the indexes into the targa data.
			k += 4;
			index += 4;
		}

		// Set the targa image data index back to the preceding row at the beginning of the column since its reading it in upside down.
		k -= (width * 8);
	}

	// Release the targa image data now that it was copied into the destination array.
	delete[] targaImage;
	targaImage = 0;

	return true;
}


bool TriangleApp::InitShader(WCHAR* vsFilename, WCHAR* psFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	unsigned int numElements;
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;


	// Initialize the pointers this function will use to null.
	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

	// Compile the vertex shader code.
	result = D3DCompileFromFile(vsFilename, NULL, NULL, "TextureVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		&vertexShaderBuffer, &errorMessage);
	//if (FAILED(result))
	//{
	//	// If the shader failed to compile it should have writen something to the error message.
	//	if (errorMessage)
	//	{
	//		//OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
	//	}
	//	// If there was nothing in the error message then it simply could not find the shader file itself.
	//	else
	//	{
	//		MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
	//	}

	//	return false;
	//}

	// Compile the pixel shader code.
	result = D3DCompileFromFile(psFilename, NULL, NULL, "TexturePixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		&pixelShaderBuffer, &errorMessage);
	//if (FAILED(result))
	//{
	//	// If the shader failed to compile it should have writen something to the error message.
	//	if (errorMessage)
	//	{
	//		OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
	//	}
	//	// If there was nothing in the error message then it simply could not find the file itself.
	//	else
	//	{
	//		MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
	//	}

	//	return false;
	//}

	// Create the vertex shader from the buffer.
	result = md3dDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if (FAILED(result))
	{
		return false;
	}

	// Create the pixel shader from the buffer.
	result = md3dDevice->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if (FAILED(result))
	{
		return false;
	}

	// Create the vertex input layout description.
	// This setup needs to match the VertexType stucture in the ModelClass and in the shader.
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	// Get a count of the elements in the layout.
	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	result = md3dDevice->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), &m_layout);
	if (FAILED(result))
	{
		return false;
	}

	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = md3dDevice->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if (FAILED(result))
	{
		return false;
	}

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

	// Create the texture sampler state.
	result = md3dDevice->CreateSamplerState(&samplerDesc, &m_sampleState);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}
