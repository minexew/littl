/*
    Copyright (C) 2011 Xeatheran Minexew

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

#include <littl/BaseIO.hpp>

#include <zlib.h>

namespace li
{
    class ZlibCompressor : public OutputStream
    {
        protected:
            Reference<OutputStream> output;
            Array<uint8_t> buffer;

            int compression;
            bool haveStream;
            z_stream stream;

        public:
            ZlibCompressor( OutputStream* output, int compression = 5 ) : output( output ), buffer( 0x1000 ), compression( compression ), haveStream( false )
            {
            }

            virtual ~ZlibCompressor()
            {
                reset();
            }

            virtual void reset()
            {
                if ( !haveStream )
                    return;

                int status = 0;

                do
                {
                    stream.next_out = buffer.getPtr();
                    stream.avail_out = buffer.getCapacity();
//printf( " -- finalize: status is %i, writing into %u...", status, stream.avail_out );
                    status = deflate( &stream, Z_FINISH );
//printf( "%u used\n", buffer.getCapacity() - stream.avail_out );
                    output->write( buffer.getPtr(), buffer.getCapacity() - stream.avail_out );
                }
                while ( status == Z_OK );

                if ( status != Z_STREAM_END )
                    printf( "ZlibCompressor.~ZlibCompressor error: status %i '%s'\n", status, zError( status ) );

                deflateEnd( &stream );

                haveStream = false;
            }

            virtual bool isWritable()
            {
                return output->isWritable();
            }

            virtual size_t rawWrite( const void* data, size_t count ) override
            {
                if ( !haveStream )
                {
                    stream.zalloc = Z_NULL;
                    stream.zfree = Z_NULL;

                    deflateInit( &stream, compression );

                    haveStream = true;
                }

                size_t have = 0;

                stream.next_in = ( Bytef* ) data;
                stream.avail_in = count;

                int status = 0;

                do
                {
                    stream.next_out = buffer.getPtr();
                    stream.avail_out = buffer.getCapacity();
//printf( " -- status is %i, %u more bytes to compress into %u...", status, stream.avail_in, stream.avail_out );
                    status = deflate( &stream, Z_NO_FLUSH );
//printf( "%u used\n", buffer.getCapacity() - stream.avail_out );
                    have += output->write( buffer.getPtr(), buffer.getCapacity() - stream.avail_out );
                }
                while ( status == Z_OK && stream.avail_in > 0 );

                if ( status != Z_OK )
                    printf( "ZlibCompressor.rawWrite error: status %i '%s'\n", status, zError( status ) );

                return have;
            }

            void setCompression( int compression )
            {
                this->compression = compression;
            }
    };
}
