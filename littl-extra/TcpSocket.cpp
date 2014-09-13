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

#include <littl/TcpSocket.hpp>

namespace li
{
    class TcpSocketImpl : public TcpSocket
    {
        private:
            State state;

            SOCKET sock;
            sockaddr_in peer;

            // general socket properties
            bool isBlocking, isActuallyBlocking, delayEnabled;

            // safe receiving related stuff
            bool receiving;
            Array<uint8_t> recvBuffer;
            size_t bytesReceived;

            // message-related stuff
            bool headerReceived;
            int32_t messageLength;

            void setActuallyBlocking( bool blocking );
            void updateSocket();

        private:
            TcpSocketImpl(const TcpSocketImpl&);

        public:
            TcpSocketImpl();
            TcpSocketImpl(bool blocking);
            virtual ~TcpSocketImpl();

            virtual const char* getErrorDesc() override;
            virtual const char* getPeerIP() override;
            virtual State getState() override { return state; }

            virtual void setBlocking( bool blocking ) override;
            virtual void setDelayedSending( bool enabled ) override;

            virtual bool listen( uint16_t port ) override;
            virtual std::unique_ptr<TcpSocket> accept( bool block ) override;

            virtual bool connect( const char* host, uint16_t port, bool block ) override;
            virtual bool connectFinished( bool &success ) override;
            virtual void disconnect() override;

            virtual bool read( void* output, size_t length, Timeout timeout, bool peek ) override;

            virtual bool receive( ArrayIOStream& buffer, Timeout timeout ) override;
            virtual bool send( const void* data, size_t length ) override;

            virtual size_t readUnbuffered( void* buffer, size_t maxlen ) override;

            virtual bool finite() override { return false; }
            virtual bool seekable() override { return false; }

            virtual void flush() override {}

            virtual FilePos getPos() override { return 0; }
            virtual FileSize getSize() override { return 0; }
            virtual bool setPos( FilePos pos ) override { return false; }

            virtual bool eof() override { return false; }

            virtual size_t read( void* out, size_t length ) override;
            virtual size_t write( const void* in, size_t length ) override;
    };

    TcpSocketImpl::TcpSocketImpl()
    {
        receiving = false;
        bytesReceived = 0;
        headerReceived = false;
        messageLength = -1;
    }

    TcpSocketImpl::TcpSocketImpl(bool blocking)
    {
        state = idle;

        sock = INVALID_SOCKET;
        isBlocking = blocking;
        isActuallyBlocking = false;
        delayEnabled = false;

        receiving = false;
        bytesReceived = 0;
        headerReceived = false;
        messageLength = -1;
    }

    TcpSocketImpl::~TcpSocketImpl()
    {
        disconnect();
    }

    std::unique_ptr<TcpSocket> TcpSocketImpl::accept( bool block )
    {
        if ( state != listening )
            return nullptr;

        // Set blocking mode
        setActuallyBlocking( block );

        // Try to accept an incoming connection
        int addrlen = sizeof( peer );
        SOCKET clientSocket = ::accept( sock, ( sockaddr* ) &peer, ( socklen_t* ) &addrlen );

        if ( clientSocket == INVALID_SOCKET )
            return nullptr;

        // Create and return a new class instance
        std::unique_ptr<TcpSocketImpl> inst(new TcpSocketImpl);

        inst->state = host;
        inst->sock = clientSocket;
        inst->peer = peer;
        inst->isBlocking = isBlocking;
        inst->isActuallyBlocking = true;
        inst->delayEnabled = delayEnabled;

        inst->updateSocket();
        return std::move( inst );
    }

    bool TcpSocketImpl::connect( const char* host, uint16_t port, bool block )
    {
        socketStartup();

        // Create a socket, closing any possibly open
        disconnect();

        // Address info structures
        addrinfo hints, *results, *current;

        // Set up hints for GetAddrInfo
        memset( &hints, 0, sizeof( hints ) );
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        // This has been always obnoxious
        char portString[16];
        snprintf( portString, sizeof( portString ), "%d", ( int ) port );

        // Try to resolve the address now
        if ( getaddrinfo( host, portString, &hints, &results ) != 0 )
            return false;

        // Browse the results and try to find a suitable entry
        for ( current = results; current != nullptr; current = current->ai_next )
            if ( current->ai_family == AF_INET && current->ai_socktype == SOCK_STREAM && current->ai_protocol == IPPROTO_TCP )
                break;

        // None found?
        if ( current == nullptr )
        {
            // FIXME: Error reporting doesn't work for this one
            freeaddrinfo( results );
            return false;
        }

        // 'current' now points to a suitable addrinfo, let's go on and try to connect to it
        sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

        if ( sock == INVALID_SOCKET )
        {
            freeaddrinfo( results );
            return false;
        }

        isActuallyBlocking = true;
        setActuallyBlocking( block );

        int res = ::connect( sock, current->ai_addr, current->ai_addrlen );

        if ( res == SOCKET_ERROR
#ifdef __li_MSW
                && WSAGetLastError() != WSAEWOULDBLOCK )
#else
                && errno != EINPROGRESS )
#endif
        {
            freeaddrinfo( results );
            disconnect();
            return false;
        }

