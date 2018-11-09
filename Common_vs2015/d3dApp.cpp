//***************************************************************************************
// d3dApp.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "d3dApp.h"
#include <WindowsX.h>
#include <sstream>

namespace
{
	// This is just used to forward Windows messages from a global window
	// procedure to our member function window procedure because we cannot
	// assign a member function to WNDCLASS::lpfnWndProc.
	D3DApp* gd3dApp = 0;
}

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Forward hwnd on because we can get messages (e.g., WM_CREATE)
	// before CreateWindow returns, and thus before mhMainWnd is valid.
	return gd3dApp->MsgProc(hwnd, msg, wParam, lParam);
}

D3DApp::D3DApp(HINSTANCE hInstance)
:	mhAppInst(hInstance),
	mMainWndCaption(L"D3D11 Application"),
	md3dDriverType(D3D_DRIVER_TYPE_HARDWARE),
	mClientWidth(800),
	mClientHeight(600),
	mEnable4xMsaa(false),
	mhMainWnd(0),
	mAppPaused(false),
	mMinimized(false),
	mMaximized(false),
	mResizing(false),
	m4xMsaaQuality(0),
 
	md3dDevice(0),
	md3dImmediateContext(0),
	mSwapChain(0),
	mDepthStencilBuffer(0),
	mRenderTargetView(0),
	mDepthStencilView(0)
{
	ZeroMemory(&mScreenViewport, sizeof(D3D11_VIEWPORT));

	// Get a pointer to the application object so we can forward 
	// Windows messages to the object's window procedure through
	// the global window procedure.
	gd3dApp = this;
}

D3DApp::~D3DApp()
{
	ReleaseCOM(mRenderTargetView);
	ReleaseCOM(mDepthStencilView);
	ReleaseCOM(mSwapChain);
	ReleaseCOM(mDepthStencilBuffer);

	// Restore all default settings.
	if( md3dImmediateContext )
		md3dImmediateContext->ClearState();

	ReleaseCOM(md3dImmediateContext);
	ReleaseCOM(md3dDevice);
}

HINSTANCE D3DApp::AppInst()const
{
	return mhAppInst;
}

HWND D3DApp::MainWnd()const
{
	return mhMainWnd;
}

float D3DApp::AspectRatio()const
{
	return static_cast<float>(mClientWidth) / mClientHeight;
}

int D3DApp::Run()
{
	MSG msg = {0};
 
	mTimer.Reset();

	while(msg.message != WM_QUIT)
	{
		// If there are Window messages then process them.	������յ�Window��Ϣ��������Щ��Ϣ
		if(PeekMessage( &msg, 0, 0, 0, PM_REMOVE ))
		{
            TranslateMessage( &msg );
            DispatchMessage( &msg );
		}
		// Otherwise, do animation/game stuff.���������ж���/��Ϸ
		else
        {	
			mTimer.Tick();

			if( !mAppPaused )
			{
				CalculateFrameStats();
				UpdateScene(mTimer.DeltaTime());	
				DrawScene();
			}
			else
			{
				Sleep(100);
			}
        }
    }

	return (int)msg.wParam;
}

bool D3DApp::Init()
{
	if(!InitMainWindow())
		return false;

	if(!InitDirect3D())
		return false;

	return true;
}
 
