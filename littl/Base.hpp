#pragma once

#ifdef _MSC_VER
// warning C4345: behavior change: an object of POD type constructed with an initializer of the form () will be default-initialized
// Reason to ignore: Spams when there is a li::List of structs without constructor
#pragma warning ( disable : 4345 )

// warning C4521: 'class' : multiple copy constructors specified
// Reason to ignore: why the hell is this even a warning?!
#pragma warning ( disable : 4521 )

// warning C6258: using TerminateThread does not allow proper thread clean up
// Reason to ignore: We know.
#pragma warning ( disable : 6258 )

// VS don't have no cinttypes
#include <inttypes.h>
#elif __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 7)
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#else
#include <cinttypes>
#include <unistd.h>
#endif

#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <stdint.h>
#include <typeinfo>

// basic platform-related stuff
#if ( defined( __WINDOWS__ ) || defined( _WIN32 ) || defined( _WIN64 ) )
// MS Windows
#include <windows.h>

#define __li_MSW
#define li_MSW
#define li_newLine "\r\n"
#define li_stricmp _stricmp
#else
// all POSIX (Linux, OS X)
#include <unistd.h>

#if defined(__APPLE__)
#define li_Apple
#endif

#define __li_POSIX
#define li_newLine "\n"
#define li_stricmp strcasecmp
#endif

// Clang has this
#ifndef __has_feature
#define __has_feature(x) 0
#endif

//--- GCC-specific ---
#ifdef __GNUC__
#if __GNUC__ == 4
#define li_GCC4 1
#define li_functionName __PRETTY_FUNCTION__
#else
#define li_functionName __func__
#endif

#if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 7 )
#if !__has_feature(cxx_nullptr)
#define nullptr __null
#endif

#if !__has_feature(cxx_override_control)
#define override
#endif

using std::ptrdiff_t;
#endif

#define li_enum_class( name_ ) enum class name_
#define li_force_inline( proto_ ) proto_ __attribute__((always_inline))
#define li_noreturn( proto_ ) proto_ __attribute__((noreturn))

#elif defined( _MSC_VER )
//--- Visual C++-specific ---
#define snprintf sprintf_s
#define li_functionName __FUNCTION__
#define li_enum_class( name_ ) enum name_ : unsigned
#define li_force_inline( proto_ ) __forceinline proto_
#define li_noreturn( proto_ ) __declspec( noreturn ) proto_

#elif defined( DOXYGEN )
//--- Doxygen versions ---
#define li_enum_class( name_ ) enum name_
#define li_force_inline( proto_ ) proto_
#define li_noreturn( proto_ ) proto_
#endif

#ifndef lengthof
#define lengthof( array_ ) ( sizeof( array_ ) / sizeof( *( array_ ) ) )
#endif

#define repeat(__n) for ( unsigned __repeat_i = 0; __repeat_i < unsigned(__n); __repeat_i++ )
#define repeat_i( n_, i_ ) for ( unsigned i_ = 0; i_ < unsigned( n_ ); i_++ )

#ifndef li_avoid_libstdcxx
#include <new>
#endif

#define li_ReferencedClass_override( ClassName ) ClassName* reference() { li::ReferencedClass::reference(); return this; }

// Doesn't work in VS2010 nor GCC 4.6, but should in the future
#define li_ReferencedClass_override2 auto reference() -> decltype(this) { li::ReferencedClass::reference(); return this; }

#define li_tryCall(object_, method_) { if ((object_) != nullptr) (object_)->method_; }

namespace li
{
    template<typename Type> inline Type limitTo( Type value, Type min, Type max )
    {
        if ( value < min )
            return min;
        else if ( value > max )
            return max;
        else
            return value;
    }

    template<typename Type> inline Type minimum( Type a, Type b )
    {
        return a < b ? a : b;
    }

    template<typename Type> inline Type minimum( Type a, Type b, Type c )
    {
        return minimum( minimum( a, b ), c );
    }

    template<typename Type> inline Type maximum( Type a, Type b )
    {
        return a > b ? a : b;
    }

    template<typename Type> inline Type maximum( Type a, Type b, Type c )
    {
        return maximum( maximum( a, b ), c );
    }

    template<typename Type> inline Type round( Type value )
    {
        return value < ( Type ) 0.0 ? ceil( value - ( Type ) 0.5 ) : floor( value + ( Type ) 0.5 );
    }

    template<typename Type> inline void constructPointer( Type* p )
    {
        new( static_cast<void*>( p ) ) Type();
    }

    template<typename Type, typename Type2> inline void constructPointer( Type* p, Type2 par )
    {
        new( static_cast<void*>( p ) ) Type( par );
    }

