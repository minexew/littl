/*
    Copyright (c) 2008-2013 Xeatheran Minexew

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

namespace li
{
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

    template <typename T> class IListable
    {
        public:
            virtual void listBegin() = 0;
            virtual T listNext() = 0;
    };

    class ISeekable
    {
        public:
            virtual uint64_t getPos() = 0;
            virtual uint64_t getSize() = 0;
            virtual bool setPos( uint64_t pos ) = 0;

            bool rewind()
            {
                return setPos( 0 );
            }

            bool seek( int64_t bytes )
            {
                return setPos( getPos() + bytes );
            }
    };

    class InputStream: virtual public ReferencedClass
    {
        public:
            li_ReferencedClass_override( InputStream )

            virtual bool isReadable() = 0;
            virtual size_t rawRead( void* out, size_t length ) = 0;

            size_t read( void* out, size_t length )
            {
                return rawRead( out, length );
            }

            template <typename T> T read()
            {
                T temp = 0;
                rawRead( &temp, sizeof( T ) );
                return temp;
            }

            template <typename T> bool readLE(T* value)
            {
                static_assert(sizeof(T) == 0, "Not implemented for this data type");
                return false;
            }

            template <typename T> size_t readItems( T* out, size_t count )
            {
                return rawRead( out, count * sizeof( T ) ) / sizeof( T );
            }

            template <typename T> T readUnsafe()
            {
                T temp;
                read( &temp, sizeof( T ) );
                return temp;
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

            String readLine( bool useMacEnding = false )
            {
                if ( !isReadable() )
                    return String();

                char next;
                String line;

                while ( read( &next, 1 ) )
                    switch ( next )
                    {
                        case '\n': return line;
                        case '\r': if ( useMacEnding ) return line; break;
                        default: line.append( next );
                    }

                return line;
            }

            String readString()
            {
                String value;
                Unicode::Character c;

                while ( readCharUtf8( c ) && c != 0 && c != Unicode::invalidChar )
                    value += c;

                return value;
            }
    };

    class SeekableInputStream: virtual public InputStream, virtual public ISeekable
    {
        public:
            li_ReferencedClass_override( SeekableInputStream )

            String readLine()
            {
                if ( !isReadable() )
                    return String();

                char next = 0;
                String line;

                while ( read( &next, 1 ) )
                {
                    switch ( next )
                    {
                        case '\n':
                            return line;

                        case '\r':
                            if ( read( &next, 1 ) && next != '\n' )
                                seek( -1 );

                            return line;

                        default:
                            line.append( next );
                    }
                }

                return line;
            }

            int readLines( List<String>& list )
            {
                if ( !isReadable() )
                    return -1;

                int count = 0;
                for ( String line; !( line = readLine() ).isEmpty(); count++ )
                    list.add( line );

                list.add( String() );
                return count + 1;
            }

            String readWhole()
            {
                if ( !isReadable() )
                   return String();

                size_t streamSize = ( size_t ) getSize();

                char* buffer = new char [streamSize + 1];
                read( buffer, streamSize );
                buffer[streamSize] = 0;

                String whole = buffer;
                delete[] buffer;

                return whole;
            }
    };

    class OutputStream: virtual public ReferencedClass
    {
        public:
            li_ReferencedClass_override( OutputStream )

            virtual bool isWritable() = 0;
            //virtual size_t write( const void* input, size_t length ) = 0;
            virtual size_t rawWrite( const void* in, size_t length ) = 0;

            size_t copyFrom( InputStream* input )
            {
                uint8_t buffer[0x1000];
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

            bool write( char* text )
            {
                return write( ( const char* )text );
            }

            size_t write( const void* data, size_t length )
            {
                return rawWrite( data, length );
            }

            template <typename T> size_t write( const T& what )
            {
                return write( &what, sizeof( what ) );
            }

            template <typename T> bool writeLE(const T value)
            {
                static_assert(sizeof(T) == 0, "Not implemented for this data type");
                return false;
            }

    	    bool write( const String& text )
            {
                return write( text.c_str(), text.getNumBytes() ) > 0;
            }

            template <typename T> size_t writeItems( const T* data, size_t count )
            {
                return rawWrite( data, count * sizeof( T ) ) / sizeof( T );
            }

    	    bool writeLine( const String& data = String() )
            {
                return write( data + li_newLine );
            }

    	    /*bool writeError( const String& data )
            {
                return write( ( String )"Error: " + data + __li_lineEnd );
            }*/

            void writeString( const char* str )
            {
                if ( str )
                    write( str, strlen( str ) );

                write<char>( 0 );
            }
    };

    class SeekableOutputStream: virtual public OutputStream, virtual public ISeekable
    {
        public:
            li_ReferencedClass_override( SeekableOutputStream )
    };

    class IOStream: virtual public InputStream, virtual public OutputStream
    {
        public:
            li_ReferencedClass_override( IOStream )
    };

    class SeekableIOStream: public IOStream, public SeekableInputStream, public SeekableOutputStream
    {
        public:
            li_ReferencedClass_override( SeekableIOStream )
    };

    class ArrayIOStream: public Array<uint8_t>, public SeekableIOStream
    {
        protected:
            size_t index, size;

        private:
            ArrayIOStream(const ArrayIOStream&);

        public:
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

            virtual uint64_t getPos()
            {
                return index;
            }

            virtual uint64_t getSize()
            {
                return size;
            }

            virtual uint64_t getSize() const
            {
                return size;
            }

            void growBuffer( size_t amount )
            {
                resize( size + amount, true );
            }

            virtual bool isReadable()
            {
                return index < size;
            }

            virtual bool isWritable()
            {
                return true;
            }

            virtual size_t rawRead( void* out, size_t length )
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

            /*virtual size_t rawWrite( const void* in, size_t length )
            {
                return write( in, length );
            }*/

            virtual bool setPos( uint64_t pos )
            {
                index = ( size_t ) pos;
                return true;
            }

            void setSize( size_t size )
            {
                this->size = size;
            }

            virtual size_t rawWrite( const void* input, size_t length )
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
    };

    class SeekableInputStreamSegment : public SeekableInputStream
    {
        Reference<SeekableInputStream> stream;
        uint64_t pos, segmentOffset, segmentLength;

        private:
            SeekableInputStreamSegment(const SeekableInputStreamSegment&);

        public:
            SeekableInputStreamSegment( SeekableInputStream* stream, uint64_t offset, uint64_t length )
                    : stream( stream ), pos( 0 ), segmentOffset( offset ), segmentLength( length )
            {
                stream->setPos( segmentOffset );
            }

            virtual uint64_t getPos() override
            {
                return pos;
            }

            virtual uint64_t getSize() override
            {
                return segmentLength;
            }

            virtual bool setPos( uint64_t pos ) override
            {
                if ( pos > segmentLength )
                    pos = segmentLength;

                this->pos = pos;
                return stream->setPos( segmentOffset + pos );
            }

            virtual bool isReadable() override
            {
                return pos < segmentLength;
            }

            virtual size_t rawRead( void* out, size_t length ) override
            {
                if ( pos >= segmentLength )
                    return 0;

                if ( pos + length >= segmentLength )
                    length = (size_t)(segmentLength - pos);

                pos += length;
                return stream->rawRead( out, length );
            }
    };

    inline bool isReadable( InputStream* input )
    {
        return input && input->isReadable();
    }

    inline bool isWritable( OutputStream* output )
    {
        return output && output->isWritable();
    }

