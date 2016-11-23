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

#include <littl/Array.hpp>

namespace li
{
    template <typename T = char>
    class StreamBuffer : public Array<T>
    {
        friend class TcpSocket;

        unsigned long length, pos;

        public:
            StreamBuffer() : length( 0 ), pos( 0 )
            {
            }

            void clear()
            {
                Array<T>::resize( 0, false );
                length = 0;
                pos = 0;
            }

            unsigned getLength() const
            {
                return length;
            }

            unsigned getPos() const
            {
                return pos;
            }

            template <typename T2> T2 read()
            {
                Array<T>::resize( pos + sizeof( T2 ), true );

                T2 temp = *( T2* )( Array<T>::getPtr( pos ) );
                seek( sizeof( T2 ) );

                return temp;
            }

            String readString()
            {
                String result;

                while ( pos < length )
                {
                    Unicode::Char nextChar;
                    unsigned numRead = Utf8::decode( nextChar, ( char* )Array<T>::getPtr( pos ), length - pos );

                    pos += numRead;

                    if ( numRead > 0 && nextChar )
                        result += UnicodeChar( nextChar );
                    else
                        break;
                }

                return result;
            }

            void seek( long offset, bool relative = true )
            {
                pos = ( relative ? pos : 0 ) + offset;

                if ( pos > length )
                    length = pos;
            }

            template <typename T2> void write( T2 item )
            {
                Array<T>::resize( pos + sizeof( T2 ), true );

                *( T2* )( Array<T>::getPtr( pos ) ) = item;
                seek( sizeof( T2 ) );
            }

            void writeString( const String& string )
            {
                if ( !string.isEmpty() )
                {
                    Array<T>::resize( pos + string.getNumBytes() + 1, true );

                    memcpy( Array<T>::getPtr( pos ), string, string.getNumBytes() + 1 );
                    seek( string.getNumBytes() + 1 );
                }
                else
                    write<char>( 0 );
            }

            void shiftLeft( unsigned count )
            {
                length -= count;
                Array<T>::move( 0, count, length );
            }

            void shiftRight( unsigned count )
            {
                Array<T>::resize( length + count, true );
                Array<T>::move( count, 0, length );
                length += count;
            }

            bool loadFromFile( const char* name )
            {
                FILE* file = fopen( name, "rb" );

                if ( !file )
                    return false;

                clear();

                fseek( file, 0, SEEK_END );
                length = ftell( file );

                Array<T>::resize( length, true );

                fseek( file, 0, SEEK_SET );
                fread( **this, length , 1, file );
                fclose( file );
                return true;
            }

            bool saveToFile( const char* name )
            {
                FILE* file = fopen( name, "wb" );

                if ( !file )
                    return false;

                fwrite( **this, length, 1, file );
                fclose( file );
                return true;
            }

            void xorData( unsigned char val )
            {
                for ( unsigned i = 0; i < length; i++ )
                    ( *this )[i] ^= val;
            }
    };
}
