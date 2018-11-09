//***************************************************************************************
// d3dApp.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "GraphicsCard.h"
#include <WindowsX.h>
#include <sstream>


GraphicsCard::GraphicsCard() : mNumModes(0), mDisplayModeList(NULL), mVideoCardMemory(0)
{
	InitGraphicsCard();
}

GraphicsCard::~GraphicsCard()
{
	if (mDisplayModeList)
	{
		delete[] mDisplayModeList;
		mDisplayModeList = NULL;
	}
	
}

bool GraphicsCard::InitGraphicsCard()
{
	HRESULT ret;

	// 用以创建工厂接口，该对象用以枚举出显卡设备
	IDXGIFactory * dxgiFactory = 0;
	ret = CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)&dxgiFactory);

	// Use the factory to create an adapter for the primary graphics interface (video card).
	// 用于枚举显卡，第一个参数代表了要枚举的适配器的索引，获取到的枚举适配器（显卡）放入 dxgiAdapter
	IDXGIAdapter* dxgiAdapter = 0;
	ret = dxgiFactory->EnumAdapters(0, &dxgiAdapter);//枚举适配器（显卡）
	ret = dxgiAdapter->GetDesc(&mDxgiAdapterDesc);	 //获取显卡描述信息

	// Enumerate the primary adapter output (monitor).枚举出 显示器
	IDXGIOutput* dxgiAdapterOutput; // 显示器设备接口类，可以根据给定的rgba格式查询支持的显示模式 (分辨率和刷新率)
	ret = dxgiAdapter->EnumOutputs(0, &dxgiAdapterOutput);

	// Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).
	// 获取符合DXGI_FORMAT_R8G8B8A8_UNORM模式的数量
	ret = dxgiAdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &mNumModes, NULL);

	// 创建一个列表用来保存当前显示器和显卡组合可以支持的显示模式
	mDisplayModeList = new DXGI_MODE_DESC[mNumModes];

	// Now fill the display mode list structures.
	ret = dxgiAdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &mNumModes, mDisplayModeList);


	dxgiAdapterOutput->Release();
	dxgiAdapter->Release();
	dxgiFactory->Release();

	dxgiAdapterOutput = NULL;
	dxgiAdapter = NULL;
	dxgiFactory = NULL;

	return true;
}

UINT GraphicsCard::GetModesNum()
{
	return mNumModes;
}

std::wstring GraphicsCard::GetGraphicsCardDes()
{
	return mDxgiAdapterDesc.Description;
}

UINT GraphicsCard::GetGraphicsCardMemory()
{
	return mDxgiAdapterDesc.DedicatedVideoMemory / 1024 / 1024;
}

bool GraphicsCard::GetRefreshRate(int uWidth, int uHeight, int* Numerator, int* Denominator)
{
	for (int i = 0; i < mNumModes; i++)
	{
		if (mDisplayModeList[i].Width == (unsigned int)uWidth)
		{
			if (mDisplayModeList[i].Height == (unsigned int)uHeight)
			{
				*Numerator = mDisplayModeList[i].RefreshRate.Numerator; // 分子可能是是60
				*Denominator = mDisplayModeList[i].RefreshRate.Denominator; // 分母可能是1
			}
		}
	}

	return true;
}

