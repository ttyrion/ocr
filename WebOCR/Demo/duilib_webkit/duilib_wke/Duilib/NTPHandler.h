#pragma once
#include <stdint.h>
#include <winsock2.h>

#define NTP_SVR "us.pool.ntp.org"
#define NTP_PORT 123

class NTPHandler
{
public:
    NTPHandler();
    ~NTPHandler();

public: 
    static UINT64 GetNTPSvrTime();
};

