/*
    Copyright (c) 2009-2012 Xeatheran Minexew

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

#include <littl/Stream.hpp>

namespace li
{
    class TcpSocket : public IOStream
    {
        public:
            enum State
            {
                idle, listening, connecting, host, connected
            };

            using InputStream::read;
            using OutputStream::write;

            static std::unique_ptr<TcpSocket> create( bool blocking = false );
            virtual ~TcpSocket() {}

            virtual const char* getPeerIP() = 0;
            virtual State getState() = 0;

            virtual void setBlocking( bool blocking ) = 0;
            virtual void setDelayedSending( bool enabled ) = 0;

            // Connections
            virtual bool listen( uint16_t port ) = 0;
            virtual std::unique_ptr<TcpSocket> accept( bool block ) = 0;

            virtual bool connect( const char* host, uint16_t port, bool block ) = 0;
            virtual bool connectFinished( bool &success, int* errno_out ) = 0;
            virtual void disconnect() = 0;

            // Safe receive
            virtual bool read( void* output, size_t length, Timeout timeout, bool peek = false ) = 0;

            // Message-based communication
            virtual bool receive( ArrayIOStream& buffer, Timeout timeout = Timeout(0) ) = 0;

            virtual bool send( const void* data, size_t length ) = 0;
            bool send( const ArrayIOStream& buffer ) { return send( buffer.c_array(), ( size_t ) buffer.getSize() ); }

            // Direct access
            virtual size_t readUnbuffered( void* buffer, size_t maxlen ) = 0;
    };
}