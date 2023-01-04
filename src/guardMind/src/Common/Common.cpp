#include "Common.h"
#include <locale>
#include <stdio.h>
#include <iostream>
#ifdef _WIN32
#include <io.h>
#include <direct.h> 
#include <windows.h>
#include <codecvt>
#else
#include<sys/time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <iconv.h>
#include <sys/file.h>
#endif
#include <random>
#include <chrono>
#include <sstream>
#include <cstring>
#include <algorithm>
#include "Config.h"
using namespace std::chrono;

static int m_maxLogFileCount = 50;

void SetLogFileMaxCount(int count)
{
	m_maxLogFileCount = count;
}


bool FloatEqualZero(float f)
{
	if (fabs(f)<=0.0000001)
	{
		return true;
	}
	return false;
}


const char* GetDirectorySpace()
{
#if defined _WIN32
	return "\\";
#else
	return "/";
#endif
}
const char* GetNewLine()
{
#if defined _WIN32
	return "\n";
#else
	return "\n";
#endif
}
#if defined _WIN32
#else
int code_convert(const char *from_charset, const char *to_charset, char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
	iconv_t cd;
	char **pin = &inbuf;
	char **pout = &outbuf;

	cd = iconv_open(to_charset, from_charset);
	if (cd == 0) return -1;
	memset(outbuf, 0, outlen);
	if (iconv(cd, pin, &inlen, pout, &outlen) == -1) return -1;
	iconv_close(cd);
	return 0;
}
/*UNICODE��תΪGB2312��*/
int u2g(char *inbuf, int inlen, char *outbuf, int outlen)
{
	return code_convert("utf-8", "gb18030", inbuf, inlen, outbuf, outlen);
}
/*GB2312��תΪUNICODE��*/
int g2u(char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
	return code_convert("gb18030", "utf-8", inbuf, inlen, outbuf, outlen);
}
#endif

string wstring2string(const wstring s)
{
	std::string ret;
#if defined _WIN32
	int nLen = WideCharToMultiByte(CP_ACP, 0, s.data(), -1, NULL, 0, NULL, NULL);
	if (nLen != 0)
	{
		char* pResult = new char[nLen];
		WideCharToMultiByte(CP_ACP, 0, s.data(), -1, pResult, nLen, NULL, NULL);
		ret = pResult;
		delete[]pResult;
	}
#else
#endif
	return ret;
}
wstring string2wstring(const string s)
{
	std::wstring ret;
#if defined _WIN32
	int nLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, s.c_str(), -1, NULL, 0);
	if (nLen == 0)
	{
		return NULL;
	}
	wchar_t* pResult = new wchar_t[nLen];
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, s.c_str(), -1, pResult, nLen);
	ret = pResult;
	delete[]pResult;
#else

#endif
	return ret;
}
std::string UnicodeToUTF8(const std::wstring & wstr)
{
	std::string ret;
#if defined _WIN32
	try {
		std::wstring_convert< std::codecvt_utf8<wchar_t> > wcv;
		ret = wcv.to_bytes(wstr);
	}
	catch (const std::exception & e) {
		std::cerr << e.what() << std::endl;
	}
#else
#endif
	return ret;
}
std::wstring UTF8ToUnicode(const std::string & str)
{
	std::wstring ret;
#if defined _WIN32
	try {
		std::wstring_convert< std::codecvt_utf8<wchar_t> > wcv;
		ret = wcv.from_bytes(str);
	}
	catch (const std::exception & e) {
		std::cerr << e.what() << std::endl;
	}
#else
#endif
	return ret;
}
std::string UTF8ToGB(const std::string & str)
{
	string ret;
#if defined _WIN32
	ret = wstring2string(UTF8ToUnicode(str));
#else
	//cout << "UTF8ToGB str=" << str<< endl;
	size_t len_in = str.length();
	char *in = new char[len_in];
	memset(in, 0, len_in);
	memcpy(in, str.c_str(), len_in);

	size_t len_out = len_in * 4;
	char *out = new char[len_out];
	memset(out, 0, len_out);
	int code = code_convert("utf-8", "gb18030", in, len_in, out, len_out);
	ret = out;
	delete[]out;
	delete[]in;
	//cout << "UTF8ToGB code=" << code << endl;
	//cout << "UTF8ToGB out=" << out << endl;
	//cout << "UTF8ToGB len_out=" << len_out << endl;
	if (0 != code)
	{
		LOG_ERROR("code convert failed");
	}
#endif
	return ret;
}

std::string GBToUTF8(const std::string & str)
{
	string ret;
#if defined _WIN32
	ret = UnicodeToUTF8(string2wstring(str));
#else
	size_t len_in = str.length();
	char *in = new char[len_in];
	memset(in, 0, len_in);
	memcpy(in, str.c_str(), len_in);

	size_t len_out = len_in * 4;
	char *out = new char[len_out];
	memset(out, 0, len_out);
	int code = code_convert("gb18030", "utf-8", in, len_in, out, len_out);
	ret = out;
	delete[]out;
	delete[]in;
	if (0 != code)
	{
		LOG_ERROR("code convert failed");
	}
#endif
	return ret;
}


std::string Char2HexString(unsigned char ch)
{
	std::string strOut;
	char szBuf[100] = { 0 };
	sprintf(szBuf, "%02X", ch);
	strOut = szBuf;
	return strOut;
}


