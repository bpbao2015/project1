#include <stdio.h>
#include <stdlib.h>
#include "mysock.h"
#include <string.h>
#include <time.h>
#include <sys/time.h>


#define BUFSIZE 8192 //8k

struct Base64Date6
{
        unsigned int d4:6;
        unsigned int d3:6;
        unsigned int d2:6;
        unsigned int d1:6;
};
// 协议中加密部分使用的是base64方法
char ConvertToBase64  (char   c6);
void EncodeBase64     (char   *dbuf, char *buf128, int len);


char ConvertToBase64(char uc)
{
	if(uc < 26)
	{
		return 'A' + uc;
	}
	if(uc < 52)
	{
		return 'a' + (uc - 26);
	}
	if(uc < 62)
	{
		return '0' + (uc - 52);
	}
	if(uc == 62)
	{
		return '+';
	}
	return '/';
}

// base64的实现
void EncodeBase64(char *dbuf, char *buf128, int len)
{
	struct Base64Date6 *ddd      = NULL;
	int           i        = 0;
	char          buf[256] = {0};
	char          *tmp     = NULL;
	char          cc       = '\0';

	memset(buf, 0, 256);
	strncpy(buf, buf128, 256);
	for(i = 1; i <= len/3; i++)
	{
		tmp             = buf + (i-1)*3;
		cc              = tmp[2];
		tmp[2]          = tmp[0];
		tmp[0]          = cc;
		ddd             = (struct Base64Date6 *)tmp;
		dbuf[(i-1)*4+0] = ConvertToBase64((unsigned int)ddd->d1);
		dbuf[(i-1)*4+1] = ConvertToBase64((unsigned int)ddd->d2);
		dbuf[(i-1)*4+2] = ConvertToBase64((unsigned int)ddd->d3);
		dbuf[(i-1)*4+3] = ConvertToBase64((unsigned int)ddd->d4);
	}
	if(len % 3 == 1)
	{
		tmp             = buf + (i-1)*3;
		cc              = tmp[2];
		tmp[2]          = tmp[0];
		tmp[0]          = cc;
		ddd             = (struct Base64Date6 *)tmp;
		dbuf[(i-1)*4+0] = ConvertToBase64((unsigned int)ddd->d1);
		dbuf[(i-1)*4+1] = ConvertToBase64((unsigned int)ddd->d2);
		dbuf[(i-1)*4+2] = '=';
		dbuf[(i-1)*4+3] = '=';
	}
	if(len%3 == 2)
	{
		tmp             = buf+(i-1)*3;
		cc              = tmp[2];
		tmp[2]          = tmp[0];
		tmp[0]          = cc;
		ddd             = (struct Base64Date6 *)tmp;
		dbuf[(i-1)*4+0] = ConvertToBase64((unsigned int)ddd->d1);
		dbuf[(i-1)*4+1] = ConvertToBase64((unsigned int)ddd->d2);
		dbuf[(i-1)*4+2] = ConvertToBase64((unsigned int)ddd->d3);
		dbuf[(i-1)*4+3] = '=';
	}
	return;
}

