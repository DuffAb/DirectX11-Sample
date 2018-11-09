//***************************************************************************************
// BoxDemo.cpp by Frank Luna (C) 2011 All Rights Reserved.
//
// Demonstrates rendering a colored box.
//
// Controls:
//		Hold the left mouse button down and move the mouse to rotate.
//      Hold the right mouse button down to zoom in and out.
//
//***************************************************************************************

#include "d3dApp.h"
#include "d3dx11Effect.h"
#include "MathHelper.h"

#include <d3dcompiler.h>

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

class BoxApp : public D3DApp
{
public:
	BoxApp(HINSTANCE hInstance);
	~BoxApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene(); 

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BuildGeometryBuffers();
	void BuildFX();
	void BuildVertexLayout();

private:
	ID3D11Buffer* mBoxVB;
	ID3D11Buffer* mBoxIB;

	ID3DX11Effect*				 mFX;
	ID3DX11EffectTechnique*		 mTech;
	ID3DX11EffectMatrixVariable* mfxWorldViewProj;

	ID3D11InputLayout* mInputLayout;

	XMFLOAT4X4 mWorld;
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
				   PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	BoxApp theApp(hInstance);
	
	if( !theApp.Init() )
		return 0;
	
	return theApp.Run();
}
 

BoxApp::BoxApp(HINSTANCE hInstance)
: D3DApp(hInstance), mBoxVB(0), mBoxIB(0), mFX(0), mTech(0),
  mfxWorldViewProj(0), mInputLayout(0), 
  mTheta(1.5f*MathHelper::Pi), mPhi(0.25f*MathHelper::Pi), mRadius(5.0f)
{
	mMainWndCaption = L"Box Demo";
	
	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);
}

BoxApp::~BoxApp()
{
	ReleaseCOM(mBoxVB);
	ReleaseCOM(mBoxIB);
	ReleaseCOM(mFX);
	ReleaseCOM(mInputLayout);
}

bool BoxApp::Init()
{
	if(!D3DApp::Init())
		return false;

	BuildGeometryBuffers();
	BuildFX();
	BuildVertexLayout();

	return true;
}

void BoxApp::OnResize()
{
	D3DApp::OnResize();

	// The window resized, so update the aspect ratio and recompute the projection matrix.
	// 当窗口大小改变时，需要更新横纵比，并重新计算投影矩阵
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void BoxApp::UpdateScene(float dt)
{
	// Convert Spherical to Cartesian coordinates.将球面转换为笛卡尔坐标。
	float x = mRadius*sinf(mPhi)*cosf(mTheta);
	float z = mRadius*sinf(mPhi)*sinf(mTheta);
	float y = mRadius*cosf(mPhi);

	// Build the view matrix.创建视矩阵
	XMVECTOR pos    = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up     = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);
}

