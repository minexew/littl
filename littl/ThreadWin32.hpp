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

#include <littl/Base.hpp>

namespace li
{
    // For Thread.setName()
    const DWORD MS_VC_EXCEPTION=0x406D1388;

    #pragma pack(push,8)
    typedef struct tagTHREADNAME_INFO
    {
        DWORD dwType;       // Must be 0x1000.
        LPCSTR szName;      // Pointer to name (in user addr space).
        DWORD dwThreadID;   // Thread ID (-1=caller thread).
        DWORD dwFlags;      // Reserved for future use, must be zero.
    } THREADNAME_INFO;
    #pragma pack(pop)

    template<typename ParamType>
    class ThreadLauncher
    {
        bool running;
        void ( *threadFunction )( ParamType );
        
        struct ThreadStartInfo
        {
            ThreadLauncher* launcher;
            void ( *threadFunction )( ParamType );
            void ( *exitFunction )( ParamType );
            ParamType param;
        }
        info;
        
        HANDLE threadHandle;
        DWORD threadId;
        int priority;

#ifdef _MSC_VER
        const char* name;

        static void setThreadName( DWORD dwThreadID, const char* threadName )
        {
            THREADNAME_INFO info;
            info.dwType = 0x1000;
            info.szName = threadName;
            info.dwThreadID = dwThreadID;
            info.dwFlags = 0;

            __try
            {
                RaiseException( MS_VC_EXCEPTION, 0, sizeof( info ) / sizeof( ULONG_PTR ), ( ULONG_PTR* ) &info );
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
            }
        }
#endif

        static DWORD threadStartRoutine( ThreadStartInfo* info )
        {
            info->threadFunction( info->param );
            info->launcher->running = false;

            if ( info->exitFunction )
                info->exitFunction( info->param );

            return 0;
        }

        ThreadLauncher( const ThreadLauncher& );
        const ThreadLauncher& operator = ( const ThreadLauncher& );

        public:
            ThreadLauncher( void ( *run )( ParamType param ) );
            ~ThreadLauncher();

            bool kill();
            bool isRunning() const { return running; }
            bool setName( const char* name );
            void setPriority( int priority );
            bool start( ParamType param, void ( *exitFunction )( ParamType ) );
            bool waitFor( long time = -1 );
    };

#define __li_member( type ) template<typename ParamType> type ThreadLauncher<ParamType>::
#define __li_member_ template<typename ParamType> ThreadLauncher<ParamType>::

    __li_member_ ThreadLauncher( void ( *run )( ParamType param ) )
        : running( false ), threadFunction( run )
    {
        threadHandle = INVALID_HANDLE_VALUE;
        threadId = 0;
        priority = THREAD_PRIORITY_NORMAL;

#ifdef _MSC_VER
        name = nullptr;
#endif
    }

    __li_member_ ~ThreadLauncher()
    {
        if ( threadHandle != INVALID_HANDLE_VALUE )
            CloseHandle( threadHandle );
    }

    __li_member( bool ) kill()
    {
        if ( running && threadHandle != INVALID_HANDLE_VALUE )
        {
            TerminateThread( threadHandle, 0 );
            running = false;
            threadHandle = INVALID_HANDLE_VALUE;
            return true;
        }
    }

    __li_member( bool ) setName( const char* name )
    {
#ifdef _MSC_VER
        this->name = name;
        return true;
#else
        return false;
#endif
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

        info.launcher = this;
        info.threadFunction = threadFunction;
        info.exitFunction = exitFunction;
        info.param = param;
        
        running = true;
        threadHandle = CreateThread( 0, 0, ( LPTHREAD_START_ROUTINE )( threadStartRoutine ), &info, 0, &threadId );

#ifdef _MSC_VER
        if (name != nullptr)
            setThreadName( GetThreadId( threadHandle ), name );
#endif

        setPriority( priority );
        if ( threadHandle == INVALID_HANDLE_VALUE )
        {
            running = false;
            return false;
        }
        else
            return true;
    }

    __li_member( bool ) waitFor( long time )
    {
        if ( threadHandle == INVALID_HANDLE_VALUE )
            return false;
        else
            return !WaitForSingleObject( threadHandle, time < 0 ? INFINITE : time );
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

            bool setName( const char* name )
            {
                return launcher.setName( name );
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
        HANDLE semaphoreHandle;

        Semaphore( const Semaphore& );
        const Semaphore& operator = ( const Semaphore& );

        public:
            Semaphore( int maxCount )
                    : semaphoreHandle( 0 )
            {
                semaphoreHandle = CreateSemaphore( 0, maxCount, maxCount, 0 );
            }

            ~Semaphore()
            {
                CloseHandle( semaphoreHandle );
            }

            bool enter( long time = -1 )
            {
                return !WaitForSingleObject( semaphoreHandle, time < 0 ? INFINITE : time );
            }

            void leave()
            {
                ReleaseSemaphore( semaphoreHandle, 1, 0 );
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

    class ConditionVar
    {
        HANDLE event;

        public:
            ConditionVar()
            {
                event = CreateEvent( nullptr, false, false, nullptr );
            }

            ~ConditionVar()
            {
                CloseHandle( event );
            }

            void set()
            {
                SetEvent( event );
            }

            bool waitFor()
            {
                return WaitForSingleObject( event, INFINITE ) == WAIT_OBJECT_0;
            }
    };

    inline bool interlockedCompareExchange(volatile int32_t* value_ptr, int32_t compareTo, int32_t newValue)
    {
        return InterlockedCompareExchange((volatile LONG*) value_ptr, newValue, compareTo) == compareTo;
    }

    inline int32_t interlockedDecrement(volatile int32_t* a_ptr)
    {
        return InterlockedDecrement((volatile LONG*) a_ptr);
    }

    inline int32_t interlockedIncrement(volatile int32_t* a_ptr)
    {
        return InterlockedIncrement((volatile LONG*) a_ptr);
    }
}