///* ����ch�ַ���sign�����е���� */
int GetIndexOfSigns(char ch)
{
	if (ch >= '0' && ch <= '9')
	{
		return ch - '0';
	}
	if (ch >= 'A' && ch <= 'F')
	{
		return ch - 'A' + 10;
	}
	if (ch >= 'a' && ch <= 'f')
	{
		return ch - 'a' + 10;
	}
	return -1;
}
long Hex2Dec(const char *source)
{
	long sum = 0;
	long t = 1;
	int i, len;

	len = strlen(source);
	for (i = len - 1; i >= 0; i--)
	{
		sum += t * GetIndexOfSigns(*(source + i));
		t *= 16;
	}

	return sum;
}

string DecIntToHexStr(long long num)
{
	string str;
	long long Temp = num / 16;
	int left = num % 16;
	if (Temp > 0)
		str += DecIntToHexStr(Temp);
	if (left < 10)
		str += (left + '0');
	else
		str += ('A' + left - 10);
	return str;
}

string DecStrToHexStr(string str)
{
	long long Dec = 0;
	for (int i = 0; i < str.size(); ++i)
		Dec = Dec * 10 + str[i] - '0';
	return DecIntToHexStr(Dec);
}

void Hex2ASCII(char hexIn, char* pAscOut)
{
	pAscOut[0] = (hexIn & 0xf0)>>4;
	pAscOut[1] = (hexIn & 0x0f);
}

std::string GetTimeStamp()
{
	//std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> timePoint = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
	//auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(timePoint.time_since_epoch());
	//std::time_t timestamp = tmp.count();
	//uint64_t milli = timestamp;
	//milli += (uint64_t)8 * 60 * 60 * 1000;//ת��ʱ��������ʱ��+8Сʱ
	//auto mTime = std::chrono::milliseconds(milli);
	//auto tp = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>(mTime);
	//auto tt = std::chrono::system_clock::to_time_t(tp);
	//std::tm* info = std::gmtime(&tt);
	//char date[27] = { 0 };
	//sprintf(date, "%04d%02d%02d%02d%02d%02d.%ld", info->tm_year + 1900, info->tm_mon + 1, info->tm_mday, info->tm_hour, info->tm_min, info->tm_sec, timestamp % 1000);
	//return std::string(date);
	std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> timePoint = 
		std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
	auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(timePoint.time_since_epoch());
	std::time_t timestamp = tmp.count();
	uint64_t milli = timestamp;
	char milSeconds[5] = { 0 };
	sprintf(milSeconds, "%03d", timestamp % 1000);
	string date = GetTimeStampSecond() + string(".") + milSeconds;
	return date;
}

