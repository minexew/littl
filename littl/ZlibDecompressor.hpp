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
    class ZlibDecompressor : public InputStream
    {
        protected:
            Reference<InputStream> input;
            Array<uint8_t> buffer;

            z_stream stream;

            int64_t compressedSize;
            uint64_t compressedPos;

            void getMore()
            {
                size_t count = buffer.getCapacity();

                if ( compressedSize >= 0 && compressedPos + count > ( size_t ) compressedSize )
                    count = ( size_t )( compressedSize - compressedPos );

                size_t have = input->read( buffer.getPtr(), count );
                compressedPos += have;

                stream.avail_in = have;
                stream.next_in = ( Bytef* ) buffer.getPtr();
            }

        public:
            ZlibDecompressor( InputStream* input, int64_t compressedSize = -1 ) : input( input ), buffer( 0x1000 ), compressedSize( compressedSize ), compressedPos( 0 )
            {
                getMore();

                stream.zalloc = Z_NULL;
                stream.zfree = Z_NULL;

                inflateInit( &stream );
            }

            virtual bool isReadable()
            {
                return stream.avail_in > 0 || input->isReadable();
            }

            virtual size_t rawRead( void* data, size_t count )
            {
                stream.avail_out = count;
                stream.next_out = ( Bytef* ) data;

                while ( stream.avail_out > 0 )
                {
                    if ( stream.avail_in == 0 )
                    {
                        getMore();

                        if ( stream.avail_in == 0 )
                            break;
                    }

                    if ( inflate( &stream, Z_SYNC_FLUSH ) != Z_OK )
                        break;
                }

                return count - stream.avail_out;
            }
    };
}
