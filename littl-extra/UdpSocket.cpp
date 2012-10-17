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

#include <littl/UdpSocket.hpp>

namespace li
{
    class UdpSocketImpl : public UdpSocket
    {
        private:
            SOCKET sock;
            sockaddr peer;

            // general socket properties
            bool isActuallyBlocking;

            bool createSocket();
            void setActuallyBlocking( bool blocking );

        private:
            UdpSocketImpl(const UdpSocketImpl&);

        public:
            UdpSocketImpl();
            virtual ~UdpSocketImpl();

            virtual const char* getErrorDesc() override;
            virtual const char* getPeerIP() override;

            virtual SockAddress* getBroadcastAddress( int port ) override;
            virtual void releaseAddress( SockAddress* addr ) override { free( addr ); }

            virtual bool bind( uint16_t port ) override;
            virtual void disconnect() override;

            virtual bool receive( ArrayIOStream& buffer, Timeout timeout = Timeout(0) ) override;
            virtual bool send( SockAddress* to, const void* data, size_t length ) override;
    };

    UdpSocketImpl::UdpSocketImpl()
    {
        sock = INVALID_SOCKET;
        isActuallyBlocking = false;
    }

    UdpSocketImpl::~UdpSocketImpl()
    {
        disconnect();
    }

    bool UdpSocketImpl::createSocket()
    {
        sock = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );

        if ( sock == INVALID_SOCKET )
            return false;

        int yes = 1;

        if ( setsockopt( sock, SOL_SOCKET, SO_BROADCAST, ( const char* ) &yes, sizeof( yes ) ) < 0
                || setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &yes, sizeof( yes ) ) < 0
                )
        {
            disconnect();
            return false;
        }

        return true;
    }

    void UdpSocketImpl::disconnect()
    {
        if ( sock != INVALID_SOCKET )
        {
#ifdef __li_MSW
            closesocket( sock );
#else
            close( sock );
#endif
            sock = INVALID_SOCKET;
        }
    }

    SockAddress* UdpSocketImpl::getBroadcastAddress( int port )
    {
        /*char hostname[256];

        if ( gethostname( hostname, sizeof( hostname ) - 1 ) == SOCKET_ERROR )
            return nullptr;

        addrinfo hints, *addr, *results = nullptr;

        memset( &hints, 0, sizeof( hints ) );
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_protocol = IPPROTO_UDP;

        addr = resolveAddr( hostname, -1, &hints, results, AF_INET, SOCK_DGRAM, IPPROTO_UDP );

        if ( addr == nullptr )
        {
            freeaddrinfo( results );
            return nullptr;
        }

        ( ( sockaddr_in* ) addr->ai_addr )->sin_addr.S_un.S_un_b.s_b4 = 0;
        //printf("IPv4 address %s\n", inet_ntoa(((sockaddr_in*) addr->ai_addr)->sin_addr) );

        void *ai = malloc( sizeof( sockaddr ) );
        memcpy( ai, addr->ai_addr, sizeof( sockaddr ) );

        freeaddrinfo( results );
        return reinterpret_cast<SockAddress*>( ai );*/

        auto ai = ( sockaddr_in* )calloc( 1, sizeof( sockaddr_in ) );
        ai->sin_family = AF_INET;
        ai->sin_port = htons( port );
        ai->sin_addr.s_addr = inet_addr("255.255.255.255");
        return reinterpret_cast<SockAddress*>( ai );
    }

    const char* UdpSocketImpl::getErrorDesc()
    {
        return getLastSocketErrorDesc();
    }
    
    const char* UdpSocketImpl::getPeerIP()
    {
        // This won't work with IPv6, obviously
        return inet_ntoa( ( ( sockaddr_in* ) &peer )->sin_addr );
    }

    bool UdpSocketImpl::bind( uint16_t port )
    {
        socketStartup();

        // Create a socket, closing any possibly open
        disconnect();
        createSocket();

        if ( sock == INVALID_SOCKET )
            return false;

        isActuallyBlocking = true;

        // Build address structure
        sockaddr_in addr;
        addr.sin_len = sizeof(sockaddr_in);
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons( port );                // byteswap to network order

        // Bind and try to start listening
        if ( ::bind( sock, ( sockaddr* )( &addr ), sizeof( addr ) ) == SOCKET_ERROR )
        {
            disconnect();
            return false;
        }

        return true;
    }

    bool UdpSocketImpl::receive( ArrayIOStream& buffer, Timeout timeout )
    {
        if ( sock == INVALID_SOCKET )
            return 0;

        buffer.clear( true );
        setActuallyBlocking( timeout.infinite );

        socklen_t fromlen = sizeof( peer );

        do
        {
            int incoming = -1;

#ifdef __li_MSW
            DWORD dummy;
            WSAIoctl(sock, FIONREAD, nullptr, 0, &incoming, sizeof(incoming), &dummy, nullptr, nullptr);
#else
            ioctl(sock, FIONREAD, &incoming);
#endif

            if ( incoming > 0 )
            {
                buffer.resize( incoming, true );

                ssize_t got = recvfrom( sock, ( char* ) buffer.c_array(), ( size_t ) incoming, 0, &peer, &fromlen );

                if ( got <= 0 )
                {
#ifdef __li_MSW
                    if ( got == 0 || WSAGetLastError() != WSAEWOULDBLOCK )
#else
                    if ( got == 0 || errno != EAGAIN )
#endif
                        disconnect();

                    return false;
                }
                else
                {
                    buffer.setSize( got );
                    return true;
                }
            }

            pauseThread(1);
        }
        while ( !timeout.timedOut() );

        return false;
    }

    bool UdpSocketImpl::send( SockAddress* to, const void* data, size_t length )
    {
        if ( sock == INVALID_SOCKET && !createSocket() )
            return false;

        ssize_t res = sendto( sock, ( const char* ) data, ( int ) length, 0, ( sockaddr* ) to, sizeof( sockaddr_in ) );

        return res >= 0 && ( size_t ) res == length;
    }

    void UdpSocketImpl::setActuallyBlocking( bool blocking )
    {
        if ( isActuallyBlocking != blocking )
        {
#ifdef __li_MSW
            unsigned long nonBlocking = !blocking;
            ioctlsocket( sock, FIONBIO, &nonBlocking );
#else
            int flags = fcntl( sock, F_GETFL, 0 );

            if ( blocking )
                flags &= ~O_NONBLOCK;
            else
                flags |= O_NONBLOCK;

            fcntl( sock, F_SETFL, flags );
#endif

            isActuallyBlocking = blocking;
        }
    }

    UdpSocket* UdpSocket::create()
    {
        return new UdpSocketImpl();
    }
}
