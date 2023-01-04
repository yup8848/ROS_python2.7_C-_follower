#pragma once

enum E_ERROR
{
	err_Success = 0,//成功
	err_SocketInit = -1,//socket出初始化失败
	err_SocketWrite = -2,//socket写数据失败
	err_SocketRead = -3,//socket读数据失败
	err_LibEventBase = -4,//livent event
	err_LoadDllFailed = -5,//加载DLL失败
	err_FrameInvalid = -6,//协议帧解析失败
	err_Connect = -7,//链接失败
	err_SubScribe = -8,//MQ订阅
	err_Publish = -9,//MQ发布
	err_ResultFailed = -10,//命令执行失败
	err_ParamInvalid = -11,//参数非法
	err_CRC = -12,//crc校验
	err_DataInvalid = -13,//数据非法
	err_DeviceNotFound = -14,//未找到设备
	err_Exosip = -15,
	err_JsonParser = -16,
	err_CMS = -17,
	err_ProtocolUnsupport = -18,
	err_NotConnect = -19,
	err_Handle = -20,
	err_Timeout = -21,
	err_FunctionUnsupport = -22,
	err_UnSubScribe = -23,
	err_xml = -24,
	err_CommandUnsupport = -25,
	err_CommandFailed = -26,
	err_Curl = -27,
	err_File = -28,
	err_internal_error = -29,
	err_socket_bind = -30,
	err_check_netadapter = -31,
	err_open_file_error = -32,
	err_dev_allready_connected = -33,
	err_buffer = -34,
	err_Frame_Not_All = -35,
	err_Device_Not_In_LoadPoint = -36,
	err_unkown = -1000
};


#define ERROR_MESSAGE_OK "OK"