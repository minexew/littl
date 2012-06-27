/*
    Copyright (c) 2008-2012 Xeatheran Minexew

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

namespace li
{
    class File: public SeekableIOStream
    {
        private:
            FILE* handle;

        protected:
            File( const char* fileName, const char* mode = "rb" )
            {
                handle = fopen( fileName, mode );
            }

            virtual ~File()
    		{
                if ( handle )
                    fclose( handle );
            }

    	public:
            li_ReferencedClass_override( File )

    		File( FILE* handle ) : handle( handle )
    		{
            }

            static File* open( const char* fileName, bool forWriting = false, bool required = false )
            {
                FILE* handle = fopen( fileName, forWriting ? "wb" : "rb" );

                if ( handle == nullptr )
                {
                    if ( required )
                        throw Exception( "FileOpenError", "Failed to open " + FileName::format( fileName ) );

                    return nullptr;
                }

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
                return path.leftPart( maximum( 0, path.findLastChar( '/' ), path.findLastChar( '\\' ) ) );
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

            static bool queryFileSize( const char* fileName, uint64_t& fileSize )
            {
#ifdef __li_MSW
                // TODO: Unicode filename support
                HANDLE hFile = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

                if (hFile == INVALID_HANDLE_VALUE)
                    return false;

                LARGE_INTEGER li;
                GetFileSizeEx(hFile, &li);
                fileSize = li.QuadPart;
                return true;
#else
                struct stat st; 

                if ( stat( filename, &st ) == 0 )
                {
                    fileSize = st.st_size;
                    return true;
                }

                return false;
#endif
            }

            virtual size_t rawRead( void* out, size_t readSize )
            {
                return fread( out, 1, readSize, handle );
            }

            virtual size_t rawWrite( const void* in, size_t writeSize )
            {
                return fwrite( in, 1, writeSize, handle );
            }

            virtual size_t read( void* out, size_t readSize )
            {
                return fread( out, 1, readSize, handle );
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