void D3DApp::OnResize()
{
	assert(md3dImmediateContext);
	assert(md3dDevice);
	assert(mSwapChain);

	// Release the old views, as they hold references to the buffers we
	// will be destroying.  Also release the old depth/stencil buffer.
	ReleaseCOM(mRenderTargetView);
	ReleaseCOM(mDepthStencilView);
	ReleaseCOM(mDepthStencilBuffer);

	// 5��Resize the swap chain and recreate the render target view.
	HR(mSwapChain->ResizeBuffers(1, mClientWidth, mClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
	// 5��Ϊ�������ĺ�̨����������һ����ȾĿ����ͼ mRenderTargetView
	// ����������Ϊһ����ȾĿ�����ɫ����Դʱ������ҪΪ������������ͼ����ȾĿ����ͼ��ID3D11RenderTargetView���� ��ɫ����Դ��ͼ��ID3D11ShaderResourceView��
	ID3D11Texture2D* backBuffer;
	
	// IDXGISwapChain::GetBuffer�������ڻ�ȡһ���������ĺ�̨������ָ�롣�÷����ĵ�һ��������ʾ��Ҫ��ȡ�ĺ�̨������������ֵ
	//�����ں�̨���������������Դ���1�������������ָ������ֵ���������ǵ���ʾ�����У�����ֻʹ��һ����̨�����������Ը�����ֵ��Ϊ0��
	// �ڶ��������ǻ������Ľӿ����ͣ���ͨ����һ��2D����ID3D11Texture2D������������������ָ���̨��������ָ��
	HR(mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));

	// ����ʹ��ID3D11Device::CreateRenderTargetView����������ȾĿ����ͼ����һ������ָ���˽�Ҫ��Ϊ��ȾĿ�����Դ���������У�
	// ��ȾĿ���Ǻ�̨����������������Ϊ��̨������������һ����ȾĿ����ͼ�����ڶ���������һ��ָ��D3D11_RENDER_TARGET_VIEW_DESC�ṹ���ָ�룬
	// �ýṹ����������Դ�е�Ԫ�ص��������͡�����ڴ�����Դʱʹ�õ���ĳ��ǿ���͸�ʽ�������������͸�ʽ������ò�������Ϊ�գ���ʾ����Դ�ĵ�һ��mipmap���
	//����̨������Ҳֻ��һ��mipmap��Σ���Ϊ��ͼ��ʽ������������ͨ��ָ�뷵���˴��������ȾĿ����ͼ����
	HR(md3dDevice->CreateRenderTargetView(backBuffer, 0, &mRenderTargetView));

	// ÿ����һ��IDXGISwapChain::GetBuffer��������̨��������COM���ü����ͻ����ϵ���һ�Σ�����������ڴ���Ƭ�εĽ�β���ͷ�����ReleaseCOM����ԭ��
	ReleaseCOM(backBuffer);

	// 6��Create the depth/stencil buffer and view.
	// 6������ ���/ģ�建���� �Լ���ص� ���/ģ����ͼ
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width     = mClientWidth;
	depthStencilDesc.Height    = mClientHeight;
	depthStencilDesc.MipLevels = 1;//�༶��������㣨mipmap level��������
	depthStencilDesc.ArraySize = 1;//���������е������������������/ģ�建������˵������ֻ��Ҫһ������
	depthStencilDesc.Format    = DXGI_FORMAT_D24_UNORM_S8_UINT;//DXGI_FORMAT_D24_UNORM_S8_UINT���޷���24λ��Ȼ�������ÿ�����ֵ��ȡֵ��ΧΪ[0,1]��Ϊģ�建����Ԥ��8λ���޷�����������ÿ��ģ��ֵ��ȡֵ��ΧΪ[0,255]

	// Use 4X MSAA? --must match swap chain MSAA values.
	if( mEnable4xMsaa )
	{	//���ز�����������������
		depthStencilDesc.SampleDesc.Count   = 4;
		depthStencilDesc.SampleDesc.Quality = m4xMsaaQuality-1;
	}
	// No MSAA
	else
	{
		depthStencilDesc.SampleDesc.Count   = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
	}

	depthStencilDesc.Usage          = D3D11_USAGE_DEFAULT;//D3D11_USAGE_DEFAULT����ʾGPU��graphics processing unit��ͼ�δ������������Դִ�ж�д������CPU���ܶ�д������Դ���������/ģ�建����������ʹ��D3D11_USAGE_DEFAULT��־ֵ����ΪGPU��ִ�����ж�д���/ģ�建�����Ĳ���
	depthStencilDesc.BindFlags      = D3D11_BIND_DEPTH_STENCIL;//D3D11_BIND_DEPTH_STENCIL���ñ�־ֵ��ʾ������Ϊһ�����/ģ�建�����󶨵������ϣ�����]��ʹ�øñ�־�����޷�Ϊ����Դ����ID3D11DepthStencilView��ͼ
	depthStencilDesc.CPUAccessFlags = 0;// ָ��CPU����Դ�ķ���Ȩ��
	depthStencilDesc.MiscFlags      = 0;// ��ѡ�ı�־ֵ�������/ģ�建�����޹أ�������Ϊ0

	// CreateTexture2D�ĵڶ���������һ��ָ���ʼ�����ݵ�ָ�룬��Щ��ʼ�����������������������������������������/ģ�建������
	// �������ǲ���ҪΪ������κγ�ʼ�����ݡ���ִ����Ȼ����ģ�����ʱ��Direct3D���Զ������/ģ�建����д�����ݡ����ԣ����������ｫ�ڶ�������ָ��Ϊ��ֵ
	HR(md3dDevice->CreateTexture2D(&depthStencilDesc, 0, &mDepthStencilBuffer));

	// ����һ���󶨵������ϵ����/ģ����ͼ | ΪmDepthStencilBuffer������ID3D11DepthStencilView��ͼ
	// CreateDepthStencilView�ĵڶ���������һ��ָ��D3D11_DEPTH_STENCIL_VIEW_DESC��ָ�롣����ṹ����������Դ�����Ԫ���������ͣ���ʽ����
	// �����Դ��һ�������͵ĸ�ʽ����typeless���������������Ϊ��ֵ����ʾ����һ����Դ�ĵ�һ��mipmap�ȼ�����ͼ�����/ģ�建��Ҳֻ��ʹ��һ�� mipmap�ȼ�����
	// ��Ϊ����ָ�������/ģ�建�壨Texture2D���ĸ�ʽ�����Խ������������Ϊ��ֵ
	HR(md3dDevice->CreateDepthStencilView(mDepthStencilBuffer, 0, &mDepthStencilView));

	// 7��Bind the render target view and depth/stencil view to the pipeline.
	// 7������ȾĿ����ͼ�����/ģ����ͼ�󶨵���Ⱦ���ߵ�����ϲ��׶Σ�ʹ���ǿ��Ա�Direct3Dʹ��
	md3dImmediateContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);
	
	// 8��Set the viewport transform.
	// 8�������ӿ�
	mScreenViewport.TopLeftX = 0;
	mScreenViewport.TopLeftY = 0;
	mScreenViewport.Width    = static_cast<float>(mClientWidth);
	mScreenViewport.Height   = static_cast<float>(mClientHeight);
	mScreenViewport.MinDepth = 0.0f;// ��Ȼ���������Сֵ
	mScreenViewport.MaxDepth = 1.0f;// ��Ȼ����������ֵ
	md3dImmediateContext->RSSetViewports(1, &mScreenViewport);
}
 
