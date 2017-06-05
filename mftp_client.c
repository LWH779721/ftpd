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

	ret = 0;
    return ret;
fail_label:
	close(*srv);
	ret = -1;
	return ret;	
}

int main(int argc,char ** args)
{
    int srv,i;
	struct sockaddr_in server;
	socklen_t len = sizeof(server);
	char buf[1024];
    FILE *fp = NULL;

	if (start_up(&srv))
	{
		printf("start up server err\n");
		return -1;
	}
	
    bzero(&server,sizeof server);
	server.sin_family = AF_INET;
	server.sin_port = htons(SERVER_PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
	
	while (1)
	{
	    printf("mftp>");
	    fgets(buf, sizeof buf - 1, stdin);
	    i = strlen(buf);
        sendto(srv, buf, i, 0, (struct sockaddr *)&server, len);
        while((i = recvfrom (srv, buf, sizeof(buf), 0, (struct sockaddr *)&server, &len)) > 0)
        {
            buf[i] = '\0';
            printf("from %s:%s",inet_ntoa(server.sin_addr),buf);
            if (memcmp(buf, "file", 4) == 0)
            {
                fp = fopen("tmp.data","wb");
            }
            else if (memcmp(buf, "end", 3) == 0)
            {
                if (fp)
                { 
                    //fflush(fp);
                    fclose(fp);
                    fp = NULL;
                }
                break;
            }
            else
            {
                if (fp) fwrite(buf, 1, i, fp);    
            }
        }
	}
	
fail_label:
	close(srv);
	return 0;
}
