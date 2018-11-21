#include "NTPHandler.h"
#include <iostream>


NTPHandler::NTPHandler()
{
}


NTPHandler::~NTPHandler()
{
}

#define CLEAN_AND_RETURN(a) \
closesocket(s); \
WSACleanup(); \
return a;


UINT64 NTPHandler::GetNTPSvrTime() {
    WSADATA wsaData;
    int failed = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (failed) {
        return 0;
    }


    // the packet we send
    unsigned char msg[48] = { 010,0,0,0,0,0,0,0,0 };
    // the buffer we get back
    unsigned long  buf[1024] = { 0 };

    struct protoent *proto;
    struct sockaddr_in server_addr;
    SOCKET s;
    proto = getprotobyname("udp");
    s = socket(PF_INET, SOCK_DGRAM, proto->p_proto);
    if (s == INVALID_SOCKET) {
        return 0;
    }

    timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    failed = setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(timeval));
    if (failed) {
        CLEAN_AND_RETURN(0);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    struct hostent *phost = gethostbyname(NTP_SVR);
    server_addr.sin_addr = *((struct in_addr*)phost->h_addr);
    char* pt = inet_ntoa(*((struct in_addr*)phost->h_addr));
    server_addr.sin_port = htons(NTP_PORT);
    int bytes = sendto(s, (const char*)msg, sizeof(msg), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (bytes == SOCKET_ERROR) {
        CLEAN_AND_RETURN(0);
    }

    tv.tv_sec = 5;
    tv.tv_usec = 0;
    failed = setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(timeval));
    if (failed) {
        CLEAN_AND_RETURN(0);
    }
    bytes = recv(s, (char*)buf, sizeof(buf), 0);
    if (bytes == 0 || bytes == SOCKET_ERROR) {
        CLEAN_AND_RETURN(0);
    }
    unsigned long time = ntohl((time_t)buf[10]);
    time -= 2208988800U;
    closesocket(s);
    WSACleanup();

    return time;
}