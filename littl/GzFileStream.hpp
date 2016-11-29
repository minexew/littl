#pragma once

#include <littl/Stream.hpp>

#include <zlib.h>

namespace li
{
    class GzFileStream: public IOStream
    {
        private:
            gzFile file;

        public:
            GzFileStream( const char* fileName, const char* mode = "rb" )
            {
                this->file = gzopen( fileName, mode );
            }

            virtual ~GzFileStream()
    		{
                if ( file != nullptr )
                    gzclose( file );
            }

            operator bool() const
            {
                return file != 0;
            }

            virtual bool finite() override { return true; }
            virtual bool seekable() override { return false; }

            virtual void flush() override
            {
                if ( file )
                    gzflush( file, Z_SYNC_FLUSH );
            }

            virtual const char* getErrorDesc() override
            {
                if ( !file )
                    return nullptr;

                int err;
                return gzerror( file, &err );
            }

            virtual uint64_t getPos() override
            {
                if ( file )
                    return gztell64( file );
                else
                    return 0;
            }

            virtual uint64_t getSize() override
            {
                // FIXME
                return 0;
            }

            virtual bool setPos( uint64_t pos ) override
            {
                return gzseek64( file, pos, SEEK_SET ) == pos;
            }

            // *** InputStream methods ***

            virtual bool eof() override
            {
                return !file || gzeof( file );
            }

            virtual size_t read( void* out, size_t readSize ) override
            {
                if ( file )
                {
                    int numRead = gzread( file, out, readSize );
                    return numRead > 0 ? numRead : 0;
                }
                else
                    return 0;
            }

            // *** OutputStream methods ***

            virtual size_t write( const void* in, size_t writeSize ) override
            {
                if ( file )
                {
                    int numWritten = gzwrite( file, in, writeSize );
                    return numWritten > 0 ? numWritten : 0;
                }
                else
                    return 0;
            }
    };
}
