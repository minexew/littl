/*
    Copyright (c) 2010-2011 Xeatheran Minexew

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
    class Exception
    {
        public:
            struct Saved { String functionName, name, description; };

        public:
            String functionName, name, description;

            Exception( const char* name, const char* description = nullptr )
                    : name( name ), description( description )
            {
            }

            Exception( const char* functionName, const char* name, const char* description )
                    : functionName( functionName ), name( name ), description( description )
            {
            }

            virtual ~Exception()
            {
            }

            String format()
            {
                String text = "  Runtime Exception: " + name + "\n";

                if ( !functionName.isEmpty() )
                    text += "    in:             " + functionName + "\n";

                text += "    description:    " + description;

                return text;
            }

            const String& getDesc() const
            {
                return description;
            }

            const String& getName() const
            {
                return name;
            }

            void print()
            {
                String text = "\n  ";

                for ( size_t i = 0; i < 19 + name.getNumChars(); i++ )
                    text += "_";

                text += "\n";
                text += format();

                printf( "%s\n", text.c_str() );
            }

            static void rethrow( const Saved& saved )
            {
#ifndef littl_no_exceptions
                throw Exception( saved.functionName, saved.name, saved.description );
#endif
            }

            void save( Saved& saved )
            {
                saved.functionName = functionName;
                saved.name = name;
                saved.description = description;
            }
    };

    inline void throwException(const char* functionName, const char* name, const char* description)
    {
#ifndef littl_no_exceptions
        throw Exception(functionName, name, description);
#endif
    }
}
