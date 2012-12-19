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

#include <littl/Base.hpp>

#ifndef li_MSW
#include <sys/time.h>
#endif

namespace li
{
#ifdef li_MSW
    class PerfTimer
    {
        public:
            typedef uint64_t Counter;

        private:
            LARGE_INTEGER freq;

        public:
            PerfTimer()
            {
                QueryPerformanceFrequency( &freq );
            }

            Counter getCurrentMicros()
            {
                LARGE_INTEGER counter;
                QueryPerformanceCounter( &counter );

                return counter.QuadPart * 1000000 / freq.QuadPart;
            }

            Counter getCurrentMillis()
            {
                LARGE_INTEGER counter;
                QueryPerformanceCounter( &counter );

                return counter.QuadPart * 1000 / freq.QuadPart;
            }

            static void tinySleepIfPossible()
            {
            }
    };
#else
    class PerfTimer
    {
        public:
            typedef uint64_t Counter;
        
        public:
            PerfTimer()
            {
            }

            Counter getCurrentMicros()
            {
                struct timeval tv;
                gettimeofday( &tv, nullptr );

                return tv.tv_sec * 1000000 + tv.tv_usec;
            }

            Counter getCurrentMillis()
            {
                struct timeval tv;
                gettimeofday( &tv, nullptr );

                return tv.tv_sec * 1000 + tv.tv_usec / 1000;
            }

            static void tinySleepIfPossible()
            {
                usleep( 1 );
            }
    };
#endif
}

/*
uint64_t GetPIDTimeInNanoseconds(void)
{
    uint64_t        start;
    uint64_t        end;
    uint64_t        elapsed;
    uint64_t        elapsedNano;
    static mach_timebase_info_data_t    sTimebaseInfo;

    // Start the clock.

    start = mach_absolute_time();

    // Call getpid. This will produce inaccurate results because 
    // we're only making a single system call. For more accurate 
    // results you should call getpid multiple times and average 
    // the results.

    (void) getpid();

    // Stop the clock.

    end = mach_absolute_time();

    // Calculate the duration.

    elapsed = end - start;

    // Convert to nanoseconds.

    // If this is the first time we've run, get the timebase.
    // We can use denom == 0 to indicate that sTimebaseInfo is 
    // uninitialised because it makes no sense to have a zero 
    // denominator is a fraction.

    if ( sTimebaseInfo.denom == 0 ) {
        (void) mach_timebase_info(&sTimebaseInfo);
    }

    // Do the maths. We hope that the multiplication doesn't 
    // overflow; the price you pay for working in fixed point.

    elapsedNano = elapsed * sTimebaseInfo.numer / sTimebaseInfo.denom;

    printf("multiplier %u / %u\n", sTimebaseInfo.numer, sTimebaseInfo.denom);
    return elapsedNano;
}
*/