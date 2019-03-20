#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <ctype.h>
#include <errno.h>
#include <sys/sendfile.h>

#include "logger/logger.h"
#include "mftp.h"

extern int errno;

long start_up(int *srv)
{
	struct sockaddr_in server;
	socklen_t len;
	int ret;

	if (0 > (*srv = socket(AF_INET, SOCK_DGRAM, 0)))
	{
		logger(err, "create socker err");
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

long send_file(int srv, struct sockaddr *client, char *file, long start)
{
    FILE *fp;
    int len;
    struct udp_pack pack = {
        .id = 0,
        .rtype = 1,
        .send = 0
    };
    
    if (NULL == (fp = fopen(file, "rb")))
    {
        logger(err, "failed when open file");
        return -1;//send_err(srv, client);        
    }
    
    fseek(fp, 0L, SEEK_END);
    pack.total = ftell(fp);
    rewind(fp);
    
    if (pack.total < 1024*1024)
    {
        sendfile( srv, fileno(fp), NULL, pack.total);    
    }
    else
    {
        if (start)
        {
            pack.send = start;
            fseek(fp, start, SEEK_SET);
        }
    
        while ((pack.len = fread(pack.buf, sizeof(char), 1024, fp)) > 0) {
            pack.send += pack.len;
            sendto(srv, &pack, sizeof pack, 0, client, sizeof *client);
            pack.id++;
            logger(info, "pack id : %d",pack.id);
            usleep(20*1000);
        }
    }
    
    fclose(fp);
    return 0;
}

long do_cmd(char *buf, struct sockaddr *client, int srv)
{   
    char cmd[20] = {0},file[20];
    int len,start = 0;
    char *p = buf;
    
    sscanf(p,"%s", cmd);
    if (strcmp(cmd, "ls") == 0)
    {
        logger(info, "ls");
        //sscanf(buf,"%s %s %d", cmd, file, &start);
        FILE *fp;
        struct mftp_head head;
        char result[1024];
         
        fp = popen("ls", "r");
        if (NULL == fp)
        {
            logger(err, "err when popen");
            return -1;
        }
        
        head.flag = 1;
        if ((len = fread(result, sizeof(result), 1024, fp)) > 0) {
            head.len = len;
            sendto(srv, (char *)&head, sizeof head, 0, client, sizeof *client);
            
            sendto(srv, result, len, 0, client, sizeof *client);
        }
   
        pclose(fp);
        return 0;
    }
    else if (strstr(cmd, "get"))
    {
        /*if (NULL == (fp = fopen(file, "rb")))
        {
            printf("errrr");
            return -1;
        }
        
        memcpy(buf, "file", strlen("file"));
        buf[strlen("file")] = '\0';
        sendto(srv, buf, strlen(buf), 0, client, sizeof *client);*/
        return send_file(srv, client, file, start);
    }

    return 0;
}

void usage()
{
    printf("usage ./mftp [-type] [-option]                                      \n");
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
    struct mftp_head head;
    
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
        if ((rlen = recvfrom (srv, (char *)&head, sizeof(head), 0, (struct sockaddr *)&client_addr, &len)) > 0)
        {
            if (head.flag == 0)
            {   
                if (head.len > 1024 
                    || (rlen = recvfrom (srv, buf, head.len, 0, (struct sockaddr *)&client_addr, &len)) <= 0)
                {
                    logger(info, "recv cmd err");
                    continue;
                }
                
                buf[rlen] = '\0';
                printf("from %s:%s",inet_ntoa(client_addr.sin_addr),buf);
                do_cmd(buf, (struct sockaddr *)&client_addr, srv);    
            }
        }
	}
	
fail_label:
	close(srv);
	return 0;
}
