/*
    Copyright (C) 2010, 2011, 2012 Xeatheran Minexew

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

#include <littl/List.hpp>
#include <littl/StringCaseCompare.hpp>
#include <littl/Utf8.hpp>

#include <cinttypes>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace li
{
#define default_init_seq data( 0 ), numBytes( 0 ), capacity( 0 ), numChars( 0 )

    template <int blockSize = 0x10> class StringTpl
    {
        public:
            enum Base { undefined, decimal, hexadecimal };

        protected:
            char* data;
            size_t numBytes, capacity;
            intptr_t numChars;

        public:
            StringTpl() : default_init_seq
            {
            }

            StringTpl( StringTpl&& other ) : data( other.data ), numBytes( other.numBytes ), capacity( other.capacity ), numChars( other.numChars )
            {
                other.data = nullptr;
                other.numBytes = 0;
                other.capacity = 0;
                other.numChars = 0;
            }

            StringTpl( const char* stringUtf8 ) : default_init_seq
            {
                set( stringUtf8 );
            }

            StringTpl( const char* stringUtf8, size_t numBytes ) : default_init_seq
            {
                set( stringUtf8, numBytes );
            }

            StringTpl( char c ) : default_init_seq
            {
                append( c );
            }

            StringTpl( UnicodeChar c ) : default_init_seq
            {
                append( c );
            }

            StringTpl( const StringTpl& other ) : default_init_seq
            {
                set( other );
            }

            StringTpl( const StringTpl* other ) : default_init_seq
            {
                set( *other );
            }

            StringTpl( int value ) : default_init_seq
            {
                appendNumber( value );
            }

            StringTpl( unsigned value ) : default_init_seq
            {
                appendNumber( value );
            }

            StringTpl( long value ) : default_init_seq
            {
                appendNumber( value );
            }

            StringTpl( long long value ) : default_init_seq
            {
                appendNumber( value );
            }

            StringTpl( float value ) : default_init_seq
            {
                appendNumber( value );
            }

            StringTpl( double value ) : default_init_seq
            {
                appendNumber( value );
            }

            ~StringTpl()
            {
                clear();
            }

            void append( char c );
            void append( Unicode::Char c );
            void append( const UnicodeChar& c );
            void append( const char* stringUtf8 );
            void append( const char* stringUtf8, size_t numBytes );
            void append( const StringTpl& other );
            bool beginsWith( Unicode::Char c ) const;
            bool beginsWith( const StringTpl& other ) const;
            Unicode::Char charAt( size_t offset ) const;

            void clear()
            {
                if ( data )
                    free( data );

                numChars = 0;
                numBytes = capacity = 0;
                data = 0;
            }

            static int compare( const char* s1, const char* s2, bool caseSensitive = true )
            {
                const int s1empty = ( s1 == nullptr || *s1 == 0 );
                const int s2empty = ( s2 == nullptr || *s2 == 0 );

                if ( s1empty || s2empty )
                    return s1empty - s2empty;

                if ( caseSensitive )
                    return strcmp( s1, s2 );
                else
                    return stringCaseCompare( s1, s2 );
            }

            const char* c_str() const
            {
                return ( data != nullptr ) ? data : "";
            }

            void debug() const
            {
                printf( "'%s' [%i, %i, %i]\n", data, numChars, numBytes, capacity );
            }

            StringTpl<blockSize> dropLeftPart( size_t length ) const;
            StringTpl<blockSize> dropLeft( size_t length ) const { return dropLeftPart( length ); }
            StringTpl<blockSize> dropRightPart( size_t length ) const;
            StringTpl<blockSize> dropRight( size_t length ) const { return dropRightPart( length ); }
            bool endsWith( UnicodeChar c );
            bool endsWith( const StringTpl& other ) const;

            bool equals( const char* other, bool caseSensitive = true ) const
            {
                return compare( c_str(), other, caseSensitive ) == 0;
            }

            static bool equals( const char* str, const char* other, bool caseSensitive = true )
            {
                return compare( str, other, caseSensitive ) == 0;
            }

            intptr_t findChar( Unicode::Char c, size_t beginAt = 0 ) const;
            intptr_t findDifferentChar( Unicode::Char c, size_t beginAt = 0 ) const;
            intptr_t findLastChar( Unicode::Char c, size_t beginAt = 0 ) const;
            intptr_t findLastSubString( const StringTpl& pattern, size_t beginAt = 0 );
            intptr_t findSubString( const StringTpl& pattern, size_t beginAt = 0 ) const;
            static StringTpl<blockSize> formatBool( bool value, bool useText = false );
            static StringTpl<blockSize> formatFloat( float value, int width = -1 );
            static StringTpl<blockSize> formatInt( int value, int width = -1, Base base = decimal );
            char* getBuffer() { return data; }
            Unicode::Char getChar( size_t& index ) const;
            StringTpl<blockSize> getFiltered( int (* filter)( int c ) ) const;

            static size_t getHash( const char* string );
            static size_t getHash( const StringTpl& string ) { return getHash( string.c_str() ); }
            size_t getHash() const { return getHash( *this ); }

            size_t getNumBytes() const
            {
                return numBytes;
            }

            size_t getNumChars()
            {
                if ( numChars < 0 )
                    numChars = getNumCharsUncached();

                return numChars;
            }

            size_t getNumCharsCached( intptr_t& cache ) const
            {
                if ( cache < 0 )
                    cache = getNumCharsUncached();

                return cache;
            }

            size_t getNumCharsUncached() const
            {
                if ( numChars < 0 )
                    return Utf8::numChars( data );
                else
                    return numChars;
            }

            bool isEmpty() const
            {
                return data == 0 || Utf8::isEmpty( data );
            }

            StringTpl<blockSize> leftPart( size_t length ) const;
            StringTpl<blockSize> left( size_t length ) const { return leftPart( length ); }
            unsigned parse( List<StringTpl>& tokens, Unicode::Char separator, Unicode::Char escape = Unicode::invalidChar, bool strict = false ) const;
            StringTpl<blockSize> replaceAll( const StringTpl& pattern, const StringTpl& replaceWith ) const;
            StringTpl<blockSize> rightPart( size_t length ) const;
            StringTpl<blockSize> right( size_t length ) const { return rightPart( length ); };
            void set( const char* stringUtf8 );
            void set( const char* stringUtf8, size_t numBytes );
            void set( const StringTpl& other );
            void setBuffer( size_t requestedCapacity );
            void split( Unicode::Char separator, StringTpl& left, StringTpl& right, bool rightDefault = false );
            void split( const StringTpl& separator, StringTpl& left, StringTpl& right, bool rightDefault = false );
            StringTpl<blockSize> subString( size_t begin, size_t length ) const;

            int toAscii( Array<char>& output );

            bool toBool() const { return toBool( data ); }
            static bool toBool( const char* text )
            {
                if ( text == nullptr )
                    return false;

                if ( equals( text, "true", false ) || equals( text, "yes", false ) )
                    return true;

                return strtol( text, nullptr, 0 ) != 0;
            }

            double toDouble() const { return toDouble( data ); }
            static double toDouble( const char* text ) { return text != nullptr ? strtod( text, nullptr ) : 0; }

            float toFloat() const { return toFloat( data ); }
            static float toFloat( const char* text ) { return text != nullptr ? strtof( text, nullptr ) : 0; }

            long toInt() const { return toInt( data ); }
            static long toInt( const char* text ) { return text != nullptr ? strtol( text, nullptr, 0 ) : 0; }

            uint64_t toUnsigned( Base base = undefined ) const { return parseUnsigned( data, base ); }
            static uint64_t parseUnsigned( const char* text, Base base = undefined );

            operator const char* () const
            {
                return data;
            }

            StringTpl& operator = ( StringTpl&& other )
            {
                clear();

                data = other.data;
                numBytes = other.numBytes;
                capacity = other.capacity;
                numChars = other.numChars;

                other.data = nullptr;
                other.numBytes = 0;
                other.capacity = 0;
                other.numChars = 0;

                return *this;
            }

#define operator_set( type )\
            StringTpl& operator = ( type other )\
            {\
                set( other );\
                return *this;\
            }

#define operator_append( type )\
            StringTpl& operator += ( type other )\
            {\
                append( other );\
                return *this;\
            }\
\
            StringTpl operator + ( type other ) const\
            {\
                StringTpl result( *this );\
                result += other;\
                return result;\
            }

#define operator_append_num( type, fmt )\
            void appendNumber( type num )\
            {\
                char buffer[20];\
\
                snprintf( buffer, 20, fmt, static_cast<type>( num ) );\
                append( buffer );\
            }\
\
            StringTpl& operator += ( type other )\
            {\
                appendNumber( other );\
                return *this;\
            }\
\
            StringTpl operator + ( type other ) const\
            {\
                StringTpl result( *this );\
                result += other;\
                return result;\
            }

#define operator_compare( type )\
            bool operator == ( type other ) const\
            {\
                return compare( c_str(), other ) == 0;\
            }\
\
            bool operator != ( type other ) const\
            {\
                return compare( c_str(), other ) != 0;\
            }\
\
            bool operator < ( type other ) const\
            {\
                return compare( c_str(), other ) < 0;\
            }\
\
            bool operator <= ( type other ) const\
            {\
                return compare( c_str(), other ) <= 0;\
            }\
\
            bool operator > ( type other ) const\
            {\
                return compare( c_str(), other ) > 0;\
            }\
\
            bool operator >= ( type other ) const\
            {\
                return compare( c_str(), other ) >= 0;\
            }

#define operator_to_int( type )\
            operator type () const\
            {\
                return data ? strtol( data, 0, 0 ) : 0;\
            }

#define operator_to_num( type )\
            operator type () const\
            {\
                return ( type )( data ? strtod( data, 0 ) : 0.0 );\
            }

            operator_set( const char* )
            operator_set( const StringTpl& )

            operator_append( char )
            operator_append( char* )
            operator_append( const UnicodeChar& )
            operator_append( const char* )
            operator_append( const StringTpl& )

            operator_append_num( int, "%i" )
            operator_append_num( unsigned, "%u" )
            operator_append_num( long, "%li" )
            operator_append_num( unsigned long, "%lu" )
            operator_append_num( long long, "%" PRIi64 )
            operator_append_num( float, "%g" )
            operator_append_num( double, "%g" )

            //operator_append_num_2( HexInt, "0x%X", unsigned )

            operator_compare( const char* )
            operator_compare( const StringTpl& )

            /*operator_to_int( short )
            operator_to_int( int )
            operator_to_int( unsigned )
            operator_to_int( unsigned short )
            operator_to_int( long )
            operator_to_num( float )
            operator_to_num( double )*/
    };