LRESULT D3DApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch( msg )
	{
	// WM_ACTIVATE is sent when the window is activated or deactivated.  
	// We pause the game when the window is deactivated and unpause it 
	// when it becomes active.  
	case WM_ACTIVATE:
		if( LOWORD(wParam) == WA_INACTIVE )
		{
			mAppPaused = true;
			mTimer.Stop();
		}
		else
		{
			mAppPaused = false;
			mTimer.Start();
		}
		return 0;

	// WM_SIZE is sent when the user resizes the window.  
	case WM_SIZE:
		// Save the new client area dimensions.
		mClientWidth  = LOWORD(lParam);
		mClientHeight = HIWORD(lParam);
		if( md3dDevice )
		{
			if( wParam == SIZE_MINIMIZED )
			{
				mAppPaused = true;
				mMinimized = true;
				mMaximized = false;
			}
			else if( wParam == SIZE_MAXIMIZED )
			{
				mAppPaused = false;
				mMinimized = false;
				mMaximized = true;
				OnResize();
			}
			else if( wParam == SIZE_RESTORED )
			{
				
				// Restoring from minimized state?
				if( mMinimized )
				{
					mAppPaused = false;
					mMinimized = false;
					OnResize();
				}

				// Restoring from maximized state?
				else if( mMaximized )
				{
					mAppPaused = false;
					mMaximized = false;
					OnResize();
				}
				else if( mResizing )
				{
					// If user is dragging the resize bars, we do not resize 
					// the buffers here because as the user continuously 
					// drags the resize bars, a stream of WM_SIZE messages are
					// sent to the window, and it would be pointless (and slow)
					// to resize for each WM_SIZE message received from dragging
					// the resize bars.  So instead, we reset after the user is 
					// done resizing the window and releases the resize bars, which 
					// sends a WM_EXITSIZEMOVE message.
				}
				else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
				{
					OnResize();
				}
			}
		}
		return 0;

	// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
		mAppPaused = true;
		mResizing  = true;
		mTimer.Stop();
		return 0;

	// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
	// Here we reset everything based on the new window dimensions.
	// ���û������ƶ����ڱ߿�ʱ�ᷢ��WM_EXITSIZEMOVE��Ϣ��Ȼ�����ǻ�����µĴ��ڴ�С��������ͼ�α���
	case WM_EXITSIZEMOVE:
		mAppPaused = false;
		mResizing  = false;
		mTimer.Start();
		OnResize();
		return 0;
 
	// WM_DESTROY is sent when the window is being destroyed.
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	// The WM_MENUCHAR message is sent when a menu is active and the user presses 
	// a key that does not correspond to any mnemonic or accelerator key. 
	// ����û�����Alt��һ����˵��ƥ����ַ�ʱ����������ʾ����ʽ�˵���ʹ���߰���һ���뵯��ʽ�˵������Ŀ��ƥ����ַ���ʱ�� 
	case WM_MENUCHAR:
        // Don't beep when we alt-enter.
        return MAKELRESULT(0, MNC_CLOSE);

	// Catch this message so to prevent the window from becoming too small.
	// ��ֹ���ڱ�ù�С��
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200; 
		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}


