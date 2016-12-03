/*
    Copyright (c) 2011 Xeatheran Minexew

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
    HttpClient::HttpClient()
    {
    }

    HttpClient::~HttpClient()
    {
        for ( auto session : sessions )
            delete session;
    }

    void HttpClient::cancelAllRequests()
    {
        for ( auto session : sessions )
            session->cancelAllRequests();
    }

    bool HttpClient::isRunning()
    {
        for ( auto session : sessions )
        {
            if ( session->isRunning() )
                return true;
        }

        return false;
    }

    HttpSession* HttpClient::getSession( const String& host )
    {
        for ( auto session : sessions )
        {
            if ( session->getHostName() == host )
                return session;
        }

        HttpSession* session = new HttpSession( host );
        sessions.add( session );

        return session;
    }

    void HttpClient::request( HttpRequest* request )
    {
        // Parse the URI
        String uri = request->resource;

        if ( uri.beginsWith( "http://" ) )
            uri = uri.dropLeft( 7 );

        intptr_t slash = uri.findChar( '/' );
        String host;

        if ( slash < 0 )
        {
            host = uri;
            request->resource = "/";
        }
        else
        {
            host = uri.leftPart( slash );

            // We need the slash
            request->resource = uri.dropLeft( slash );
        }

        // Get or create the session
        HttpSession* session = getSession( host );

        // Request!
        session->request( request );
    }

    bool HttpClient::waitFor( long time )
    {
        for ( auto session : sessions )
        {
            if ( !session->waitFor( time ) )
                return false;
        }

        return true;
    }
}
