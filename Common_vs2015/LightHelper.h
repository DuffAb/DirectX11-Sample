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

	XMFLOAT4 Ambient;  // �ɹ�Դ����Ļ����������
	XMFLOAT4 Diffuse;  // �ɹ�Դ�����������������
	XMFLOAT4 Specular; // �ɹ�Դ����ĸ߹������
	XMFLOAT3 Direction;// �ƹⷽ��
	float Pad; // Pad the last float so we can set an array of lights if we wanted. �����Ҫ������������һ�����������Ա����ǿ�������һ���
};

// ���Դ
struct PointLight
{
	PointLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular;

	// Packed into 4D vector: (Position, Range)  �����4Dʸ��: (Position, Range)
	XMFLOAT3 Position;// �ƹ�λ��
	float Range; // ���շ�Χ���뿪��Դ�ľ���������ֵ�ĵ㲻�ᱻ������

	// Packed into 4D vector: (A0, A1, A2, Pad)  �����4Dʸ��: (A0, A1, A2, Pad)
	XMFLOAT3 Att;
	float Pad; // Pad the last float so we can set an array of lights if we wanted.  ռλ���һ��float���������ǾͿ������ù�Դ������
};

// �۹��
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
	float Spot;// ��ָ�����ڿ��ƾ۹�Ƶ�Բ׶�������С�����ֵֻ���ھ۹��

	// Packed into 4D vector: (Att, Pad)
	XMFLOAT3 Att;// ���գ�a0��a1��a2����˳��洢3��˥��������˥������ֻ���ڵ�;۹�ƣ����ڿ��ƹ�ǿ�����˥���ĳ̶�
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