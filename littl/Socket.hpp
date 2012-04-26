/*
    Copyright (c) 2009-2011 Xeatheran Minexew

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
#include <littl/StreamBuffer.hpp>

namespace li
{
    enum TcpSocketState
    {
        TcpSocket_idle,
        TcpSocket_listener,
        TcpSocket_server,
        TcpSocket_client
    };

    // Platform-specific opaque handle
    struct TcpSocketImpl;

    class TcpSocket : public IOStream
    {
        protected:
            TcpSocketImpl* socketHandle;

            // general socket properties
            TcpSocketState state;
            bool isBlocking, nagleEnabled;

            // safe receiving related stuff
            bool receiving;
            Array<uint8_t> buffer;
            size_t numReceived;

            // message-related stuff
            bool headerReceived;
            uint32_t messageLength;

            TcpSocket( TcpSocketImpl* handle, TcpSocketState handleState, bool blocking );

            void refresh();
            static bool bootUp();

        private:
            TcpSocket( const TcpSocket& );

        public:
            li_ReferencedClass_override( TcpSocket )

            TcpSocket( bool blocking = false );
            virtual ~TcpSocket();

            /* connection */
            bool listen( unsigned short port, int max = -1 );
            TcpSocket* accept();

            bool connect( const char* addr, unsigned short port, bool async = false );
            void disconnect();

            virtual bool isReadable();
            virtual bool isWritable();
            static const char* getLocalIP();
            const char* getPeerIP();

            //
            void clearInternalBuffer();
            void setBlocking( bool blocking = true );
            void setNagle( bool enabled );

            // InputStream implementation
            virtual size_t rawRead( void* out, size_t length );

            // safe receive
            bool read( void* output, size_t length, long timeout, bool keepBuffered = false );

            // message receive
            //template <typename T> bool receive( Array<T>& output, unsigned* lengthOut = 0, long timeout = 0 );
            //template <typename T> bool receive( StreamBuffer<T>& output, long timeout = 0 );
            bool receive( ArrayIOStream& buffer, Timeout timeout = Timeout( 0 ) );

            // raw write
            virtual size_t rawWrite( const void* in, size_t length );
            virtual size_t write( const void* input, size_t length ) { return rawWrite( input, length ); }

            // message I/O
            bool send( const void* data, unsigned length );

            /*template <typename T> bool send( const Array<T>& buffer, size_t count )
            {
                return send( *buffer, count * sizeof( T ) );
            }*/

            /*template <typename T> bool send( const StreamBuffer<T>& buffer )
            {
                //printf( "[TcpSocket]:W Sending %u-byte message.\n", buffer.getLength() );
                return send( *buffer, buffer.getLength() );
            }*/

            bool send( const ArrayIOStream& buffer ) { return send( buffer.getPtrUnsafe(), ( size_t ) buffer.getSize() ); }

            // MSW: unitialize WinSock2
            static void shutDown();
    };

    /*template <typename T> bool TcpSocket::receive( Array<T>& output, unsigned* lengthOut, long timeout )
    {
        // is it even possible to receive messages on this socket??
        if ( state != TcpSocket_client && state != TcpSocket_server )
            return false;

        //printf( "[TcpSocket]:R Attempting to receive message header...\n" );

        if ( !headerReceived )
        {
            if ( !read( &messageLength, sizeof( messageLength ), timeout ) )
            {
                //printf( "[TcpSocket]:R Header receive failed.\n" );
                return false;
            }
            else
            {
                //printf( "[TcpSocket]:R Received header: %u bytes.\n", messageLength );
                headerReceived = true;
            }
        }

        output.resize( messageLength );
        bool result = read( output.getPtr( 0 ), messageLength, timeout );

        if ( result )
        {
            headerReceived = false;

            if ( lengthOut )
                *lengthOut = messageLength / sizeof( T );

            return true;
        }
        else
            return false;
    }

    template <typename T> bool TcpSocket::receive( StreamBuffer<T>& output, long timeout )
    {
        unsigned receivedLength = 0;

        output.clear();

        if ( receive( ( Array<T>& ) output, &receivedLength, timeout ) )
        {
            output.length = receivedLength;
            return true;
        }
        else
            return false;
    }*/
}
