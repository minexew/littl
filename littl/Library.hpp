/*
    Copyright (C) 2011, 2013 Xeatheran Minexew

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

#include <littl/Base.hpp>

#ifdef li_MSW
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace li
{
    class Library
    {
        protected:
#ifdef li_MSW
            HINSTANCE instance;

            Library( HINSTANCE instance )
                    : instance( instance )
            {
            }
#else
            void* instance;
        
            Library( void* instance )
                : instance( instance )
            {
            }
#endif

        public:
            static Library* open( const char* fileName )
            {
#ifdef li_MSW
                HINSTANCE instance = LoadLibraryA( fileName );
#else
                void* instance = dlopen( fileName, RTLD_LAZY );
#endif

                if ( instance != nullptr )
                    return new Library( instance );
                else
                    return nullptr;
            }

            ~Library()
            {
                if ( instance != nullptr )
#ifdef li_MSW
                    FreeLibrary( instance );
#else
                    dlclose( instance );
#endif
            }

            template<typename T> T getEntry( const char* name )
            {
#ifdef li_MSW
                return ( T ) GetProcAddress( instance, name );
#else
                return ( T ) dlsym( instance, name );
#endif
            }
    };
}