bool D3DApp::InitMainWindow()
{
	int posX = CW_USEDEFAULT;
	int posY = CW_USEDEFAULT;

	WNDCLASS wc;
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = MainWndProc; 
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = mhAppInst;
	wc.hIcon         = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor       = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = L"D3DWndClassName";

	if( !RegisterClass(&wc) )
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

#if 0
	// ��ȡϵͳ�ĵ�ǰ�ֱ���
	mClientWidth  = GetSystemMetrics(SM_CXSCREEN);
	mClientHeight = GetSystemMetrics(SM_CYSCREEN);
	
	DEVMODE devMode;
	memset(&devMode, 0, sizeof(devMode));
	devMode.dmSize = sizeof(devMode);
	// ȫ���ķֱ���
	devMode.dmPelsWidth  = mClientWidth;
	devMode.dmPelsHeight = mClientHeight;
	devMode.dmBitsPerPel = 32;
	devMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

	// Change the display settings to full screen.
	ChangeDisplaySettings(&devMode, CDS_FULLSCREEN);

	// Set the position of the window to the top left corner.
	posX = posY = 0;

#endif
#if 0
	//���ô��ھ�����ʾ
	posX = (GetSystemMetrics(SM_CXSCREEN) - mClientWidth)  / 2;
	posY = (GetSystemMetrics(SM_CYSCREEN) - mClientHeight) / 2;
#endif

	// Compute window rectangle dimensions based on requested client area dimensions.
	RECT R = { 0, 0, mClientWidth, mClientHeight };
    AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width  = R.right - R.left;
	int height = R.bottom - R.top;

	mhMainWnd = CreateWindow(L"D3DWndClassName", mMainWndCaption.c_str(), 
		WS_OVERLAPPEDWINDOW, posX, posY, width, height, 0, 0, mhAppInst, 0);
	if( !mhMainWnd )
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(mhMainWnd, SW_SHOW);
	UpdateWindow(mhMainWnd);

	return true;
}

