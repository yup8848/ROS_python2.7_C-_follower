#pragma once
#include <string>
#include <vector>
#include "tinyxml.h"
using namespace std;

struct StConfigItem
{
	int logLevel;       //日志级别
	int logFileSwitch;  //是否打印到文件日志
	int logStdSwitch;   //是否到标准控制台 (正常运行下关闭影响性能)
	int logFileCount;   //日志文件最大值 (按MB计算 默认80 为80MB 超过有创建新文件) 
	StConfigItem()
	{
		logLevel = 2;
		logFileSwitch = 1;
		logStdSwitch = 0;
		logFileCount = 80;
	}
};

class CGmsConfig
{
public:

	static CGmsConfig* GetInstance()
	{
		static CGmsConfig instance;
		return &instance;
	}
	const StConfigItem& GetConfig();
private:
	CGmsConfig();
	virtual ~CGmsConfig();

	// 禁止复制构造函数
	CGmsConfig(const CGmsConfig&) = delete;
	// 禁止对象赋值操作符
	CGmsConfig& operator=(const CGmsConfig&) = delete;

	int LoadConfig();


private:
	StConfigItem m_configItem;

	string m_configFile;
};
