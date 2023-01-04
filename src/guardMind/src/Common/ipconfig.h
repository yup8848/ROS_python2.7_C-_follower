#ifndef __IP_CONFIG_H__
#define __IP_CONFIG_H__


#include <stdio.h>
#ifdef _WIN32
#include <winsock2.h>

#else

#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

#endif



int		GetLocalIPAddress(int index, char *ipaddr, int *num);

int		FindLocalServer(const char *ip);

#endif
