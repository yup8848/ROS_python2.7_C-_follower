#pragma once
/*
xPos
边界框左上角点的X轴坐标，取值范围[0.001,1]
yPos
边界框左上角点的Y轴坐标，取值范围[0.001,1]
Width
边界框的宽度，取值范围[0.001,1]
Height
边界框的高度，取值范围[0.001,1]
*/
int ImageCut(unsigned char *imgSrc, int imageSize, unsigned char* imgDst, double xPos, double yPos, double Width, double Height);

int ImageCut(unsigned char *imgSrc, int imageSize, unsigned char* imgDst, int xPos, int yPos, int Width, int Height);