#ifdef li_little_endian
    template<> inline bool InputStream::readLE<uint8_t>(uint8_t* value)
    {
        return readItems(value, 1) == 1;
    }
    
    template<> inline bool InputStream::readLE<uint16_t>(uint16_t* value)
    {
        return readItems(value, 1) == 1;
    }

    template<> inline bool InputStream::readLE<uint32_t>(uint32_t* value)
    {
        return readItems(value, 1) == 1;
    }

    template<> inline bool InputStream::readLE<uint64_t>(uint64_t* value)
    {
        return readItems(value, 1) == 1;
    }

    template<> inline bool InputStream::readLE<int8_t>(int8_t* value)
    {
        return readItems(value, 1) == 1;
    }

    template<> inline bool InputStream::readLE<int16_t>(int16_t* value)
    {
        return readItems(value, 1) == 1;
    }

    template<> inline bool InputStream::readLE<int32_t>(int32_t* value)
    {
        return readItems(value, 1) == 1;
    }

    template<> inline bool InputStream::readLE<int64_t>(int64_t* value)
    {
        return readItems(value, 1) == 1;
    }

    template<> inline bool OutputStream::writeLE<uint8_t>(uint8_t value)
    {
        return writeItems(&value, 1) == 1;
    }

    template<> inline bool OutputStream::writeLE<uint16_t>(uint16_t value)
    {
        return writeItems(&value, 1) == 1;
    }

    template<> inline bool OutputStream::writeLE<uint32_t>(uint32_t value)
    {
        return writeItems(&value, 1) == 1;
    }

    template<> inline bool OutputStream::writeLE<uint64_t>(uint64_t value)
    {
        return writeItems(&value, 1) == 1;
    }

    template<> inline bool OutputStream::writeLE<int8_t>(int8_t value)
    {
        return writeItems(&value, 1) == 1;
    }

    template<> inline bool OutputStream::writeLE<int16_t>(int16_t value)
    {
        return writeItems(&value, 1) == 1;
    }

    template<> inline bool OutputStream::writeLE<int32_t>(int32_t value)
    {
        return writeItems(&value, 1) == 1;
    }

    template<> inline bool OutputStream::writeLE<int64_t>(int64_t value)
    {
        return writeItems(&value, 1) == 1;
    }
#endif
}
