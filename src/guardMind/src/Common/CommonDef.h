#pragma once

#define CORE_SIZE_LINUX 1024*1024*500

//PC端发送心跳/数据包时间间隔
#define PC_HEARTBEAT_TIMEOUT_SECOND						30
//UDP/TCP设备在timeout时间内无任何数据过来，即断开连接
#define KEEP_ALIVE_TIMEOUT_SECOND_NORMAL				60//must > PC_HEARTBEAT_TIMEOUT_SECOND
#define KEEP_ALIVE_TIMEOUT_SECOND_DEFAULT				300//must > PC_HEARTBEAT_TIMEOUT_SECOND
#define KEEP_ALIVE_TIMEOUT_SECOND_NO_CASE				-1//
#define TCP_CONNECT_TIMEOUT_SECOND						5//TCP连接超时时间设置
#define RECONNECT_TIMEOUT_SECOND						180//设备重连时间间隔
#define DEV_HEARTBEAT_CLOCK_EXP							3//平台心跳时间倍数，如平台5秒，则5*4=20s
#define DEV_TARGET_UPLOAD_CLOCK_MILLSECOND				300//目标上送时间间隔（降低频率，部分电脑MQ发送慢问题，暂未找到原因

//cms定义套接字设备连接方式
#define DEV_SOCKET_TYPE_TCP			"0"
#define DEV_SOCKET_TYPE_UDP			"1"
#define DEV_SOCKET_TYPE_TCPSERVER	"2"
#define DEV_SOCKET_TYPE_UDPSERVER	"3"

