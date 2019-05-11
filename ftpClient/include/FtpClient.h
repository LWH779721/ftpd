#ifndef __FTPCLIENT_H__
#define __FTPCLIENT_H__

#include <string>

using namespace std;
class FtpClient
{
    private:
       string user;
       string passwd;
       string ip;
       int port;
       bool logined = false;
       SOCKET sclient;
       char buf[255] = {0};

       int sendCmd(SOCKET fd, const char *cmd, const char *param);
    public:
       FtpClient(string &user, string &passwd, string &ip, int port);
       virtual ~FtpClient();
       bool login();
       bool upload(string &dst, string &src);
       bool upload(string &dst, char *buf, int size);
       bool cd(string &path);
       bool mkdir(string path);
       bool bye();
};

#endif
