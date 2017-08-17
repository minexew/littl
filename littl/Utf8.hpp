/*
    Copyright (C) 2011, 2012, 2013 Xeatheran Minexew

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

#pragma once

#include "Unicode.hpp"

#include <littl/Algorithm.hpp>

/*
  thanks to the HTML Tidy project (http://sourceforge.net/projects/tidy/)
    for the UTF-8 {en,de}coding code
*/

namespace li
{
#define li_this Utf8Tpl<dummy_>

    template <int dummy_>
    class Utf8Tpl
    {
        public:
            static unsigned int beginDecode( Unicode::Char& c, unsigned char first );
            static size_t decode( Unicode::Char& c, const char* buffer, size_t bufferLength );

            template <typename Allocator>
            static wchar_t* decodeToWideStringAlloc( const char* utf8String, size_t& numChars_out );

            static size_t decodeToWideStringBuffer( wchar_t* buffer, size_t bufferSizeInWchars, const char* utf8String );

            static unsigned int encode( Unicode::Char c, char* buffer );
            static bool isEmpty( const char* string );
            static size_t numChars( const char* string );
    };

    typedef Utf8Tpl<0> Utf8;

#define li_member( type ) template <int dummy_> type li_this::

    li_member( unsigned int ) beginDecode( Unicode::Char& c, unsigned char first )
    {
        if ( first <= 0x7F )
        {
            c = first;
            return 1;
        }
        else if ( ( first & 0xE0 ) == 0xC0 )
        {
            c = first & 31;
            return 2;
        }
        else if ( ( first & 0xF0 ) == 0xE0 )
        {
            c = first & 15;
            return 3;
        }
        else if ( ( first & 0xF8 ) == 0xF0 )
        {
            c = first & 7;
            return 4;
        }
        else if ( ( first & 0xFC ) == 0xF8 )
        {
            c = first & 3;
            return 5;
        }
        else if ( ( first & 0xFE ) == 0xFC )
        {
            c = first & 1;
            return 6;
        }
        else
            return 0;
    }

    li_member( size_t ) decode( Unicode::Char& c, const char* buffer, size_t bufferLength )
    {
        const size_t numBytes = beginDecode( c, buffer[0] );

        if ( numBytes == 0 )
        {
            c = Unicode::invalidChar;
            return 0;
        }

        if ( numBytes > bufferLength )
        {
            c = Unicode::invalidChar;
            return 0;
        }

        for ( size_t i = 1; i < numBytes && i < bufferLength; i++ )
        {
            if ( !buffer[i] || ( buffer[i] & 0xC0 ) != 0x80 )
            {
                c = Unicode::invalidChar;
                return 0;
            }

            c = ( c << 6 ) | ( buffer[i] & 0x3F );
        }

        return numBytes;
    }

    template <int dummy_>
    template <typename Allocator>
    wchar_t* li_this::decodeToWideStringAlloc( const char* utf8String, size_t& numChars_out )
    {
        if ( isEmpty( utf8String ) )
            return nullptr;

        const size_t bytesIn = strlen( utf8String );

        // Start out conservative
        size_t bufferSizeInWchars = align<4>( bytesIn / 2 );

        wchar_t* buffer = Allocator::allocate( bufferSizeInWchars + 1 );

        numChars_out = 0;

        for ( size_t bytesRead = 0; bytesRead < bytesIn; )
        {
            UnicodeChar c;

            unsigned int charLength = beginDecode( c, *utf8String );

            if ( bytesRead + charLength > bytesIn )
                return Allocator::release( buffer ),
                        nullptr;

            decode( c, utf8String + bytesRead, charLength );

            if ( numChars_out + 1 > bufferSizeInWchars )
            {
                bufferSizeInWchars = bufferSizeInWchars * 3 / 2;
                buffer = Allocator::resize( buffer, bufferSizeInWchars + 1 );
            }

            buffer[numChars_out++] = c;
            bytesRead += charLength;
        }

        buffer[numChars_out] = 0;
        return buffer;
    }

    li_member( size_t ) decodeToWideStringBuffer( wchar_t* buffer, size_t bufferSizeInWchars, const char* utf8String )
    {
        if ( isEmpty( utf8String ) )
            return 0;

        const size_t bytesIn = strlen( utf8String );

        size_t numChars_out = 0;

        for ( size_t bytesRead = 0; bytesRead < bytesIn; )
        {
            UnicodeChar c;

            unsigned int charLength = beginDecode( c, *utf8String );

            if ( bytesRead + charLength > bytesIn )
                return 0;

            decode( c, utf8String + bytesRead, charLength );

            if ( numChars_out + 1 > bufferSizeInWchars - 1 )
                return 0;

            buffer[numChars_out++] = c;
            bytesRead += charLength;
        }

        buffer[numChars_out] = 0;
        return numChars_out;
    }

    li_member( unsigned int ) encode( Unicode::Char c, char* buffer )
    {
        if ( c <= 0x7F )
        {
            buffer[0] = static_cast<char>( c );
            buffer[1] = 0;
            return 1;
        }
        else if ( c <= 0x7FF )
        {
            buffer[0] = 0xC0 | ( ( c >> 6 ) & 0x1F );
            buffer[1] = 0x80 | ( c & 0x3F );
            buffer[2] = 0;
            return 2;
        }
        else if ( c <= 0xFFFF )
        {
            buffer[0] = 0xE0 | ( ( c >> 12 ) & 0x0F );
            buffer[1] = 0x80 | ( ( c >> 6 ) & 0x3F);
            buffer[2] = 0x80 | ( c & 0x3F);
            buffer[3] = 0;
            return 3;
        }
        else if ( c <= 0x1FFFFF )
        {
            buffer[0] = 0xF0 | ( ( c >> 18 ) & 0x07 );
            buffer[1] = 0x80 | ( ( c >> 12 ) & 0x3F );
            buffer[2] = 0x80 | ( ( c >> 6 ) & 0x3F );
            buffer[3] = 0x80 | ( c & 0x3F );
            buffer[4] = 0;
            return 4;
        }
        else if ( c <= 0x3FFFFFF )
        {
            buffer[0] = 0xF8 | ( c >> 24 );
            buffer[1] = 0x80 | ( ( c >> 18 ) & 0x3F );
            buffer[2] = 0x80 | ( ( c >> 12 ) & 0x3F );
            buffer[3] = 0x80 | ( ( c >> 6 ) & 0x3F );
            buffer[4] = 0x80 | ( c & 0x3F );
            buffer[5] = 0;
            return 5;
        }
        else if ( c <= 0x7FFFFFFF )
        {
            buffer[0] = 0xFC | ( c >> 30 );
            buffer[1] = 0x80 | ( ( c >> 24 ) & 0x3F );
            buffer[2] = 0x80 | ( ( c >> 18 ) & 0x3F );
            buffer[3] = 0x80 | ( ( c >> 12 ) & 0x3F );
            buffer[4] = 0x80 | ( ( c >> 6 ) & 0x3F );
            buffer[5] = 0x80 | ( c & 0x3F );
            buffer[6] = 0;
            return 6;
        }
        else
            return 0;
    }

    li_member( bool ) isEmpty( const char* string )
    {
        // TODO: check for BOM/other non-chars??
        return !( string && *string );
    }

    li_member( size_t ) numChars( const char* string )
    {
        size_t length = 0;

        if ( !string )
            return 0;

        for ( size_t i = 0; string[i]; i++ )
            if ( ( string[i] & 0xC0 ) != 0x80 )
                length++;

        return length;
    }

#undef li_member

#undef li_this
}
