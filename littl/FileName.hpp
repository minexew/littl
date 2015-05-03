/*
    Copyright (c) 2011-2013 Xeatheran Minexew

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

#include <littl/String.hpp>

namespace li
{
    class FileName
    {
        String path;

        public:
            FileName( const String& path )
            {
                if ( path.endsWith( '/' ) || path.endsWith( '\\' ) )
                    this->path = path.dropRightPart( 1 );
                else
                    this->path = path;
            }

            static String getAbsolutePath( const char* path )
            {
#ifdef li_MSW
                // FIXME: Use Unicode
                char fullPath[4096];

                GetFullPathNameA( path, sizeof( fullPath ), fullPath, NULL );

                return String( fullPath );
#elif !defined(_3DS)
                char* realPath = realpath( path, nullptr );
                String absolutePath( realPath );

                free( realPath );
                return absolutePath;
#else
                return path;
#endif
            }

            String getAbsolutePath()
            {
                return getAbsolutePath( path );
            }

            static String format( const char* fileName )
            {
                return ( String ) "`" + fileName + "`";
            }

            String getDirectory() const
            {
                intptr_t offset = std::max( path.findLastChar( '/' ), path.findLastChar( '\\' ) );

                if ( offset < 0 )
                    return ".";

                return path.leftPart( offset );
            }

            String getExtension() const
            {
                intptr_t offset = path.findLastChar( '.' );

                if ( offset < 0 )
                    return path;

                return path.dropLeftPart( offset + 1 );
            }

            String getFileName() const
            {
                intptr_t offset = std::max( path.findLastChar( '/' ), path.findLastChar( '\\' ) );

                if ( offset < 0 )
                    return path;

                return path.dropLeftPart( offset + 1 );
            }
    };
}
