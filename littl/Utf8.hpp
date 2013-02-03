#pragma once

/*
  thanks to the HTML Tidy project (http://sourceforge.net/projects/tidy/)
    for the UTF-8 {en,de}coding code
*/

namespace li
{
    typedef unsigned Utf8Char;

    class Unicode
    {
        public:
            typedef unsigned Char;

            static const Char invalidChar = 0xFFFFFFFF;

            struct Character
            {
                Char c;

                Character( Char c = invalidChar ) : c( c ) {}

                Character& operator = ( Char c ) { this->c = c; return *this; }
                operator Char& () { return c; }
                operator const Char& () const { return c; }
            };

            static bool isAlpha( Char c )
            {
                return ( c >= 'A' && c <= 'Z' ) || ( c >= 'a' && c <= 'z' );
            }

            static bool isAlphaNumeric( Char c )
            {
                return isAlpha( c ) || isNumeric( c );
            }

            static bool isNumeric( Char c )
            {
                return c >= '0' && c <= '9';
            }
    };

    class Utf8
    {
        public:
            const static Unicode::Char invalidChar = 0xFFFFFFFF;

            static unsigned beginDecode( Unicode::Char& c, unsigned char first )
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

            static unsigned decode( Unicode::Char& c, const char* buffer, size_t bufferLength )
            {
                unsigned numBytes = beginDecode( c, buffer[0] );

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

                for ( unsigned i = 1; i < numBytes && i < bufferLength; i++ )
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

            static unsigned encode( Unicode::Char c, char* buffer )
            {
                if ( c <= 0x7F )
                {
                    buffer[0] = ( char ) c;
                    buffer[1] = 0;
                    return 1;
                }
                else if ( c <= 0x7FF )
                {
                    buffer[0] = 0xC0 | ( char )( c >> 6 );
                    buffer[1] = 0x80 | ( c & 0x3F );
                    buffer[2] = 0;
                    return 2;
                }
                else if ( c <= 0xFFFF )
                {
                    buffer[0] = 0xE0 | ( char )( c >> 12 );
                    buffer[1] = 0x80 | ( ( c >> 6 ) & 0x3F);
                    buffer[2] = 0x80 | ( c & 0x3F);
                    buffer[3] = 0;
                    return 3;
                }
                else if ( c <= 0x1FFFFF )
                {
                    buffer[0] = 0xF0 | ( char )( c >> 18 );
                    buffer[1] = 0x80 | ( ( c >> 12 ) & 0x3F );
                    buffer[2] = 0x80 | ( ( c >> 6 ) & 0x3F );
                    buffer[3] = 0x80 | ( c & 0x3F );
                    buffer[4] = 0;
                    return 4;
                }
                else if ( c <= 0x3FFFFFF )
                {
                    buffer[0] = 0xF8 | ( c >> 24 );
                    buffer[1] = 0x80 | ( char )( c >> 18 );
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

            static bool isEmpty( const char* string )
            {
                // TODO: check for BOM/other non-chars??
                return !( string && *string );
            }

            static size_t numChars( const char* string )
            {
                size_t length = 0;

                if ( !string )
                    return 0;

                for ( size_t i = 0; string[i]; i++ )
                    if ( ( string[i] & 0xC0 ) != 0x80 )
                        length++;

                return length;
            }
    };

    typedef Unicode::Character Utf8Character, UnicodeCharacter;
}
