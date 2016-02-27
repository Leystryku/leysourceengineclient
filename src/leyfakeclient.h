#ifndef LEYFAKESERV_H
#define LEYFAKESERV_H

#ifdef _WIN32
#pragma once
#define lastneterror() WSAGetLastError()
#else
#define lastneterror() errno
#endif

#include "stdafx.h"





#endif
