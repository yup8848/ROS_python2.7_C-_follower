#include "ipconfig.h"



#define MAX_IP_ADDRESS_NUM		32
typedef struct __IP_ADDRESS_T
{
	int	ipver;		//4		6
	char	ethName[36];
	char	addr[64];
}IP_ADDRESS_T;
int				localIPAddressNum = 0;
IP_ADDRESS_T	localIPAddressList[MAX_IP_ADDRESS_NUM];

int		GetLocalIPAddress(int index, char *ipaddr, int *num)
{
	if (localIPAddressNum > 0)
	{
		if (index < localIPAddressNum)
		{
			strcpy(ipaddr, localIPAddressList[index].addr);

			if (NULL != num)		*num = localIPAddressNum;
		}

		return 0;
	}


	memset(&localIPAddressList[0], 0x00, sizeof(localIPAddressList));


#ifdef _WIN32
	char szHostname[128] = {0};
	gethostname(szHostname, sizeof(szHostname));
	if ( (int)strlen(szHostname) > 0)
	{
		hostent *p = gethostbyname(szHostname);
		if (NULL != p)
		{
			char *pAddr = NULL;
			int i = 0;
			do
			{
				pAddr = p->h_addr_list[i];
				if (NULL == pAddr)			break;
				struct in_addr inAddr;
				memcpy(&inAddr, pAddr, p->h_length);
				strcpy(localIPAddressList[i].addr, inet_ntoa(inAddr));

				i++;
				if (i>=MAX_IP_ADDRESS_NUM)		break;
			} while (pAddr);

			if (i>0)
			{
				localIPAddressNum = i;
				return GetLocalIPAddress(index, ipaddr, num);
			}
		}
	}
#else
	struct ifaddrs * ifAddrStruct=NULL;
	void * tmpAddrPtr=NULL;
	int i=0;

	getifaddrs(&ifAddrStruct);


	while (ifAddrStruct!=NULL) 
	{
		if (ifAddrStruct->ifa_addr->sa_family==AF_INET) 
		{ 
			// check it is IP4
			// is a valid IP4 Address
			tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
			char addressBuffer[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);

			localIPAddressList[i].ipver = 4;
			strcpy(localIPAddressList[i].ethName, ifAddrStruct->ifa_name);
			strcpy(localIPAddressList[i].addr, addressBuffer);
			i++;

			if (i>=MAX_IP_ADDRESS_NUM)		break;
		} 
		else if (ifAddrStruct->ifa_addr->sa_family==AF_INET6) 
		{	
			// check it is IP6
			// is a valid IP6 Address
			tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
			char addressBuffer[INET6_ADDRSTRLEN];
			inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);

			localIPAddressList[i].ipver = 6;
			strcpy(localIPAddressList[i].ethName, ifAddrStruct->ifa_name);
			strcpy(localIPAddressList[i].addr, addressBuffer);
			i++;

			if (i>=MAX_IP_ADDRESS_NUM)		break;
		}
		ifAddrStruct=ifAddrStruct->ifa_next;
	}

	if (i>0)
	{
		localIPAddressNum = i;
		return GetLocalIPAddress(index, ipaddr, num);
	}
#endif

	return -1;
}


int FindLocalServer(const char *ip)
{
	int index = 0, num = 0;
	char szLocalIP[32];
	do
	{
		memset(szLocalIP, 0x00, sizeof(szLocalIP));
		GetLocalIPAddress(index, szLocalIP, &num);
		if (0 == strcmp(szLocalIP, ip))
			return 0;

		index++;
	} while (index < num);

	return -1;
}