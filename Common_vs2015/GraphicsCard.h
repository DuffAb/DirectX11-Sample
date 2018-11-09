//***************************************************************************************
// GraphicsCard.h by liangdefeng (C) 2018 All Rights Reserved.
//
// Simple graphics card information class.  
//***************************************************************************************

#ifndef GRAPHICS_CARD_H
#define GRAPHICS_CARD_H

#include "d3dUtil.h"
#include "GameTimer.h"
#include <string>

class GraphicsCard
{
public:
	GraphicsCard();
	~GraphicsCard();
	
private:
	bool InitGraphicsCard();

public:
	// ��ȡ�Կ�����������
	std::wstring GetGraphicsCardDes();

	// 
	UINT GetModesNum();

	// ��ȡ�Դ�Ĵ�С
	UINT GetGraphicsCardMemory();

	// 
	bool GetRefreshRate(int uWidth, int uHeight, int* Numerator, int* Denominator);


private:
	wchar_t mGraphicsCardDes[128];
	UINT mNumModes;							// ��ʾ�����Կ�֧�ֵ���ʾģʽ����
	DXGI_MODE_DESC* mDisplayModeList;       // ��ʱ������ʾ��֧�ֵ�ģʽ������
	DXGI_ADAPTER_DESC mDxgiAdapterDesc;		// �����Կ�������Ϣ
	UINT mVideoCardMemory;
};

#endif // GRAPHICS_CARD_H