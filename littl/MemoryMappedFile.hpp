/*
    Copyright (c) 2012 Xeatheran Minexew

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

#ifndef __li_MSW
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#endif

namespace li
{
    class MemoryMappedFile
    {
        protected:
#ifdef __li_MSW
            HANDLE hFile, hFileMapping;
#else
            int fd;
#endif

        private:
            MemoryMappedFile() {}
            MemoryMappedFile( const MemoryMappedFile& );

        public:
            static MemoryMappedFile* open( const char* fileName, bool forWriting = false )
            {
#ifdef __li_MSW
                // FIXME: Replace slashes in path with backslashes
                // TODO: Unicode support
                HANDLE hFile = CreateFileA( fileName, forWriting ? (GENERIC_READ | GENERIC_WRITE) : GENERIC_READ, FILE_SHARE_READ, NULL,
                        forWriting ? CREATE_ALWAYS : OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

                if ( hFile == INVALID_HANDLE_VALUE )
                    return nullptr;

                HANDLE hFileMapping = CreateFileMapping( hFile, NULL, forWriting ? PAGE_READWRITE : PAGE_READONLY, 0, 0, NULL );

                if ( hFileMapping == NULL )
                {
                    CloseHandle( hFile );
                    return nullptr;
                }

                MemoryMappedFile* mmf = new MemoryMappedFile;
                mmf->hFile = hFile;
                mmf->hFileMapping = hFileMapping;
                return mmf;
#else
                int fd = ::open( fileName, forWriting ? ( O_CREAT | O_WRONLY | O_TRUNC ) : O_RDONLY, 0644 );

                if (fd == -1)
                    return nullptr;

                MemoryMappedFile* mmf = new MemoryMappedFile;
                mmf->fd = fd;
                return mmf;
#endif
            }

            ~MemoryMappedFile()
            {
#ifdef __li_MSW
                CloseHandle(hFileMapping);
                CloseHandle(hFile);
#else
                close(fd);
#endif
            }
    };
}