    template<typename Type> inline void destructPointer( Type* p )
    {
        p->~Type();
    }

    class ReferencedClass
    {
        size_t referenceCount;

        protected:
            ReferencedClass() : referenceCount( 1 )
            {
                //puts( "++ REFC" );
            }

            virtual ~ReferencedClass()
            {
                //puts( "-- REFC" );
            }

            void reference()
            {
                referenceCount++;
            }

        public:
            bool hasOtherReferences()
            {
                return referenceCount > 1;
            }

            void release()
            {
                //printf( "~ReferencedClass<%s> %" PRIuPTR " -> %" PRIuPTR "\n", typeid( *this ).name(), referenceCount, referenceCount - 1 );

#ifdef li_MSW
                if ( referenceCount <= 0 )
                {
                    MessageBoxA( 0, "Negative reference count!", "li::ReferencedClass", MB_ICONERROR );
                    *( char* ) 0 = 0;
                }
#endif

                if ( --referenceCount == 0 )
                    delete this;
            }
    };

    template <class T> void destroy( T*& c )
    {
        if ( c != nullptr )
        {
            delete c;
            c = nullptr;
        }
    }

    template <class T> void release( T*& c )
    {
        if ( c != nullptr )
        {
            c->release();
            c = nullptr;
        }
    }

    template <class T = ReferencedClass> class Reference
    {
        protected:
            T* instance;

        private:
            Reference( const Reference<T>& other );

        public:
            Reference( T* instance = 0 ) : instance( instance )
            {
            }

            Reference( Reference<T>& other ) : instance( other->reference() )
            {
            }

            Reference( Reference<T>&& other ) : instance( other.detach() )
            {
            }

            ~Reference()
            {
                release();
            }

            T* detach()
            {
                T* instance = this->instance;
                this->instance = 0;
                return instance;
            }

            void release()
            {
                li::release( instance );
            }

            Reference<T>& operator = ( T* instance )
            {
                release();
                this->instance = instance;

                return *this;
            }

            Reference<T>& operator = ( Reference<T>&& other )
            {
                release();
                this->instance = other.detach();

                return *this;
            }

            T* operator -> () { return instance; }
            operator T* () { return instance; }
            T*& operator * () { return instance; }
    };

    template <class Class>
    class Object
    {
        private:
            Class* ptr;

            Object( const Object<Class>& other );

        public:
            Object( Class* ptr = 0 ) : ptr( ptr )
            {
            }

            Object( Object<Class>&& other ) : ptr( other.detach() )
            {
            }

            ~Object()
            {
                release();
            }

            Class* detach()
            {
                Class* pointer = ptr;
                ptr = 0;
                return pointer;
            }

            void release()
            {
                if ( ptr )
                    delete ptr;

                ptr = 0;
            }

            Object<Class>& operator = ( Object<Class>&& other )
            {
                release();
                ptr = other.detach();

                return *this;
            }

            Object<Class>& operator = ( Class* newPtr )
            {
                release();
                ptr = newPtr;

                return *this;
            }

            operator Class* () { return ptr; }
            operator const Class* () const { return ptr; }

            Class* operator -> () { return ptr; }
            const Class* operator -> () const { return ptr; }
    };

    /*template <class T> class Var
    {
        T value;

        public:
            Var( T value = 0 ) : value( value )
            {
            }

            operator T& ()
            {
                return value;
            }

            operator const T& () const
            {
                return value;
            }

            Var<T>& operator = ( T value )
            {
                this->value = value;

                return *this;
            }

            T operator -> ()
            {
#ifdef li_MSW
                if ( !value )
                    MessageBoxA( 0, "Dereferencing null pointer!", "li::Reference", MB_ICONERROR );
#endif

                return value;
            }

            template <typename T2> T operator + ( const T2& other )
            {
                return value + other;
            }
    };*/

    inline void pauseThread( unsigned milliSeconds )
    {
#ifdef li_MSW
        Sleep( milliSeconds );
#else
        usleep( milliSeconds * 1000 );
#endif
    }

    inline unsigned relativeTime()
    {
#ifdef li_MSW
        return GetTickCount();
#else
        return clock() * 1000 / CLOCKS_PER_SEC;
#endif
    }

    inline int stringCaseCompare( const char* a, const char* b )
    {
#ifdef li_stricmp
        return li_stricmp( a, b );
#else
#error "littl: No case-inpendent string compare (stricmp/strcasecmp) function available"
#endif
    }

    static inline void throwException(const char* functionName, const char* name, const char* description);
}
