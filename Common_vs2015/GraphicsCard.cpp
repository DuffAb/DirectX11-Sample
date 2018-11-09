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

	// ���Դ��������ӿڣ��ö�������ö�ٳ��Կ��豸
	IDXGIFactory * dxgiFactory = 0;
	ret = CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)&dxgiFactory);

	// Use the factory to create an adapter for the primary graphics interface (video card).
	// ����ö���Կ�����һ������������Ҫö�ٵ�����������������ȡ����ö�����������Կ������� dxgiAdapter
	IDXGIAdapter* dxgiAdapter = 0;
	ret = dxgiFactory->EnumAdapters(0, &dxgiAdapter);//ö�����������Կ���
	ret = dxgiAdapter->GetDesc(&mDxgiAdapterDesc);	 //��ȡ�Կ�������Ϣ

	// Enumerate the primary adapter output (monitor).ö�ٳ� ��ʾ��
	IDXGIOutput* dxgiAdapterOutput; // ��ʾ���豸�ӿ��࣬���Ը��ݸ�����rgba��ʽ��ѯ֧�ֵ���ʾģʽ (�ֱ��ʺ�ˢ����)
	ret = dxgiAdapter->EnumOutputs(0, &dxgiAdapterOutput);

	// Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).
	// ��ȡ����DXGI_FORMAT_R8G8B8A8_UNORMģʽ������
	ret = dxgiAdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &mNumModes, NULL);

	// ����һ���б��������浱ǰ��ʾ�����Կ���Ͽ���֧�ֵ���ʾģʽ
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
				*Numerator = mDisplayModeList[i].RefreshRate.Numerator; // ���ӿ�������60
				*Denominator = mDisplayModeList[i].RefreshRate.Denominator; // ��ĸ������1
			}
		}
	}

	return true;
}

