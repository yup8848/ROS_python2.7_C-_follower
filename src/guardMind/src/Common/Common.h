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

//16���ƴ�ӡbyte������־
void HexDump(const char *buf, int len, int addr);

//�ֽ�ֵת�ַ�����12-->"C"
string Char2HexString(unsigned char ch);

//16�����ֽ�ת�ַ�����0xab--->0xa,0xb
void Hex2ASCII(char hexIn, char* pAscOut);
long Hex2Dec(const char *source);
string DecIntToHexStr(long long num);
string DecStrToHexStr(string str);

//��ȡϵͳʱ�����YYYYMMDDHHMMSS.ms
string GetTimeStamp();
//��ȡϵͳʱ�����YYYYMMDDHHMMSS
string GetTimeStampSecond();
unsigned short DagGetYear();
unsigned char DagGetMonth();
unsigned char DagGetDay();
unsigned char DagGetHour();
unsigned char DagGetMinute();
unsigned char DagGetSecond();

//��ȡϵͳʱ�����YYYYMMDD
string GetTimeStampDate();

string GetMilliSecondsString();
long long GetMilliSeconds();
long long GetSecondsSincePowerOn();
long long GetMilliSecondsSincePowerOn();

//�����ļ����������ļ��������еݹ����
int readFileList(const char *basePath, vector<string>& vFile, bool bContainerPath, bool recursion = false);

int RemoveFileBeforeDays(const char* filename, int days);
int RemoveFileFromNow(const char* filename, unsigned int timeInSecondFromNow);
//�����ļ�����ָ���㼶������Ŀ¼����ѡ���еݹ����
int readFileDir(const char *basePath, vector<string>& vFile, bool bContainerPath,bool bSub,int depath);

//�ֽ���ת��Ϊ�ַ�����0xab12-->"ab12"
string ByteStream2HexString(const char* pData, int nLen, int nStep=16);

//�ָ��ַ�������ת��Ϊcharֵ�洢��pOut��ʹ�����Ipv4/netmask/gateway
int SplitString(std::string s, char* pOut, int nLen, std::string c);
//�ָ��ַ���
void SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c);

//��ȡ·��Ŀ¼�ָ���
const char* GetDirectorySpace();
//��ȡ���з�
const char* GetNewLine();

//�ַ����Ƚ�
bool StrCompare(const string& str1, const string& str2);

//��С��ת��
short BigLitEndianChange2Short(const unsigned char *pData);
int BigLitEndianChange2Int(const unsigned char *pData);

//��������ȡN�ֽ�ֵ
short GetShort(const unsigned char *pData);
int GetInt(const unsigned char *pData);
float GetFloat(const unsigned char *pData);
double GetDouble(const unsigned char *pData);
unsigned long long GetLongLong(const unsigned char *pData);

//��ȡ�����
unsigned  int GetRandom();


void SetLogFileMaxCount(int count);
//��־��ʼ��
int InitLog(string strPath, string strName);


void ZipCompressFile(vector<string> file);
string GetCurDirectory();


time_t StringToDatetime(string str);

string DatetimeToString(time_t time);

tm StringToTm(string str);
string TmToString(tm tm);
tm StringToTm(string str, string format);

//�ۼӺ�
//len:���鳤��
//bytes������
//offset����ʼλ��
unsigned char CheckSum(const unsigned char* bytes, int len, int offset);
//����
unsigned char XORSum(const unsigned char* bytes, int len, int offset);

unsigned char VIBRATION_CRC8(unsigned char* pData, int len);

unsigned short CRC16(const unsigned char* pDataIn, int iLenIn);

//%09s��linux�£�ǰ��Ჹ�ո�windows�²�����0
//�Լ�ʵ��ͳһ�ӿ�:�ַ���ǰ�油0
//source��ԭʼ�ַ���
//expectLength:��������
string Fill0InFront(string source, int expectLength);


int32_t createDirectory(string directoryPath);
//��γ��ת�ȷ���
int ChangetoSexagesimal(float Num, int &Hour, int &Minute, int &Second, int& MilliSecond);

