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

#include "Common.hpp"

namespace li
{
#ifdef __li_MSW
    bool wsaStarted = false;
    WSADATA wsaData;
#endif

    const char* getLastSocketErrorDesc()
    {
#ifdef __li_MSW
        static char errorBuffer[4096];
        
        DWORD count = FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(), 0, errorBuffer, sizeof( errorBuffer ) - 1, NULL );
        errorBuffer[count] = 0;
        
        // Drop any trailing newlines
        while ( count > 0 && ( errorBuffer[count - 1] == '\r' || errorBuffer[count - 1] == '\n' ) )
            errorBuffer[--count] = 0;
        
        return errorBuffer;
#else
        return strerror( errno );
#endif
    }
    
    addrinfo* resolveAddr( const char* hostname, int port, addrinfo* hints, addrinfo*& results, int family, int socktype, int protocol )
    {
        addrinfo *current;
        char portString[16];

        // This has been always obnoxious
        if ( port >= 0 )
            snprintf( portString, sizeof( portString ), "%d", ( int ) port );

        // Try to resolve the address
        if ( getaddrinfo( hostname, ( port >= 0 ) ? portString : nullptr, hints, &results ) != 0 )
            return false;

        // Browse the results and try to find a suitable entry
        for ( current = results; current != nullptr; current = current->ai_next )
            if ( current->ai_family == family && current->ai_socktype == socktype && current->ai_protocol == protocol )
                break;

        return current;
    }

    bool socketStartup()
    {
#ifdef __li_MSW
        if ( wsaStarted )
            return true;

        wsaStarted = ( WSAStartup( MAKEWORD( 2, 2 ), &wsaData ) == 0 );
        return wsaStarted;
#else
        return true;
#endif
    }
}
