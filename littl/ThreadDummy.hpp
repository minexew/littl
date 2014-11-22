/*
    Copyright (C) 2010, 2011, 2012 Xeatheran Minexew

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

namespace li
{
    class Mutex
    {
        public:
            Mutex()
            {
            }

            ~Mutex()
            {
            }

            void enter()
            {
            }

            void leave()
            {
            }
    };

    class ConditionVar
    {
        public:
            ConditionVar()
            {
            }

            ~ConditionVar()
            {
            }

            void set()
            {
            }

            bool waitFor()
            {
                return true;
            }
    };

    inline bool interlockedCompareExchange(volatile int32_t* value_ptr, int32_t compareTo, int32_t newValue)
    {
        auto ret = *value_ptr;

        if (*value_ptr == compareTo)
            *value_ptr = newValue;

        return ret;
    }

    inline int32_t interlockedDecrement(volatile int32_t* a_ptr)
    {
        return --(*a_ptr);
    }

    inline int32_t interlockedIncrement(volatile int32_t* a_ptr)
    {
        return ++(*a_ptr);
    }
}