//第一个参数代表邮件服务器smtp server的IP或者域名
//第二个参数代表登陆邮件服务器的用户名
//第三个参数代表密码
//返回值0代表成功，1代表失败
int SendMail(const char *IP, const char *username, const char *passwd, const char *touser, const char *subject, const char *message)
{
	//在堆中申请一个buffer，来存放与服务器交互时的消息内容
	char *buf = (char *)malloc(BUFSIZE);
	char base[256];//代表base64编码格式的字符串
	char asc[256];//代表ascii编码格式的字符串
	int rc = 0;//这个代表函数的返回值
	int num = 0;

	time_t tTimeVal;
	struct tm *ptTimeStruct;
	char body[BUFSIZE] = {0};
	tTimeVal = time(NULL);
//	printf("%s\n", (char *)tTimeVal);
	ptTimeStruct = localtime(&tTimeVal);
//	printf("%s\n", (char *)ptTimeStruct);
	sprintf(body, "From: \"%s\" <%s>\r\n", username, username);
	sprintf(body, "%sTo: <%s>\r\n", body, touser);
	sprintf(body, "%sReferences:\r\nIn-Reply-To:\r\nMIME-Version: 1.0\r\nContent-Type: multipart/alternative;\r\nX-Mailer: Microsoft Outlook 15.0\r\nContent-Language: zh-cn\r\nContent-Type: text/plain;\r\n", body);
	sprintf(body, "%sDate: %02d, %02d %02d %04d %02d:%02d:%02d %d\r\n", body, ptTimeStruct->tm_wday, ptTimeStruct->tm_mday, ptTimeStruct->tm_mon + 1, ptTimeStruct->tm_year + 1900, ptTimeStruct->tm_hour, ptTimeStruct->tm_min, ptTimeStruct->tm_sec, ptTimeStruct->tm_isdst);
	sprintf(body, "%sSubject: %s%02d:%02d\r\n\r\n", body, subject, ptTimeStruct->tm_min, ptTimeStruct->tm_sec);
	sprintf(body, "%s%s\r\n", body, message);
	printf("\n%s\n", body);


	//连接到参数IP指定的smtp服务器
	
	//使用TCP协议客户端连接到指定的服务端
	//destIP为来自服务端的IP地址，port为服务端的端口号。返回值=0成功,-1失败
	rc =  app_client_connect(IP, 25);//25就是smtp协议的标准端口
	if (rc == -1)
		return -1;
	
	//清空buf
	memset(buf, 0, BUFSIZE);
	

	//使用TCP协议客户端接收数据
	//buf为接收数据缓冲区，bufsize为缓冲区大小（单位：字节）。返回值>0成功接收数据字节数，=0连接正常关闭，-1失败
	rc = app_client_recv(buf, BUFSIZE);
	//printf("buf = %s\n", buf);	

	//使用TCP协议客户端发送数据
	//buf为发送数据缓冲区， bufsize为发送数据大小哦（单位：字节）。返回值>0成功发送数据字节数，=0连接正常关闭，-1失败
	
	memset(buf, 0, BUFSIZE);
	strcpy(buf, "EHLO boss\r\n");//smtp协议的每个字符串后面都是\r\n结尾的
	rc =  app_client_send(buf, strlen(buf));//给服务器发送EHLO消息

	memset(buf, 0, BUFSIZE);
	rc = app_client_recv(buf, BUFSIZE);//从服务端接收消息
	//printf("buf = %s\n", buf); 

	memset(buf, 0, BUFSIZE);
	strcpy(buf, "AUTH LOGIN\r\n");
	rc = app_client_send(buf, strlen(buf));//给服务器发送请求登陆消息


        memset(buf, 0, BUFSIZE);
        rc = app_client_recv(buf, BUFSIZE);//从服务端接收消息
	//memset(asc, 0, sizeof(asc));
	//b.Decode(&buf[4], asc);
        //printf("buf = %s\n", asc); 

	memset(buf, 0, BUFSIZE);
	memset(base, 0, sizeof(base));
	memset(asc, 0, sizeof(asc));
	strcpy(asc, username);
	EncodeBase64(base, asc, strlen(asc));
//	b.Encode(asc, strlen(asc), base);//将用户名转化为base64编码
	sprintf(buf, "%s\r\n", base);//将转化后后的结果放入buf
	rc = app_client_send(buf, strlen(buf));

	memset(buf, 0, BUFSIZE);
	rc = app_client_recv(buf, BUFSIZE);//从服务端接收消息
	printf("buf = %s\n", buf);
        //memset(asc, 0, sizeof(asc));
        //b.Decode(&buf[4], asc);
        //printf("buf = %s\n", asc); 


        memset(buf, 0, BUFSIZE);
        memset(base, 0, sizeof(base));
        memset(asc, 0, sizeof(asc));
        strcpy(asc, passwd);
	EncodeBase64(base, asc, strlen(asc));
      //  b.Encode(asc, strlen(asc), base);//将密码转化为base64编码
        sprintf(buf, "%s\r\n", base);//将转化后后的结果放入buf
        rc = app_client_send(buf, strlen(buf));
	printf("buf = %s\n", buf);
	memset(buf, 0, BUFSIZE);
        rc = app_client_recv(buf, BUFSIZE);//从服务端接收消息

	//printf("buf = %s\n", buf);

	//解析来自服务端的消息，判断登陆是否成功
	sscanf(buf, "%d %s\r\n", &num, asc);//可以把从服务端收到的消息分为两部分
	//第一部分转化为int，放入变量num中，第二部分转为字符串，放入asc中
	//free(buf);//记得将堆内存释放
	printf("num = %d, %s\n", num, asc);
	if (num == 235)
	{
		// MAIL FROM
		char mailFrom[100]="MAIL FROM: <";
		strcat(mailFrom,username);
		strcat(mailFrom,">\r\n");
		memset(buf, 0, 1500);
		strcpy(buf, mailFrom);
		rc = app_client_send(buf, strlen(buf));
		memset(buf, 0, BUFSIZE);
                rc = app_client_recv(buf, BUFSIZE);//从服务端接收消息
		printf("%s\n", buf);
		// RCPT TO 第一个收件人
		sprintf(buf, "RCPT TO:<%s>\r\n", touser);
		rc = app_client_send(buf, strlen(buf));
		memset(buf, 0, BUFSIZE);
                rc = app_client_recv(buf, BUFSIZE);//从服务端接收消息
		printf("%s\n", buf);

		// DATA 准备开始发送邮件内容
		strcpy(buf, "DATA\r\n");
		rc = app_client_send(buf, strlen(buf));
		memset(buf, 0, BUFSIZE);
                rc = app_client_recv(buf, BUFSIZE);//从服务端接收消息
		printf("%s\n", buf);

		// 发送邮件内容，\r\n.\r\n内容结束标记
		memset(buf, 0, BUFSIZE);
		sprintf(buf, "%s\r\n.\r\n", body);
		rc = app_client_send(buf, strlen(buf));
		memset(buf, 0, BUFSIZE);
                rc = app_client_recv(buf, BUFSIZE);//从服务端接收消息
		printf("%s\n", buf);

		// QUIT
                strcpy(buf, "QUIT\r\n");
		rc = app_client_send(buf, strlen(buf));
		memset(buf, 0, BUFSIZE);
                rc = app_client_recv(buf, BUFSIZE);//从服务端接收消息
		printf("%s\n", buf);
	        free(buf);//记得将堆内存释放
		return 0;//成功登陆邮箱
	}else
	{
		free(buf);//记得将堆内存释放
		return -1;// 登陆失败
	}

}

int main(int argc, char *argv[])
{
	int i;
	//for(i = 0; i < 100000; i++)//穷举一般需要消耗很多时间，为了缩短时间，假设密码是a开头的6位数字
//	{
//		sprintf(pass, "a%05d", i);
		if (SendMail("smtp.163.com", argv[1], argv[2], argv[3], "会计分录", "This is a test mail") == 0)
		{
//			printf("%s\n", pass);
			return 0;
		}
//	}//循环完成auth_smtp没有一次成功，那么就打印失败
//	printf("fail\n");	
	return 0;
}

