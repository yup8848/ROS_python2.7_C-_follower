#pragma once
#include <cstdio>
#include <thread>
#include <mutex>
#include "LogMacro.h"
using namespace std;

/*buffer size*/
#define   _LOG_BUFFSIZE  1024*10
//#define   _LOG_BUFFSIZE  1024*1024*4
/*Stream IO buffer size*/
//#define   _SYS_BUFFSIZE  1024*1024*8
#define	  _LOG_PATH_LEN  256
#define   _LOG_MODULE_LEN 32

#define _LOG_FILE_COUNT_DEFAULT			100
#define _LOG_FILE_MAX_SIZE 50*1024*1024	//������־��־�ļ����50MB

#define _LOG_STD_SWITCH true
#define _LOG_FILE_SWITCH true

typedef  enum LogLevel {
	LL_TRACE = 1,
	LL_DEBUG = 2,
	LL_WARNING = 3, 
	LL_ERROR = 4,
}LogLevel;

class Log_Writer
{
	public:
		Log_Writer()
		{
			m_system_level = LL_TRACE;
			m_fp = NULL;
			m_issync = false;
			m_isappend = true;
			m_filelocation[0] ='\0'; 
			m_bLogStdSwitch = false;
			m_nLogFileMaxSize = _LOG_FILE_MAX_SIZE;
			m_bLogFileSwitch = _LOG_FILE_SWITCH;
		}
		~Log_Writer(){
			logclose();
		}
		bool loginit(LogLevel l, const  char *filelocation);
		bool log(LogLevel l, const char *logformat,...);
		LogLevel get_level();
		bool logclose();
		void set_param(bool logStdSwitch = _LOG_STD_SWITCH, bool logFileSwitch = _LOG_FILE_SWITCH, bool bAppend = true,
			bool aIssync = false, int nLogFileMaxSize = _LOG_FILE_MAX_SIZE);
	private:
		const char* logLevelToString(LogLevel l);
		bool checklevel(LogLevel l);
		int premakestr(char* m_buffer, LogLevel l);
		bool _write(char *_pbuffer, int len);
		bool _loginit();//��־�ļ�������С���������ļ�
	private:
		enum LogLevel m_system_level;
		FILE* m_fp;
		bool m_issync;
		bool m_isappend;
		char m_filelocation[_LOG_PATH_LEN];
		mutex m_mutex;
		//static char m_buffer[_LOG_BUFFSIZE];
		bool m_bLogStdSwitch;
		bool m_bLogFileSwitch;
		int m_nLogFileMaxSize;
};



extern Log_Writer INFO_W;
//logFileSwitch:��־�Ƿ�������ļ�
//logStdSwitch:��־�Ƿ��������׼���
bool log_init(LogLevel l, const char* pFile);


void log_set_param(bool logStdSwitch = _LOG_STD_SWITCH, bool logFileSwitch = _LOG_FILE_SWITCH, bool bAppend = true,
	bool aIssync = false, int nLogFileMaxSize = _LOG_FILE_MAX_SIZE);

