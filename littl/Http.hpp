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

#pragma once

#include <littl/String.hpp>
#include <littl/TcpSocket.hpp>
#include <littl/Thread.hpp>

namespace li
{
    class HttpRequest;

    class Uri
    {
        public:
            struct Parts
            {
                String protocol;
                String username, password;
                String host, port;
                String resource;
            };

            static String escape( const String& uri );
            static void parse( String uri, Parts& parts );
    };

    class HttpRequestListener
    {
        public:
            virtual ~HttpRequestListener()
            {
            }

            virtual bool onDataReady( HttpRequest* request, uint64_t length )
            {
                // return true if interested in the data
                return false;
            }

            virtual void onData( HttpRequest* request, const void* data, uint64_t length )
            {
            }

            virtual void onStatusChange( HttpRequest* request )
            {
            }
    };

    class HttpRequest
    {
        friend class HttpClient;
        friend class HttpSession;

        public:
            enum Method
            {
                GET
            };

            enum Status
            {
                ready,
                queued,
                processing,
                successful,
                failed,
                aborted
            };

        protected:
            Status status;
            HttpRequestListener* listener;

            String resource, failReason;
            Method method;
            Timeout timeout;

            bool connectionLost;

            void changeStatus( Status newStatus );
            void fail( const String& failReason );

        public:
            HttpRequest( const char* resource, HttpRequestListener* listener, Method method = GET );
            ~HttpRequest();

            const String& getFailReason() const { return failReason; }
            Status getStatus() const { return status; }
            void setTimeout( const Timeout& to ) { timeout = to; }
    };

    class HttpSession : protected Thread, protected Mutex
    {
        String host;

        std::unique_ptr<TcpSocket> session;
        HttpRequest* currentRequest;
        List<HttpRequest*> queue;

        size_t bufferSize;

        bool readLine( String& line, const Timeout& to );

        protected:
            virtual void run();

        public:
            HttpSession( const char* host );

            void cancelAllRequests();
            const String& getHostName() const;
            bool isRunning() { return Thread::isRunning(); }
            void request( HttpRequest* request );
            void setBufferSize( size_t size ) { bufferSize = size; }
            bool waitFor( long time = -1 ) { return Thread::waitFor( time ); }
    };

    class HttpClient
    {
        List<HttpRequest*> queue;
        List<HttpSession*> sessions;

        HttpSession* getSession( const String& host );

        public:
            HttpClient();
            ~HttpClient();

            void cancelAllRequests();
            bool isRunning();
            void request( HttpRequest* request );
            bool waitFor( long time = -1 );
    };
}
