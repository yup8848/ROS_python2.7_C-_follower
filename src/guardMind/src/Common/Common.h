#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <time.h>
#include "Log.h"
#define _USE_MATH_DEFINES
#include <math.h>
using namespace std;

bool FloatEqualZero(float f);
string wstring2string(const wstring s);
wstring string2wstring(const string s);
string UTF8ToGB(const string & str);

string GBToUTF8(const string & str);

//16进制打印byte数据日志
void HexDump(const char *buf, int len, int addr);

//字节值转字符串：12-->"C"
string Char2HexString(unsigned char ch);

//16进制字节转字符串：0xab--->0xa,0xb
void Hex2ASCII(char hexIn, char* pAscOut);
long Hex2Dec(const char *source);
string DecIntToHexStr(long long num);
string DecStrToHexStr(string str);

//获取系统时间戳：YYYYMMDDHHMMSS.ms
string GetTimeStamp();
//获取系统时间戳：YYYYMMDDHHMMSS
string GetTimeStampSecond();
unsigned short DagGetYear();
unsigned char DagGetMonth();
unsigned char DagGetDay();
unsigned char DagGetHour();
unsigned char DagGetMinute();
unsigned char DagGetSecond();

//获取系统时间戳：YYYYMMDD
string GetTimeStampDate();

string GetMilliSecondsString();
long long GetMilliSeconds();
long long GetSecondsSincePowerOn();
long long GetMilliSecondsSincePowerOn();

//遍历文件夹中所有文件，不进行递归遍历
int readFileList(const char *basePath, vector<string>& vFile, bool bContainerPath, bool recursion = false);

int RemoveFileBeforeDays(const char* filename, int days);
int RemoveFileFromNow(const char* filename, unsigned int timeInSecondFromNow);
//遍历文件夹中指定层级的所有目录，可选进行递归遍历
int readFileDir(const char *basePath, vector<string>& vFile, bool bContainerPath,bool bSub,int depath);

//字节流转换为字符串：0xab12-->"ab12"
string ByteStream2HexString(const char* pData, int nLen, int nStep=16);

//分割字符串，并转换为char值存储到pOut，使用与对Ipv4/netmask/gateway
int SplitString(std::string s, char* pOut, int nLen, std::string c);
//分割字符串
void SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c);

//获取路径目录分隔符
const char* GetDirectorySpace();
//获取换行符
const char* GetNewLine();

//字符串比较
bool StrCompare(const string& str1, const string& str2);

//大小端转换
short BigLitEndianChange2Short(const unsigned char *pData);
int BigLitEndianChange2Int(const unsigned char *pData);

//从数组中取N字节值
short GetShort(const unsigned char *pData);
int GetInt(const unsigned char *pData);
float GetFloat(const unsigned char *pData);
double GetDouble(const unsigned char *pData);
unsigned long long GetLongLong(const unsigned char *pData);

//获取随机数
unsigned  int GetRandom();


void SetLogFileMaxCount(int count);
//日志初始化
int InitLog(string strPath, string strName);


void ZipCompressFile(vector<string> file);
string GetCurDirectory();


time_t StringToDatetime(string str);

string DatetimeToString(time_t time);

tm StringToTm(string str);
string TmToString(tm tm);
tm StringToTm(string str, string format);

//累加和
//len:数组长度
//bytes：数组
//offset：起始位置
unsigned char CheckSum(const unsigned char* bytes, int len, int offset);
//异或和
unsigned char XORSum(const unsigned char* bytes, int len, int offset);

unsigned char VIBRATION_CRC8(unsigned char* pData, int len);

unsigned short CRC16(const unsigned char* pDataIn, int iLenIn);

//%09s在linux下，前面会补空格，windows下补的是0
//自己实现统一接口:字符串前面补0
//source：原始字符串
//expectLength:期望长度
string Fill0InFront(string source, int expectLength);


int32_t createDirectory(string directoryPath);
//经纬度转度分秒
int ChangetoSexagesimal(float Num, int &Hour, int &Minute, int &Second, int& MilliSecond);

