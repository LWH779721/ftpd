#include <WINSOCK2.H>
#include <STDIO.H>
#include <iostream>
#include "../include/FtpClient.h"

FtpClient::FtpClient(string &user, string &passwd, string &ip, int port)
{
    this->user = user;
    this->passwd = passwd;
    this->ip = ip;
    this->port = port;
}

FtpClient::~FtpClient()
{
    WSACleanup();
}

int FtpClient::sendCmd(SOCKET fd, const char *cmd, const char *param)
{
    char *p;

    if (cmd)
    {
        if (param)
        {
            sprintf(buf, "%s %s\n\r", cmd, param);
        }
        else
        {
            sprintf(buf, "%s\n\r", cmd);
        }

        send(fd, buf, strlen(buf), 0);
    }

    int ret = recv(fd, buf, sizeof(buf), 0);
    if (ret > 0)
    {
        //std::cout << buf << std::endl;
        buf[ret] = 0;
    }

    p = buf;
    if (!isdigit(p[0])
        || p[3] != ' ')
    {
        p = buf + ret - 3;
        while (*--p != '\r');
        p++;
    }

    std::cout << p << std::endl;
    return atoi(p);
}

bool FtpClient::login()
{
    int ret;

    if (this->logined == true)
    {
        std::cout << "alreay logon!" << std::endl;
        return true;
    }

    WORD sockVersion = MAKEWORD(2, 2);
    WSADATA data;
    if(WSAStartup(sockVersion, &data)!=0)
    {
        return false;
    }

    sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sclient == INVALID_SOCKET)
    {
        std::cout << "invalid socket!" << std::endl;
        return false;
    }

    sockaddr_in serAddr;
    serAddr.sin_family = AF_INET;
    serAddr.sin_port = htons(this->port);
    serAddr.sin_addr.S_un.S_addr = inet_addr(this->ip.c_str());
    if (connect(sclient, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
    {
        closesocket(sclient);
        return false;
    }

    ret = sendCmd(sclient, NULL, NULL);
    if (ret != 220)
    {
        std::cout << "welcome err!" << std::endl;
        closesocket(sclient);
        return false;
    }

    ret = sendCmd(sclient, "USER", this->user.c_str());
    if (ret != 331)
    {
        std::cout << "user err!" << std::endl;
        closesocket(sclient);
        return false;
    }

    ret = sendCmd(sclient, "PASS", this->passwd.c_str());
    if (ret != 230)
    {
        std::cout << "pass err!" << std::endl;
        closesocket(sclient);
        return false;
    }

    this->logined = true;
    return true;
}

bool FtpClient::cd(string &path)
{
    int ret;
    ret = sendCmd(sclient, "CWD", path.c_str());
    if (ret == 550)
    {
        std::cout << "failed when cd" << std::endl;
        return false;
    }

    return true;
}

bool FtpClient::upload(string &dst, string &src)
{
    char *buf_ptr;
    int port_num;

    if (sendCmd(sclient, "PASV", NULL) != 227)
    {
        return false;
	}

	buf_ptr = strrchr(buf, ')');
	if (buf_ptr) *buf_ptr = '\0';

	buf_ptr = strrchr(buf, ',');
	*buf_ptr = '\0';
	port_num = atoi(buf_ptr + 1);

	buf_ptr = strrchr(buf, ',');
	*buf_ptr = '\0';
	port_num += atoi(buf_ptr + 1) * 256;

	std::cout << port_num << std::endl;

	int datafd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (datafd == INVALID_SOCKET)
    {
        std::cout << "invalid socket!" << std::endl;
        return false;
    }

    sockaddr_in serAddr;
    serAddr.sin_family = AF_INET;
    serAddr.sin_port = htons(port_num);
    serAddr.sin_addr.S_un.S_addr = inet_addr(this->ip.c_str());
    if (connect(datafd, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
    {
        closesocket(datafd);
        return false;
    }

    string path = dst.substr(0, dst.find_last_of("/"));
    std::cout << dst << " " << path << std::endl;
    //mkdir(path);
    if (cd(path) == false)
    {
        mkdir(path);
        cd(path);
    }

STOR_FLAG:
    string fname = dst.substr(dst.find_last_of("/") + 1);
    int ret = sendCmd(sclient, "STOR", fname.c_str());
	switch (ret) {
	case 125:
	case 150:
		break;
	default:
	    string path = dst.substr(0, dst.find_last_of("/"));
	    std::cout << dst << " " << path << std::endl;
	    mkdir(path);
	    cd(path);
	    goto STOR_FLAG;
	}

    FILE *fp = fopen(src.c_str(), "rb");
    if (fp == NULL)
    {
        std::cout << "err when file open" << std::endl;
    }

    while ((ret = fread(buf, 1, sizeof(buf), fp)) >0)
    {
        send(datafd, buf, ret, 0);
    }

    closesocket(datafd);
    fclose(fp);

    if (sendCmd(sclient, NULL, NULL) != 226) {
	}

	bye();
}

bool FtpClient::upload(string &dst, char *fbuf, int size)
{
    char *buf_ptr;
    int port_num;

    if (sendCmd(sclient, "PASV", NULL) != 227)
    {
        return false;
	}

	buf_ptr = strrchr(buf, ')');
	if (buf_ptr) *buf_ptr = '\0';

	buf_ptr = strrchr(buf, ',');
	*buf_ptr = '\0';
	port_num = atoi(buf_ptr + 1);

	buf_ptr = strrchr(buf, ',');
	*buf_ptr = '\0';
	port_num += atoi(buf_ptr + 1) * 256;

	std::cout << port_num << std::endl;

	int datafd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (datafd == INVALID_SOCKET)
    {
        std::cout << "invalid socket!" << std::endl;
        return false;
    }

    sockaddr_in serAddr;
    serAddr.sin_family = AF_INET;
    serAddr.sin_port = htons(port_num);
    serAddr.sin_addr.S_un.S_addr = inet_addr(this->ip.c_str());
    if (connect(datafd, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
    {
        closesocket(datafd);
        return false;
    }

    int ret = sendCmd(sclient, "STOR", dst.c_str());
	switch (ret) {
	case 125:
	case 150:
		break;
	default:
	    break;
	}

    int sended = 0;
    while (sended < size)
    {
        ret = send(datafd, fbuf + sended, size - sended, 0);
        if (ret > 0)
        {
            sended += ret;
        }
    }

    closesocket(datafd);

    if (sendCmd(sclient, NULL, NULL) != 226) {
	}

	bye();
}

bool FtpClient::bye()
{
    int ret;
    ret = sendCmd(sclient, "QUIT", NULL);
    if (ret != 221)
    {
        std::cout << "failed when quit" << std::endl;
        return false;
    }

    return true;
}

bool FtpClient::mkdir(string path)
{
    int ret;
    ret = sendCmd(sclient, "MKD", path.c_str());
    if (ret != 221)
    {
        std::cout << "failed when quit" << std::endl;
        return false;
    }

    return true;
}
