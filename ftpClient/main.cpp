#include<WINSOCK2.H>
#include<STDIO.H>
#include<iostream>
#include<cstring>
#include <string>
#include "include/FtpClient.h"

using namespace std;

#pragma comment(lib, "ws2_32.lib")

int main()
{
    string user("test");
    string passwd("123456");
    string ip("127.0.0.1");
    FtpClient a(user, passwd, ip, 21);
    bool ok = a.login();
    if (ok)
    {
        std::cout<< "login ok" << std::endl;
    }

    string path = "hello";
    a.cd(path);
    string dst = "/b/a.go";
    string src = "C:\\Users\\lwh\\a.go";
    a.upload(dst, src);

    return 0;
}
