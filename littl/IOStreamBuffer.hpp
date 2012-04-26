
namespace li
{
    class SeekableStreamBuffer : public ISeekable
    {
        protected:
            size_t pos;

        public:
            SeekableStreamBuffer() : pos( 0 )
            {
            }

            virtual uint64_t getPos()
            {
                return pos;
            }

            virtual bool setPos( uint64_t newPos )
            {
                pos = ( size_t ) newPos;
                return true;
            }

            virtual uint64_t size()
            {
                return 0;
            }
    };

    class InputStreamBuffer : public InputStream, virtual public SeekableStreamBuffer
    {
        InputStream* stream;
        Array<unsigned char> buffer;
        size_t numBuffered;

        public:
            InputStreamBuffer( InputStream* stream )
                    : stream( stream ), numBuffered( 0 )
            {
            }

            virtual ~InputStreamBuffer()
            {
                stream->release();
            }

            virtual bool isReadable()
            {
                return pos < numBuffered || stream->isReadable();
            }

            virtual size_t rawRead( void* out, size_t length )
            {
                if ( pos + length > numBuffered )
                {
                    size_t numToBuffer = pos + length - numBuffered;

                    buffer.resize( pos + length, true );
                    numBuffered += stream->read( buffer.getPtr( numBuffered ), numToBuffer );
                }

                size_t numToCopy = minimum( length, numBuffered - pos );

                memcpy( out, buffer.getPtr( pos ), numToCopy );
                pos += numToCopy;
                return numToCopy;
            }
    };

    class OutputStreamBuffer : public OutputStream, virtual public SeekableStreamBuffer
    {
        OutputStream* stream;
        Array<unsigned char> buffer;
        size_t numBuffered;

        public:
            OutputStreamBuffer( OutputStream* stream )
                    : stream( stream ), numBuffered( 0 )
            {
            }

            virtual ~OutputStreamBuffer()
            {
                stream->write( buffer.getPtr(), numBuffered );
                stream->release();
            }

            virtual bool isWritable()
            {
                return true;
            }

            virtual size_t rawWrite( const void* in, size_t length )
            {
                buffer.resize( pos + length, true );

                memcpy( buffer.getPtr( pos ), in, length );
                numBuffered = maximum( numBuffered, pos + length );

                pos += length;
                return length;
            }
    };

    // For some reason, it can't be done the simple way. I tried.

    class IOStreamBuffer: public SeekableIOStream, public InputStreamBuffer, public OutputStreamBuffer
    {
        public:
            IOStreamBuffer( IOStream* stream ) : InputStreamBuffer( stream->reference() ), OutputStreamBuffer( stream )
            {
            }

            virtual uint64_t getPos()
            {
                return SeekableStreamBuffer::getPos();
            }

            virtual bool isReadable()
            {
                return InputStreamBuffer::isReadable();
            }

            virtual bool isWritable()
            {
                return OutputStreamBuffer::isWritable();
            }

            virtual size_t rawRead( void* out, size_t length )
            {
                return InputStreamBuffer::rawRead( out, length );
            }

            virtual size_t rawWrite( const void* in, size_t length )
            {
                return OutputStreamBuffer::rawWrite( in, length );
            }

            virtual bool setPos( uint64_t newPos )
            {
                return SeekableStreamBuffer::setPos( newPos );
            }

            virtual uint64_t size()
            {
                return SeekableStreamBuffer::size();
            }
    };

}
