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
    struct mftp_head head;
    
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
	    
	    head.flag = 0;
	    head.len = i;
	    sendto(srv, (char *)&head, sizeof head, 0, (struct sockaddr *)&server, len);
        sendto(srv, buf, i, 0, (struct sockaddr *)&server, len);
        
        if((i = recvfrom (srv, (char *)&head, sizeof head, 0, (struct sockaddr *)&server, &len)) > 0)
        {
            logger(info, "head.flag :%s", head.flag);
            if (head.flag == 1)
            {
                if (head.len > 1024 
                    || (i = recvfrom (srv, buf, head.len, 0, (struct sockaddr *)&server, &len)) <= 0)
                {
                    logger(err, "recv cmd err");
                    continue;
                }
            
                logger(info, "result :%s", buf);
            }
        }
        /*while((i = recvfrom (srv, buf, sizeof(buf), 0, (struct sockaddr *)&server, &len)) > 0)
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
        }*/
    #if 0    
        struct udp_pack pack;
        //static int id = -1;
        int total = 431104;
        while((i = recvfrom (srv, &pack, sizeof pack, 0, (struct sockaddr *)&server, &len)) > 0)
        {
            if (pack.rtype == 0)
            {
                logger(debug, "%s",pack.buf);
                continue;    
            }
            
            if (pack.rtype == 1
                && NULL == fp)
            {
                if (NULL == (fp = fopen("tmp.data","ab+")))
                {
                    logger(err, "failed when open file");
                    continue;
                }
            }

            logger(debug, "pack.id : %d", pack.id);
            /*if ((pack.id - id) != 1)
            {
                logger(err, "failed when recv data, cls file");
                system("rm tmp.data");
                break;    
            }
            
            id++;*/
            //logger(debug, "%d : %d\n", pack.total, pack.send);
            //logger(debug, "%d : %d\n", pack.id, id);
            fwrite(pack.buf, 1, pack.len, fp);
            
            /*if (pack.send == pack.total
                || pack.len == 0)
            {
                fclose(fp); 
                fp = NULL;
                break;
            }*/
            total += pack.len;
            if (total == pack.total)
            {
                fclose(fp); 
                fp = NULL;
                break;
            }
        }
    #endif    
	}
	
fail_label:
	close(srv);
	return 0;
}
