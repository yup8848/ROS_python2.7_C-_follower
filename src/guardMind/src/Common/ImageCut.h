#pragma once
/*
xPos
�߽�����Ͻǵ��X�����꣬ȡֵ��Χ[0.001,1]
yPos
�߽�����Ͻǵ��Y�����꣬ȡֵ��Χ[0.001,1]
Width
�߽��Ŀ�ȣ�ȡֵ��Χ[0.001,1]
Height
�߽��ĸ߶ȣ�ȡֵ��Χ[0.001,1]
*/
int ImageCut(unsigned char *imgSrc, int imageSize, unsigned char* imgDst, double xPos, double yPos, double Width, double Height);

int ImageCut(unsigned char *imgSrc, int imageSize, unsigned char* imgDst, int xPos, int yPos, int Width, int Height);