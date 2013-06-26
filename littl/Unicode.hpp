/*
    Copyright (C) 2011, 2012, 2013 Xeatheran Minexew

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

#include "Base.hpp"

namespace li
{
    class Unicode
    {
        public:
            typedef unsigned Char;

            static const Char backspaceChar =   0x00000008;
            static const Char tabChar =         0x00000009;
            static const Char lineFeedChar =    0x0000000A;
            static const Char invalidChar =     0xFFFFFFFF;

            static bool isAlpha( Char c )
            {
                return ( c >= 'A' && c <= 'Z' ) || ( c >= 'a' && c <= 'z' );
            }

            static bool isAlphaNumeric( Char c )
            {
                return isAlpha( c ) || isNumeric( c );
            }

            static bool isNumeric( Char c )
            {
                return c >= '0' && c <= '9';
            }
    };

    struct UnicodeChar
    {
        Unicode::Char c;

        UnicodeChar( Unicode::Char c = Unicode::invalidChar ) : c( c ) {}

        UnicodeChar& operator = ( Unicode::Char c ) { this->c = c; return *this; }
        operator Unicode::Char& () { return c; }
        operator const Unicode::Char& () const { return c; }
    };
}
