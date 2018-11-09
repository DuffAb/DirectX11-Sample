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
	// ��ܷ�������������Ҫ������Щ����ʵ������Ĺ���
	virtual bool Init();
	virtual void OnResize(); 
	virtual void UpdateScene(float dt)=0;
	virtual void DrawScene()=0; 
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	// Convenience overrides for handling mouse input.
	// ������������¼��ı�����غ���
	virtual void OnMouseDown(WPARAM btnState, int x, int y){ }
	virtual void OnMouseUp(WPARAM btnState, int x, int y)  { }
	virtual void OnMouseMove(WPARAM btnState, int x, int y){ }

protected:
	bool InitMainWindow();
	bool InitDirect3D();

	// ����ÿ���ƽ��֡����ÿ֡��ƽ��ʱ�䣨��λΪ���룩
	void CalculateFrameStats();

protected:

	HINSTANCE mhAppInst;     // Ӧ�ó���ʵ�����
	HWND      mhMainWnd;     // �����ھ��
	bool      mAppPaused;    // �����Ƿ�����ͣ״̬
	bool      mMinimized;    // �����Ƿ���С��
	bool      mMaximized;    // �����Ƿ����
	bool      mResizing;     // �����Ƿ��ڸı��С��״̬
	UINT      m4xMsaaQuality;// 4X MSAA�����ȼ�

	GameTimer mTimer;

	ID3D11Device*			md3dDevice;			 //ID3D11Device�ӿ����ڼ����ʾ���������ܺͷ�����Դ
	ID3D11DeviceContext*	md3dImmediateContext;//ID3D11DeviceContext�ӿ��������ù���״̬������Դ�󶨵�ͼ�ι��ߺ�������Ⱦ����
	IDXGISwapChain*			mSwapChain;
	ID3D11Texture2D*		mDepthStencilBuffer;
	ID3D11RenderTargetView* mRenderTargetView;
	ID3D11DepthStencilView* mDepthStencilView;
	D3D11_VIEWPORT mScreenViewport;

	// Derived class should set these in derived constructor to customize starting values.
	std::wstring	mMainWndCaption;
	D3D_DRIVER_TYPE md3dDriverType;
	// ���ڵĳ�ʼ��С��D3DAppĬ��Ϊ800x600��ע�⣬�����ڴ�С�����н׶θı�ʱ����ЩֵҲ����֮�ı�
	int  mClientWidth;
	int  mClientHeight;
	//  ����Ϊtrue��ʹ��4XMSAA(��4.1.8)��Ĭ��Ϊfalse
	bool mEnable4xMsaa;
};

#endif // D3DAPP_H