void BoxApp::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

	// 创建了输入布局对象之后，它不会自动绑定到设备上，我们必须调用下面的语句来实现绑定。
	md3dImmediateContext->IASetInputLayout(mInputLayout);
    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Vertex);
    UINT offset = 0;
	// 在创建顶点缓冲区后，我们必须把它绑定到设备的输入槽上，只有这样才能将顶点送入管线，这一工作使用如下方法完成。
	// 1．StartSlot：顶点缓冲区所要绑定的起始输入槽。一共有16个输入槽，索引依次为0到15。
	// 2．NumBuffers：顶点缓冲区所要绑定的输入槽的数量，如果起始输入槽为索引k，我们绑定了n个缓冲，那么缓冲将绑定在索引为Ik，Ik + 1……Ik + n - 1的输入槽上。
	// 3．ppVertexBuffers：指向顶点缓冲区数组的第一个元素的指针。
	// 4．pStrides：指向步长数组的第一个元素的指针（该数组的每个元素对应一个顶点缓冲区，也就是，第i个步长对应于第i个顶点缓冲区）。这个步长是指顶点缓冲区中的元素的字节长度。
	// 5．pOffsets：指向偏移数组的第一个元素的指针（该数组的每个元素对应一个顶点缓冲区，也就是，第i个偏移量对应于第i个顶点缓冲区）。这个偏移量是指从顶点缓冲区的起始位置开始，到输入装配阶段将要开始读取数据的位置之间的字节长度。当希望跳过顶点缓冲区前面的一部分数据时，可以使用该参数
    md3dImmediateContext->IASetVertexBuffers(0, 1, &mBoxVB, &stride, &offset);
	
	// 我们使用ID3D11DeviceContext::IASetIndexBuffer方法将一个索引缓冲区绑定到输入装配阶段(绑定到管线上)
	md3dImmediateContext->IASetIndexBuffer(mBoxIB, DXGI_FORMAT_R32_UINT, 0);

	// Set constants
	XMMATRIX world = XMLoadFloat4x4(&mWorld);
	XMMATRIX view  = XMLoadFloat4x4(&mView);
	XMMATRIX proj  = XMLoadFloat4x4(&mProj);
	XMMATRIX worldViewProj = world*view*proj;

	mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));

    D3DX11_TECHNIQUE_DESC techDesc;
    mTech->GetDesc( &techDesc );
    for(UINT p = 0; p < techDesc.Passes; ++p)
    {
		//ID3D11EffectTechnique::GetPassByIndex方法返回一个指定索引的pass对象的ID3D11EffectPass接口指针。
		// Apply方法更新存储在GPU内存中的常量缓冲、将着色器程序绑定到管线、并启用在pass中指定的各种渲染状态
        mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
        
		// 36 indices for the box.
		// 1．IndexCount：在当前绘图操作中使用的索引的数量。在一次绘图操作中不一定使用索引缓冲区中的全部索引；也就是说，我们可以绘制索引的一个连续子集。
		// 2．StartIndexLocation：指定从索引缓冲区的哪个位置开始读取索引数据。
		// 3．BaseVertexLocation：在绘图调用中与索引相加的一个整数
		md3dImmediateContext->DrawIndexed(36, 0, 0);
    }

	HR(mSwapChain->Present(0, 0));
}

void BoxApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void BoxApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void BoxApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if( (btnState & MK_LBUTTON) != 0 )
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		// Update angles based on input to orbit camera around box.
		mTheta += dx;
		mPhi   += dy;

		// Restrict the angle mPhi.
		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi-0.1f);
	}
	else if( (btnState & MK_RBUTTON) != 0 )
	{
		// Make each pixel correspond to 0.005 unit in the scene.
		float dx = 0.005f*static_cast<float>(x - mLastMousePos.x);
		float dy = 0.005f*static_cast<float>(y - mLastMousePos.y);

		// Update the camera radius based on input.
		mRadius += dx - dy;

		// Restrict the radius.
		mRadius = MathHelper::Clamp(mRadius, 3.0f, 15.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void BoxApp::BuildGeometryBuffers()
{
	// Create vertex buffer
    Vertex vertices[] =
    {
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4((const float*)&Colors::White)   },
		{ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4((const float*)&Colors::Black)   },
		{ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4((const float*)&Colors::Red)     },
		{ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4((const float*)&Colors::Green)   },
		{ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4((const float*)&Colors::Blue)    },
		{ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4((const float*)&Colors::Yellow)  },
		{ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4((const float*)&Colors::Cyan)    },
		{ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4((const float*)&Colors::Magenta) }
    };

	// 1．填写一个D3D11_BUFFER_DESC结构体，描述我们所要创建的缓冲区
    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;		 //表示在创建资源后，资源中的内容不会改变
    vbd.ByteWidth = sizeof(Vertex) * 8;		 //我们将要创建的顶点缓冲区的大小，单位为字节
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;//对于顶点缓冲区，该参数应设为D3D11_BIND_VERTEX_BUFFER
    vbd.CPUAccessFlags = 0;		//指定CPU对资源的访问权限。设置为0则表示CPU无需读写缓冲
    vbd.MiscFlags = 0;			//我们不需要为顶点缓冲区指定任何杂项（miscellaneous）标志值，所以该参数设为0
	vbd.StructureByteStride = 0;//存储在结构化缓冲中的一个元素的大小，以字节为单位。这个属性只用于结构化缓冲，其他缓冲可以设置为0。所谓结构化缓冲，是指存储其中的元素大小都相等的缓冲
	
	// 2．填写一个D3D11_SUBRESOURCE_DATA结构体，为缓冲区指定初始化数据
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = vertices;//包含初始化数据的系统内存数组的指针。当缓冲区可以存储n个顶点时，对应的初始化数组也应至少包含n个顶点，从而使整个缓冲区得到初始化

	// 3．调用ID3D11Device::CreateBuffer方法来创建缓冲区
    HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mBoxVB));


	// Create the index buffer

	UINT indices[] = {
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3, 
		4, 3, 7
	};

	D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT) * 36;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = indices;
    HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mBoxIB));
}
 
