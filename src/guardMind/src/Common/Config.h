#pragma once
#include <string>
#include <vector>
#include "tinyxml.h"
using namespace std;

struct StConfigItem
{
	int logLevel;       //��־����
	int logFileSwitch;  //�Ƿ��ӡ���ļ���־
	int logStdSwitch;   //�Ƿ񵽱�׼����̨ (���������¹ر�Ӱ������)
	int logFileCount;   //��־�ļ����ֵ (��MB���� Ĭ��80 Ϊ80MB �����д������ļ�) 
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

	// ��ֹ���ƹ��캯��
	CGmsConfig(const CGmsConfig&) = delete;
	// ��ֹ����ֵ������
	CGmsConfig& operator=(const CGmsConfig&) = delete;

	int LoadConfig();


private:
	StConfigItem m_configItem;

	string m_configFile;
};
