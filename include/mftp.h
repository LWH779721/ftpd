#ifndef __MFTP_H__
#define __MFTP_H__

#ifdef __Cplusplus
extern .C{
#endif

#define SERVER_PORT       8080

struct mftp_head
{
    unsigned flag:2;            //00:cmd, 01:cmd result, 10:file pack, 11:file raw
    unsigned :30;               //revert
    unsigned len;               //data bytes len, if file total bytes <= 4GB ; if string len of string
};

struct mftp_file_pack
{
    unsigned id;                //pack id
    unsigned len;               //data len of this pack
};

struct udp_pack
{
    unsigned id;          // pack id      
    char rtype;           // result type  0: print ; 1: file
    long total;           // file total bytes
    long send;            // now send bytes
    long len;             // buf len
    char buf[1024];       //     
};

#ifdef __Cplusplus
}
#endif

#endif