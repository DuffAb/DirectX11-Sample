//--------------------------------------------------------------------------------------
// File: TargaTextureLoader.cpp
//
// Functions for loading a DDS texture and creating a Direct3D runtime resource for it
//
// Note these functions are useful as a light-weight runtime loader for DDS files. For
// a full-featured DDS file reader, writer, and texture processing pipeline see
// the 'Texconv' sample and the 'DirectXTex' library.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// http://go.microsoft.com/fwlink/?LinkId=248926
// http://go.microsoft.com/fwlink/?LinkId=248929
//--------------------------------------------------------------------------------------

#include "TargaTextureLoader.h"

#include <assert.h>
#include <algorithm>
#include <memory>

#if !defined(NO_D3D11_DEBUG_NAME) && ( defined(_DEBUG) || defined(PROFILE) )
#pragma comment(lib,"dxguid.lib")
#endif

using namespace DirectX;

//--------------------------------------------------------------------------------------
// Macros
//--------------------------------------------------------------------------------------
#ifndef MAKEFOURCC
    #define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((uint32_t)(uint8_t)(ch0) | ((uint32_t)(uint8_t)(ch1) << 8) |       \
                ((uint32_t)(uint8_t)(ch2) << 16) | ((uint32_t)(uint8_t)(ch3) << 24 ))
#endif /* defined(MAKEFOURCC) */

//--------------------------------------------------------------------------------------
// DDS file structure definitions
//
// See DDS.h in the 'Texconv' sample and the 'DirectXTex' library
//--------------------------------------------------------------------------------------
#pragma pack(push,1)
struct TargaHeader
{
	unsigned char data1[12];
	unsigned short width;
	unsigned short height;
	unsigned char bpp;
	unsigned char data2;
};

namespace {
	struct handle_closer { void operator()(HANDLE h) { if (h) CloseHandle(h); } };

	typedef public std::unique_ptr<void, handle_closer> ScopedHandle;

	inline HANDLE safe_handle(HANDLE h) { return (h == INVALID_HANDLE_VALUE) ? 0 : h; }

	HRESULT LoadTextureDataFromFile(
		_In_z_ const wchar_t* fileName,
		std::unique_ptr<uint8_t[]>& targaData,
		const TargaHeader** header,
		const uint8_t** bitData,
		size_t* bitSize)
	{
		if (!header || !bitData || !bitSize)
		{
			return E_POINTER;
		}

		// open the file
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
		ScopedHandle hFile(safe_handle(CreateFile2(fileName,
			GENERIC_READ,
			FILE_SHARE_READ,
			OPEN_EXISTING,
			nullptr)));
#else
		ScopedHandle hFile(safe_handle(CreateFileW(fileName,
			GENERIC_READ,
			FILE_SHARE_READ,
			nullptr,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			nullptr)));
#endif

		if (!hFile)
		{
			return HRESULT_FROM_WIN32(GetLastError());
		}

		// Get the file size
		FILE_STANDARD_INFO fileInfo;
		if (!GetFileInformationByHandleEx(hFile.get(), FileStandardInfo, &fileInfo, sizeof(fileInfo)))
		{
			return HRESULT_FROM_WIN32(GetLastError());
		}

		// File is too big for 32-bit allocation, so reject read
		if (fileInfo.EndOfFile.HighPart > 0)
		{
			return E_FAIL;
		}

		DWORD BytesRead = 0;
		TargaHeader *hdr = new TargaHeader();
		if (!ReadFile(hFile.get(), hdr, sizeof(TargaHeader),	&BytesRead,	nullptr))
		{
			return HRESULT_FROM_WIN32(GetLastError());
		}
#if 0
		FILE* fPtr;
		_wfopen_s(&fPtr, fileName, L"rb");
		fread(hdr, sizeof(TargaHeader), 1, fPtr);
		// ... 
		fclose(fPtr);
#endif
		// Calculate the size of the 32 bit image data.
		uint32_t uTargaSize = 0;
		uint32_t uHeight = hdr->height;
		uint32_t uWidth = hdr->width;
		if ((int)hdr->bpp != 32)
		{
			return HRESULT_FROM_WIN32(GetLastError());
		}

		uTargaSize = uWidth * uWidth * 4;
		
		// create enough space for the file data
		std::unique_ptr<uint8_t[]> targa;
		targa.reset(new (std::nothrow) uint8_t[uTargaSize]);
		if (!ReadFile(hFile.get(), targa.get(), uTargaSize, &BytesRead, nullptr))
		{
			return HRESULT_FROM_WIN32(GetLastError());
		}

		targaData.reset(new (std::nothrow) uint8_t[uTargaSize]);
		
		int index = 0, k = 0;
		k = (uWidth * uHeight * 4) - (uWidth * 4);

		// Now copy the targa image data into the targa destination array in the correct order since the targa format is stored upside down.
		for (int i = 0; i < uHeight; ++i)
		{
			for (int j = 0; j < uWidth; ++j)
			{
				targaData[index + 0] = targa[k + 2];  // Red.
				targaData[index + 1] = targa[k + 1];  // Green.
				targaData[index + 2] = targa[k + 0];  // Blue
				targaData[index + 3] = targa[k + 3];  // Alpha

				// Increment the indexes into the targa data.
				k += 4;
				index += 4;
			}
			// Set the targa image data index back to the preceding row at the beginning of the column since its reading it in upside down.
			k -= (uWidth * 8);
		}

		*header = hdr;

		return S_OK;
	}

	
}
_Use_decl_annotations_
HRESULT DirectX::CreateTargaTextureFromMemory(
	ID3D11Device* d3dDevice,
	const uint8_t* ddsData,
	size_t ddsDataSize,
	ID3D11Resource** texture,
	ID3D11ShaderResourceView** textureView,
	size_t maxsize)
{
	HRESULT ret = 0;

	return ret;
}

