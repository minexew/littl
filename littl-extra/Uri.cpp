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

#include <littl.hpp>

namespace li
{
    String Uri::escape( const String& uri )
    {
        // http://www.blooberry.com/indexdot/html/topics/urlencoding.htm

        String escaped;

        for ( unsigned i = 0; i < uri.getNumBytes(); i++ )
        {
            uint8_t c = uri[i];

            if ( ( c >= '0' && c <= '9' ) || ( c >= 'a' && c < 'z' ) || ( c >= 'A' && c <= 'Z' )
                    || c == '$' || c == '-' || c == '_' || c == '.' || c == '+' || c == '!' || c == '*' || c == '\'' || c == '(' || c == ')'
                    || c == '&' || c == ',' || c == '/' || c == ':' || c == ';' || c == '=' || c == '?' || c == '@' )
                escaped += Utf8Character( c );
            else
            {
                char encoded[4];
                snprintf( encoded, sizeof( encoded ), "%%%02X", c );

                escaped += encoded;
            }
        }

        //printf( "Escaped `%s` -> `%s`\n", uri.c_str(), escaped.c_str() );
        return escaped;
    }

    void Uri::parse( String uri, Parts& parts )
    {
        // uri = protocol://username:password@hostname:port/resource
        uri.split( "://", parts.protocol, uri, true );

        // uri = username:password@hostname:port/resource
        String userinfo;
        uri.split( '@', userinfo, uri, true );
        userinfo.split( ':', parts.username, parts.password );

        // uri = hostname:port/resource
        uri.split( '/', uri, parts.resource );

        // uri = hostname:port
        uri.split( ':', parts.host, parts.port );
    }
}
