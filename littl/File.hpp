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

#include <littl/BaseIO.hpp>
#include <littl/Exception.hpp>
#include <littl/FileName.hpp>
#include <littl/String.hpp>

#ifndef __li_MSW
#include <sys/stat.h>
#endif

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

    class File: public SeekableIOStream
    {
        li_refcounted_class( File )

        private:
            enum LastAccess { Access_none, Access_read, Access_write };

            FILE* handle;
            LastAccess lastAccess;

        protected:
            File( const char* fileName, const char* mode = "rb" )
            {
                handle = fopen( fileName, mode );
            }

            ~File()
    		{
                if ( handle )
                    fclose( handle );
            }

    	public:
    		File( FILE* handle ) : handle( handle )
    		{
    		    lastAccess = Access_none;
            }

            static File* open( const char* fileName, bool forWriting = false )
            {
                FILE* handle = fopen( fileName, forWriting ? "wb" : "rb" );

                if ( handle == nullptr )
                    return nullptr;

                return new File( handle );
            }

            static File* open( const char* fileName, const char* mode )
            {
                FILE* handle = fopen( fileName, mode );

                if ( handle == nullptr )
                    return nullptr;

                return new File( handle );
            }

            static bool copy( const char* from, const char* to )
            {
                Reference<File> source = File::open( from );
                Reference<File> dest = File::open( to, true );

                if ( !li::isReadable( source ) || !li::isWritable( dest ) )
                    return false;

                return dest->copyFrom( source ) == source->getSize();
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

                return ( String ) "`" + fileName + "`";
            }

            static String getDirectoryFromPath( const String& path )
            {
                return path.leftPart( maximum<intptr_t>( 0, path.findLastChar( '/' ), path.findLastChar( '\\' ) ) );
            }

            virtual uint64_t getPos()
            {
                if ( handle )
                    return ftell( handle );
                else
                    return 0;
            }

            virtual uint64_t getSize()
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

            bool isOk() const
            {
                return handle != 0;
            }

            virtual bool isReadable()
            {
                return handle && !feof( handle );
            }

            virtual bool isWritable()
            {
                return handle != 0;
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

            virtual size_t rawRead( void* out, size_t readSize )
            {
                if ( lastAccess == Access_write )
                    fflush( handle );

                lastAccess = Access_read;

                return fread( out, 1, readSize, handle );
            }

            virtual size_t rawWrite( const void* in, size_t writeSize )
            {
                if ( lastAccess == Access_read )
                    fflush( handle );

                lastAccess = Access_write;

                return fwrite( in, 1, writeSize, handle );
            }

            virtual size_t read( void* out, size_t readSize )
            {
                return rawRead( out, readSize );
            }

            static String read( const char* fileName )
            {
                File file( fileName );

                if ( file.isReadable() )
                    return file.readWhole();
                else
                    return String();
            }

            virtual bool setPos( uint64_t pos )
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

            static String load( const char* fileName )
            {
                return File( fileName ).readWhole();
            }

            static int load( const char* fileName, List<String>& list )
            {
                return File( fileName ).readLines( list );
            }

            static bool load( const char* fileName, void* data, size_t length )
            {
                File file( fileName, "rb" );

                if ( file.handle == nullptr )
                    return false;

                return file.read( data, length ) == length;
            }

            static bool save( const char* fileName, const String& text )
            {
                File file( fileName, "wb" );

                if ( file.handle == nullptr )
                    return false;

                return file.write( text.c_str(), text.getNumBytes() ) == text.getNumBytes();
            }

            static bool save( const char* fileName, const void* data, size_t length )
            {
                File file( fileName, "wb" );

                if ( file.handle == nullptr )
                    return false;

                return file.write( data, length ) == length;
            }
    };
}
