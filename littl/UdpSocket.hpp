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

#pragma once

#include <littl/BaseIO.hpp>

namespace li
{
    struct SockAddress;

    class UdpSocket: public ReferencedClass
    {
        public:
            static UdpSocket* create();
            virtual ~UdpSocket() {}

            virtual const char* getErrorDesc() = 0;
            virtual const char* getPeerIP() = 0;

            virtual SockAddress* getBroadcastAddress( int port ) = 0;
            virtual void releaseAddress( SockAddress* addr ) = 0;

            virtual bool bind( uint16_t port ) = 0;
            virtual void disconnect() = 0;

            virtual bool receive( ArrayIOStream& buffer, Timeout timeout = Timeout(0) ) = 0;
            virtual bool send( SockAddress* to, const void* data, size_t length ) = 0;
            bool send( SockAddress* to, const ArrayIOStream& buffer ) { return send( to, buffer.c_array(), ( size_t ) buffer.getSize() ); }

            li_ReferencedClass_override( UdpSocket )
    };
}