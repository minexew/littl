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

#include <littl/Base.hpp>

#include <cstdlib>
#include <cstring>
#include <memory>

namespace li
{
    template<typename Type> inline void constructPointer(Type* p)
    {
        new(static_cast<void*>(p)) Type();
    }

    template<typename Type, typename Type2> inline void constructPointer(Type* p, Type2 par)
    {
        new(static_cast<void*>(p)) Type(std::forward<Type2>(par));
    }

    template<typename Type> inline void destructPointer(Type* p)
    {
        p->~Type();
    }

    template <typename T = uint8_t>
    class Allocator
    {
        public:
            static T* allocate( size_t count )
            {
                T* pointer = ( T* )malloc( sizeof( T ) * count );
                clear( pointer, count );
                return pointer;
            }

            static void clear( T* pointer, size_t count )
            {
                set( pointer, count, 0 );
            }

            static T* release( void* pointer )
            {
                free( pointer );
                return nullptr;
            }

            static void move( T* destination, const T* source, size_t count )
            {
                if ( destination && source && count )
                    memmove( destination, source, sizeof( T ) * count );
            }

            static void set( T* pointer, size_t count, unsigned char value )
            {
                if ( pointer != 0 && count > 0 )
                    memset( pointer, value, sizeof( T ) * count );
            }

            static T* resize( void* pointer, size_t count )
            {
                return ( T* )realloc( pointer, sizeof( T ) * count );
            }
    };
}
