#include "Config.h"
#include "Common.h"
#include "ErrorCodeDef.h"
#include "Log.h"

CGmsConfig::CGmsConfig()
{
	string curDir = GetCurDirectory();
#ifdef _WIN32
	m_configFile = curDir + "/gms_config.xml";
#else
	m_configFile = curDir + "/gms_config.xml";
#endif
	LoadConfig();
}
CGmsConfig::~CGmsConfig()
{
}
//加载固定路径下 xml配置文件
int CGmsConfig::LoadConfig()
{
	TiXmlDocument doc;
	if (!doc.LoadFile(m_configFile.c_str()))
	{
		LOG_ERROR("load %s failed", m_configFile.c_str());
		return err_xml;
	}

	TiXmlHandle xmlRoot(&doc);
	TiXmlElement* e = xmlRoot.FirstChildElement("cms_server_ip").Element();			
	e = xmlRoot.FirstChildElement("gms_log_output_level").Element();
	if (e)
	{
		if (e->GetText() != NULL)
		{
			m_configItem.logLevel = atoi(e->GetText());
		}
	}
	if (m_configItem.logLevel < LL_TRACE || m_configItem.logLevel > LL_ERROR)
	{
		m_configItem.logLevel = LL_DEBUG;
	}

	e = xmlRoot.FirstChildElement("gms_log_file_switch").Element();
	if (e)
	{
		if (e->GetText() != NULL)
		{
			m_configItem.logFileSwitch = atoi(e->GetText());
		}
	}

	e = xmlRoot.FirstChildElement("gms_log_std_switch").Element();
	if (e)
	{
		if (e->GetText() != NULL)
		{
			m_configItem.logStdSwitch = atoi(e->GetText());
		}
	}

	e = xmlRoot.FirstChildElement("gms_log_output_level").Element();
	if (e)
	{
		if (e->GetText() != NULL)
		{
			m_configItem.logLevel = atoi(e->GetText());
		}
	}
	if (m_configItem.logLevel < LL_TRACE || m_configItem.logLevel > LL_ERROR)
	{
		m_configItem.logLevel = LL_DEBUG;
	}

	e = xmlRoot.FirstChildElement("gms_log_file_count").Element();
	if (e)
	{
		if (e->GetText() != NULL)
		{
			m_configItem.logFileCount = atoi(e->GetText());
		}
	}
	if (m_configItem.logFileCount <= 0 || m_configItem.logFileCount > 500)
	{
		m_configItem.logFileCount = _LOG_FILE_COUNT_DEFAULT;
	}
	return err_Success;
}

const StConfigItem& CGmsConfig::GetConfig()
{
	return m_configItem;
}