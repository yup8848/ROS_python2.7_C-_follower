#pragma once
enum E_CONN_MESS_TYPE
{
	MESS_READ = 0,//收到数据
	MESS_WRITE,//写入数据
	MESS_NO_ALIVE,//设备可能断开连接，需要客户端重新发送数据包建立连接(UDP)
	MESS_CLOSE,//关闭连接，需要客户端重新建立连接并发送数据包
	MESS_CONNECTED,//异步连接成功
	MESS_EXIT
};

//待连接设备的协议类型
enum E_PROTOCOL_TYPE
{
	PROTOCOL_NO_CASE = 0,//上层能通过Ip+端口,或者deviceId获取到设备协议和设备类型
	PROTOCOL_GUARD_MIND,
	PROTOCOL_UNKNOWN,
	PROTOCOL_TEST
};

enum E_SOCKET_TYPE
{
	SOCKET_TCP = 0,
	SOCKET_UDP = 1
};