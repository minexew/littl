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

#include <littl/FileName.hpp>
#include <littl/Stream.hpp>
#include <littl/String.hpp>

#ifndef __li_MSW
#include <sys/stat.h>
#endif

#include <cerrno>

// FIXME: fopen on Windows is NOT UTF-8!

namespace li
{
    struct FileStat
    {
        enum { is_file = 1, is_directory = 2 };

        int flags;
        uint64_t sizeInBytes;
        time_t creationTime, modificationTime;
    };

#ifdef __li_MSW
    inline time_t FileTimeToTime_t( const FILETIME& ft )
    {
        ULARGE_INTEGER ull;
        ull.LowPart = ft.dwLowDateTime;
        ull.HighPart = ft.dwHighDateTime;

        return ( time_t )( ull.QuadPart / 10000000ULL - 11644473600ULL );
    }
#endif

    class File: public IOStream
    {
        private:
            enum LastAccess { Access_none, Access_read, Access_write };

            FILE* handle;
            LastAccess lastAccess;

            File( const File& other ) = delete;

    	public:
            using InputStream::read;
            using OutputStream::write;

            File( const char* fileName, const char* mode )
            {
                handle = fopen( fileName, mode );
            }

            File( const char* fileName, bool forWriting = false )
            {
                handle = fopen( fileName, forWriting ? "wb" : "rb" );
            }

            File( File&& other )
                    : handle( other.handle ), lastAccess( other.lastAccess )
            {
                other.handle = nullptr;
            }

    		File( FILE* handle ) : handle( handle )
    		{
    		    lastAccess = Access_none;
            }

            virtual ~File()
    		{
                close();
            }

            void close()
            {
                if ( handle )
                {
                    fclose( handle );
                    handle = nullptr;
                }
            }

            static File* open( const char* fileName, bool forWriting = false )
            {
                File f( fileName, forWriting );

                if ( f )
                    return new File( std::move( f ) );
                else
                    return nullptr;
            }

            static File* open( const char* fileName, const char* mode )
            {
                File f( fileName, mode );

                if ( f )
                    return new File( std::move( f ) );
                else
                    return nullptr;
            }

            operator bool() const
            {
                return handle != 0;
            }

            FILE* release()
            {
                FILE* handle = this->handle;
                this->handle = nullptr;
                return handle;
            }

            // *** Stream methods ***

            virtual bool finite() override { return true; }
            virtual bool seekable() override { return true; }

            virtual void flush() override
            {
                fflush( handle );
            }

            virtual const char* getErrorDesc() override
            {
                return ferror( handle ) ? strerror( errno ) : nullptr;
            }

            virtual uint64_t getPos() override
            {
                if ( handle )
                    return ftell( handle );
                else
                    return 0;
            }

            virtual uint64_t getSize() override
            {
                if ( !handle )
                    return 0;

#if defined( __li_MSW ) && !defined( __GNUC__ )
                // Not available in MinGW. Yet.
                uint64_t latestPos = _ftelli64( handle );
                _fseeki64( handle, 0, SEEK_END );
                uint64_t size = _ftelli64( handle );
                _fseeki64( handle, latestPos, SEEK_SET );
#else
                size_t latestPos = ftell( handle );
                fseek( handle, 0, SEEK_END );
                size_t size = ftell( handle );
                fseek( handle, latestPos, SEEK_SET );
#endif

                lastAccess = Access_none;

                return size;
            }

            virtual bool setPos( uint64_t pos ) override
            {
                if ( handle )
                {
#if defined( __li_MSW ) && !defined( __GNUC__ )
                    _fseeki64( handle, pos, SEEK_SET );
#else
                    fseek( handle, pos, SEEK_SET );
#endif
                    return true;
                }
                else
                    return false;
            }

            // *** InputStream methods ***

            virtual bool eof() override
            {
                return !handle || feof( handle );
            }

            virtual size_t read( void* out, size_t readSize ) override
            {
                if ( lastAccess == Access_write )
                    fflush( handle );

                lastAccess = Access_read;

                return fread( out, 1, readSize, handle );
            }

            // *** OutputStream methods ***

            virtual size_t write( const void* in, size_t writeSize ) override
            {
                if ( lastAccess == Access_read )
                    fflush( handle );

                lastAccess = Access_write;

                return fwrite( in, 1, writeSize, handle );
            }

            // *** static methods ***

            static bool copy( const char* from, const char* to )
            {
                File source( from );
                File dest( to, true );

                if ( !source || !dest )
                    return false;

                return dest.copyFrom( &source ) == source.getSize();
            }

            static String formatFileName( const char* fileName )
            {
                /*size_t length = strlen( fileName );

                for ( size_t i = 0; i < length; )
                {
                    Unicode::Char c;

                    i += Utf8::decode( c, fileName + i, length - i );

                    if ( c == ' ' || c == '/' || c == ',' )
                        return ( String ) "`" + fileName + "`";
                }

                return fileName;*/

                return String( "`" ) + fileName + "`";
            }

            static bool moveFile( const char* fileName, const char* newFileName )
            {
                remove( newFileName );
                return rename( fileName, newFileName ) == 0;
            }

            static bool queryFileSize( const char* fileName, uint64_t& fileSize )
            {
                FileStat stat;

                if ( !statFileOrDirectory( fileName, &stat ) )
                    return false;

                if ( ( stat.flags & FileStat::is_file ) == 0 )
                    return false;

                fileSize = stat.sizeInBytes;
                return true;
            }

            static bool statFileOrDirectory( const char* fileName, FileStat* stat_out )
            {
#ifdef __li_MSW
                WIN32_FILE_ATTRIBUTE_DATA data;

                // TODO: Unicode filename support
                if ( !GetFileAttributesExA( fileName, GetFileExInfoStandard, &data ) )
                    return false;

                stat_out->flags = 0;

                if ( data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
                    stat_out->flags |= FileStat::is_directory;
                else
                    stat_out->flags |= FileStat::is_file;

                stat_out->sizeInBytes = ( ( uint64_t ) data.nFileSizeHigh << 32 ) | data.nFileSizeLow;
                stat_out->creationTime = FileTimeToTime_t( data.ftCreationTime );
                stat_out->modificationTime = FileTimeToTime_t( data.ftLastWriteTime );

                return true;
#else
                struct stat st;

                if ( stat( fileName, &st ) != 0 )
                    return false;

                stat_out->flags = 0;

                if ( S_ISDIR( st.st_mode ) )
                    stat_out->flags |= FileStat::is_directory;
                else
                    // assume regular file otherwise
                    stat_out->flags |= FileStat::is_file;

                stat_out->sizeInBytes = st.st_size;
                stat_out->creationTime = st.st_ctime;
                stat_out->modificationTime = st.st_mtime;

                return true;
#endif
            }

            static String getContents( const char* fileName )
            {
                return File( fileName ).readWhole();
            }

            static int getLines( const char* fileName, List<String>& list )
            {
                return File( fileName ).readLines( list );
            }

            static bool loadIntoBuffer( const char* fileName, void* data, size_t length )
            {
                File file( fileName, "rb" );

                if ( file.handle == nullptr )
                    return false;

                return file.read( data, length ) == length;
            }

            static bool setContents( const char* fileName, const String& text )
            {
                File file( fileName, "wb" );

                if ( file.handle == nullptr )
                    return false;

                return file.write( text.c_str(), text.getNumBytes() ) == text.getNumBytes();
            }

            static bool saveBuffer( const char* fileName, const void* data, size_t length )
            {
                File file( fileName, "wb" );

                if ( file.handle == nullptr )
                    return false;

                return file.write( data, length ) == length;
            }
    };
}