#define __li_member( type ) template <int blockSize> type StringTpl<blockSize>::
#define __li_member_ template <int blockSize> StringTpl<blockSize>::

    __li_member( void ) append( char c )
    {
        if ( c == 0 )
            return;

        char buffer[] = { c, 0 };

        if ( numChars >= 0 )
            numChars++;

        setBuffer( numBytes + 2 );
        strncpy( data + numBytes, buffer, 2 );

        numBytes++;
    }

    __li_member( void ) append( Unicode::Char c )
    {
        if ( c == 0 )
            return;

        char buffer[7];
        unsigned numBytesInc = Utf8::encode( c, buffer );

        if ( numBytesInc )
        {
            if ( numChars >= 0 )
                numChars++;

            setBuffer( numBytes + numBytesInc + 1 );
            strncpy( data + numBytes, buffer, numBytesInc + 1 );

            numBytes += numBytesInc;
        }
    }

    __li_member( void ) append( const UnicodeChar& c )
    {
        append( c.c );
    }

    __li_member( void ) append( const char* stringUtf8 )
    {
        if ( !stringUtf8 || !*stringUtf8 )
            return;

        numChars = -1;
        size_t numBytesInc = strlen( stringUtf8 );

        setBuffer( numBytes + numBytesInc + 1 );
        memcpy( data + numBytes, stringUtf8, numBytesInc + 1 );

        numBytes += numBytesInc;
    }

    __li_member( void ) append( const char* stringUtf8, size_t numBytes )
    {
        if ( !stringUtf8 || !*stringUtf8 )
            return;

        numChars = -1;

        setBuffer( this->numBytes + numBytes );
        memcpy( data + this->numBytes, stringUtf8, numBytes );
        
        this->numBytes += numBytes;
        data[this->numBytes] = 0;
    }

    __li_member( void ) append( const StringTpl& other )
    {
        if ( other.isEmpty() )
            return;

        if ( numChars >= 0 && other.numChars >= 0 )
            numChars += other.numChars;
        else
            numChars = -1;

        setBuffer( numBytes + other.numBytes + 1 );
        strncpy( data + numBytes, other.data, other.numBytes + 1 );

        numBytes += other.numBytes;
    }

    __li_member( bool ) beginsWith( Unicode::Char c ) const
    {
        size_t index = 0;

        return getChar( index ) == c;
    }

    __li_member( bool ) beginsWith( const StringTpl& other ) const
    {
        intptr_t cache = -1;
        size_t index = 0, otherIndex = 0;

        while ( index < getNumCharsCached( cache ) )
        {
            const Unicode::Char thisChar = getChar( index ),
                    otherChar = other.getChar( otherIndex );

            if ( otherChar == Unicode::invalidChar )
                return true;
            else if ( thisChar != otherChar )
                return false;
        }

        return false;
    }

    __li_member( Unicode::Char ) charAt( size_t offset ) const
    {
        size_t index = 0;

        for ( size_t i = 0; i < offset; i++ )
            if ( getChar( index ) == Unicode::invalidChar )
                return Unicode::invalidChar;

        return getChar( index );
    }

    __li_member( StringTpl<blockSize> ) dropLeftPart( size_t length ) const
    {
        intptr_t cache = -1;

        if ( length >= getNumCharsCached( cache ) )
            return StringTpl();
        else if ( length == 0 )
            return this;
        else
            return rightPart( getNumCharsCached( cache ) - length );
    }

    __li_member( StringTpl<blockSize> ) dropRightPart( size_t length ) const
    {
        intptr_t cache = -1;

        if ( length >= getNumCharsCached( cache ) )
            return StringTpl();
        else if ( length == 0 )
            return this;
        else
            return leftPart( getNumCharsCached( cache ) - length );
    }

    __li_member( bool ) endsWith( UnicodeChar c )
    {
        intptr_t cache = -1;
        size_t index = 0;

        if ( getNumCharsCached( cache ) < 1 )
            return ( c == Unicode::invalidChar );

        for ( size_t i = 0; i < getNumCharsCached( cache ) - 1; i++ )
            if ( getChar( index ) == Unicode::invalidChar )
                return ( c == Unicode::invalidChar );

        return getChar( index ) == c;
    }

    __li_member( bool ) endsWith( const StringTpl& other ) const
    {
        intptr_t cache = -1, otherLength = -1;
        ptrdiff_t i;
        size_t index = 0, otherIndex = 0;

        if ( other.getNumCharsCached( otherLength ) < 1 )
            return true;

        if ( getNumCharsCached( cache ) < 1 )
            return false;

        for ( i = 0; i < cache - otherLength; i++ )
            if ( getChar( index ) == Unicode::invalidChar )
                return false;

        for ( ; i < cache; i++ )
            if ( getChar( index ) != other.getChar( otherIndex ) )
                return false;

        return true;
    }

    __li_member( intptr_t ) findChar( Unicode::Char c, size_t beginAt ) const
    {
        size_t index = 0;

        for ( size_t i = 0; i < beginAt; i++ )
            if ( getChar( index ) == Unicode::invalidChar )
                return -1;

        for ( size_t charIndex = beginAt; index < getNumBytes(); charIndex++ )
            if ( getChar( index ) == c )
                return charIndex;

        return -1;
    }

    __li_member( intptr_t ) findDifferentChar( Unicode::Char c, size_t beginAt ) const
    {
        size_t index = 0;

        for ( size_t i = 0; i < beginAt; i++ )
            if ( getChar( index ) == Unicode::invalidChar )
                return -1;

        for ( size_t charIndex = beginAt; index < getNumBytes(); charIndex++ )
            if ( getChar( index ) != c )
                return charIndex;

        return -1;
    }

    __li_member( intptr_t ) findLastChar( Unicode::Char c, size_t beginAt ) const
    {
        intptr_t lastFound = -1, cache = -1;
        size_t index = beginAt;

        while ( index < getNumCharsCached( cache ) )
        {
            intptr_t newIndex = findChar( c, index );

            if ( newIndex < 0 )
                break;
            else
            {
                lastFound = newIndex;
                index = newIndex + 1;
            }
        }

        return lastFound;
    }

    __li_member( intptr_t ) findLastSubString( const StringTpl& pattern, size_t beginAt )
    {
        intptr_t lastFound = -1;
        size_t index = beginAt;

        while ( index < getNumChars() )
        {
            int newIndex = findSubString( pattern, index );

            if ( newIndex < 0 )
                break;
            else
            {
                lastFound = newIndex;
                index = newIndex + 1;
            }
        }

        return lastFound;
    }

    __li_member( intptr_t ) findSubString( const StringTpl& pattern, size_t beginAt ) const
    {
        size_t index = 0;

        for ( size_t i = 0; i < beginAt; i++ )
            if ( getChar( index ) == Unicode::invalidChar )
                return -1;

        for ( size_t charIndex = beginAt; index < getNumBytes(); charIndex++ )
        {
            size_t otherSubIndex = 0;

            const Unicode::Char thisChar = getChar( index ),
                    otherChar = pattern.getChar( otherSubIndex );

            if ( thisChar == otherChar )
            {
                size_t subIndex = index;

                while ( subIndex <= getNumBytes() )
                {
                    const Unicode::Char thisSubChar = getChar( subIndex ),
                            otherSubChar = pattern.getChar( otherSubIndex );

                    if ( otherSubChar == Unicode::invalidChar )
                        return charIndex;
                    else if ( thisSubChar != otherSubChar )
                        break;
                }
            }
        }

        return -1;
    }

    __li_member( StringTpl<blockSize> ) formatBool( bool value, bool useText )
    {
        if ( useText )
            return value ? "true" : "false";
        else
            return value ? "1" : "0";
    }

    __li_member( StringTpl<blockSize> ) formatFloat( float value, int width )
    {
        StringTpl format = "%" + ( width >= 0 ? StringTpl( width ) : "" ) + "g";
        StringTpl numBuffer;

        numBuffer.setBuffer( std::max( width, 30 ) );
        snprintf( numBuffer.data, numBuffer.capacity, format, value );

        return numBuffer.c_str();
    }

    __li_member( StringTpl<blockSize> ) formatInt( int value, int width, Base base )
    {
        StringTpl format = "%" + ( width >= 0 ? StringTpl( width ) : "" ) + ( base == decimal ? "i" : "X" );
        StringTpl numBuffer;

        numBuffer.setBuffer( std::max( width, 30 ) );
        snprintf( numBuffer.data, numBuffer.capacity, format, value );

        return numBuffer.c_str();
    }

    __li_member( Unicode::Char ) getChar( size_t& index ) const
    {
        Unicode::Char result;

        if ( index >= getNumBytes() )
            return Unicode::invalidChar;

        size_t length = Utf8::decode( result, data + index, getNumBytes() - index );

        if ( length )
        {
            index += length;
            return result;
        }
        else
            return Unicode::invalidChar;
    }

    __li_member( size_t ) getHash( const char* string )
    {
        size_t hash = 0xE424AA76;

        if ( string == nullptr )
            return hash;

        while ( *string )
        {
            //printf( "String: before char %u = '%c' hash = %08X\n", i, data[i], hash );
            hash = ( ( hash << 13 ) | ( hash >> 19 ) ) ^ *string;
            string++;
        }

        return hash;
    }

    __li_member( StringTpl<blockSize> ) leftPart( size_t length ) const
    {
        if ( length == 0 )
            return StringTpl();

        size_t index = 0;
        StringTpl result;

        for ( size_t i = 0; i < length; i++ )
        {
            Unicode::Char next = getChar( index );

            if ( next == Unicode::invalidChar )
                return result;
            else
                result.append( UnicodeChar( next ) );
        }

        return result;
    }

    __li_member( unsigned ) parse( List<StringTpl>& tokens, Unicode::Char separator, Unicode::Char escape, bool strict ) const
    {
        StringTpl buffer;
        size_t index = 0;
        unsigned numTokens = 0;
        bool escaped = false;

        while ( index < getNumBytes() )
        {
            const Unicode::Char next = getChar( index );

            if ( next == Unicode::invalidChar )
                break;
            else if ( next == escape && !escaped )
                escaped = true;
            else if ( next == separator && !escaped )
            {
                if ( strict || !buffer.isEmpty() )
                {
                    tokens.add( buffer );
                    buffer.clear();
                    numTokens++;
                }
            }
            else
            {
                buffer.append( UnicodeChar( next ) );
                escaped = false;
            }
        }

        if ( !buffer.isEmpty() )
        {
            tokens.add( buffer );
            numTokens++;
        }

        return numTokens;
    }

    __li_member( StringTpl<blockSize> ) replaceAll( const StringTpl& pattern, const StringTpl& replaceWith ) const
    {
        size_t index = 0, patternLength = pattern.getNumCharsUncached();
        StringTpl result;

        while ( index < getNumBytes() )
        {
            int newIndex = findSubString( pattern, index );

            if ( newIndex < 0 )
                return result + dropLeftPart( index );

            result += subString( index, newIndex - index ) + replaceWith;
            index = newIndex + patternLength;
        }

        return result;
    }

    __li_member( StringTpl<blockSize> ) rightPart( size_t length ) const
    {
        intptr_t cache = -1;

        if ( length > getNumCharsCached( cache ) )
            length = getNumCharsCached( cache );
        else if ( length == getNumCharsCached( cache ) )
            return this;
        else if ( length == 0 )
            return StringTpl();

        size_t index = 0;

        for ( size_t i = 0; i < getNumCharsCached( cache ) - length; i++ )
            if ( getChar( index ) == Unicode::invalidChar )
                return StringTpl();

        StringTpl result;

        for ( size_t i = 0; i < length; i++ )
        {
            const Unicode::Char next = getChar( index );

            if ( next == Unicode::invalidChar )
                return result;
            else
                result.append( UnicodeChar( next ) );
        }

        return result;
    }

    __li_member( void ) set( const char* stringUtf8 )
    {
        if ( stringUtf8 && *stringUtf8 )
        {
            numChars = -1;
            numBytes = strlen( stringUtf8 );

            setBuffer( numBytes + 1 );
            strncpy( data, stringUtf8, numBytes + 1 );
        }
        else
            clear();
    }

    __li_member( void ) set( const char* stringUtf8, size_t numBytes )
    {
        if ( numBytes > 0 )
        {
            numChars = -1;
            this->numBytes = numBytes;

            setBuffer( numBytes + 1 );
            strncpy( data, stringUtf8, numBytes );
            data[numBytes] = 0;
        }
        else
            clear();
    }

    __li_member( void ) set( const StringTpl& other )
    {
        numChars = other.numChars;
        numBytes = other.numBytes;

        setBuffer( ( numBytes == 0 ) ? 0 : ( numBytes + 1 ) );

        if ( data && other.data )
            strncpy( data, other.data, numBytes + 1 );
    }

    __li_member( void ) setBuffer( size_t requestedCapacity )
    {
        // Round-up to a multiple of block size
        size_t newCapacity = ( requestedCapacity == 0 ) ? 0 : ( ( requestedCapacity & ~( blockSize - 1 ) ) + blockSize );

        if ( newCapacity != capacity )
        {
            capacity = newCapacity;

            char* newData = reinterpret_cast<char*>( realloc( data, capacity ) );

            if ( newData == nullptr )
                abort();

            data = newData;
        }
    }

    __li_member( void ) split( Unicode::Char separator, StringTpl& leftSide, StringTpl& rightSide, bool rightDefault )
    {
        intptr_t pos = findChar( separator );

        StringTpl outputLeft, outputRight;

        if ( pos < 0 )
        {
            if ( rightDefault )
                outputRight = *this;
            else
                outputLeft = *this;
        }
        else
        {
            outputLeft = left( pos );
            outputRight = dropLeft( pos + 1 );
        }

        leftSide = outputLeft;
        rightSide = outputRight;
    }

    __li_member( void ) split( const StringTpl& separator, StringTpl& leftSide, StringTpl& rightSide, bool rightDefault )
    {
        intptr_t pos = findSubString( separator );

        StringTpl outputLeft, outputRight;

        if ( pos < 0 )
        {
            if ( rightDefault )
                outputRight = *this;
            else
                outputLeft = *this;
        }
        else
        {
            outputLeft = left( pos );
            outputRight = dropLeft( pos + separator.getNumCharsUncached() );
        }

        leftSide = outputLeft;
        rightSide = outputRight;
    }

    __li_member( StringTpl<blockSize> ) subString( size_t begin, size_t length ) const
    {
        if ( length == 0 )
            return StringTpl();

        size_t index = 0;

        for ( size_t i = 0; i < begin; i++ )
            if ( getChar( index ) == Unicode::invalidChar )
                return StringTpl();

        StringTpl result;

        for ( size_t i = 0; i < length; i++ )
        {
            const Unicode::Char next = getChar( index );

            if ( next == Unicode::invalidChar )
                return result;
            else
                result.append( UnicodeChar( next ) );
        }

        return result;
    }

    __li_member( int ) toAscii( Array<char>& output )
    {
        if ( !data )
            return 0;

        size_t index = 0, chars = 0;

        for ( ; index < getNumBytes(); chars++ )
        {
            const Unicode::Char c = getChar( index );
            output[chars] = ( c < 0x80 ) ? c : '?';
        }

        output[chars] = 0;
        return chars;
    }

    __li_member( StringTpl<blockSize> ) getFiltered( int (* filter)( int c ) ) const
    {
        StringTpl result;

        size_t index = 0;

        while ( true )
        {
            const Unicode::Char next = getChar( index );

            if ( next == Unicode::invalidChar )
                return result;
            else
                result.append( UnicodeChar( filter( next ) ) );
        }

        return result;
    }

    __li_member( uint64_t ) parseUnsigned( const char* text, Base base )
    {
        if ( text == nullptr )
            return 0;

        if ( base == undefined )
            return strtoul( text, nullptr, 0 );
        else if ( base == decimal )
            return strtoul( text, nullptr, 10 );
        else
            return strtoul( text, nullptr, 16 );
    }

#undef __li_member
#undef __li_member_

    typedef StringTpl<> String;

    template <typename T> String operator + ( T left, const String& right )
    {
        return String( left ) + right;
    }

    /*template <typename T> String operator == ( T left, const String& right )
    {
        return right == left;
    }*/

    /*template <typename T> String operator != ( T left, const String& right )
    {
        return right != left;
    }*/

    class StringIter
    {
        const String& string;
        size_t index;
        Unicode::Char next;

        void readNext()
        {
            if ( index >= string.getNumBytes() )
            {
                next = 0;
                return;
            }

            size_t got = Utf8::decode( next, string.c_str() + index, string.getNumBytes() - index );

            if ( !got || next == Unicode::invalidChar )
                next = 0;

            index += got;
        }

        StringIter& operator = (const StringIter&);

        public:
            StringIter( const String& string ) : string( string ), index( 0 ), next( Unicode::invalidChar )
            {
                readNext();
            }

            operator Unicode::Char () const
            {
                return next;
            }

            StringIter& operator ++ ( int )
            {
                readNext();
                return *this;
            }
    };
}
