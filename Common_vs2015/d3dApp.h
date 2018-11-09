//***************************************************************************************
// d3dApp.h by Frank Luna (C) 2011 All Rights Reserved.
//
// Simple Direct3D demo application class.  
// Make sure you link: d3d11.lib d3dx11d.lib D3DCompiler.lib D3DX11EffectsD.lib 
//                     dxerr.lib dxgi.lib dxguid.lib.
// Link d3dx11.lib and D3DX11Effects.lib for release mode builds instead
//   of d3dx11d.lib and D3DX11EffectsD.lib.
//***************************************************************************************

#ifndef D3DAPP_H
#define D3DAPP_H

#include "d3dUtil.h"
#include "GameTimer.h"
#include <string>

class D3DApp
{
public:
	D3DApp(HINSTANCE hInstance);
	virtual ~D3DApp();
	
	HINSTANCE AppInst()const;
	HWND      MainWnd()const;
	float     AspectRatio()const;
	
	int Run();
 
	// Framework methods.  Derived client class overrides these methods to implement specific application requirements.
	// 框架方法。派生类需要重载这些方法实现所需的功能
	virtual bool Init();
	virtual void OnResize(); 
	virtual void UpdateScene(float dt)=0;
	virtual void DrawScene()=0; 
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	// Convenience overrides for handling mouse input.
	// 处理鼠标输入事件的便捷重载函数
	virtual void OnMouseDown(WPARAM btnState, int x, int y){ }
	virtual void OnMouseUp(WPARAM btnState, int x, int y)  { }
	virtual void OnMouseMove(WPARAM btnState, int x, int y){ }

protected:
	bool InitMainWindow();
	bool InitDirect3D();

	// 计算每秒的平均帧数和每帧的平均时间（单位为毫秒）
	void CalculateFrameStats();

protected:

	HINSTANCE mhAppInst;     // 应用程序实例句柄
	HWND      mhMainWnd;     // 主窗口句柄
	bool      mAppPaused;    // 程序是否处在暂停状态
	bool      mMinimized;    // 程序是否最小化
	bool      mMaximized;    // 程序是否最大化
	bool      mResizing;     // 程序是否处在改变大小的状态
	UINT      m4xMsaaQuality;// 4X MSAA质量等级

	GameTimer mTimer;

	ID3D11Device*			md3dDevice;			 //ID3D11Device接口用于检测显示适配器功能和分配资源
	ID3D11DeviceContext*	md3dImmediateContext;//ID3D11DeviceContext接口用于设置管线状态、将资源绑定到图形管线和生成渲染命令
	IDXGISwapChain*			mSwapChain;
	ID3D11Texture2D*		mDepthStencilBuffer;
	ID3D11RenderTargetView* mRenderTargetView;
	ID3D11DepthStencilView* mDepthStencilView;
	D3D11_VIEWPORT mScreenViewport;

	// Derived class should set these in derived constructor to customize starting values.
	std::wstring	mMainWndCaption;
	D3D_DRIVER_TYPE md3dDriverType;
	// 窗口的初始大小。D3DApp默认为800x600。注意，当窗口大小在运行阶段改变时，这些值也会随之改变
	int  mClientWidth;
	int  mClientHeight;
	//  设置为true则使用4XMSAA(§4.1.8)，默认为false
	bool mEnable4xMsaa;
};

#endif // D3DAPP_H