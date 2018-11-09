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
		// If there are Window messages then process them.	如果接收到Window消息，则处理这些消息
		if(PeekMessage( &msg, 0, 0, 0, PM_REMOVE ))
		{
            TranslateMessage( &msg );
            DispatchMessage( &msg );
		}
		// Otherwise, do animation/game stuff.否则，则运行动画/游戏
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

	// 5．Resize the swap chain and recreate the render target view.
	HR(mSwapChain->ResizeBuffers(1, mClientWidth, mClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
	// 5．为交换链的后台缓冲区创建一个渲染目标视图 mRenderTargetView
	// 当把纹理作为一个渲染目标和着色器资源时，我们要为它创建两种视图：渲染目标视图（ID3D11RenderTargetView）和 着色器资源视图（ID3D11ShaderResourceView）
	ID3D11Texture2D* backBuffer;
	
	// IDXGISwapChain::GetBuffer方法用于获取一个交换链的后台缓冲区指针。该方法的第一个参数表示所要获取的后台缓冲区的索引值
	//（由于后台缓冲区的数量可以大于1，所以这里必须指定索引值）。在我们的演示程序中，我们只使用一个后台缓冲区，所以该索引值设为0。
	// 第二个参数是缓冲区的接口类型，它通常是一个2D纹理（ID3D11Texture2D）。第三个参数返回指向后台缓冲区的指针
	HR(mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));

	// 我们使用ID3D11Device::CreateRenderTargetView方法创建渲染目标视图。第一个参数指定了将要作为渲染目标的资源，在例子中，
	// 渲染目标是后台缓冲区（即，我们为后台缓冲区创建了一个渲染目标视图）。第二个参数是一个指向D3D11_RENDER_TARGET_VIEW_DESC结构体的指针，
	// 该结构体描述了资源中的元素的数据类型。如果在创建资源时使用的是某种强类型格式（即，非弱类型格式），则该参数可以为空，表示以资源的第一个mipmap层次
	//（后台缓冲区也只有一个mipmap层次）作为视图格式。第三个参数通过指针返回了创建后的渲染目标视图对象
	HR(md3dDevice->CreateRenderTargetView(backBuffer, 0, &mRenderTargetView));

	// 每调用一次IDXGISwapChain::GetBuffer方法，后台缓冲区的COM引用计数就会向上递增一次，这便是我们在代码片段的结尾处释放它（ReleaseCOM）的原因
	ReleaseCOM(backBuffer);

	// 6．Create the depth/stencil buffer and view.
	// 6．创建 深度/模板缓冲区 以及相关的 深度/模板视图
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width     = mClientWidth;
	depthStencilDesc.Height    = mClientHeight;
	depthStencilDesc.MipLevels = 1;//多级渐近纹理层（mipmap level）的数量
	depthStencilDesc.ArraySize = 1;//纹理数组中的纹理数量。对于深度/模板缓冲区来说，我们只需要一个纹理
	depthStencilDesc.Format    = DXGI_FORMAT_D24_UNORM_S8_UINT;//DXGI_FORMAT_D24_UNORM_S8_UINT：无符号24位深度缓冲区，每个深度值的取值范围为[0,1]。为模板缓冲区预留8位（无符号整数），每个模板值的取值范围为[0,255]

	// Use 4X MSAA? --must match swap chain MSAA values.
	if( mEnable4xMsaa )
	{	//多重采样数量和质量级别
		depthStencilDesc.SampleDesc.Count   = 4;
		depthStencilDesc.SampleDesc.Quality = m4xMsaaQuality-1;
	}
	// No MSAA
	else
	{
		depthStencilDesc.SampleDesc.Count   = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
	}

	depthStencilDesc.Usage          = D3D11_USAGE_DEFAULT;//D3D11_USAGE_DEFAULT：表示GPU（graphics processing unit，图形处理器）会对资源执行读写操作。CPU不能读写这种资源。对于深度/模板缓冲区，我们使用D3D11_USAGE_DEFAULT标志值，因为GPU会执行所有读写深度/模板缓冲区的操作
	depthStencilDesc.BindFlags      = D3D11_BIND_DEPTH_STENCIL;//D3D11_BIND_DEPTH_STENCIL：该标志值表示纹理将作为一个深度/模板缓冲区绑定到管线上，如果沒有使用该标志，就无法为该资源创建ID3D11DepthStencilView视图
	depthStencilDesc.CPUAccessFlags = 0;// 指定CPU对资源的访问权限
	depthStencilDesc.MiscFlags      = 0;// 可选的标志值，与深度/模板缓冲区无关，所以设为0

	// CreateTexture2D的第二个参数是一个指向初始化数据的指针，这些初始化数据用来填充纹理。不过，由于这个纹理被用作深度/模板缓冲区，
	// 所以我们不需要为它填充任何初始化数据。当执行深度缓存和模板操作时，Direct3D会自动向深度/模板缓冲区写入数据。所以，我们在这里将第二个参数指定为空值
	HR(md3dDevice->CreateTexture2D(&depthStencilDesc, 0, &mDepthStencilBuffer));

	// 创建一个绑定到管线上的深度/模板视图 | 为mDepthStencilBuffer纹理创建ID3D11DepthStencilView视图
	// CreateDepthStencilView的第二个参数是一个指向D3D11_DEPTH_STENCIL_VIEW_DESC的指针。这个结构体描述了资源中这个元素数据类型（格式）。
	// 如果资源是一个有类型的格式（非typeless），这个参数可以为空值，表示创建一个资源的第一个mipmap等级的视图（深度/模板缓冲也只能使用一个 mipmap等级）。
	// 因为我们指定了深度/模板缓冲（Texture2D）的格式，所以将这个参数设置为空值
	HR(md3dDevice->CreateDepthStencilView(mDepthStencilBuffer, 0, &mDepthStencilView));

	// 7．Bind the render target view and depth/stencil view to the pipeline.
	// 7．将渲染目标视图和深度/模板视图绑定到渲染管线的输出合并阶段，使它们可以被Direct3D使用
	md3dImmediateContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);
	
	// 8．Set the viewport transform.
	// 8．设置视口
	mScreenViewport.TopLeftX = 0;
	mScreenViewport.TopLeftY = 0;
	mScreenViewport.Width    = static_cast<float>(mClientWidth);
	mScreenViewport.Height   = static_cast<float>(mClientHeight);
	mScreenViewport.MinDepth = 0.0f;// 深度缓冲区的最小值
	mScreenViewport.MaxDepth = 1.0f;// 深度缓冲区的最大值
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
	// 当用户结束移动窗口边框时会发送WM_EXITSIZEMOVE消息，然后我们会基于新的窗口大小重置所有图形变量
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
	// 如果用户按下Alt和一个与菜单项不匹配的字符时，或者在显示弹出式菜单而使用者按下一个与弹出式菜单里的项目不匹配的字符键时。 
	case WM_MENUCHAR:
        // Don't beep when we alt-enter.
        return MAKELRESULT(0, MNC_CLOSE);

	// Catch this message so to prevent the window from becoming too small.
	// 防止窗口变得过小。
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
	// 获取系统的当前分辨率
	mClientWidth  = GetSystemMetrics(SM_CXSCREEN);
	mClientHeight = GetSystemMetrics(SM_CYSCREEN);
	
	DEVMODE devMode;
	memset(&devMode, 0, sizeof(devMode));
	devMode.dmSize = sizeof(devMode);
	// 全屏的分辨率
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
	//设置窗口举重显示
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
	// 1．使用D3D11CreateDevice方法创建ID3D11Device和ID3D11DeviceContext。
	D3D_FEATURE_LEVEL featureLevel;
	HRESULT hr = D3D11CreateDevice(
			0,                 // default adapter 指定要为哪个物理显卡创建设备对象。当该参数设为空值时，表示使用主显卡
			md3dDriverType,	   // 一般来讲，该参数总是指定为D3D_DRIVER_TYPE_HARDWARE，表示使用3D硬件来加快渲染速度
			0,                 // no software device 我们总是将该参数设为空值，因为我们使用硬件进行渲染
			createDeviceFlags, // 当以release模式生成程序时，该参数通常设为0（无附加标志值）；当以debug模式生成程序时，该参数应设为：D3D11_CREATE_DEVICE_DEBUG
			0, 0,              // default feature level array
			D3D11_SDK_VERSION,
			&md3dDevice,	   // 返回创建后的设备对象
			&featureLevel,	   // 返回pFeatureLevels数组中第一个支持的特征等级（如果pFeatureLevels 为null，则返回可支持的最高等级）
			&md3dImmediateContext);// 返回创建后的设备上下文
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
	// 2．使用ID3D11Device::CheckMultisampleQualityLevels方法检测设备支持的4X多重采样质量等级。
	HR(md3dDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m4xMsaaQuality));
	// 因为4X MSAA总是被支持的，所以返回的质量等级总是大于0
	assert( m4xMsaaQuality > 0 );

	// 3．Fill out a DXGI_SWAP_CHAIN_DESC to describe our swap chain.
	// 3．填充一个IDXGI_SWAP_CHAIN_DESC结构体，该结构体描述了所要创建的交换链的特性
	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width  = mClientWidth;			  // 后台缓冲区宽度
	sd.BufferDesc.Height = mClientHeight;			  // 后台缓冲区高度
	sd.BufferDesc.RefreshRate.Numerator = 60;		  // 显示刷新率
	sd.BufferDesc.RefreshRate.Denominator = 1;
	// 因为大多数显示器不支持超过24位以上的颜色，再多的颜色也是浪费，所以我们将后台缓冲区的像素格式设置为DXGI_FORMAT_R8G8B8A8_UNORM（红、绿、蓝、alpha各8位）
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;// 后台缓冲区像素格式
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// Use 4X MSAA? 
	if( mEnable4xMsaa )
	{	//多重采样数量和质量级别
		sd.SampleDesc.Count   = 4;
		sd.SampleDesc.Quality = m4xMsaaQuality-1;
	}
	// No MSAA
	else
	{
		sd.SampleDesc.Count   = 1;
		sd.SampleDesc.Quality = 0;
	}

	sd.BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT;//设为DXGI_USAGE_RENDER_TARGET_OUTPUT，因为我们要将场景渲染到后台缓冲区（即，将它用作渲染目标）
	sd.BufferCount  = 1;		//交换链中的后台缓冲区数量
	sd.OutputWindow = mhMainWnd;//我们将要渲染到的窗口的句柄
	sd.Windowed     = true;		//当设为true时，程序以窗口模式运行；当设为false时，程序以全屏（full-screen）模式运行
	sd.SwapEffect   = DXGI_SWAP_EFFECT_DISCARD;//设为DXGI_SWAP_EFFECT_DISCARD，让显卡驱动程序选择最高效的显示模式
	sd.Flags        = 0;

	// To correctly create the swap chain, we must use the IDXGIFactory that was
	// used to create the device.  If we tried to use a different IDXGIFactory instance
	// (by calling CreateDXGIFactory), we get an error: "IDXGIFactory::CreateSwapChain: 
	// This function is being called with a device from a different IDXGIFactory."
	// 4．使用“COM查询”查询创建md3dDevice的那个IDXGIFactory实例，这个实例用于创建md3dDevice和一个IDXGISwapChain实例。
	IDXGIDevice* dxgiDevice = 0;
	HR(md3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice));
	      
	IDXGIAdapter* dxgiAdapter = 0;// 显卡设备接口类，可以用来枚举出显示设备 (比如显示器)，可以获取显卡的一些信息
	HR(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter));

	IDXGIFactory* dxgiFactory = 0;// 用以创建接口工厂类，该对象用以枚举出显卡设备
	HR(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory));
	
	// 交换链（IDXGISwapChain）是通过IDXGIFactory实例的IDXGIFactory::CreateSwapChain方法创建的
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

// 计算FPS
void D3DApp::CalculateFrameStats()
{
	// Code computes the average frames per second, and also the 
	// average time it takes to render one frame.  These stats 
	// are appended to the window caption bar.
	// 计算每秒平均帧数的代码，还计算了绘制一帧的平均时间，这些统计信息会显示在窗口标题栏中
	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	// Compute averages over one second period.// 计算一秒时间内的平均值
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