bool D3DApp::InitDirect3D()
{
	// Create the device and device context.

	UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	// 1��ʹ��D3D11CreateDevice��������ID3D11Device��ID3D11DeviceContext��
	D3D_FEATURE_LEVEL featureLevel;
	HRESULT hr = D3D11CreateDevice(
			0,                 // default adapter ָ��ҪΪ�ĸ������Կ������豸���󡣵��ò�����Ϊ��ֵʱ����ʾʹ�����Կ�
			md3dDriverType,	   // һ���������ò�������ָ��ΪD3D_DRIVER_TYPE_HARDWARE����ʾʹ��3DӲ�����ӿ���Ⱦ�ٶ�
			0,                 // no software device �������ǽ��ò�����Ϊ��ֵ����Ϊ����ʹ��Ӳ��������Ⱦ
			createDeviceFlags, // ����releaseģʽ���ɳ���ʱ���ò���ͨ����Ϊ0���޸��ӱ�־ֵ��������debugģʽ���ɳ���ʱ���ò���Ӧ��Ϊ��D3D11_CREATE_DEVICE_DEBUG
			0, 0,              // default feature level array
			D3D11_SDK_VERSION,
			&md3dDevice,	   // ���ش�������豸����
			&featureLevel,	   // ����pFeatureLevels�����е�һ��֧�ֵ������ȼ������pFeatureLevels Ϊnull���򷵻ؿ�֧�ֵ���ߵȼ���
			&md3dImmediateContext);// ���ش�������豸������
	if( FAILED(hr) )
	{
		MessageBox(0, L"D3D11CreateDevice Failed.", 0, 0);
		return false;
	}

	if( featureLevel != D3D_FEATURE_LEVEL_11_0 )
	{
		MessageBox(0, L"Direct3D Feature Level 11 unsupported.", 0, 0);
		return false;
	}

	// Check 4X MSAA quality support for our back buffer format.
	// All Direct3D 11 capable devices support 4X MSAA for all render 
	// target formats, so we only need to check quality support.
	// 2��ʹ��ID3D11Device::CheckMultisampleQualityLevels��������豸֧�ֵ�4X���ز��������ȼ���
	HR(md3dDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m4xMsaaQuality));
	// ��Ϊ4X MSAA���Ǳ�֧�ֵģ����Է��ص������ȼ����Ǵ���0
	assert( m4xMsaaQuality > 0 );

	// 3��Fill out a DXGI_SWAP_CHAIN_DESC to describe our swap chain.
	// 3�����һ��IDXGI_SWAP_CHAIN_DESC�ṹ�壬�ýṹ����������Ҫ�����Ľ�����������
	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width  = mClientWidth;			  // ��̨���������
	sd.BufferDesc.Height = mClientHeight;			  // ��̨�������߶�
	sd.BufferDesc.RefreshRate.Numerator = 60;		  // ��ʾˢ����
	sd.BufferDesc.RefreshRate.Denominator = 1;
	// ��Ϊ�������ʾ����֧�ֳ���24λ���ϵ���ɫ���ٶ����ɫҲ���˷ѣ��������ǽ���̨�����������ظ�ʽ����ΪDXGI_FORMAT_R8G8B8A8_UNORM���졢�̡�����alpha��8λ��
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;// ��̨���������ظ�ʽ
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// Use 4X MSAA? 
	if( mEnable4xMsaa )
	{	//���ز�����������������
		sd.SampleDesc.Count   = 4;
		sd.SampleDesc.Quality = m4xMsaaQuality-1;
	}
	// No MSAA
	else
	{
		sd.SampleDesc.Count   = 1;
		sd.SampleDesc.Quality = 0;
	}

	sd.BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT;//��ΪDXGI_USAGE_RENDER_TARGET_OUTPUT����Ϊ����Ҫ��������Ⱦ����̨����������������������ȾĿ�꣩
	sd.BufferCount  = 1;		//�������еĺ�̨����������
	sd.OutputWindow = mhMainWnd;//���ǽ�Ҫ��Ⱦ���Ĵ��ڵľ��
	sd.Windowed     = true;		//����Ϊtrueʱ�������Դ���ģʽ���У�����Ϊfalseʱ��������ȫ����full-screen��ģʽ����
	sd.SwapEffect   = DXGI_SWAP_EFFECT_DISCARD;//��ΪDXGI_SWAP_EFFECT_DISCARD�����Կ���������ѡ�����Ч����ʾģʽ
	sd.Flags        = 0;

	// To correctly create the swap chain, we must use the IDXGIFactory that was
	// used to create the device.  If we tried to use a different IDXGIFactory instance
	// (by calling CreateDXGIFactory), we get an error: "IDXGIFactory::CreateSwapChain: 
	// This function is being called with a device from a different IDXGIFactory."
	// 4��ʹ�á�COM��ѯ����ѯ����md3dDevice���Ǹ�IDXGIFactoryʵ�������ʵ�����ڴ���md3dDevice��һ��IDXGISwapChainʵ����
	IDXGIDevice* dxgiDevice = 0;
	HR(md3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice));
	      
	IDXGIAdapter* dxgiAdapter = 0;// �Կ��豸�ӿ��࣬��������ö�ٳ���ʾ�豸 (������ʾ��)�����Ի�ȡ�Կ���һЩ��Ϣ
	HR(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter));

	IDXGIFactory* dxgiFactory = 0;// ���Դ����ӿڹ����࣬�ö�������ö�ٳ��Կ��豸
	HR(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory));
	
	// ��������IDXGISwapChain����ͨ��IDXGIFactoryʵ����IDXGIFactory::CreateSwapChain����������
	HR(dxgiFactory->CreateSwapChain(md3dDevice,	&sd, &mSwapChain));
	
	ReleaseCOM(dxgiDevice);
	ReleaseCOM(dxgiAdapter);
	ReleaseCOM(dxgiFactory);

	// The remaining steps that need to be carried out for d3d creation
	// also need to be executed every time the window is resized.  So
	// just call the OnResize method here to avoid code duplication.
	OnResize();

	return true;
}

// ����FPS
void D3DApp::CalculateFrameStats()
{
	// Code computes the average frames per second, and also the 
	// average time it takes to render one frame.  These stats 
	// are appended to the window caption bar.
	// ����ÿ��ƽ��֡���Ĵ��룬�������˻���һ֡��ƽ��ʱ�䣬��Щͳ����Ϣ����ʾ�ڴ��ڱ�������
	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	// Compute averages over one second period.// ����һ��ʱ���ڵ�ƽ��ֵ
	if( (mTimer.TotalTime() - timeElapsed) >= 1.0f )
	{
		float fps = (float)frameCnt; // fps = frameCnt / 1
		float mspf = 1000.0f / fps;

		std::wostringstream outs;   
		outs.precision(6);
		outs << mMainWndCaption << L"    "
			 << L"FPS: " << fps << L"    " 
			 << L"Frame Time: " << mspf << L" (ms)";
		//SetWindowText(mhMainWnd, outs.str().c_str());
		
		// Reset for next average.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}


