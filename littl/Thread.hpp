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
#include <pthread.h>
#endif

#define li_synchronized(mutex_) CriticalSection li_cs_((mutex_));

namespace li
{
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
        
#ifdef __li_MSW
        HANDLE threadHandle;
        DWORD threadId;
        int priority;

        static DWORD threadStartRoutine( ThreadStartInfo* info )
        {
            info->threadFunction( info->param );
            info->launcher->running = false;

            if ( info->exitFunction )
                info->exitFunction( info->param );

            return 0;
        }
#else
        pthread_t thread;
        
        static void* threadStartRoutine(void *userarg)
        {
            ThreadStartInfo* info = (ThreadStartInfo *) userarg;
            
            info->threadFunction(info->param);
            info->launcher->running = false;
            
            return NULL;
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
        : running( false ), threadFunction( run )
    {
#ifdef __li_MSW
        threadHandle = INVALID_HANDLE_VALUE;
        threadId = 0;
        priority = THREAD_PRIORITY_NORMAL;
#else
        thread = 0;
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
#else
        return 0 == pthread_cancel(thread);
#endif
    }

    __li_member( void ) setPriority( int priority )
    {
#ifdef __li_MSW
        this->priority = priority;

        if ( threadHandle != INVALID_HANDLE_VALUE )
            SetThreadPriority( threadHandle, priority );
#endif
    }

    __li_member( bool ) start( ParamType param, void ( *exitFunction )( ParamType ) )
    {
        if ( running )
            return false;

        info.launcher = this;
        info.threadFunction = threadFunction;
        info.exitFunction = exitFunction;
        info.param = param;
        
#ifdef __li_MSW
        running = true;
        threadHandle = CreateThread( 0, 0, ( LPTHREAD_START_ROUTINE )( threadStartRoutine ), &info, 0, &threadId );
        setPriority( priority );
        if ( threadHandle == INVALID_HANDLE_VALUE )
        {
            running = false;
            return false;
        }
        else
            return true;
#else
        running = true;
        if (0 != pthread_create(&thread, NULL, threadStartRoutine, &info))
        {
            running = false;
            return false;
        }
        else
            return true;
#endif
    }

    __li_member( bool ) waitFor( long time )
    {
#ifdef __li_MSW
        if ( threadHandle == INVALID_HANDLE_VALUE )
            return false;
        else
            return !WaitForSingleObject( threadHandle, time < 0 ? INFINITE : time );
#else
        return 0 == pthread_join(thread, NULL);
#endif
    }

#undef __li_member
#undef __li_member_

    class Thread
    {
        public:
#ifdef __li_MSW
            enum Priority
            {
                lowest = THREAD_PRIORITY_LOWEST,
                belowNormal = THREAD_PRIORITY_BELOW_NORMAL,
                normal = THREAD_PRIORITY_NORMAL,
                aboveNormal = THREAD_PRIORITY_ABOVE_NORMAL,
                highest = THREAD_PRIORITY_HIGHEST
            };
#else
            enum Priority
            {
                lowest,
                belowNormal,
                normal,
                aboveNormal,
                highest
            };
#endif

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
#ifdef __li_MSW
        CRITICAL_SECTION criticalSection;
#else
        pthread_mutex_t mutex;
#endif

        public:
            Mutex()
            {
#ifdef __li_MSW
                InitializeCriticalSection( &criticalSection );
#else
                pthread_mutexattr_t mta;
                
                mutex = PTHREAD_MUTEX_INITIALIZER;
                
                pthread_mutexattr_init(&mta);
                pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE);
                
                pthread_mutex_init(&mutex, &mta);
#endif
            }

            ~Mutex()
            {
#ifdef __li_MSW
                DeleteCriticalSection( &criticalSection );
#else
                pthread_mutex_destroy(&mutex);
#endif
            }

            void enter()
            {
#ifdef __li_MSW
                EnterCriticalSection( &criticalSection );
#else
                pthread_mutex_lock(&mutex);
#endif
            }

            void leave()
            {
#ifdef __li_MSW
                LeaveCriticalSection( &criticalSection );
#else
                pthread_mutex_unlock(&mutex);
#endif
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
