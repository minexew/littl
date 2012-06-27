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
#include <littl/Utf8.hpp>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#if !defined( __GNUC__ )
#define strtof( a_, b_ ) ( ( float ) strtod( a_, b_ ) )
#define PRIuPTR "Iu"
#define PRIu64 "I64u"
#endif

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

            StringTpl( Unicode::Character c ) : default_init_seq
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
            void append( Utf8Char c );
            void append( const Utf8Character& c );
            void append( const char* stringUtf8 );
            void append( const StringTpl& other );
            bool beginsWith( Utf8Char c ) const;
            bool beginsWith( const StringTpl& other ) const;
            Utf8Char charAt( size_t offset ) const;

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
                if ( !s1 )
                    return s2 ? 1 : 0;

                if ( !s2 )
                    return -1;

                if ( caseSensitive )
                    return strcmp( s1, s2 );
                else
                    return stringCaseCompare( s1, s2 );
            }

            const char* c_str() const
            {
                return data;
            }

            void debug() const
            {
                printf( "'%s' [%i, %i, %i]\n", data, numChars, numBytes, capacity );
            }

            StringTpl<blockSize> dropLeftPart( size_t length ) const;
            StringTpl<blockSize> dropLeft( size_t length ) const { return dropLeftPart( length ); }
            StringTpl<blockSize> dropRightPart( size_t length ) const;
            StringTpl<blockSize> dropRight( size_t length ) const { return dropRightPart( length ); }
            bool endsWith( Unicode::Character c );
            bool endsWith( const StringTpl& other ) const;

            bool equals( const char* other, bool caseSensitive = true ) const
            {
                return compare( c_str(), other, caseSensitive ) == 0;
            }

            static bool equals( const char* str, const char* other, bool caseSensitive = true )
            {
                return compare( str, other, caseSensitive ) == 0;
            }

            int findChar( Utf8Char c, size_t beginAt = 0 ) const;
            int findDifferentChar( Utf8Char c, size_t beginAt = 0 ) const;
            int findLastChar( Utf8Char c, size_t beginAt = 0 ) const;
            int findLastSubString( const StringTpl& pattern, size_t beginAt = 0 );
            int findSubString( const StringTpl& pattern, size_t beginAt = 0 ) const;
            static StringTpl<blockSize> formatBool( bool value, bool useText = false );
            static StringTpl<blockSize> formatFloat( float value, int width = -1 );
            static StringTpl<blockSize> formatInt( int value, int width = -1, Base base = decimal );
            char* getBuffer() { return data; }
            Utf8Char getChar( size_t& index ) const;
            StringTpl<blockSize> getFiltered( int (* filter)( int c ) ) const;

            static uint32_t getHash( const char* string );
            static uint32_t getHash( const StringTpl& string ) { return getHash( string.c_str() ); }
            uint32_t getHash() const { return getHash( *this ); }

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
            unsigned parse( List<StringTpl>& tokens, Utf8Char separator, Utf8Char escape = Utf8::invalidChar, bool strict = false ) const;
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

            int toInt() const { return toInt( data ); }
            static int toInt( const char* text ) { return text != nullptr ? strtol( text, nullptr, 0 ) : 0; }

            uint64_t toUnsigned( Base base = undefined ) const { return parseUnsigned( data, base ); }
            static uint64_t parseUnsigned( const char* text, Base base = undefined );

            operator bool () const
            {
                return !isEmpty();
            }

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
                snprintf( buffer, 20, fmt, ( type ) num );\
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
            operator_append( const Utf8Character& )
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

    __li_member( void ) append( Utf8Char c )
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

    __li_member( void ) append( const Utf8Character& c )
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
        strncpy( data + numBytes, stringUtf8, numBytesInc + 1 );

        numBytes += numBytesInc;
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

    __li_member( bool ) beginsWith( Utf8Char c ) const
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
            Utf8Char thisChar = getChar( index ),
                    otherChar = other.getChar( otherIndex );

            if ( otherChar == Utf8::invalidChar )
                return true;
            else if ( thisChar != otherChar )
                return false;
        }

        return false;
    }

    __li_member( Utf8Char ) charAt( size_t offset ) const
    {
        size_t index = 0;

        for ( size_t i = 0; i < offset; i++ )
            if ( getChar( index ) == Utf8::invalidChar )
                return Utf8::invalidChar;

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

    __li_member( bool ) endsWith( Unicode::Character c )
    {
        intptr_t cache = -1;
        size_t index = 0;

        if ( getNumCharsCached( cache ) < 1 )
            return ( c == Utf8::invalidChar );

        for ( size_t i = 0; i < getNumCharsCached( cache ) - 1; i++ )
            if ( getChar( index ) == Utf8::invalidChar )
                return ( c == Utf8::invalidChar );

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
            if ( getChar( index ) == Utf8::invalidChar )
                return false;

        for ( ; i < cache; i++ )
            if ( getChar( index ) != other.getChar( otherIndex ) )
                return false;

        return true;
    }

    __li_member( int ) findChar( Utf8Char c, size_t beginAt ) const
    {
        size_t index = 0;

        for ( size_t i = 0; i < beginAt; i++ )
            if ( getChar( index ) == Utf8::invalidChar )
                return -1;

        for ( size_t charIndex = beginAt; index < getNumBytes(); charIndex++ )
            if ( getChar( index ) == c )
                return charIndex;

        return -1;
    }

    __li_member( int ) findDifferentChar( Utf8Char c, size_t beginAt ) const
    {
        size_t index = 0;

        for ( size_t i = 0; i < beginAt; i++ )
            if ( getChar( index ) == Utf8::invalidChar )
                return -1;

        for ( size_t charIndex = beginAt; index < getNumBytes(); charIndex++ )
            if ( getChar( index ) != c )
                return charIndex;

        return -1;
    }

    __li_member( int ) findLastChar( Utf8Char c, size_t beginAt ) const
    {
        intptr_t lastFound = -1, cache = -1;
        size_t index = beginAt;

        while ( index < getNumCharsCached( cache ) )
        {
            int newIndex = findChar( c, index );

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

    __li_member( int ) findLastSubString( const StringTpl& pattern, size_t beginAt )
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

    __li_member( int ) findSubString( const StringTpl& pattern, size_t beginAt ) const
    {
        size_t index = 0;

        for ( size_t i = 0; i < beginAt; i++ )
            if ( getChar( index ) == Utf8::invalidChar )
                return -1;

        for ( size_t charIndex = beginAt; index < getNumBytes(); charIndex++ )
        {
            size_t otherSubIndex = 0;

            Utf8Char thisChar = getChar( index ),
                    otherChar = pattern.getChar( otherSubIndex );

            if ( thisChar == otherChar )
            {
                size_t subIndex = index;

                while ( subIndex <= getNumBytes() )
                {
                    Utf8Char thisSubChar = getChar( subIndex ),
                            otherSubChar = pattern.getChar( otherSubIndex );

                    if ( otherSubChar == Utf8::invalidChar )
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
        StringTpl format = "%" + ( width >= 0 ? ( StringTpl ) width : "" ) + "g";
        StringTpl numBuffer;

        numBuffer.setBuffer( maximum( width, 30 ) );
        snprintf( numBuffer.data, numBuffer.capacity, format, value );

        return numBuffer.c_str();
    }

    __li_member( StringTpl<blockSize> ) formatInt( int value, int width, Base base )
    {
        StringTpl format = "%" + ( width >= 0 ? ( StringTpl ) width : "" ) + ( base == decimal ? "i" : "X" );
        StringTpl numBuffer;

        numBuffer.setBuffer( maximum( width, 30 ) );
        snprintf( numBuffer.data, numBuffer.capacity, format, value );

        return numBuffer.c_str();
    }

    __li_member( Utf8Char ) getChar( size_t& index ) const
    {
        Utf8Char result;

        if ( index >= getNumBytes() )
            return Utf8::invalidChar;

        unsigned length = Utf8::decode( result, data + index, getNumBytes() - index );

        if ( length )
        {
            index += length;
            return result;
        }
        else
            return Utf8::invalidChar;
    }

    __li_member( uint32_t ) getHash( const char* string )
    {
        uint32_t hash = 0xE424AA76;

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
            Utf8Char next = getChar( index );

            if ( next == Utf8::invalidChar )
                return result;
            else
                result.append( Utf8Character( next ) );
        }

        return result;
    }

    __li_member( unsigned ) parse( List<StringTpl>& tokens, Utf8Char separator, Utf8Char escape, bool strict ) const
    {
        StringTpl buffer;
        size_t index = 0;
        unsigned numTokens = 0;
        bool escaped = false;

        while ( index < getNumBytes() )
        {
            Utf8Char next = getChar( index );

            if ( next == Utf8::invalidChar )
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
                buffer.append( Utf8Character( next ) );
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
            if ( getChar( index ) == Utf8::invalidChar )
                return StringTpl();

        StringTpl result;

        for ( size_t i = 0; i < length; i++ )
        {
            Utf8Char next = getChar( index );

            if ( next == Utf8::invalidChar )
                return result;
            else
                result.append( Utf8Character( next ) );
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

            char* newData = ( char* ) realloc( data, capacity );

            // Low on memory? Well, it seems like a good idea to rather freeze for some time instead of crashing.
            if ( !newData && capacity > 0 )
            {
                printf( "[littl Warning]    OUT OF MEMORY!\n" );
                printf( "[littl Warning]    Application will wait to allocate %" PRIuPTR " bytes.\n", capacity );
            }

            while ( !newData && capacity > 0 )
            {
                newData = ( char* ) realloc( data, capacity );
                pauseThread( 5 );
            }

            data = newData;
        }
    }

    __li_member( void ) split( Unicode::Char separator, StringTpl& leftSide, StringTpl& rightSide, bool rightDefault )
    {
        int pos = findChar( separator );

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
        int pos = findSubString( separator );

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
            if ( getChar( index ) == Utf8::invalidChar )
                return StringTpl();

        StringTpl result;

        for ( size_t i = 0; i < length; i++ )
        {
            Utf8Char next = getChar( index );

            if ( next == Utf8::invalidChar )
                return result;
            else
                result.append( Utf8Character( next ) );
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
            Utf8Char c = getChar( index );
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
            Utf8Char next = getChar( index );

            if ( next == Utf8::invalidChar )
                return result;
            else
                result.append( Utf8Character( filter( next ) ) );
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
        return ( String ) left + right;
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
        unsigned index;
        Utf8Char next;

        void readNext()
        {
            if ( index >= string.getNumBytes() )
            {
                next = 0;
                return;
            }

            unsigned got = Utf8::decode( next, string.c_str() + index, string.getNumBytes() - index );

            if ( !got || next == Utf8::invalidChar )
                next = 0;

            index += got;
        }

        public:
            StringIter( const String& string ) : string( string ), index( 0 ), next( Utf8::invalidChar )
            {
                readNext();
            }

            operator Utf8Char () const
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