_Use_decl_annotations_
HRESULT DirectX::CreateTargaTextureFromFile(
	ID3D11Device* d3dDevice,
	ID3D11DeviceContext* d3dContext,
	const wchar_t* szFileName,
	ID3D11Resource** texture,
	ID3D11ShaderResourceView** textureView,
	size_t maxsize)
{
	HRESULT ret = 0;

	
	if (texture)
	{
		*texture = nullptr;
	}
	if (textureView)
	{
		*textureView = nullptr;
	}

	if (!d3dDevice || !szFileName || (!texture && !textureView))
	{
		return E_INVALIDARG;
	}

	const TargaHeader* header = nullptr;
	const uint8_t* bitData = nullptr;
	size_t bitSize = 0;
	size_t rowPitch = 0;

	std::unique_ptr<uint8_t[]> ddsData;
	ret = LoadTextureDataFromFile(szFileName, ddsData, &header, &bitData, &bitSize);
	if (ret != S_OK)
	{
		return ret;
	}

	D3D11_TEXTURE2D_DESC TexDec;
	// Setup the description of the texture.
	TexDec.Height = header->height;
	TexDec.Width = header->width;
	TexDec.MipLevels = 0;
	TexDec.ArraySize = 1;
	TexDec.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	TexDec.SampleDesc.Count = 1;
	TexDec.SampleDesc.Quality = 0;
	TexDec.Usage = D3D11_USAGE_DEFAULT;
	TexDec.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	TexDec.CPUAccessFlags = 0;
	TexDec.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	
	d3dDevice->CreateTexture2D(&TexDec, NULL, (ID3D11Texture2D**)texture);
	// Set the row pitch of the targa image data.
	rowPitch = (header->width * 4) * sizeof(unsigned char);

	// Copy the targa image data into the texture.
	d3dContext->UpdateSubresource(*texture, 0, NULL, ddsData.get(), rowPitch, 0);

	// Setup the shader resource view description.
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = TexDec.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;

	// Create the shader resource view for the texture.
	d3dDevice->CreateShaderResourceView(*texture, &srvDesc, textureView);

	// Generate mipmaps for this texture.
	d3dContext->GenerateMips(*textureView);
	return ret;
}