void BoxApp::BuildFX()
{
	DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
    shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif
 
	//ID3D10Blob只是一个通用内存块，它有两个方法：
	//（a）LPVOID GetBufferPointer：返回指向数据的一个void*，所以在使用时应该对它执行相应的类型转换
	//（b）SIZE_T GetBufferSize：返回缓冲的大小，以字节为单位
	ID3DBlob* compiledShader = 0;
	ID3DBlob* compilationMsgs = 0;

	/*HRESULT hr = D3DX11CompileFromFile(L"FX/color.fx", 0, 0, 0, "fx_5_0", shaderFlags, 
		0, 0, &compiledShader, &compilationMsgs, 0);*/
	HRESULT hr = D3DCompileFromFile(L"FX/color.fx", 0, 0, 0, "fx_5_0", shaderFlags,	0, &compiledShader, &compilationMsgs);

	// compilationMsgs can store errors or warnings.
	if( compilationMsgs != 0 )
	{
		MessageBoxA(0, (char*)compilationMsgs->GetBufferPointer(), 0, 0);
		ReleaseCOM(compilationMsgs);
	}

	// Even if there are no compilationMsgs, check to make sure there were no other errors.
	if(FAILED(hr))
	{
		DXTrace(__FILEW__, (DWORD)__LINE__, hr, L"D3DCompileFromFile", true);
	}

	HR(D3DX11CreateEffectFromMemory(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), 0, md3dDevice, &mFX));

	// Done with compiled shader.
	ReleaseCOM(compiledShader);

	mTech    = mFX->GetTechniqueByName("ColorTech");
	// ID3D11Effect::GetVariableByName方法返回一个ID3D11EffectVariable指针。它是一种通用effect变量类型；
	// 要获得指向特定类型变量的指针（例如，矩阵、向量、标量），你必须使用相应的As-方法（例如，AsMatrix、AsVector、AsScalar）
	mfxWorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
}

void BoxApp::BuildVertexLayout()
{
	// Create the vertex input layout.
	// D3D11_INPUT_ELEMENT_DESC 数组中的每个元素描述了顶点结构体的一个分量，称为输入布局描述
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	// Create the input layout
    D3DX11_PASS_DESC passDesc;
    mTech->GetPassByIndex(0)->GetDesc(&passDesc);

	// 指定了输入布局描述之后，我们就可以使用ID3D11Device::CreateInputLayout方法获取一个表示输入布局的ID3D11InputLayout接口的指针
	// 1．pInputElementDescs：一个用于描述顶点结构体的D3D11_INPUT_ELEMENT_DESC数组。
	// 2．NumElements：D3D11_INPUT_ELEMENT_DESC数组的元素数量。
	// 3．pShaderBytecodeWithInputSignature：指向顶点着色器参数的字节码的指针。
	// 4．BytecodeLength：顶点着色器参数的字节码长度，单位为字节。
	// 5．ppInputLayout：返回创建后的ID3D11InputLayout指针。
	HR(md3dDevice->CreateInputLayout(vertexDesc, 2, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &mInputLayout));
}
 