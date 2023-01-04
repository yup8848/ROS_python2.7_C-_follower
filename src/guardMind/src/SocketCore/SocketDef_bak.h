#pragma once
enum E_CONN_MESS_TYPE
{
	MESS_READ = 0,//�յ�����
	MESS_WRITE,//д������
	MESS_NO_ALIVE,//�豸���ܶϿ����ӣ���Ҫ�ͻ������·������ݰ���������(UDP)
	MESS_CLOSE,//�ر����ӣ���Ҫ�ͻ������½������Ӳ��������ݰ�
	MESS_CONNECTED,//�첽���ӳɹ�
	MESS_EXIT
};

//�������豸��Э������
enum E_PROTOCOL_TYPE
{
	PROTOCOL_NO_CASE = 0,//�ϲ���ͨ��Ip+�˿�,����deviceId��ȡ���豸Э����豸����
	PROTOCOL_GUARD_MIND,
	PROTOCOL_UNKNOWN,
	PROTOCOL_TEST
};

enum E_SOCKET_TYPE
{
	SOCKET_TCP = 0,
	SOCKET_UDP = 1
};