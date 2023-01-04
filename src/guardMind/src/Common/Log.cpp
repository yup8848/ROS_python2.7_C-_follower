#include "Log.h"
#include <stdarg.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <iostream>
#include <cstdio>
#include <chrono>
#include <algorithm>
#include "Common.h"
#ifdef _WIN32
#include <io.h>
#include <direct.h> 
#else
#include <unistd.h>
#include <sys/stat.h>
#include <sys/file.h>
#endif
#include <stdint.h>
#include <string>
using namespace std;

Log_Writer INFO_W;

bool log_init(LogLevel l, const char* pFile)
{
	createDirectory(pFile);
	char _location_str[_LOG_PATH_LEN]={0};
    snprintf(_location_str, _LOG_PATH_LEN, "%s", pFile);
    INFO_W.loginit(l, _location_str);

    return true;
}
void log_set_param(bool logStdSwitch, bool logFileSwitch, bool bAppend, bool aIssync, int nLogFileMaxSize)
{
	INFO_W.set_param(logStdSwitch, logFileSwitch, bAppend, aIssync, nLogFileMaxSize);
}

const char* Log_Writer::logLevelToString(LogLevel l) {
        switch ( l ) {
			case LL_DEBUG:
				return "DEBUG";
			case LL_TRACE:
				return "TRACE";
			case LL_WARNING:
				return "WARN" ;
			case LL_ERROR:
				return "ERROR";
			default:
				return "UNKNOWN";
        }
}

void Log_Writer::set_param(bool logStdSwitch, bool logFileSwitch, bool bAppend, bool aIssync, int nLogFileMaxSize)
{
	m_nLogFileMaxSize = nLogFileMaxSize;
	m_isappend = bAppend;
	m_issync = aIssync;
	m_bLogStdSwitch = logStdSwitch;
	m_bLogFileSwitch = logFileSwitch;
}
bool Log_Writer::checklevel(LogLevel level)
{
	if(level >= m_system_level)
		return true;
	else
		return false;
}

bool Log_Writer::loginit(LogLevel level, const  char *filelocation)
{
	m_system_level = level;
	MACRO_RET(NULL != m_fp, false);
	MACRO_RET(!m_bLogFileSwitch, false);
	if(strlen(filelocation) >= (sizeof(m_filelocation) -1))
	{
		//fprintf(stderr, "the path of log file is too long:%d limit:%d\n", strlen(filelocation), sizeof(m_filelocation) -1);
		return false;
	}
	//本地存储filelocation  以防止在栈上的非法调用调用
	memset(m_filelocation, 0, sizeof(m_filelocation));
	strncpy(m_filelocation, filelocation, strlen(filelocation));
	
	if('\0' == m_filelocation[0])
	{
		m_fp = stdout;
		//fprintf(stderr, "now all the running-information are going to put to stderr\n");
		return false;
	}
	
	m_fp = fopen(m_filelocation, m_isappend ? "a":"w");
	if(m_fp == NULL)
	{
		//fprintf(stderr, "cannot open log file,file location is %s\n", m_filelocation);
		return false;
	}
	//setvbuf (m_fp, io_cached_buf, _IOLBF, sizeof(io_cached_buf)); //buf set _IONBF  _IOLBF  _IOFBF
	//setvbuf (m_fp,  (char *)NULL, _IOLBF, 0);
	//fprintf(stderr, "now all the running-information are going to the file %s\n", m_filelocation);
	return true;
}

int Log_Writer::premakestr(char* buffer, LogLevel l)
{
	//cout << "Log_Writer::premakestr 111" << endl;
#ifdef _WIN32
	auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	struct tm* ptm = localtime(&tt);
	
	return sprintf(buffer, "%s: %02d-%02d %02d:%02d:%02d", logLevelToString(l),
		(int)ptm->tm_mon + 1, (int)ptm->tm_mday, (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
#else
	time_t now;
	//cout << "Log_Writer::premakestr 222" << endl;
	now = time(&now);;
	struct tm vtm;
	//cout << "Log_Writer::premakestr 333" << endl;
	localtime_r(&now, &vtm);
	//cout << "Log_Writer::premakestr 444" << endl;
    return snprintf(buffer, _LOG_BUFFSIZE, "%s: %02d-%02d %02d:%02d:%02d", logLevelToString(l),
		vtm.tm_mon + 1, vtm.tm_mday, vtm.tm_hour, vtm.tm_min, vtm.tm_sec);
	//cout << "Log_Writer::premakestr 555" << endl;
#endif 
}

bool Log_Writer::log(LogLevel l, const char* logformat,...)
{
	MACRO_RET(!checklevel(l), false)
	int _size;
	int prestrlen = 0;

	//cout << "Log_Writer::log 111" << endl;
	shared_ptr<char> spResBuf(new char[_LOG_BUFFSIZE], std::default_delete<char[]>());
	memset(spResBuf.get(), 0, _LOG_BUFFSIZE);
	char *pBufStart = NULL;
	char* pLog = spResBuf.get();
	pBufStart = pLog;

	prestrlen = premakestr(pLog, l);
	pLog += prestrlen;
	
	va_list args;
	va_start(args, logformat);

	_size = vsnprintf(pLog, _LOG_BUFFSIZE - prestrlen-1, logformat, args);
	va_end(args);

	if (m_bLogStdSwitch)
	{
		if (strlen(pBufStart) < 4096)
		{
			cout << pBufStart << endl << flush;
		}
	}
	if (m_bLogFileSwitch)
	{
		_write(pBufStart, strlen(pBufStart));//_size返回的长度可能大于实际长度
	}
	return true;
}
bool Log_Writer::_write(char *_pbuffer, int len)
{
	//if(0 != _access(m_filelocation, 0))
	//{
	//	std::lock_guard<std::mutex> guard(m_mutex);
	//	//锁内校验 access 看是否在等待锁过程中被其他线程loginit了  避免多线程多次close 和init
	//	if(0 != _access(m_filelocation, 0))
	//	{
	//		logclose();
	//		loginit(m_system_level, m_filelocation, m_isappend, m_issync, m_bLogFileSwitch, m_bLogStdSwitch);
	//	}
	//}
	{
		std::lock_guard<std::mutex> guard(m_mutex);
		if (NULL == m_fp)
			;//fprintf(stderr, "write log file failed\r\n");
		else
		{
			if (len == fwrite(_pbuffer, 1, len, m_fp)) //only write 1 item
			{
				//if(m_issync)
				fflush(m_fp);
				*_pbuffer = '\0';
				_loginit();
			}
			else
			{
				int x = errno;
				//fprintf(stderr, "Failed to write to logfile. errno:%s    message:%s\r\n", strerror(x), _pbuffer);
				return false;
			}
		}
	}
	return true;
}

bool Log_Writer::_loginit()
{
	if (m_fp)
	{
		int nFileLen = ftell(m_fp);
		if (nFileLen > _LOG_FILE_MAX_SIZE)
		{
			logclose();
			string strFile = m_filelocation;
			int nPos = strFile.find_last_of('/');
			if (string::npos != nPos)
			{
				string strDir = strFile.substr(0, nPos+1);
				string strFileName = GetTimeStampSecond();
				InitLog(strDir, strFileName);
				return true;
			}			
		}
	}
	return false;
}

LogLevel Log_Writer::get_level()
{
	return m_system_level; 
}

bool Log_Writer::logclose()
{
	if(m_fp == NULL)
		return false;

	fflush(m_fp);
	fclose(m_fp);
	m_fp = NULL;
	return true;
}



