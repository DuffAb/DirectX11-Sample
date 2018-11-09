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
	// 获取显卡的描述名称
	std::wstring GetGraphicsCardDes();

	// 
	UINT GetModesNum();

	// 获取显存的大小
	UINT GetGraphicsCardMemory();

	// 
	bool GetRefreshRate(int uWidth, int uHeight, int* Numerator, int* Denominator);


private:
	wchar_t mGraphicsCardDes[128];
	UINT mNumModes;							// 显示器和显卡支持的显示模式数量
	DXGI_MODE_DESC* mDisplayModeList;       // 临时保存显示器支持的模式的数组
	DXGI_ADAPTER_DESC mDxgiAdapterDesc;		// 存有显卡描述信息
	UINT mVideoCardMemory;
};

#endif // GRAPHICS_CARD_H