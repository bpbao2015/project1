//朱景尧 by 2015-4-20

//gcc -o proxy tcp-proxy.c -lpthread -D__UNIX -L. -lmysock
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>

#ifndef __UNIX
#include <process.h>
#pragma comment(lib, "mysock.lib")
#pragma warning(disable:4996)
#else
#include <pthread.h>
#endif

#include "mysock.h"

/////////////////////////TCP代理抓包端程序/////////////////////////////////

#define BUFSIZE 16384 //数据缓冲区大小，16k
int sock = 0;
typedef struct
{
	int src_sock;
	int dest_sock;
	char direct[100];
} sock_pair;


#ifndef __UNIX
void proxy_func(void *p)//处理客户端到服务端数据线程
#else
void *proxy_func(void *p)
#endif
{
	sock_pair *sock = (sock_pair *)p;
	int src_sock = sock->src_sock;
	int dest_sock = sock->dest_sock;
	char direct[100] = { 0 };
	strcpy(direct, sock->direct);
	free(sock);

	char *buf = (char *)malloc(BUFSIZE);
	while (1)
	{
		memset(buf, 0, BUFSIZE);
		int rc = tcp_recv(src_sock, buf, BUFSIZE);
		if (rc < 0)
		{
			if (errno != 0)
				printf("%s recv fail:%s\n", direct, strerror(errno));
			break;
		}
		if (rc == 0)
		{
			break;
		}
		rc = tcp_send(dest_sock, buf, rc);
		printf("%s:\n%s\n", direct, buf);
		if (rc < 0)
		{
			printf("%s send fail:%s\n", direct, strerror(errno));
			break;
		}
		if (rc == 0)
		{
			break;
		}
	}
	close_socket(src_sock);
	close_socket(dest_sock);
	free(buf);

#ifndef __UNIX
	return;
#else
	return NULL;
#endif	
}

#ifndef __UNIX
void control_func(void *p)
#else
void *control_func(void *p)
#endif
{
	while (1)
	{
		char buf[100] = { 0 };
		fgets(buf, sizeof(buf), stdin);
		
		if (strncmp(buf, "exit", 4) == 0)
		{
			exit(0);
		}
	}

#ifndef __UNIX
	return;
#else
	return NULL;
#endif	
}

int main(int argc, char *args[])//TCP代理抓包端程序
{
	if (argc < 3)
	{
		printf("usage:%s IP port\n", args[0]);
		return 0;
	}
	init_socket();
	sock = create_socket(1);//建立TCP socket
	int port = atoi(args[2]);
	if (bind_socket(sock, port) != 0)
	{
		printf("bind fail, %s\n", strerror(errno));
		return 0;
	}
	if (tcp_listen(sock) != 0)
	{
		printf("listen fail, %s\n", strerror(errno));
		return 0;
	}


#ifndef __UNIX
	_beginthread(control_func, 0, NULL);//启动控制线程
#else
	pthread_t thr;
	pthread_create(&thr, NULL, control_func, NULL);//启动控制线程
	pthread_detach(thr);
#endif	

	while (1)
	{
		char IP[20] = { 0 };
		int src_sock = tcp_accept(sock, IP);
		if (src_sock < 0)
		{
			if (errno != 0)
				printf("accept fail, %s\n", strerror(errno));
			break;//accept失败，程序退出
		}

		int dest_sock = create_socket(1);
		if (tcp_connect(dest_sock, args[1], port) != 0)
		{
			printf("connect to %s fail, %s\n", args[1], strerror(errno));
			continue;//连接目标主机失败
		}

		//根据系统时间为每次会话生成一个不重复的随机数
		unsigned int ID1 = (unsigned int)time(NULL);
		unsigned int ID2 = (unsigned int)clock();

		sock_pair *sock1 = (sock_pair *)malloc(sizeof(sock_pair));
		sock_pair *sock2 = (sock_pair *)malloc(sizeof(sock_pair));
		sock1->src_sock = src_sock;
		sock1->dest_sock = dest_sock;
		sprintf(sock1->direct, "%010u%05u-%sto%s", ID1, ID2, IP, args[1]);

		sock2->src_sock = dest_sock;
		sock2->dest_sock = src_sock;
		sprintf(sock2->direct, "%010u%05u-%sto%s", ID1, ID2, args[1], IP);

#ifndef __UNIX
		_beginthread(proxy_func, 0, sock1);//启动c2s线程
		_beginthread(proxy_func, 0, sock2);//启动s2c线程
#else
		pthread_t thr1;
		pthread_create(&thr1, NULL, proxy_func, sock1);//启动c2s线程
		pthread_detach(thr1);

		pthread_t thr2;
		pthread_create(&thr2, NULL, proxy_func, sock2);//启动s2c线程
		pthread_detach(thr2);

#endif	
	}
	close_socket(sock);
	free_socket();
	return 0;
}



