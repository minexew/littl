/*
    Copyright (c) 2011, 2012 Xeatheran Minexew

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

#include <littl/Http.hpp>

namespace li
{
    HttpRequest::HttpRequest( const char* resource, HttpRequestListener* listener, Method method )
            : status( ready ), listener( listener ), resource( resource ), method( method ), connectionLost( false )
    {
    }

    HttpRequest::~HttpRequest()
    {
        delete listener;
    }

    void HttpRequest::changeStatus( Status newStatus )
    {
        status = newStatus;
        listener->onStatusChange( this );
    }

    void HttpRequest::fail( const String& failReason )
    {
        this->failReason = failReason;
        changeStatus( failed );
    }

    HttpSession::HttpSession( const char* host )
            : host( host ), currentRequest( 0 ), bufferSize( 0x1000 )
    {
    }

    void HttpSession::cancelAllRequests()
    {
        enter();

        iterate2 ( i, queue )
            i->changeStatus( HttpRequest::aborted );

        queue.clear();

        leave();
    }

    const String& HttpSession::getHostName() const
    {
        return host;
    }

    bool HttpSession::readLine( String& line, const Timeout& to )
    {
        while ( !to.timedOut() )
        {
            char next = 0;

            if ( !session->read( &next, 1, to.getRemaining() ) )
                return false;

            if ( next == 0 || next == '\n' )
                break;
            else if ( next != '\r' )
                line += UnicodeChar( next );
        }

        return true;
    }

    void HttpSession::request( HttpRequest* request )
    {
        enter();

        queue.add( request );
        request->changeStatus( HttpRequest::queued );

        if ( !isRunning() )
            start();

        leave();
    }

    void HttpSession::run()
    {
        while ( !shouldEnd )
        {
            if ( !currentRequest )
            {
                enter();
                if ( !queue.isEmpty() )
                {
                    currentRequest = queue[0];
                    queue.remove( 0 );
                }
                leave();

                if ( !currentRequest )
                    break;
            }

            //printf( "HttpSession: processing request for '%s' @ '%s'\n", currentRequest->resource.c_str(), host.c_str() );

            if ( !session )
            {
                session = TcpSocket::create( true );

                if ( !session->connect( host, 80, true ) )
                {
                    currentRequest->fail( "Unable to connect to " + host );

                    delete currentRequest;
                    currentRequest = 0;
                    continue;
                }
            }

            currentRequest->changeStatus( HttpRequest::processing );

            session->OutputStream::write( "GET " + Uri::escape( currentRequest->resource ) + " HTTP/1.1\r\n" );
            session->OutputStream::write( "Host: " + host + "\r\n" );
            session->OutputStream::write( "\r\n" );

            currentRequest->timeout.reset();

            bool failed = false, retry = false, close = false;
            uint64_t dataLength = 0;

            for ( String responseLine; ; responseLine.clear() )
            {
                if ( !readLine( responseLine, currentRequest->timeout ) )
                {
                    session.reset();

                    if ( currentRequest->connectionLost )
                    {
                        // Already failed for this reason before
                        currentRequest->fail( "Connection lost or timed out" );
                        failed = true;
                    }
                    else
                    {
                        // Second chance
                        currentRequest->connectionLost = true;
                        retry = true;
                    }

                    break;
                }

                if ( responseLine.isEmpty() )
                    break;

                if ( responseLine.beginsWith( "HTTP" ) )
                {
                    List<String> tokens;
                    responseLine.parse( tokens, ' ' );

                    if ( !tokens[1].beginsWith( '2' ) )
                    {
                        currentRequest->fail( "HTTP error: " + responseLine );
                        failed = true;
                    }
                }
                else if ( responseLine.beginsWith( "Content-Length: " ) )
                {
                    List<String> tokens;
                    responseLine.parse( tokens, ' ' );

                    dataLength = tokens[1].toUnsigned();
                }
                else if ( responseLine.equals( "Connection: close", false ) )
                    close = true;
            }

            if ( retry )
                continue;

            if ( dataLength > 0 )
            {
                bool wants = failed ? false : currentRequest->listener->onDataReady( currentRequest, dataLength );

                while ( dataLength > 0 )
                {
                    // TODO: alloca?
#ifdef __GNUC__
                    char receiveBuffer[bufferSize];
                    char* buffer = receiveBuffer;
#else
                    Array<char> receiveBuffer( bufferSize );
                    char* buffer = *receiveBuffer;
#endif

                    size_t length = ( dataLength > bufferSize ) ? bufferSize : ( size_t ) dataLength;

                    if ( !session->read( buffer, length, currentRequest->timeout.getRemaining() ) )
                    {
                        currentRequest->fail( "Connection lost or timed out" );

                        session.reset();
                        break;
                    }

                    if ( wants )
                        currentRequest->listener->onData( currentRequest, buffer, length );

                    dataLength -= length;
                }
            }

            if ( !failed )
                currentRequest->changeStatus( HttpRequest::successful );

            if ( close )
                session.reset();

            delete currentRequest;
            currentRequest = 0;
        }
    }
}
