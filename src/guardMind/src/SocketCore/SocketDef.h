#pragma once
typedef enum
{
	MESS_READ = 0,//�յ�����
	MESS_WRITE,//д������
	MESS_NO_ALIVE,//�豸���ܶϿ����ӣ���Ҫ�ͻ������·������ݰ���������(UDP)
	MESS_CLOSE,//�ر����ӣ���Ҫ�ͻ������½������Ӳ��������ݰ�
	MESS_CONNECTED,//�첽���ӳɹ�
	MESS_EXIT
}eCONN_MESS_Type;

//�������豸��Э������
typedef enum
{
    PROTOCOL_NO_CASE = 0,//�ϲ���ͨ��Ip+�˿�,����deviceId��ȡ���豸Э����豸����
    PROTOCOL_GUARD_MIND,
    PROTOCOL_UNKNOWN,
    PROTOCOL_MINDTODISPLAY,
    PROTOCOL_DISPLAYTOMIND,
    PROTOCOL_DISPLAYTOGARAGE,
    PROTOCOL_GARAGETODISPLAY
}eProtocolType;

typedef enum
{
	SOCKET_TCP = 0,
	SOCKET_UDP = 1
}eSocketType;
