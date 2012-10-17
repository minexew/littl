/*
    Copyright (c) 2012 Xeatheran Minexew

    This software is provided 'as-is', without any express or implied
    warranty. In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution.
*/

#if ( defined( __WINDOWS__ ) || defined( _WIN32 ) || defined( _WIN64 ) )
// This needs to be done before including any other littl headers, pulling in Windows.h
#define _WIN32_WINNT 0x501
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>

typedef int SOCKET;
static const SOCKET INVALID_SOCKET = -1;
static const int SOCKET_ERROR = -1;
#endif

#include <littl/Base.hpp>

namespace li
{
#ifdef __li_MSW
    extern bool wsaStarted;
    extern WSADATA wsaData;
#endif

    addrinfo* resolveAddr( const char* hostname, int port, addrinfo* hints, addrinfo*& results,
            int family, int socktype, int protocol );
    bool socketStartup();
    
    const char* getLastSocketErrorDesc();
}