        memcpy( &peer, current->ai_addr, std::min<int>( sizeof( peer ), current->ai_addrlen ) );

        // We can release the lookup results now
        freeaddrinfo( results );

        // Final settings
        state = ( res == SOCKET_ERROR ) ? connecting : connected;
        updateSocket();
        return true;
    }

    bool TcpSocketImpl::connectFinished( bool &success )
    {
        if ( state != connecting )
        {
            success = false;
            return true;
        }

        fd_set writefds, exceptfds;
        timeval zeroTime = { 0, 0 };

        // Set up the sets
        FD_ZERO( &writefds );
        FD_SET( sock, &writefds );

        FD_ZERO( &exceptfds );
        FD_SET( sock, &exceptfds );

        int res = select( 0, 0, &writefds, &exceptfds, &zeroTime );

        if ( res == 0 )
            return false;
        else if ( res == SOCKET_ERROR )
        {
            disconnect();
            success = false;
            return true;
        }

        if ( FD_ISSET( sock, &writefds ) )
        {
            state = connected;
            success = true;
            return true;
        }

        if ( FD_ISSET( sock, &exceptfds ) )
        {
            disconnect();
            success = false;
            return true;
        }

        return false;
    }

    void TcpSocketImpl::disconnect()
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

        state = idle;
    }

    const char* TcpSocketImpl::getErrorDesc()
    {
        return getLastSocketErrorDesc();
    }

    const char* TcpSocketImpl::getPeerIP()
    {
        if ( state != host && state != connected )
            return nullptr;

        // This won't work with IPv6, obviously
        return inet_ntoa( peer.sin_addr );
    }

    /*bool TcpSocketImpl::isReadable()
    {
        if ( state != host && state != connected )
            return false;

        // This is not 100% reliable, timeout should be always used

        fd_set socketSet;
        timeval zeroTime = { 0, 0 };

        FD_ZERO( &socketSet );
        FD_SET( sock, &socketSet );

        return select( 0, &socketSet, 0, 0, &zeroTime ) == 1;
    }*/

    /*bool TcpSocketImpl::isWritable()
    {
        if ( state != host && state != connected )
            return false;

        // TODO: Do we need both of those?
        // TODO: Can either of these tests give us false positives?

        char temp;

        if ( recv( sock, &temp, 1, MSG_PEEK ) == SOCKET_ERROR
#ifdef __li_MSW
                && WSAGetLastError() != WSAEWOULDBLOCK )
#else
                && errno != EAGAIN )
#endif
            return false;

        fd_set socketSet;
        timeval zeroTime = { 0, 0 };

        FD_ZERO( &socketSet );
        FD_SET( sock, &socketSet );

        if ( select( 0, 0, &socketSet, 0, &zeroTime ) != 1 )
            return false;

        return true;
    }*/

    bool TcpSocketImpl::listen( uint16_t port )
    {
        socketStartup();

        // Create a socket, closing any possibly open
        disconnect();

        sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

        if ( sock == INVALID_SOCKET )
            return false;

        isActuallyBlocking = true;

        // Build address structure
        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons( port );                // byteswap to network order

        // Bind and try to start listening
        if ( bind( sock, ( sockaddr* )( &addr ), sizeof( addr ) ) == SOCKET_ERROR
                || ::listen( sock, SOMAXCONN ) == SOCKET_ERROR )
        {
            disconnect();
            return false;
        }

        // Final settings
        state = listening;
        updateSocket();
        return true;
    }

    size_t TcpSocketImpl::read( void* out, size_t length )
    {
        return read( out, length, Timeout(), false ) ? length : 0;
    }

    bool TcpSocketImpl::read( void* output, size_t length, Timeout timeout, bool peek )
    {
        if ( state != host && state != connected )
            return false;

        if ( !receiving )
        {
            // Initiate a receive
            recvBuffer.resize( length, true );
            receiving = true;
            bytesReceived = 0;
        }
        else if ( recvBuffer.getCapacity() < length )
            recvBuffer.resize( length );

        // Are we full?
        if ( bytesReceived >= length )
        {
            if ( output != nullptr )
                memcpy( output, recvBuffer.c_array(), length );

            if ( !peek )
                receiving = false;

            return true;
        }

        do
        {
            int got = recv( sock, ( char* ) recvBuffer.c_array() + bytesReceived, length - bytesReceived, 0 );

            // The connection has been closed
            if ( got == 0 )
            {
                disconnect();
                return false;
            }

            // Socket error
            if ( got < 0
#ifdef __li_MSW
                    && WSAGetLastError() != WSAEWOULDBLOCK )
#else
                    && errno != EAGAIN )
#endif
            {
                disconnect();
                return false;
            }

            if ( got > 0 )
            {
                bytesReceived += got;

                // Received the whole chunk, thanks.
                if ( bytesReceived >= length )
                {
                    // TODO: DRY

                    if ( output != nullptr )
                        memcpy( output, recvBuffer.c_array(), length );

                    if ( !peek )
                        receiving = false;

                    return true;
                }
            }
        }
        while ( !timeout.timedOut() );

        return false;
    }

    size_t TcpSocketImpl::readUnbuffered( void* buffer, size_t maxlen ) 
    {
        if ( state != host && state != connected )
            return 0;

        int got = recv( sock, ( char* ) buffer, ( int ) maxlen, 0 );

        if ( got <= 0 )
        {
#ifdef __li_MSW
            if ( got == 0 || WSAGetLastError() != WSAEWOULDBLOCK )
#else
            if ( got == 0 || errno != EAGAIN )
#endif
                disconnect();

            return 0;
        }

        return got;
    }

    bool TcpSocketImpl::receive( ArrayIOStream& buffer, Timeout timeout )
    {
        if ( state != host && state != connected )
            return false;

        buffer.clear( true );

        // Get the message length first
        if ( !headerReceived )
        {
            if ( !read( &messageLength, sizeof( messageLength ), timeout, false ) )
                return false;
            else
                headerReceived = true;
        }

        // Resize buffer and (try to) receive the actual message
        buffer.resize( messageLength, true );

        if ( read( buffer.getPtr(), messageLength, timeout, false ) )
        {
            headerReceived = false;
            buffer.setSize( messageLength );
            return true;
        }
        else
            return false;
    }

    bool TcpSocketImpl::send( const void* data, size_t length )
    {
        uint32_t len = length;

        return write( &len, sizeof( len ) ) == sizeof( len ) && write( data, length ) == length;
    }

    void TcpSocketImpl::setActuallyBlocking( bool blocking )
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

    void TcpSocketImpl::setBlocking( bool blocking )
    {
        isBlocking = blocking;
        updateSocket();
    }

    void TcpSocketImpl::setDelayedSending( bool enabled )
    {
        delayEnabled = enabled;
        updateSocket();
    }

    void TcpSocketImpl::updateSocket()
    {
        setActuallyBlocking( isBlocking );

        // Set Nagle's delay
        int flag = delayEnabled ? 0 : 1;
           setsockopt( sock, IPPROTO_TCP, TCP_NODELAY, ( char* ) &flag, sizeof( int ) );
    }

    size_t TcpSocketImpl::write( const void* input, size_t length )
    {
        if ( state != host && state != connected )
            return 0;

        size_t sentTotal = 0;

        while ( sentTotal < length )
        {
            int sent = ::send( sock, ( char* ) input + sentTotal, length - sentTotal, 0 );

            // Socket error
            if ( sent <= 0
#ifdef __li_MSW
                    && WSAGetLastError() != WSAEWOULDBLOCK )
#else
                    && errno != EAGAIN )
#endif
                return sentTotal;

            if ( sent > 0 )
            {
                sentTotal += sent;

                if ( sentTotal >= length )
                    break;
            }

            // This can (and will) get pretty nasty if we have to wait for the driver to flush its buffer,
            // but there is probably no nice single-threaded solution.

            // TODO: Is there any way we can help the user code with this?

            fd_set socketSet;
            FD_ZERO( &socketSet );
            FD_SET( sock, &socketSet );

            if ( select( 0, 0, &socketSet, 0, 0 ) < 0 )
                return sentTotal;
        }

        return sentTotal;
    }

    std::unique_ptr<TcpSocket> TcpSocket::create( bool blocking )
    {
        return std::unique_ptr<TcpSocket>( new TcpSocketImpl( blocking ) );
    }
}
