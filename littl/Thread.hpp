/*
    Copyright (C) 2010, 2011 Xeatheran Minexew

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

#ifndef __li_MSW
#error NOT IMPLEMENTED FOR OTHER PLATFORMS THAN WINNT (yet)
#endif

namespace li
{
    template<typename ParamType>
    class ThreadLauncher
    {
        bool running;
        void ( *threadFunction )( ParamType );
#ifdef __li_MSW
        HANDLE threadHandle;
        DWORD threadId;
        int priority;

        struct ThreadStartInfo
        {
            ThreadLauncher* launcher;
            void ( *threadFunction )( ParamType );
            void ( *exitFunction )( ParamType );
            ParamType param;
        }
        info;

        static DWORD threadStartRoutine( ThreadStartInfo* info )
        {
            info->threadFunction( info->param );
            info->launcher->running = false;

            if ( info->exitFunction )
                info->exitFunction( info->param );

            return 0;
        }
#endif

        ThreadLauncher( const ThreadLauncher& );
        const ThreadLauncher& operator = ( const ThreadLauncher& );

        public:
            ThreadLauncher( void ( *run )( ParamType param ) );
            ~ThreadLauncher();

            bool kill();
            bool isRunning() const { return running; }
            void setPriority( int priority );
            bool start( ParamType param, void ( *exitFunction )( ParamType ) );
            bool waitFor( long time = -1 );
    };

#define __li_member( type ) template<typename ParamType> type ThreadLauncher<ParamType>::
#define __li_member_ template<typename ParamType> ThreadLauncher<ParamType>::

    __li_member_ ThreadLauncher( void ( *run )( ParamType param ) )
        : running( false ), threadFunction( run ), priority( THREAD_PRIORITY_NORMAL )
    {
#ifdef __li_MSW
        threadHandle = INVALID_HANDLE_VALUE;
        threadId = 0;
#endif
    }

    __li_member_ ~ThreadLauncher()
    {
#ifdef __li_MSW
        if ( threadHandle != INVALID_HANDLE_VALUE )
            CloseHandle( threadHandle );
#endif
    }

    __li_member( bool ) kill()
    {
#ifdef __li_MSW
        if ( running && threadHandle != INVALID_HANDLE_VALUE )
        {
            TerminateThread( threadHandle, 0 );
            running = false;
            threadHandle = INVALID_HANDLE_VALUE;
            return true;
        }
#endif
        return false;
    }

    __li_member( void ) setPriority( int priority )
    {
        this->priority = priority;

        if ( threadHandle != INVALID_HANDLE_VALUE )
            SetThreadPriority( threadHandle, priority );
    }

    __li_member( bool ) start( ParamType param, void ( *exitFunction )( ParamType ) )
    {
        if ( running )
            return false;

#ifdef __li_MSW
        info.launcher = this;
        info.threadFunction = threadFunction;
        info.exitFunction = exitFunction;
        info.param = param;
        threadHandle = CreateThread( 0, 0, ( LPTHREAD_START_ROUTINE )( threadStartRoutine ), &info, 0, &threadId );
        setPriority( priority );
        return running = ( threadHandle != INVALID_HANDLE_VALUE );
#else
        return false;
#endif
    }

    __li_member( bool ) waitFor( long time )
    {
#ifdef __li_MSW
        if ( threadHandle == INVALID_HANDLE_VALUE )
            return false;
        else
            return !WaitForSingleObject( threadHandle, time < 0 ? INFINITE : time );
#endif
        return false;
    }

#undef __li_member
#undef __li_member_

    class Thread
    {
        public:
            enum Priority
            {
                lowest = THREAD_PRIORITY_LOWEST,
                belowNormal = THREAD_PRIORITY_BELOW_NORMAL,
                normal = THREAD_PRIORITY_NORMAL,
                aboveNormal = THREAD_PRIORITY_ABOVE_NORMAL,
                highest = THREAD_PRIORITY_HIGHEST
            };

        private:
            ThreadLauncher<Thread*> launcher;

            static void threadStartRoutine( Thread* thread )
            {
                thread->run();
            }

            static void threadEndDestroyRoutine( Thread* thread )
            {
                delete thread;
            }

        protected:
            bool destroy, shouldEnd;

            virtual void run() = 0;

        public:
            Thread() : launcher( threadStartRoutine ), destroy( false ), shouldEnd( false )
            {
            }

            virtual ~Thread()
            {
            }

            void destroyOnExit( bool enable = true )
            {
                destroy = enable;
            }

            void end()
            {
                shouldEnd = true;
            }

            bool isRunning() const
            {
                return launcher.isRunning();
            }

            bool kill()
            {
                return launcher.kill();
            }

            void setPriority( Priority priority )
            {
                launcher.setPriority( priority );
            }

            bool start()
            {
                shouldEnd = false;
                return launcher.start( this, destroy ? threadEndDestroyRoutine : 0 );
            }

            bool waitFor( long time = -1 )
            {
                return launcher.waitFor( time );
            }
    };

    class Semaphore
    {
#ifdef __li_MSW
        HANDLE semaphoreHandle;
#endif

        Semaphore( const Semaphore& );
        const Semaphore& operator = ( const Semaphore& );

        public:
            Semaphore( int maxCount )
#ifdef __li_MSW
                    : semaphoreHandle( 0 )
#endif
            {
#ifdef __li_MSW
                semaphoreHandle = CreateSemaphore( 0, maxCount, maxCount, 0 );
#endif
            }

            ~Semaphore()
            {
#ifdef __li_MSW
                CloseHandle( semaphoreHandle );
#endif
            }

            bool enter( long time = -1 )
            {
#ifdef __li_MSW
                return !WaitForSingleObject( semaphoreHandle, time < 0 ? INFINITE : time );
#else
                return false;
#endif
            }

            void leave()
            {
#ifdef __li_MSW
                ReleaseSemaphore( semaphoreHandle, 1, 0 );
#endif
            }
    };

    class Mutex
    {
        CRITICAL_SECTION criticalSection;

        public:
            Mutex()
            {
                InitializeCriticalSection( &criticalSection );
            }

            ~Mutex()
            {
                DeleteCriticalSection( &criticalSection );
            }

            void enter()
            {
                EnterCriticalSection( &criticalSection );
            }

            void leave()
            {
                LeaveCriticalSection( &criticalSection );
            }
    };

    class CriticalSection
    {
        Mutex* mutex;

        public:
            CriticalSection( Mutex* mutex ) : mutex( mutex )
            {
                mutex->enter();
            }

            CriticalSection( Mutex& mutex ) : mutex( &mutex )
            {
                mutex.enter();
            }

            ~CriticalSection()
            {
                if ( mutex != nullptr )
                    mutex->leave();
            }

            void leave()
            {
                mutex->leave();
                mutex = nullptr;
            }
    };

    template <class T> class MutexVar
    {
        Mutex& mutex;
        T value;

        public:
            MutexVar( Mutex& mutex, T value = 0 ) : mutex( mutex ), value( value )
            {
            }

            operator T ()
            {
                CriticalSection cs( mutex );

                return value;
            }

            operator const T () const
            {
                CriticalSection cs( mutex );

                return value;
            }

            MutexVar<T>& operator = ( T value )
            {
                CriticalSection cs( mutex );

                this->value = value;

                return *this;
            }

            T operator -> ()
            {
                CriticalSection cs( mutex );

#ifdef __li_MSW
                if ( !value )
                    MessageBox( 0, TEXT( "Dereferencing null pointer!" ), TEXT( "li::Reference" ), MB_ICONERROR );
#endif

                return value;
            }

            template <typename T2> T operator + ( const T2& other )
            {
                CriticalSection cs( mutex );

                return value + other;
            }
    };
}
