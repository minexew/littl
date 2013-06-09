/*
    Copyright (c) 2011, 2013 Xeatheran Minexew

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
#include <littl/List.hpp>
#include <littl/String.hpp>

#ifdef li_MSW
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace li
{
    class Directory
    {
        public:
            struct Entry
            {
                String name;
                bool isDirectory;
                uint64_t size;
            };

        protected:

#ifdef li_MSW
            String path;
#else
            DIR* dir;
#endif

#ifdef li_MSW
            Directory( const char* path ) : path( path )
            {
            }
#else
            Directory( DIR* dir ) : dir( dir )
            {
            }
#endif

        public:
#ifndef li_MSW
            ~Directory()
            {
                closedir( dir );
            }
#endif

            static bool create(const char *name)
            {
#ifdef __li_MSW
                return (CreateDirectoryA(name, NULL) != FALSE);
#else
                return mkdir(name, 0755) == 0;
#endif
            }
        
            void list( List<String>& items )
            {
#ifdef __li_MSW
                // TODO: UTF-8 !!!

                HANDLE find;
                WIN32_FIND_DATAA findFileData;

                if ( ( find = FindFirstFileA( path + "\\*.*", &findFileData ) ) != INVALID_HANDLE_VALUE )
                {
                    do
                	    items.add( findFileData.cFileName );
                    while ( FindNextFileA( find, &findFileData ) );

                    FindClose( find );
                }
#else
                rewinddir( dir );

                dirent* ent;

                while ( ( ent = readdir( dir ) ) )
                    items.add( ent->d_name );
#endif
            }

            void list( List<Entry>& entries )
            {
#ifdef __li_MSW
                // TODO: UTF-8 !!!

                HANDLE find;
                WIN32_FIND_DATAA findFileData;

                if ( ( find = FindFirstFileA( path + "\\*.*", &findFileData ) ) != INVALID_HANDLE_VALUE )
                {
                    do
                    {
                        Entry entry = { findFileData.cFileName,
                                (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? true : false,
                                ( (uint64_t)findFileData.nFileSizeHigh << 32 ) | findFileData.nFileSizeLow
                        };

                	    entries.add( entry );
                    }
                    while ( FindNextFileA( find, &findFileData ) );

                    FindClose( find );
                }
#else
                rewinddir( dir );
                
                dirent* ent;
                
                while ( ( ent = readdir( dir ) ) )
                {
                    Entry entry = { ent->d_name, false /*isDirectory*/, 0 /* size */};
                    entries.add(entry);
                }
#endif
            }

            static String getCurrent()
            {
#ifdef li_MSW
                // FIXME: Use Unicode
                char currDir[4096];

                currDir[0] = 0;
                GetCurrentDirectoryA(lengthof(currDir), currDir);

                return String(currDir);
#else
                char currDir[4096];

                return String(getcwd(currDir, lengthof(currDir)));
#endif
            }

            static Directory* open( const char* path )
            {
#ifdef li_MSW
                DWORD attributes = GetFileAttributesA( path );

                if ( attributes == INVALID_FILE_ATTRIBUTES )
                    return nullptr;

                if ( attributes & FILE_ATTRIBUTE_DIRECTORY )
                    return new Directory( path );
                else
                    return nullptr;
#else
                DIR* dir = opendir( path );

                if ( dir )
                    return new Directory( dir );
                else
                    return nullptr;
#endif
            }
    };
}
