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

#ifdef _MSC_VER
// warning C4100: 'identifier' : unreferenced formal parameter
// Reason to ignore: Complains about destructPointer called on POD data
#pragma warning ( disable : 4100 )

// warning C4345: behavior change: an object of POD type constructed with an initializer of the form () will be default-initialized
// Reason to ignore: Spams when there is a li::List of structs without constructor
#pragma warning ( disable : 4345 )

// warning C4521: 'class' : multiple copy constructors specified
// Reason to ignore: why the hell is this even a warning?!
#pragma warning ( disable : 4521 )

// warning C6258: using TerminateThread does not allow proper thread clean up
// Reason to ignore: We know.
#pragma warning ( disable : 6258 )

// VS don't have no cinttypes
#include <inttypes.h>
#elif __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 7)
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#else
#include <cinttypes>
#include <unistd.h>
#endif

#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <stdint.h>
#include <typeinfo>
#include <utility>

#define li_little_endian

// basic platform-related stuff
#if ( defined( __WINDOWS__ ) || defined( _WIN32 ) || defined( _WIN64 ) )
// MS Windows
#ifndef NOMINMAX
#define NOMINMAX
#endif

#define __li_MSW
#define li_MSW
#define li_newLine "\r\n"
#else
// all POSIX (Linux, OS X)
#include <unistd.h>

#if defined(__APPLE__)
#define li_Apple
#endif

#define __li_POSIX
#define li_newLine "\n"
#endif

// Clang has this
#ifndef __has_feature
#define __has_feature(x) 0
#endif

//--- GCC-specific ---
#ifdef __GNUC__
#if __GNUC__ == 4
#define li_GCC4 1
#define li_functionName __PRETTY_FUNCTION__
#else
#define li_functionName __func__
#endif

#if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 7 )
#if !__has_feature(cxx_nullptr)
#define nullptr __null
#endif

#if !__has_feature(cxx_override_control)
#define override
#endif

using std::ptrdiff_t;
#endif

#define li_enum_class( name_ ) enum class name_
#define li_force_inline( proto_ ) proto_ __attribute__((always_inline))
#define li_noreturn( proto_ ) proto_ __attribute__((noreturn))

#elif defined( _MSC_VER )
//--- Visual C++-specific ---
#define snprintf sprintf_s
#define li_functionName __FUNCTION__
#define li_enum_class( name_ ) enum name_ : unsigned
#define li_force_inline( proto_ ) __forceinline proto_
#define li_noreturn( proto_ ) __declspec( noreturn ) proto_

#elif defined( DOXYGEN )
//--- Doxygen versions ---
#define li_enum_class( name_ ) enum name_
#define li_force_inline( proto_ ) proto_
#define li_noreturn( proto_ ) proto_
#endif

#ifndef lengthof
#define lengthof( array_ ) ( sizeof( array_ ) / sizeof( *( array_ ) ) )
#endif

#ifndef li_avoid_libstdcxx
#include <new>
#endif

#define li_stringify(x)     #x
#define li_stringify2(x)    li_stringify(x)

namespace li
{
    template<size_t alignment, typename Type> inline Type align( Type value )
    {
        static_assert(alignment && !(alignment & (alignment - 1)), "alignment must be power of 2");

        return ( value + alignment - 1 ) & ~( alignment - 1 );
    }

    template<typename Type> inline Type limitTo( Type value, Type min, Type max )
    {
        if ( value < min )
            return min;
        else if ( value > max )
            return max;
        else
            return value;
    }

    template<typename Type> inline Type round( Type value )
    {
        return value < ( Type ) 0.0 ? ceil( value - ( Type ) 0.5 ) : floor( value + ( Type ) 0.5 );
    }

    template<typename Type> inline void constructPointer( Type* p )
    {
        new( static_cast<void*>( p ) ) Type();
    }

    template<typename Type, typename Type2> inline void constructPointer( Type* p, Type2 par )
    {
        new( static_cast<void*>( p ) ) Type( std::forward<Type2>( par ) );
    }

    template<typename Type> inline void destructPointer( Type* p )
    {
        p->~Type();
    }

    inline int stringCaseCompare( const char* a, const char* b )
    {
        // http://stackoverflow.com/a/5820991/2524350

        for (;; a++, b++) {
            int d = tolower(*a) - tolower(*b);
            if (d != 0 || !*a)
                return d;
        }
    }

    inline void throwException(const char* functionName, const char* name, const char* description);
}
