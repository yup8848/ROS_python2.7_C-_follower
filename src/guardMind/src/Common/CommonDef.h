#pragma once

#define CORE_SIZE_LINUX 1024*1024*500

//PC�˷�������/���ݰ�ʱ����
#define PC_HEARTBEAT_TIMEOUT_SECOND						30
//UDP/TCP�豸��timeoutʱ�������κ����ݹ��������Ͽ�����
#define KEEP_ALIVE_TIMEOUT_SECOND_NORMAL				60//must > PC_HEARTBEAT_TIMEOUT_SECOND
#define KEEP_ALIVE_TIMEOUT_SECOND_DEFAULT				300//must > PC_HEARTBEAT_TIMEOUT_SECOND
#define KEEP_ALIVE_TIMEOUT_SECOND_NO_CASE				-1//
#define TCP_CONNECT_TIMEOUT_SECOND						5//TCP���ӳ�ʱʱ������
#define RECONNECT_TIMEOUT_SECOND						180//�豸����ʱ����
#define DEV_HEARTBEAT_CLOCK_EXP							3//ƽ̨����ʱ�䱶������ƽ̨5�룬��5*4=20s
#define DEV_TARGET_UPLOAD_CLOCK_MILLSECOND				300//Ŀ������ʱ����������Ƶ�ʣ����ֵ���MQ���������⣬��δ�ҵ�ԭ��

//cms�����׽����豸���ӷ�ʽ
#define DEV_SOCKET_TYPE_TCP			"0"
#define DEV_SOCKET_TYPE_UDP			"1"
#define DEV_SOCKET_TYPE_TCPSERVER	"2"
#define DEV_SOCKET_TYPE_UDPSERVER	"3"

