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

#include <pthread.h>

#ifdef li_Apple
#include <libkern/OSAtomic.h>
#endif

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

        pthread_t thread;
        
        static void* threadStartRoutine(void *userarg)
        {
            ThreadStartInfo* info = (ThreadStartInfo *) userarg;
            
            info->threadFunction(info->param);
            info->launcher->running = false;
            
            if ( info->exitFunction != nullptr )
                info->exitFunction( info->param );

            return NULL;
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
        thread = 0;
    }

    __li_member_ ~ThreadLauncher()
    {
    }

    __li_member( bool ) kill()
    {
        return 0 == pthread_cancel(thread);
    }

    __li_member( bool ) setName( const char* name )
    {
        return false;
    }

    __li_member( void ) setPriority( int priority )
    {
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
        if (0 != pthread_create(&thread, NULL, threadStartRoutine, &info))
        {
            running = false;
            return false;
        }
        else
            return true;
    }

    __li_member( bool ) waitFor( long time )
    {
        return 0 == pthread_join(thread, NULL);
    }

#undef __li_member
#undef __li_member_

    class Thread
    {
        public:
            enum Priority
            {
                lowest,
                belowNormal,
                normal,
                aboveNormal,
                highest
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

    class Mutex
    {
        pthread_mutex_t mutex;

        public:
            Mutex()
            {
                pthread_mutexattr_t mta;
                
                mutex = PTHREAD_MUTEX_INITIALIZER;
                
                pthread_mutexattr_init(&mta);
                pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE);
                
                pthread_mutex_init(&mutex, &mta);
            }

            ~Mutex()
            {
                pthread_mutex_destroy(&mutex);
            }

            void enter()
            {
                pthread_mutex_lock(&mutex);
            }

            void leave()
            {
                pthread_mutex_unlock(&mutex);
            }
    };

    class ConditionVar
    {
        pthread_mutex_t waiting;
        pthread_cond_t cond;

        public:
            ConditionVar()
            {
                waiting = PTHREAD_MUTEX_INITIALIZER;
                cond = PTHREAD_COND_INITIALIZER;
            }

            ~ConditionVar()
            {
            }

            void set()
            {
                pthread_mutex_lock(&waiting);
                pthread_cond_signal(&cond);
                pthread_mutex_unlock(&waiting);
            }

            bool waitFor()
            {
                pthread_mutex_lock(&waiting);
                pthread_cond_wait(&cond, &waiting);
                pthread_mutex_unlock(&waiting);
                return true;
            }
    };

    inline bool interlockedCompareExchange(volatile int32_t* value_ptr, int32_t compareTo, int32_t newValue)
    {
#if defined(li_Apple)
        return OSAtomicCompareAndSwap32(compareTo, newValue, value_ptr);
#else
        return __sync_bool_compare_and_swap(value_ptr, compareTo, newValue);
#endif
    }

    inline int32_t interlockedDecrement(volatile int32_t* a_ptr)
    {
#if defined(li_Apple)
        return OSAtomicDecrement32(a_ptr);
#else
        return __sync_fetch_and_sub(a_ptr, 1);
#endif
    }

    inline int32_t interlockedIncrement(volatile int32_t* a_ptr)
    {
#if defined(li_Apple)
        return OSAtomicIncrement32(a_ptr);
#else
        return __sync_fetch_and_add(a_ptr, 1);
#endif
    }
}
