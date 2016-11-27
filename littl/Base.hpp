/*
    Copyright (c) 2008-2016 Xeatheran Minexew

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

#define li_little_endian

#if ( defined( __WINDOWS__ ) || defined( _WIN32 ) || defined( _WIN64 ) )
#define __li_MSW
#define li_MSW
#define li_newLine "\r\n"
#else
#define __li_POSIX
#define li_newLine "\n"
#endif

#ifdef __GNUC__
#define li_functionName __func__
#elif defined( _MSC_VER )
#define li_functionName __FUNCTION__
#endif

#ifndef li_lengthof
#define li_lengthof( array_ ) ( sizeof( array_ ) / sizeof( *( array_ ) ) )
#endif

namespace li
{
    class InputStream;
    class OutputStream;
}
