#pragma once

#include <littl/BaseIO.hpp>

#include <zlib.h>

namespace li
{
    class GzFileStream: public IOStream
    {
        li_ReferencedClass_override( GzFileStream )

        private:
            gzFile file;

        protected:
            GzFileStream( gzFile file ) : file( file )
    		{
            }

            virtual ~GzFileStream()
    		{
                if ( file != nullptr )
                    gzclose( file );
            }

    	public:
            static GzFileStream* open( const char* fileName, const char* mode = "rb" )
            {
                gzFile file = gzopen( fileName, mode );

                if ( file == nullptr )
                    return nullptr;

                return new GzFileStream( file );
            }

            virtual bool isReadable()
            {
                return this && file && !gzeof( file );
            }

            virtual bool isWritable()
            {
                return this && file;
            }

            virtual size_t rawRead( void* out, size_t readSize )
            {
                if ( isReadable() && readSize > 0 )
                {
                    int numRead = gzread( file, out, readSize );
                    return numRead > 0 ? numRead : 0;
                }
                else
                    return 0;
            }

            virtual size_t rawWrite( const void* in, size_t writeSize )
            {
                return write( in, writeSize );
            }

            virtual size_t write( const void* input, size_t length )
            {
                if ( isWritable() && length > 0 )
                {
                    int numWritten = gzwrite( file, input, length );
                    return numWritten > 0 ? numWritten : 0;
                }
                else
                    return 0;
            }
    };
}
