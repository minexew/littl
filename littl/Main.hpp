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
#include <littl/Exception.hpp>
#include <littl/List.hpp>
#include <littl/String.hpp>

// static void real_main_( const li::String& app_name, li::List<li::String> args )

#define li_main( real_main_ ) extern "C" int main( int argc, char** argv )\
{\
    li::List<li::String> args;\
\
    for ( int i = 1; i < argc; i++ )\
        args.add( argv[i] );\
\
    try\
    {\
        real_main_( argv[0], ( li::List<li::String>&& ) args );\
    }\
    catch ( li::Exception ex )\
    {\
        ex.print();\
    }\
\
    printf( "\n" );\
    return 0;\
}

#define li_main_class_( main_class_ ) extern "C" int main( int argc, char** argv )\
{\
    li::List<li::String> args;\
\
    for ( int i = 1; i < argc; i++ )\
        args.add( argv[i] );\
\
    bool result = false;\
\
    try\
    {\
        li::Object<main_class_> mc = new main_class_();\
        result = mc->appmain( argv[0], ( li::List<li::String>&& ) args );\
    }\
    catch ( li::Exception ex )\
    {\
        ex.print();\
    }\
\
    printf( "\n" );\
    return result ? 0 : -1;\
}
/*
namespace li
{
    inline String getArgumentInput( const List<String>& args )
    {
        for each_in_list ( args, i )
            if ( !args[i].beginsWith( '-' ) )
                return args[i];

        return 0;
    }

    inline String getArgument( const List<String>& args, const String& beginsWith )
    {
        for each_in_list ( args, i )
            if ( args[i].beginsWith( beginsWith ) )
                return args[i].dropLeft( beginsWith.getNumCharsUncached() );

        return 0;
    }
}
*/
