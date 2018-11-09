//***************************************************************************************
// LightHelper.h by Frank Luna (C) 2011 All Rights Reserved.
//
// Helper classes for lighting.
//***************************************************************************************

#ifndef LIGHTHELPER_H
#define LIGHTHELPER_H

#include <Windows.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
using namespace DirectX;

// Note: Make sure structure alignment agrees with HLSL structure padding rules. 
//   Elements are packed into 4D vectors with the restriction that an element
//   cannot straddle a 4D vector boundary.

struct DirectionalLight
{
	DirectionalLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient;  // 由光源发射的环境光的数量
	XMFLOAT4 Diffuse;  // 由光源发射的漫反射光的数量
	XMFLOAT4 Specular; // 由光源发射的高光的数量
	XMFLOAT3 Direction;// 灯光方向
	float Pad; // Pad the last float so we can set an array of lights if we wanted. 如果需要，可以填充最后一个浮点数，以便我们可以设置一组灯
};

// 点光源
struct PointLight
{
	PointLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular;

	// Packed into 4D vector: (Position, Range)  打包到4D矢量: (Position, Range)
	XMFLOAT3 Position;// 灯光位置
	float Range; // 光照范围（离开光源的距离大于这个值的点不会被照亮）

	// Packed into 4D vector: (A0, A1, A2, Pad)  打包到4D矢量: (A0, A1, A2, Pad)
	XMFLOAT3 Att;
	float Pad; // Pad the last float so we can set an array of lights if we wanted.  占位最后一个float，这样我们就可以设置光源数组了
};

// 聚光灯
struct SpotLight
{
	SpotLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular;

	// Packed into 4D vector: (Position, Range)
	XMFLOAT3 Position;
	float Range;

	// Packed into 4D vector: (Direction, Spot)
	XMFLOAT3 Direction;
	float Spot;// 该指数用于控制聚光灯的圆锥体区域大小；这个值只用于聚光灯

	// Packed into 4D vector: (Att, Pad)
	XMFLOAT3 Att;// 按照（a0、a1和a2）的顺序存储3个衰减常量。衰减常量只用于点和聚光灯，用于控制光强随距离衰减的程度
	float Pad;   // Pad the last float so we can set an array of lights if we wanted.
};

struct Material
{
	Material() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular; // w = SpecPower
	XMFLOAT4 Reflect;
};

#endif // LIGHTHELPER_H