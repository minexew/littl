/*
    Copyright (c) 2008-2014 Xeatheran Minexew

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

#include <littl/Base.hpp>
#include <littl/String.hpp>

#include <ctime>
#include <memory>

#ifdef li_MSW
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifdef _3DS
#include <3ds.h>
#endif

namespace li
{
    typedef uint64_t FilePos;
    typedef uint64_t FileSize;

    inline void pauseThread(unsigned milliSeconds)
    {
#ifdef li_MSW
        Sleep(milliSeconds);
#elif defined(_3DS)
        svcSleepThread(milliSeconds * 1000000L);
#else
        usleep(milliSeconds * 1000);
#endif
    }

    inline unsigned relativeTime()
    {
#ifdef li_MSW
        return GetTickCount();
#else
        return clock() * 1000 / CLOCKS_PER_SEC;
#endif
    }

    struct Timeout
    {
        bool infinite;
        unsigned millis, endTime;

        Timeout() : infinite( true )
        {
        }

        Timeout( unsigned millis ) : infinite( false ), millis( millis )
        {
            reset();
        }

        int getRemaining() const
        {
            if ( infinite )
                return -1;
            else
                return millis > 0 ? int( endTime - relativeTime() ) : 0;
        }

        void reset()
        {
            if ( !infinite && millis > 0 )
                endTime = relativeTime() + millis;
        }

        bool timedOut() const
        {
            return !infinite && getRemaining() <= 0;
        }
    };

    class Stream
    {
        public:
            virtual ~Stream() {}

            virtual bool finite() = 0;
            virtual bool seekable() = 0;

            virtual void flush() = 0;
            virtual const char* getErrorDesc() = 0;

            virtual FilePos getPos() = 0;
            virtual bool setPos( FilePos pos ) = 0;
            virtual FileSize getSize() = 0;

            bool rewind()
            {
                return setPos( 0 );
            }

            bool seek( int64_t bytes )
            {
                return setPos( getPos() + bytes );
            }
    };

    class InputStream : virtual public Stream
    {
        public:
            virtual bool eof() = 0;
            virtual size_t read( void* out, size_t length ) = 0;

            template <typename T> bool readLE(T* value)
            {
                static_assert(sizeof(T) == 0, "Not implemented for this data type");
                return false;
            }

            template <typename T> bool readByte(T* value_out)
            {
                static_assert(sizeof(T) == 1, "Type is not 1 byte in size");
                return read(value_out, 1) == 1;
            }

            unsigned readCharUtf8( Unicode::Char& c )
            {
                uint8_t next;

                if ( !read( &next, 1 ) )
                    return 0;

                unsigned numBytes = Utf8::beginDecode( c, next );

                if ( numBytes <= 0 )
                    return numBytes;

                for ( unsigned i = 1; i < numBytes; i++ )
                {
                    if ( !read( &next, 1 ) || ( next & 0xC0 ) != 0x80 )
                        return 0;

                    c = ( c << 6 ) | ( next & 0x3F );
                }

                return numBytes;
            }

            Unicode::Char readCharUtf8()
            {
                Unicode::Char c;

                if ( readCharUtf8( c ) )
                    return c;
                else
                    return Unicode::invalidChar;
            }

            String readLine()
            {
                char next;
                String line;

                while ( read( &next, 1 ) )
                    switch ( next )
                    {
                        case '\n': return line;
                        case '\r': break;
                        default: line.append( next );
                    }

                return line;
            }

            size_t readLines( List<String>& list )
            {
                size_t count = 0;

                for ( String line; !( line = readLine() ).isEmpty(); count++ )
                    list.add( std::move( line ) );

                list.add( String() );
                return count + 1;
            }

            String readString()
            {
                String value;
                UnicodeChar c;

                while ( readCharUtf8( c ) && c != 0 && c != Unicode::invalidChar )
                    value += c;

                return value;
            }

            String readWhole()
            {
                size_t streamSize = ( size_t ) getSize();

                char* buffer = new char [streamSize + 1];
                read( buffer, streamSize );
                buffer[streamSize] = 0;

                String whole = buffer;
                delete[] buffer;

                return whole;
            }
    };

    class OutputStream : virtual public Stream
    {
        public:
            virtual size_t write( const void* in, size_t length ) = 0;

            template <size_t bufferSize = 4096>
            size_t copyFrom( InputStream* input )
            {
                uint8_t buffer[bufferSize];
                size_t total = 0;

                for ( ; ; )
                {
                    size_t have = input->read( buffer, sizeof( buffer ) );

                    if ( have == 0 )
                        break;

                    write( buffer, have );
                    total += have;
                }

                return total;
            }

            bool write( const char* text )
            {
                if ( text )
                {
                    size_t length = strlen( text );

                    return write( text, length ) == length;
                }

                return false;
            }

            bool write( char c )
            {
                return write( &c, 1 ) == 1;
            }

            template <typename T> bool writeLE(const T value)
            {
                static_assert(sizeof(T) == 0, "Not implemented for this data type");
                return false;
            }

    	    bool writeLine( const String& data = String() )
            {
                return write( data + li_newLine );
            }

            void writeString( const char* str )
            {
                if ( str )
                    write( str, strlen( str ) );

                write( char( 0 ) );
            }
    };

    class IOStream: virtual public InputStream, virtual public OutputStream
    {
    };

    class ArrayIOStream: public Array<uint8_t>, public IOStream
    {
        protected:
            size_t index, size;

        private:
            ArrayIOStream(const ArrayIOStream&);

        public:
            using InputStream::read;
            using OutputStream::write;

            ArrayIOStream() : index( 0 ), size( 0 )
            {
            }

            virtual ~ArrayIOStream()
            {
            }

            ArrayIOStream( size_t capacity )
                    : Array<uint8_t>( capacity ), index( 0 ), size( 0 )
            {
            }

            ArrayIOStream( const String& data ) : index( 0 ), size( data.getNumBytes() )
            {
                load( ( uint8_t* ) data.c_str(), data.getNumBytes() );
            }

            ArrayIOStream( uint8_t* data, size_t length )
                    : Array<uint8_t>( length ), index( 0 ), size( length )
            {
                load( data, length );
            }

            virtual bool finite() override { return true; }
            virtual bool seekable() override { return true; }

            virtual void flush() override {}
            virtual const char* getErrorDesc() override { return nullptr; }

            virtual FilePos getPos() override { return index; }
            virtual bool setPos( FilePos pos ) override { index = ( size_t ) pos; return true; }
            virtual FileSize getSize() override { return size; }
            FileSize getSize() const { return size; }

            virtual bool eof() override { return index >= size; }

            void clear( bool lazy = false )
            {
                Array<uint8_t>::resize( 0, lazy );

                index = 0;
                size = 0;
            }

            void dump() const
            {
                for ( size_t i = 0; i < size; i += 16 )
                {
                    unsigned j;

                    printf( "[%p]", reinterpret_cast<void*>( i ) );

                    for ( j = 0; j < 16 && i + j < size; j++ )
                        printf( " %02X", getUnsafe( i + j ) );

                    for ( ; j < 16; j++ )
                        printf( "   " );

                    printf( " | " );

                    for ( j = 0; j < 16 && i + j < size; j++ )
                        printf( "%c", getUnsafe( i + j ) );

                    printf( "\n" );
                }
            }

            template <typename T> void fastWriteItem( const T item )
            {
                size += sizeof( T );
                resize( size, true );

                *reinterpret_cast<T*>( getPtrUnsafe( index ) ) = item;
                index += sizeof( T );
            }

            template <typename T> void fastWriteItemUnsafe( const T item )
            {
                size += sizeof( T );

                *reinterpret_cast<T*>( getPtrUnsafe( index ) ) = item;
                index += sizeof( T );
            }

            void growBuffer( size_t amount )
            {
                resize( size + amount, true );
            }

            virtual size_t read( void* out, size_t length ) override
            {
                if ( index >= size )
                    return 0;

                if ( index + length > size )
                    length = size - index;

                memcpy( out, getPtrUnsafe( index ), length );
                index += length;
                return length;
            }

            template <typename T> T* readObject()
            {
                if ( index + sizeof( T ) > size )
                    return 0;

                T* obj = reinterpret_cast<T*>( getPtrUnsafe( index ) );
                index += sizeof( T );
                return obj;
            }

            void setSize( size_t size )
            {
                this->size = size;
            }

            virtual size_t write( const void* input, size_t length ) override
            {
                if ( index + length > size )
                {
                    size = index + length;
                    resize( size, true );
                }

                memcpy( getPtrUnsafe( index ), input, length );
                index += length;
                return length;
            }

            virtual void* writeEmpty( size_t length )
            {
                if ( index + length > size )
                {
                    size = index + length;
                    resize( size, true );
                }

                void* ret = getPtrUnsafe( index );
                index += length;
                return ret;
            }

            template <typename T> T* writeItemsEmpty( size_t count )
            {
                return reinterpret_cast<T*>( writeEmpty( count * sizeof( T ) ) );
            }
    };

    class InputStreamSegment : public InputStream
    {
        std::shared_ptr<InputStream> stream;
        FilePos pos, segmentOffset;
        FileSize segmentLength;

        private:
            InputStreamSegment(const InputStreamSegment&);

        public:
            InputStreamSegment( std::shared_ptr<InputStream> stream, FilePos offset, FileSize length )
                    : stream( stream ), pos( 0 ), segmentOffset( offset ), segmentLength( length )
            {
                stream->setPos( segmentOffset );
            }

            virtual bool eof() override
            {
                return pos >= segmentLength;
            }

            virtual FilePos getPos() override
            {
                return pos;
            }

            virtual FileSize getSize() override
            {
                return segmentLength;
            }

            virtual bool setPos( FilePos pos ) override
            {
                if ( pos > segmentLength )
                    pos = segmentLength;

                this->pos = pos;
                return stream->setPos( segmentOffset + pos );
            }

            virtual size_t read( void* out, size_t length ) override
            {
                if ( pos >= segmentLength )
                    return 0;

                if ( pos + length >= segmentLength )
                    length = (size_t)(segmentLength - pos);

                pos += length;
                return stream->read( out, length );
            }
    };

#ifdef li_little_endian
    template<> inline bool InputStream::readLE<uint8_t>(uint8_t* value)
    {
        return read(value, sizeof(*value)) == sizeof(*value);
    }
    
    template<> inline bool InputStream::readLE<uint16_t>(uint16_t* value)
    {
        return read(value, sizeof(*value)) == sizeof(*value);
    }

    template<> inline bool InputStream::readLE<uint32_t>(uint32_t* value)
    {
        return read(value, sizeof(*value)) == sizeof(*value);
    }

    template<> inline bool InputStream::readLE<uint64_t>(uint64_t* value)
    {
        return read(value, sizeof(*value)) == sizeof(*value);
    }

    template<> inline bool InputStream::readLE<int8_t>(int8_t* value)
    {
        return read(value, sizeof(*value)) == sizeof(*value);
    }

    template<> inline bool InputStream::readLE<int16_t>(int16_t* value)
    {
        return read(value, sizeof(*value)) == sizeof(*value);
    }

    template<> inline bool InputStream::readLE<int32_t>(int32_t* value)
    {
        return read(value, sizeof(*value)) == sizeof(*value);
    }

    template<> inline bool InputStream::readLE<int64_t>(int64_t* value)
    {
        return read(value, sizeof(*value)) == sizeof(*value);
    }

    template<> inline bool OutputStream::writeLE<uint8_t>(uint8_t value)
    {
        return write(&value, sizeof(value)) == sizeof(value);
    }

    template<> inline bool OutputStream::writeLE<uint16_t>(uint16_t value)
    {
        return write(&value, sizeof(value)) == sizeof(value);
    }

    template<> inline bool OutputStream::writeLE<uint32_t>(uint32_t value)
    {
        return write(&value, sizeof(value)) == sizeof(value);
    }

    template<> inline bool OutputStream::writeLE<uint64_t>(uint64_t value)
    {
        return write(&value, sizeof(value)) == sizeof(value);
    }

    template<> inline bool OutputStream::writeLE<int8_t>(int8_t value)
    {
        return write(&value, sizeof(value)) == sizeof(value);
    }

    template<> inline bool OutputStream::writeLE<int16_t>(int16_t value)
    {
        return write(&value, sizeof(value)) == sizeof(value);
    }

    template<> inline bool OutputStream::writeLE<int32_t>(int32_t value)
    {
        return write(&value, sizeof(value)) == sizeof(value);
    }

    template<> inline bool OutputStream::writeLE<int64_t>(int64_t value)
    {
        return write(&value, sizeof(value)) == sizeof(value);
    }
#endif
}
