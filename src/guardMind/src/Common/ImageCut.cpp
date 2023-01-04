#include "ImageCut.h"

#ifdef _WIN32
#include <comdef.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;
#else

#endif

#ifdef _WIN32
int GetCodecClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT num = 0;  // number of image encoders
	UINT size = 0; // size of the image encoder array in bytes

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1; // Failure

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1; // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			return j; // Success
		}
	} // for

	return -1; // Failure
} // GetCodecClsid

// ���ܣ��и�ͼ�񣬻�����ͼ��
// ������imgSrc��ԴͼƬ�ֽ����飬
//		 ImageSize��ԭͼƬ�Ĵ�С
//		 imgDst:Ŀ��ͼƬ�Ļ�������ַ
//		 xPos:Ŀ��ͼƬ���Ͻ�x����
//		 yPos:Ŀ��ͼƬ���Ͻ�y����
//		 Width:Ŀ��ͼƬ���
//		 Height:Ŀ��ͼƬ�߶�
// ����ֵ��ʧ�ܷ���0�����򷵻�Ŀ��ͼƬ�Ĵ�С
int ImageCut(unsigned char *imgSrc, int imageSize, unsigned char* imgDst, int xPos, int yPos, int Width, int Height)
{
	int nRet = -1;

	if (Width <= 0 || Height <= 0 || imgSrc == nullptr || imageSize <= 0)
	{
		return 0;
	}
	GdiplusStartupInput gdiplusstartupinput;

	ULONG_PTR gdiplustoken;
	DWORD nDstSize = 0;
	GdiplusStartup(&gdiplustoken, &gdiplusstartupinput, NULL);
	{
		CLSID clsid;
		nRet = GetCodecClsid(L"image/jpeg", &clsid);
		if (nRet == -1)
		{
			GdiplusShutdown(gdiplustoken);
			return 0;
		}

		//�ٴ���һ���ڴ���������Ŀ����
		HGLOBAL hDesMem = GlobalAlloc(GMEM_MOVEABLE, imageSize);
		IStream *pDesStream = NULL;
		CreateStreamOnHGlobal(hDesMem, TRUE, &pDesStream);
		BYTE *pDesData = (BYTE *)GlobalLock(hDesMem);
		//3.�����ڴ棬�������ȫ�ֿռ��С�
		CopyMemory(pDesData, imgSrc, imageSize);
		GlobalUnlock(hDesMem);
		// 4. �ؽ�Image
		Image *bmSrc = Image::FromStream(pDesStream);

		//		bmSrc->Save(L"E:\\test3.jpg", &clsid, NULL);


		int w = 0, h = 0;
		w = bmSrc->GetWidth();
		h = bmSrc->GetHeight();

		//if (w < h) //ͼƬ�����ŵ� ����Width��Height
		//{
		//	int nTemp = Width;
		//	Width = Height;
		//	Height = nTemp;
		//}
		Bitmap *bmPhoto = new Bitmap(Width, Height);	// elvsi�����

		bmPhoto->SetResolution(bmSrc->GetHorizontalResolution(), bmSrc->GetVerticalResolution());
		//bmPhoto->SetResolution(Width, Height);
		Graphics grPhoto(bmPhoto);
		grPhoto.Clear((ARGB)Color::White);
		grPhoto.SetInterpolationMode(InterpolationModeHighQualityBicubic);

		Rect dest(0, 0, w, h);
		grPhoto.DrawImage((Image*)bmSrc, dest, xPos, yPos, Width, Height, UnitPixel);
		//2.������
		IStream *pOutStream = NULL;
		ULARGE_INTEGER   pSeek;
		LARGE_INTEGER    dlibMove = { 0 };

		CreateStreamOnHGlobal(NULL, TRUE, &pOutStream);
		//��JPEGͼƬ��ʽ�������ݵ�����
		bmPhoto->Save(pOutStream, &clsid, NULL);
		pOutStream->Seek(dlibMove, STREAM_SEEK_SET, &pSeek);
		pOutStream->Read(imgDst, imageSize, &nDstSize);

		GlobalFree(hDesMem);	// �ͷ�ȫ�ֿռ�
		delete bmSrc;
		delete bmPhoto;
		pDesStream->Release();
		pOutStream->Release();
	}
	GdiplusShutdown(gdiplustoken);
	return nDstSize;
}
int ImageCut(unsigned char *imgSrc, int imageSize, unsigned char* imgDst, double xPos, double yPos, double Width, double Height)
{
	int nRet = -1;

	if (Width <= 0 || Height <= 0 || imgSrc == nullptr || imageSize <= 0)
	{
		return 0;
}
	GdiplusStartupInput gdiplusstartupinput;

	ULONG_PTR gdiplustoken;
	DWORD nDstSize = 0;
	GdiplusStartup(&gdiplustoken, &gdiplusstartupinput, NULL);
	{
		CLSID clsid;
		nRet = GetCodecClsid(L"image/jpeg", &clsid);
		if (nRet == -1)
		{
			GdiplusShutdown(gdiplustoken);
			return 0;
		}

		//�ٴ���һ���ڴ���������Ŀ����
		HGLOBAL hDesMem = GlobalAlloc(GMEM_MOVEABLE, imageSize);
		IStream *pDesStream = NULL;
		IStream *pOutStream = NULL;
		CreateStreamOnHGlobal(hDesMem, TRUE, &pDesStream);
		BYTE *pDesData = (BYTE *)GlobalLock(hDesMem);
		//3.�����ڴ棬�������ȫ�ֿռ��С�
		CopyMemory(pDesData, imgSrc, imageSize);
		GlobalUnlock(hDesMem);
		// 4. �ؽ�Image
		Image *bmSrc = Image::FromStream(pDesStream);

		int w = 0, h = 0;
		w = bmSrc->GetWidth();
		h = bmSrc->GetHeight();

		//if (w < h) //ͼƬ�����ŵ� ����Width��Height
		//{
		//	double nTemp = Width;
		//	Width = Height;
		//	Height = nTemp;
		//}
		int pixelAdd = 100;
		xPos = w * xPos;
		yPos = h * yPos;
		Width = w * Width;
		Height = h * Height;
		if (xPos - pixelAdd > 0)
		{
			xPos -= pixelAdd;
			Width += pixelAdd;
			if (xPos + Width +pixelAdd < w)
			{
				Width += pixelAdd;
			}
		}
		else
		{
			if (Width + pixelAdd < w)
			{
				Width += pixelAdd;
			}
		}

		if (yPos - pixelAdd > 0)
		{
			yPos -= pixelAdd;
			Height += pixelAdd;
			if (yPos + Height + pixelAdd < h)
			{
				Height += pixelAdd;
			}
		}
		else
		{
			if (Height + pixelAdd < h)
			{
				Height += pixelAdd;
			}
		}
		//nDstSize = ImageCut(imgSrc, imageSize, imgDst, (int)xPos, (int)yPos, (int)Width, (int)Height);
		nDstSize = ImageCut(imgSrc, imageSize, imgDst, (int)300, (int)200, (int)400, (int)400);

		GlobalFree(hDesMem);	//�ͷ�ȫ�ֿռ�
		delete bmSrc;
		if (pDesStream)
		{
			pDesStream->Release();
		}
		if (pOutStream)
		{
			pOutStream->Release();
		}
	}
	GdiplusShutdown(gdiplustoken);
	return nDstSize;
}

#else
int ImageCut(unsigned char *imgSrc, int imageSize, unsigned char* imgDst, int xPos, int yPos, int Width, int Height)
{
	return 0;
}
int ImageCut(unsigned char *imgSrc, int imageSize, unsigned char* imgDst, double xPos, double yPos, double Width, double Height)
{
	return 0;
}
#endif