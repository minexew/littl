/*
    Copyright (c) 2013 Xeatheran Minexew

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

// Source:  http://sourceforge.net/p/predef/wiki/Compilers/
// Source:  http://nadeausoftware.com/articles/2012/10/c_c_tip_how_detect_compiler_name_and_version_using_compiler_predefined_macros

#pragma once

#include <littl/Base.hpp>

#if defined(__clang__)
#define li_compiler_name        "Clang"
#define li_compiler_version     __VERSION__
#define li_compiled_using       li_compiler_name " " li_compiler_version
#elif defined(__INTEL_COMPILER)
#define li_compiler_name        "Intel"
#define li_compiler_version     __VERSION__
#define li_compiled_using       li_compiler_name " " li_compiler_version
#elif defined(__GNUC__)
#define li_compiler_name        "GCC"
#define li_compiler_version     __VERSION__
#define li_compiled_using       li_compiler_name " " li_compiler_version
#elif defined(_MSC_VER)
#define li_compiler_name        "Microsoft Visual C++"

// Kinda pointless as we only support 11.0+
#if _MSC_VER == 1700
#define li_compiler_version     "11.0 (Visual Studio 2012)"
#elif  _MSC_VER == 1600
#define li_compiler_version     "10.0 (Visual Studio 2010)"
#elif _MSC_VER == 1500
#define li_compiler_version     "9.0 (Visual Studio 2008)"
#elif _MSC_VER == 1400
#define li_compiler_version     "8.0 (Visual Studio 2005)"
#elif _MSC_VER == 1310
#define li_compiler_version     "7.1 (Visual Studio 2003)"
#elif _MSC_VER == 1300
#define li_compiler_version     "7.0"
#elif _MSC_VER == 1200
#define li_compiler_version     "6.0"
#elif _MSC_VER == 1100
#define li_compiler_version     "5.0"
#else
#define li_compiler_version     li_stringify2(_MSC_VER)
#endif

#define li_compiled_using       li_compiler_name " " li_compiler_version

#else
#define li_compiler_name        "Unknown"
#define li_compiler_version     "?"
#define li_compiled_using       "Unknown compiler"
#endif
