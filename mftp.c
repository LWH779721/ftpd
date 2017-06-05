#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "mftp.h"

extern int errno;

long start_up(int *srv)
{
	struct sockaddr_in server;
	socklen_t len;
	int ret;

	if (0 > (*srv = socket(AF_INET, SOCK_DGRAM, 0)))
	{
		print_log("create socker err");
		return -1;
	}
    
    int optVal = 1;
    ret = setsockopt( *srv, SOL_SOCKET, SO_REUSEADDR, (char*)&optVal, sizeof(optVal) );
    
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;//300*1000;

    //接受时限
    setsockopt(*srv, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout,sizeof(timeout));
    
    bzero(&server,sizeof server);
	server.sin_family = AF_INET;
	server.sin_port = htons(SERVER_PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);
	len = sizeof(server);

	if (-1 == bind(*srv,(struct sockaddr *)&server,len))
	{
		printf("bind err\n");
		goto fail_label;
	}
 	
	ret = 0;
    return ret;
fail_label:
	close(*srv);
	ret = -1;
	return ret;	
}

long do_cmd(char *buf, struct sockaddr *client, int srv)
{   
    FILE *fp;
    char cmd[20],file[20];
    int len;
    
    sscanf(buf,"%s %s",cmd,file);
    if (strstr(cmd, "ls"))
    {
        fp = popen("ls -l", "r");
        if (NULL == fp)
        {
            printf("errrr");
            return -1;
        }
    }
    else if (strstr(cmd, "get"))
    {
        if (NULL == (fp = fopen(file, "rb")))
        {
            printf("errrr");
            return -1;
        }
        
        memcpy(buf, "file", strlen("file"));
        buf[strlen("file")] = '\0';
        sendto(srv, buf, strlen(buf), 0, client, sizeof *client);
    }

    while ((len = fread(buf, sizeof(char), 1024, fp)) > 0) {
        sendto(srv, buf, len, 0, client, sizeof *client);
    }
    
    memcpy(buf, "end", strlen("end"));
    buf[strlen("end")] = '\0';
    sendto(srv, buf, strlen(buf), 0, client, sizeof *client);
    fclose(fp);
    //pclose(fp);
    return 0;
}

void usage()
{
    printf("usage ./mftp [-type] [-option]                                            \n");
    printf("        -s                  : start server                          \n");
    printf("        -s -p [port]        : start server use port                 \n");
    printf("        -c -s [ip] [port]   : start client connect server ip port   \n");
    printf("        -d                  : discover server                       \n");
    printf("        -h|-help            : print this usage                      \n");
    printf("--------------------------------------------------------------------\n");
    printf("client:                                                             \n");
    printf("        pull [remotefile] [localfile]                               \n");
    printf("        push [localfile]  [remotefile]                              \n");
}

int main(int argc,char ** args)
{
    int srv;
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);
	char buf[1024];
	long rlen;
    
    if (argc == 1
        || memcmp(args[1], "-h", 2) == 0
        || memcmp(args[1], "-help", 5) == 0)
    {
        usage();
        return 0;
    }
        
	if (start_up(&srv))
	{
		printf("start up server err\n");
		return -1;
	}
	
	while (1)
	{
	    bzero(&client_addr,sizeof client_addr);
        if ((rlen = recvfrom (srv, buf, sizeof(buf), 0, (struct sockaddr *)&client_addr, &len)) > 0)
        {
            buf[rlen] = '\0';
            printf("from %s:%s",inet_ntoa(client_addr.sin_addr),buf);
            do_cmd(buf, (struct sockaddr *)&client_addr, srv);
        }
	}
	
fail_label:
	close(srv);
	return 0;
}
