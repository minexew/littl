/*
    Copyright (c) 2010-2011 Xeatheran Minexew

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
    class Console: public IOStream
    {
        public:
            virtual ~Console()
            {
            }

            virtual bool isReadable()
            {
                return true;
            }

            virtual bool isWritable()
            {
                return true;
            }

            virtual size_t rawRead( void* out, size_t readSize )
            {
                return fread( out, 1, readSize, stdin );
            }

    		virtual size_t rawWrite( const void* in, size_t writeSize )
            {
                return fwrite( in, 1, writeSize, stdout );
            }

            /*virtual size_t read( void* output, size_t length )
            {
                return fread( output, 1, length, stdin );
            }*/

            static String readLine()
            {
                return Console().InputStream::readLine();
            }

            virtual size_t write( const void* input, size_t length )
            {
                return fwrite( input, 1, length, stdout );
            }

            static void write( const char* text )
            {
                printf( "%s", text );
            }

            static void write( const String& text )
            {
                write( text.c_str() );
            }

    	    static void writeLine( const String& text = String() )
            {
                write( text + li_newLine );
            }
    };
}