std::string GetTimeStampDate()
{
	char date[60] = { 0 };
	auto tt = std::chrono::system_clock::to_time_t
	(std::chrono::system_clock::now());
	struct tm* ptm = localtime(&tt);
	sprintf(date, "%d%02d%02d",
		(int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday);

	return std::string(date);
}
unsigned short DagGetYear()
{
	auto tt = std::chrono::system_clock::to_time_t
	(std::chrono::system_clock::now());
	struct tm* ptm = localtime(&tt);
	return ptm->tm_year + 1900;
}
unsigned char DagGetMonth()
{
	auto tt = std::chrono::system_clock::to_time_t
	(std::chrono::system_clock::now());
	struct tm* ptm = localtime(&tt);
	return ptm->tm_mon + 1;
}
unsigned char DagGetDay()
{
	auto tt = std::chrono::system_clock::to_time_t
	(std::chrono::system_clock::now());
	struct tm* ptm = localtime(&tt);
	return ptm->tm_mday;
}
unsigned char DagGetHour()
{
	auto tt = std::chrono::system_clock::to_time_t
	(std::chrono::system_clock::now());
	struct tm* ptm = localtime(&tt);
	return ptm->tm_hour;
}
unsigned char DagGetMinute()
{
	auto tt = std::chrono::system_clock::to_time_t
	(std::chrono::system_clock::now());
	struct tm* ptm = localtime(&tt);
	return ptm->tm_min;
}
unsigned char DagGetSecond()
{
	auto tt = std::chrono::system_clock::to_time_t
	(std::chrono::system_clock::now());
	struct tm* ptm = localtime(&tt);
	return ptm->tm_sec;
}
std::string GetTimeStampSecond()
{
	//SYSTEMTIME t;
	//GetLocalTime(&t);
	//sprintf(date, "%d%02d%02d%02d%02d%02d%ld",
	//	t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);
	char date[60] = { 0 };
	auto tt = std::chrono::system_clock::to_time_t
	(std::chrono::system_clock::now());
	struct tm* ptm = localtime(&tt);
	sprintf(date, "%d%02d%02d%02d%02d%02d",
		(int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
		(int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);

	return std::string(date);
}

int SplitString(std::string s, char* pOut, int nLen, std::string c)
{
	int nCount = 0;
	char *result = NULL;
	const char* p = c.c_str();
	char szData[1024] = { 0 };
	strcpy(szData, s.c_str());
	result = strtok(szData, p);
	while (result != NULL)
	{
		pOut[nCount] = atoi(result);
		result = strtok(NULL, p);
		nCount++;
		if (nCount >= nLen)
			break;
	}
	return nCount;
}

void SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c)
{
	std::string::size_type pos1, pos2;
	pos2 = s.find(c);
	pos1 = 0;
	while (std::string::npos != pos2)
	{
		if (pos2>= pos1)
		{
			v.push_back(s.substr(pos1, pos2 - pos1));
		}
		pos1 = pos2 + c.size();
		pos2 = s.find(c, pos1);
	}
	if (pos1 != s.length())
		v.push_back(s.substr(pos1));
}

std::string ByteStream2HexString(const char* pData, int nLen, int nStep)
{
	string strRet;
	char szBuf[1024] = { 0 };
	const static int maxByte2Out = 512;//�ֽ�ת16�����ַ�������ӡ�ֽ���
	int lineControl = 0;
	for (int index = 0; index < nLen && index < maxByte2Out; index++)
	{
		sprintf(szBuf, "%s %02x ", szBuf, (unsigned char)pData[index]); 
		lineControl++;

		if ((0 == lineControl % nStep))
		{
			sprintf(szBuf, "%s%s", szBuf, GetNewLine());
			strRet += szBuf;
			memset(szBuf, 0, sizeof(szBuf));
			lineControl = 0;
		}
	}
	if (0 != (lineControl % nStep))
	{
		strRet += szBuf;
	}
	if (maxByte2Out<nLen)//�м��ʡ��
	{
		strRet += " ......";
		strRet += GetNewLine();

		//���д����ֽ���
		const static int maxByte2End = 32;
		int lenRes = nLen - maxByte2Out;
		lenRes = lenRes > maxByte2End ? maxByte2End : lenRes;
		lineControl = 0;
		memset(szBuf, 0, sizeof(szBuf));
		for (int index = nLen - lenRes; index < nLen; index++)
		{
			sprintf(szBuf, "%s %02x ", szBuf, (unsigned char)pData[index]);
			lineControl++;
			 
			if ((0 == lineControl % nStep))
			{
				sprintf(szBuf, "%s%s", szBuf, GetNewLine());
				strRet += szBuf;
				memset(szBuf, 0, sizeof(szBuf));
				lineControl = 0;
			}
		}
		if (0 != (lineControl % nStep))
		{
			strRet += szBuf;
		}
	}
	return strRet;
}

void HexDump(const char *buf, int len, int addr)
{
	int i, j, k;
	char binstr[80] = {0};

	for (i = 0; i < len; i++)
	{
		if (0 == (i % 16))
		{
			sprintf(binstr, "%08x -", i + addr);
			sprintf(binstr, "%s %02x", binstr, (unsigned char)buf[i]);
		}
		else if (15 == (i % 16))
		{
			sprintf(binstr, "%s %02x", binstr, (unsigned char)buf[i]);
			sprintf(binstr, "%s  ", binstr);
			for (j = i - 15; j <= i; j++) {
				sprintf(binstr, "%s%c", binstr, ('!' < buf[j] && buf[j] <= '~') ? buf[j] : '.');
			}
			//printf("%s\n", binstr);
		}
		else {
			sprintf(binstr, "%s %02x", binstr, (unsigned char)buf[i]);
		}
	}
	if (0 != (i % 16))
	{
		k = 16 - (i % 16);
		for (j = 0; j < k; j++)
		{
			sprintf(binstr, "%s   ", binstr);
		}
		sprintf(binstr, "%s  ", binstr);
		k = 16 - k;
		for (j = i - k; j < i; j++)
		{
			sprintf(binstr, "%s%c", binstr, ('!' < buf[j] && buf[j] <= '~') ? buf[j] : '.');
		}
		//printf("%s\n", binstr);
	}
}

std::string GetMilliSecondsString()
{
	steady_clock::duration d = steady_clock::now().time_since_epoch();
	//minutes min = duration_cast<minutes>(d);
	//seconds sec = duration_cast<seconds>(d);
	milliseconds mil = duration_cast<milliseconds>(d);
	//microseconds mic = duration_cast<microseconds>(d);
	//nanoseconds nan = duration_cast<nanoseconds>(d);
	//cout << min.count() << "����" << endl;
	//cout << sec.count() << "��" << endl;
	//cout << mil.count() << "����" << endl;
	//cout << mic.count() << "΢��" << endl;
	//cout << nan.count() << "����" << endl;
	stringstream ss;
	ss << mil.count();
	return ss.str();
}
long long GetMilliSeconds()
{
	steady_clock::duration d = steady_clock::now().time_since_epoch();
	//minutes min = duration_cast<minutes>(d);
	//seconds sec = duration_cast<seconds>(d);
	milliseconds mil = duration_cast<milliseconds>(d);
	//microseconds mic = duration_cast<microseconds>(d);
	//nanoseconds nan = duration_cast<nanoseconds>(d);
	//cout << min.count() << "����" << endl;
	//cout << sec.count() << "��" << endl;
	//cout << mil.count() << "����" << endl;
	//cout << mic.count() << "΢��" << endl;
	//cout << nan.count() << "����" << endl;
	stringstream ss;
	return mil.count();
}
long long GetMilliSecondsSincePowerOn()
{
	steady_clock::duration d = steady_clock::now().time_since_epoch();
	milliseconds mil = duration_cast<milliseconds>(d);
	return mil.count();
}

long long GetSecondsSincePowerOn()
{
	steady_clock::duration d = steady_clock::now().time_since_epoch();
	seconds sec = duration_cast<seconds>(d);
	return sec.count();
}

int RemoveFileBeforeDays(const char* filename, int days)
{
	unsigned int timeInSecondFromNow = days * 24 * 60 * 60;
	return RemoveFileFromNow(filename, timeInSecondFromNow);
}
int RemoveFileFromNow(const char* filename, unsigned int timeInSecondFromNow)
{
	int result;
	//errno_t errno;
#ifdef WIN32
	struct _stat buf;
	result = _stat(filename, &buf);
#else
	struct stat buf;
	result = stat(filename, &buf);
#endif

	if (result != 0)
	{
		switch (errno)
		{
		case ENOENT:
			LOG_ERROR("File %s not found", filename);
			break;
		case EINVAL:
			LOG_ERROR("Invalid parameter to _stat.");
			break;
		default:
			/* Should never be reached. */
			//printf("Unexpected error in _stat.\n");
			break;
		}
		return -1;
	}
	else
	{
		//printf("File size     : %ld\n", buf.st_size);
		//printf("Drive         : %c:\n", buf.st_dev + 'A');
		time_t t;
#ifdef WIN32
		_tzset();
#else
		tzset();
#endif
		t = time(NULL);
		/*
			time_t      st_atime;   time of last access -�����ȡʱ��
			time_t      st_mtime;   time of last modification -����޸�ʱ��
			time_t      st_ctime;   time of last status change -���Ȩ���޸�ʱ��
		*/
		if ((t - buf.st_mtime) > timeInSecondFromNow)//1209600��14���������������ɾ������14����ļ�
		{
			remove(filename);
			LOG_DEBUG("remove file : %s, changed before %d second", filename, timeInSecondFromNow);
		}
		return 0;
	}
}
int readFileList(const char *basePath, vector<string>& vFile, bool bContainerPath, bool recursion)
{
	string strPath = basePath;
	int pos = strPath.find_last_of(GetDirectorySpace());
	if (pos != strPath.length() - 1)
	{
		strPath = strPath + GetDirectorySpace();
	}
#ifdef _WIN32
	intptr_t  hFile = 0;//Ҫ����32λ��64ϵͳ������ʹ��long ����
	struct _finddata_t fileInfo;
	string pathName, exdName;
	int n = sizeof(fileInfo);
	
	if ((hFile = _findfirst(pathName.assign(strPath).append("*").c_str(), &fileInfo)) == -1) 
	{
		return 1;
	}
	do 
	{
		if (fileInfo.attrib&_A_SUBDIR) 
		{
			string fname = string(fileInfo.name);
			if (fname != ".." && fname != ".") 
			{
				if (recursion)
				{
					string recPath = strPath + fname;
					readFileList(recPath.c_str(), vFile, bContainerPath, recursion);
				}
			}
		}
		else 
		{
			string strFile;
			if (bContainerPath)
			{
				strFile = strPath + fileInfo.name;
			}
			vFile.push_back(strFile);
		}
	} while (_findnext(hFile, &fileInfo) == 0);
	_findclose(hFile);
#else
	DIR *dp;
	struct dirent *dirp;
	if ((dp = opendir(basePath)) == NULL)
	{
		return -1;
	}

	while ((dirp = readdir(dp)) != NULL)
	{
		if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
			continue;
		string strFile;
		if (bContainerPath)
		{
			strFile = strPath + dirp->d_name;
		}
		vFile.push_back(strFile);
	}
#endif
	return 0;
}

int readFileDir(const char *basePath, vector<string>& vFile, bool bContainerPath,bool bSub,int iDepath)
{
	string strPath = basePath;
	int iDepathTmp = iDepath - 1;
#ifdef _WIN32
	intptr_t  hFile = 0;//Ҫ����32λ��64ϵͳ������ʹ��long ����
	struct _finddata_t fileInfo;
	string pathName, exdName;
	int n = sizeof(fileInfo);

	
	if ((hFile = _findfirst(pathName.assign(strPath).append("\\*").c_str(), &fileInfo)) == -1)
	{
		return 1;
	}
	do
	{
		if (fileInfo.attrib&_A_SUBDIR)
		{
			string fname = string(fileInfo.name);
			if (fname != ".." && fname != ".")
			{
				//�������ļ���
				if (bSub && iDepathTmp > 0) {
					string subDirPath = strPath + fname+"\\";
					readFileDir(subDirPath.c_str(), vFile, bContainerPath, bSub, iDepathTmp);
				}
				else {
					string strFile;
					if (bContainerPath)
					{
						strFile = strPath + fileInfo.name;
					}
					else {
						strFile = fileInfo.name;
					}
					vFile.push_back(strFile);
				}
			}
		}
		else
		{
			string strFile;
			if (bContainerPath)
			{
				strFile = strPath + fileInfo.name;
			}
			else {
				strFile = fileInfo.name;
			}
			//vFile.push_back(strFile);
		}
	} while (_findnext(hFile, &fileInfo) == 0);
	_findclose(hFile);
#else
	DIR *dp;
	struct dirent *dirp;
	if ((dp = opendir(basePath)) == NULL)
	{
		return -1;
	}

	while ((dirp = readdir(dp)) != NULL)
	{
		if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
		{
			if (iDepathTmp > 0) {
				string strFile;
				if (bContainerPath)
				{
					strFile = strPath + dirp->d_name;
				}
				vFile.push_back(strFile);
			}
		}
		else {
			//string strFile;
			//if (bContainerPath)
			//{
			//	strFile = strPath + dirp->d_name;
			//}
		}

		//vFile.push_back(strFile);
	}
#endif
	return 0;
}

//���ܷ���Int���ر���
//1��-1��Ϊ�棬����С�ǲ�ͬ�ģ�
//Ԫ�ؽ���������±������ڼ�������£��������������Ԫ�ض���һ��������£�
//����������£��ͻ���ַ���Խ�磬������ǵ��³������segment fault
//c++ stl�еıȽϺ����ǣ�bool�������ʱ��һ���ǡ���ġ��󣬻���С�����ڵ�ʱ��ֻ�ܷ���false��
bool StrCompare(const string& str1, const string& str2)
{
	if (0 <= strcmp(str1.c_str(), str2.c_str()))
		return true;
	else 
		return false;
}

unsigned int ReverseByte(char *c, int nLen)
{
	unsigned int r = 0;
	int i;
	for (i = 0; i < nLen; i++)
	{
		r |= (*(c + i) << (((nLen - 1) * 8) - 8 * i));
	}
	return r;
}

short BigLitEndianChange2Short(const unsigned char *pData)
{
	short r = 0;
	int i;
	for (i = 0; i < 2; i++)
	{
		r |= (*(pData + i) << (((2 - 1) * 8) - 8 * i));
	}
	return r;
}
int BigLitEndianChange2Int(const unsigned char *pData)
{
	int r = 0;
	int i;
	for (i = 0; i < 4; i++)
	{
		r |= (*(pData + i) << (((4 - 1) * 8) - 8 * i));
	}
	return r;
}

short GetShort(const unsigned char *pData)
{
	short r = 0;
	memcpy(&r, pData, 2);
	return r;
}
int GetInt(const unsigned char *pData)
{
	int r = 0;

	memcpy(&r, pData, 4);
	return r;
}
float GetFloat(const unsigned char *pData)
{
	float r = 0;

	memcpy(&r, pData, 4);
	return r;
}
unsigned int GetRandom()
{
	default_random_engine e(time(0));
	return e();
}

double GetDouble(const unsigned char *pData)
{
	double r = 0;

	memcpy(&r, pData, 8);
	return r;
}
unsigned long long GetLongLong(const unsigned char *pData)
{
	unsigned long long r = 0;

	memcpy(&r, pData, 8);
	return r;
}

int InitLog(string strPath, string strName)
{
	/*ֻnLogNum�����־�ļ�*/
	const size_t nLogNum = m_maxLogFileCount;
	//��ȡ��־Ŀ¼�������ļ�
	vector<string> vFiles;
	readFileList(strPath.c_str(), vFiles, true);
	if (vFiles.size() > nLogNum)//����nLogNum����־�ļ�
	{
		std::sort(vFiles.begin(), vFiles.end(), StrCompare);//�����µ�������������־�ļ�
		for (size_t i = vFiles.size() - 1; i >= nLogNum; i--)//ɾ����ɵ�vFiles.size()-nLogNum����־�ļ�
		{
			string sFile = vFiles[i];
			remove(sFile.c_str());
		}
	}

	//ѹ����־�ļ�
	vector<string>::iterator it = vFiles.begin();
	while(it != vFiles.end())
	{
		string sFile = *it;
		if (sFile.length()>4)
		{
			string ext = sFile.substr(sFile.length() - 4, 4);
			if (ext == ".zip")
			{
				it = vFiles.erase(it);//zip�ļ�����Ҫ��ѹ��
				continue;
			}
			else if (ext != ".log")
			{
				remove(sFile.c_str());
				it = vFiles.erase(it);//ɾ��������־�ļ�
				continue;
			}
		}
		else
		{
			remove(sFile.c_str());
			it = vFiles.erase(it);//ɾ��������־�ļ�
			continue;
		}
		it++;
	}

	//��ǰ��־�ļ�����YYYYMMDD.log
	//string strLogPath = "C:/DAG_LOG/iRVMS-IPC/";
	string strLogFile = strPath + strName;
	strLogFile += ".log";
	LogLevel level = LL_DEBUG;
	if (CGmsConfig::GetInstance()->GetConfig().logLevel >= LL_TRACE && CGmsConfig::GetInstance()->GetConfig().logLevel <= LL_ERROR)
	{
		level = (LogLevel)CGmsConfig::GetInstance()->GetConfig().logLevel;
	}
	log_init(level, strLogFile.c_str());
	thread thrCompress(ZipCompressFile, vFiles);
	thrCompress.detach();

	return 0;
}
#ifndef _WIN32
#include <sys/wait.h>
#endif
typedef void(*sighandler_t)(int);
static int SystemDag(const char* command)
{
#ifdef _WIN32
	system(command);
#else
	sighandler_t old_handler;
	old_handler = signal(SIGCHLD, SIG_DFL);
	system(command);
	signal(SIGCHLD, old_handler);
#endif
	return 0;;
}

void ZipCompressFile(vector<string> file)
{
#ifdef _WIN32
#else
	char command[1024] = { 0 };
	for (size_t i = 0; i < file.size(); i++)
	{
		string _file = file[i];
		int pos = _file.find_last_of("/");
		if (pos == string::npos)
		{
			continue;
		}
		string path = _file.substr(0, pos + 1);
		string name = _file.substr(pos + 1, _file.length() - pos - 5);
		string zipFile = path + name + ".zip";
		sprintf(command, "zip -jm9  %s -xi %s", zipFile.c_str(), _file.c_str());
		SystemDag(command);
	}
#endif
}

time_t StringToDatetime(string str)
{
	char *cha = (char*)str.data();             // ��stringת����char*��
	tm tm_;                                    // ����tm�ṹ�塣
	int year, month, day, hour, minute, second;// ����ʱ��ĸ���int��ʱ������
	sscanf(cha, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);// ��string�洢������ʱ�䣬ת��Ϊint��ʱ������
	tm_.tm_year = year - 1900;                 // �꣬����tm�ṹ��洢���Ǵ�1900�꿪ʼ��ʱ�䣬����tm_yearΪint��ʱ������ȥ1900��
	tm_.tm_mon = month - 1;                    // �£�����tm�ṹ����·ݴ洢��ΧΪ0-11������tm_monΪint��ʱ������ȥ1��
	tm_.tm_mday = day;                         // �ա�
	tm_.tm_hour = hour;                        // ʱ��
	tm_.tm_min = minute;                       // �֡�
	tm_.tm_sec = second;                       // �롣
	tm_.tm_isdst = 0;                          // ������ʱ��
	time_t t_ = mktime(&tm_);                  // ��tm�ṹ��ת����time_t��ʽ��
	return t_;                                 // ����ֵ�� 
}

tm StringToTm(string str) {
	char *cha = (char*)str.data();             // ��stringת����char*��
	tm tm_;                                    // ����tm�ṹ�塣
	int year, month, day, hour, minute, second;// ����ʱ��ĸ���int��ʱ������
	sscanf(cha, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);// ��string�洢������ʱ�䣬ת��Ϊint��ʱ������
	tm_.tm_year = year ;                 // �꣬����tm�ṹ��洢���Ǵ�1900�꿪ʼ��ʱ�䣬����tm_yearΪint��ʱ������ȥ1900��
	tm_.tm_mon = month ;                    // �£�����tm�ṹ����·ݴ洢��ΧΪ0-11������tm_monΪint��ʱ������ȥ1��
	tm_.tm_mday = day;                         // �ա�
	tm_.tm_hour = hour;                        // ʱ��
	tm_.tm_min = minute;                       // �֡�
	tm_.tm_sec = second;                       // �롣
	tm_.tm_isdst = 0;                          // ������ʱ��
	return tm_;                                 // ����ֵ�� 
}

tm StringToTm(string str,string format) {
	char *cha = (char*)str.data();             // ��stringת����char*��
	tm tm_;                                    // ����tm�ṹ�塣
	int year, month, day, hour, minute, second;// ����ʱ��ĸ���int��ʱ������
	sscanf(cha, format.data(), &year, &month, &day, &hour, &minute, &second);// ��string�洢������ʱ�䣬ת��Ϊint��ʱ������
	tm_.tm_year = year;                 // �꣬����tm�ṹ��洢���Ǵ�1900�꿪ʼ��ʱ�䣬����tm_yearΪint��ʱ������ȥ1900��
	tm_.tm_mon = month;                    // �£�����tm�ṹ����·ݴ洢��ΧΪ0-11������tm_monΪint��ʱ������ȥ1��
	tm_.tm_mday = day;                         // �ա�
	tm_.tm_hour = hour;                        // ʱ��
	tm_.tm_min = minute;                       // �֡�
	tm_.tm_sec = second;                       // �롣
	tm_.tm_isdst = 0;                          // ������ʱ��
	return tm_;                                 // ����ֵ�� 
}

string DatetimeToString(time_t time)
{
	tm *tm_ = localtime(&time);                // ��time_t��ʽת��Ϊtm�ṹ��
	int year, month, day, hour, minute, second;// ����ʱ��ĸ���int��ʱ������
	year = tm_->tm_year + 1900;                // ��ʱ�������꣬����tm�ṹ��洢���Ǵ�1900�꿪ʼ��ʱ�䣬������ʱ����intΪtm_year����1900��
	month = tm_->tm_mon + 1;                   // ��ʱ�������£�����tm�ṹ����·ݴ洢��ΧΪ0-11��������ʱ����intΪtm_mon����1��
	day = tm_->tm_mday;                        // ��ʱ�������ա�
	hour = tm_->tm_hour;                       // ��ʱ������ʱ��
	minute = tm_->tm_min;                      // ��ʱ�������֡�
	second = tm_->tm_sec;                      // ��ʱ�������롣
	char yearStr[5], monthStr[3], dayStr[3], hourStr[3], minuteStr[3], secondStr[3];// ����ʱ��ĸ���char*������
	sprintf(yearStr, "%d", year);              // �ꡣ
	sprintf(monthStr, "%d", month);            // �¡�
	sprintf(dayStr, "%d", day);                // �ա�
	sprintf(hourStr, "%d", hour);              // ʱ��
	sprintf(minuteStr, "%d", minute);          // �֡�
	if (minuteStr[1] == '\0')                  // �����Ϊһλ����5������Ҫת���ַ���Ϊ��λ����05��
	{
		minuteStr[2] = '\0';
		minuteStr[1] = minuteStr[0];
		minuteStr[0] = '0';
	}
	sprintf(secondStr, "%d", second);          // �롣
	if (secondStr[1] == '\0')                  // �����Ϊһλ����5������Ҫת���ַ���Ϊ��λ����05��
	{
		secondStr[2] = '\0';
		secondStr[1] = secondStr[0];
		secondStr[0] = '0';
	}
	char s[100] = { 0 };                                // ����������ʱ��char*������
	sprintf(s, "%s-%s-%s %s:%s:%s", yearStr, monthStr, dayStr, hourStr, minuteStr, secondStr);// ��������ʱ����ϲ���
	string str(s);                             // ����string����������������ʱ��char*������Ϊ���캯���Ĳ������롣
	return str;                                // ����ת������ʱ����string������
}

string TmToString(const tm tm_)
{
	int year, month, day, hour, minute, second;// ����ʱ��ĸ���int��ʱ������
	year = tm_.tm_year;                // ��ʱ�������꣬����tm�ṹ��洢���Ǵ�1900�꿪ʼ��ʱ�䣬������ʱ����intΪtm_year����1900��
	month = tm_.tm_mon;                   // ��ʱ�������£�����tm�ṹ����·ݴ洢��ΧΪ0-11��������ʱ����intΪtm_mon����1��
	day = tm_.tm_mday;                        // ��ʱ�������ա�
	hour = tm_.tm_hour;                       // ��ʱ������ʱ��
	minute = tm_.tm_min;                      // ��ʱ�������֡�
	second = tm_.tm_sec;                      // ��ʱ�������롣
	char yearStr[5], monthStr[3], dayStr[3], hourStr[3], minuteStr[3], secondStr[3];// ����ʱ��ĸ���char*������
	sprintf(yearStr, "%d", year);              // �ꡣ
	sprintf(monthStr, "%d", month);            // �¡�
	sprintf(dayStr, "%d", day);                // �ա�
	sprintf(hourStr, "%d", hour);              // ʱ��
	sprintf(minuteStr, "%d", minute);          // �֡�
	if (minuteStr[1] == '\0')                  // �����Ϊһλ����5������Ҫת���ַ���Ϊ��λ����05��
	{
		minuteStr[2] = '\0';
		minuteStr[1] = minuteStr[0];
		minuteStr[0] = '0';
	}
	sprintf(secondStr, "%d", second);          // �롣
	if (secondStr[1] == '\0')                  // �����Ϊһλ����5������Ҫת���ַ���Ϊ��λ����05��
	{
		secondStr[2] = '\0';
		secondStr[1] = secondStr[0];
		secondStr[0] = '0';
	}
	char s[100] = {0};                                // ����������ʱ��char*������
	sprintf(s, "%s-%s-%s %s:%s:%s", yearStr, monthStr, dayStr, hourStr, minuteStr, secondStr);// ��������ʱ����ϲ���
	string str(s);                             // ����string����������������ʱ��char*������Ϊ���캯���Ĳ������롣
	return str;                                // ����ת������ʱ����string������
}

string GetCurDirectory()
{
	string strPath;

#if defined _WIN32 
	char szBuf[256] = { 0 };
	//char szBuf[256] = { 0 };
	//_getcwd(szBuf, 256);
	//strPath = szBuf;
//#if defined _DEBUG
	//LOG_DEBUG("_getcwd %s", szBuf);

	GetModuleFileName(NULL, szBuf, 256);
	string s;
#ifdef _WIN64
	wchar_t szBuf64[256] = { 0 };
	s = wstring2string(szBuf64);
#else
	s = szBuf;
#endif
	int nPos = s.find_last_of(GetDirectorySpace());
	if (string::npos != nPos)
	{
		strPath = s.substr(0, nPos);
	}
//#endif
#else
	char szPath[256] = { 0 };
	getcwd(szPath, 256);
	strPath = szPath;
#endif

	return strPath;
}

#define MAX_PATH_LEN 256
#ifdef _WIN32
#define ACCESS(fileName,accessMode) _access(fileName,accessMode)
#define MKDIR(path) _mkdir(path)
#else
#define ACCESS(fileName,accessMode) access(fileName,accessMode)
#define MKDIR(path) mkdir(path,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#endif
// �����������ж��ļ����Ƿ����,�����ھʹ���
// example: /home/root/mkdir/1/2/3/4/
// ע��:���һ��������ļ��еĻ�,��Ҫ���� '\' ���� '/'
int32_t createDirectory(string directoryPath)
{
	uint32_t dirPathLen = directoryPath.length();
	if (dirPathLen > MAX_PATH_LEN)
	{
		return -1;
	}
	char tmpDirPath[MAX_PATH_LEN] = { 0 };
	for (uint32_t i = 0; i < dirPathLen; ++i)
	{
		tmpDirPath[i] = directoryPath[i];
		if (tmpDirPath[i] == '\\' || tmpDirPath[i] == '/')
		{
			if (ACCESS(tmpDirPath, 0) != 0)
			{
				int32_t ret = MKDIR(tmpDirPath);
				if (ret != 0)
				{
					return ret;
				}
			}
		}
	}
	return 0;
}

unsigned char CheckSum(const unsigned char* bytes, int len, int offset)
{
	unsigned char sum = 0;
	for (int i = offset; i < len; i++)
	{
		sum += bytes[i];
	}
	return sum;
}
unsigned char XORSum(const unsigned char* bytes, int len, int offset)
{
	unsigned char sum = 0;
	for (int i = offset; i < len; i++)
	{
		sum ^= bytes[i];
	}
	return sum;
}

string Fill0InFront(string source, int expectLength)
{
	string ret;
	if (source.length()>=expectLength)
	{
		return source;
	}

	int fix = expectLength - source.length();
	for (int i=0; i<fix; i++)
	{
		ret += "0";
	}
	ret += source;
	return ret;
}

static unsigned short const wCRC16Table[256] = {
	0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
	0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
	0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
	0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
	0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
	0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
	0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
	0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
	0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
	0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
	0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
	0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
	0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
	0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
	0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
	0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
	0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
	0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
	0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
	0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
	0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
	0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
	0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
	0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
	0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
	0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
	0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
	0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
	0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
	0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
	0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
	0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040 };



//unsigned short CRC16(const unsigned char* pDataIn, int iLenIn)
//{
//	unsigned short wResult = 0;
//	unsigned short wTableNo = 0;
//	for (int i = 0; i < iLenIn; i++)
//	{
//		wTableNo = ((wResult & 0xff) ^ (pDataIn[i] & 0xff));
//		wResult = ((wResult >> 8) & 0xff) ^ wCRC16Table[wTableNo];
//	}
//
//	return wResult;
//}

/*
*********************************************************************************************************
* �� �� ��: CRC8
* ����˵��: crc8У���㷨���ɰ汾΢��ʹ�ø��㷨����У��ֵ
* ��    �Σ�pData��У�������
			  len��У�����鳤��
* �� �� ֵ: Crc������У��ֵ
* ע    �ͣ�
*********************************************************************************************************
*/
unsigned char VIBRATION_CRC8(unsigned char* pData, int len)
{
	unsigned char Crc;
	unsigned char ch[8];
	unsigned char ch1;
	unsigned char i, j, k;
	unsigned char szData[1024] = { 0 };
	memcpy(szData, pData, len);

	//len++;
	Crc = 0xff;
	pData = szData;
	for (i = 0; i < len; i++)
	{
		ch1 = pData[i];
		for (j = 0; j < 8; j++)
		{
			ch[j] = ch1 & (unsigned char)0x01;
			ch1 >>= 1;
		}
		for (k = 0; k < 8; k++)
		{
			ch[7 - k] <<= 7;
			if (((Crc ^ ch[7 - k]) & (unsigned char)0x80) != 0)
			{
				Crc = (Crc << 1) ^ (unsigned char)0x1d;
			}
			else
			{

				Crc <<= 1;
			}

		}
	}
	Crc ^= 0xff;
	return Crc;
}
unsigned short CRC16(const unsigned char* pDataIn, int iLenIn)
{
	unsigned short CRCFull = 0xFFFF;
	char CRCLSB;
	for (int i = 0; i < iLenIn; i++)
	{
		CRCFull = (unsigned short)(CRCFull ^ pDataIn[i]);
		for (int j = 0; j < 8; j++)
		{
			CRCLSB = (char)(CRCFull & 0x0001);
			CRCFull = (unsigned short)((CRCFull >> 1) & 0x7FFF);
			if (CRCLSB == 1)
				CRCFull = (unsigned short)(CRCFull ^ 0xA001);
		}
	}
	return CRCFull;
}
//
//
///**
//���԰�16λ��float IEEE754�淶��intֵת��32λ��float
//
//	*/
//
//float halfFloatIntToFloat(int halfFloat) {
//	int floatInt = ((halfFloat & 0x8000) << 16) | (((((halfFloat >> 10) & 0x1f) - 15 + 127) & 0xff) << 23) | ((halfFloat & 0x03FF) << 13);
//	return Float.intBitsToFloat(floatInt);
//}
///**
//���԰�32��floatֵת��ΪIEEE754�淶��intֵ
//
//	*/
//
//public static int floatToHalfFloatInt(float f) {
//	int floatInt = Float.floatToIntBits(f);
//	return ((floatInt >> 16) & 0x8000) | ((((floatInt >> 23) - 127 + 15) & 0x1f) << 10) | ((floatInt >> 13) & 0x3ff);
//}
//
////��intֵÿ��λת��һ���ֽڣ�����һ���ֽ�����
//public static byte[] intToBytes(int value, int len) {
//
//	byte[] b = new byte[len];
//	for (int i = 0; i < len; i++) {
//		b[len - i - 1] = (byte)((value >> 8 * i) & 0xff);
//	}
//	return b;
//}
////��һ���ֽ����飨ÿ���ֽڴ���һ��int��λ��ת����һ��int��
//public static int bytesToInt(byte[] data, int startIndex, int len) {
//
//	int sum = 0;
//	int endIndex = startIndex + len;
//	for (int i = startIndex; i < endIndex; i++) {
//		int temp = ((int)data[i]) & 0xff;
//		int moveBits = (--len) * 8;
//		temp = temp << moveBits;
//		sum += temp;
//	}
//	return sum;
//}
//
//
//
//

int ChangetoSexagesimal(float Num, int &Hour, int &Minute, int &Second, int& MilliSecond)
{
	float e;
	Hour = (int)(Num);                              //��                        
	Minute = (int)((Num - Hour) * 60);                  //��
	Second = (int)(((Num - Hour) * 60 - Minute) * 60);   //��

	e = ((Num - Hour) * 60 - Minute) * 60 - Second; 

// 	if ((e * 1000)>=999)
// 	{
// 		Second = Second + 1;
// 		e = e - 999;
// 	}
	MilliSecond = e * 1000;

	if (60 == Second)
	{
		Second = 0;
		Minute = Minute + 1;
	}

	if (60 == Minute)
	{
		Minute = 0;
		Hour = Hour + 1;
	}
	return 1;
}

double Get2PointDistance4LatLng(double lngA, double latA, double lngB, double latB)
{
	double r = 6378.137; //�뾶   
			//��A��Ŀռ�����   
	double xA = cos(latA) * cos(lngA);
	double yA = cos(latA) * sin(lngA);
	double zA = sin(latA);

	//��B��Ŀռ�����   
	double xB = cos(latB) * cos(lngB);
	double yB = cos(latB) * sin(lngB);
	double zB = sin(latB);

	//O����Բ��   
	//�����   
	double OA = sqrt(pow(xA, 2) + pow(yA, 2) + pow(zA, 2));
	double OB = sqrt(pow(xB, 2) + pow(yB, 2) + pow(zB, 2));
	double AB = sqrt(pow(xA - xB, 2) + pow(yA - yB, 2) + pow(zA - zB, 2));

	//��OA��OB֮��ļнǻ���   
	double rad = acos((pow(OA, 2) + pow(OB, 2) - pow(AB, 2)) / (2 * OA * OB)) / 180 * M_PI;

	return rad * r;
}
