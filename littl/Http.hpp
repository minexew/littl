#pragma once

#include <littl.hpp>

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

        TcpSocket* session;
        HttpRequest* currentRequest;
        List<HttpRequest*> queue;

        size_t bufferSize;

        bool readLine( String& line, const Timeout& to );

        protected:
            virtual void run();

        public:
            HttpSession( const char* host );
            ~HttpSession();

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
