#ifndef STDAFX_H
#define STDAFX_H
#define _CRT_SECURE_NO_WARNINGS

#pragma once

#define WIN32_LEAN_AND_MEAN

#ifdef _WIN32


#include "targetver.h"


#include <windows.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment (lib, "Ws2_32.lib")
#else
#include <cstring>
#include <stdlib.h>
#include <dlfcn.h>
#include <fcntl.h>


#include <unistd.h>
#define _sleep(dildo) usleep(dildo*1000);
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>

void closesocket(int socket) { close(socket); }
#endif // _WIN32

#include <string>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <ctime>
#include <vector>
#include <stdlib.h>

#endif
