/*
    Copyright (c) 2008-2013 Xeatheran Minexew

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

#include <littl/Allocator.hpp>

#include <algorithm>

#pragma warning ( push )

// warning C4127: conditional expression is constant
// Reason to ignore: We are using it for compile-time instantiation options
#pragma warning ( disable : 4127 )

namespace li
{
    namespace ArrayOptions
    {
        enum
        {
            noBoundsChecking = 1
        };
    }

#define li_this Array<T, TCapacity, IAllocator, options>

    template<typename T, typename TCapacity = size_t, class IAllocator = Allocator<T>, int options = 0>
    class Array
    {
        T* data;
        TCapacity capacity;

        public:
            Array( TCapacity initialCapacity = 0 );
            Array( li_this&& other );
            Array( const li_this& other );
            li_this& operator = ( const li_this& );
            li_this& operator = ( li_this&& );
            ~Array();

            T* c_array() { return data; }
            const T* c_array() const { return data; }
            T* detachData();
            T& get( TCapacity field );
            T* getPtr( TCapacity field = 0 );
            T* getPtrUnsafe( TCapacity field = 0 ) { return data + field; }
            const T* getPtrUnsafe( TCapacity field = 0 ) const { return data + field; }
            const T& get( TCapacity field ) const;
            size_t getCapacity() const { return capacity; }
            T& getUnsafe( TCapacity field ) { return data[field]; }
            const T& getUnsafe( TCapacity field ) const { return data[field]; }
            void load( const T* source, TCapacity count, TCapacity offset = 0 );
            void move( TCapacity destField, TCapacity srcField, TCapacity length );
            void resize( TCapacity newCapacity, bool lazy = false );
            void set( TCapacity field, const T& value );
            void setUnsafe( TCapacity field, const T& value ) { data[field] = value; }

            T& operator [] ( TCapacity field ) { return get( field ); }
            const T& operator [] ( TCapacity field ) const { return get( field ); }

            T* operator * () { return data; }
    };

#define li_member( type ) template<typename T, typename TCapacity, class IAllocator, int options> type li_this::
#define li_member_ template<typename T, typename TCapacity, class IAllocator, int options> li_this::

    li_member_ Array( TCapacity initialCapacity ) : data( 0 ), capacity( initialCapacity )
    {
        if ( initialCapacity > 0 )
        {
            data = ( T* )IAllocator::allocate( capacity );

            for ( unsigned i = 0; i < capacity; i++ )
                constructPointer( data + i );
        }
        else
            data = nullptr;
    }

    li_member_ Array( li_this&& other ) : data( other.data ), capacity( other.capacity )
    {
        other.data = nullptr;
        other.capacity = 0;
    }

    li_member_ Array( const li_this& other ) : data( 0 ), capacity( 0 )
    {
        load( other.data, other.capacity );
    }

    li_member_ ~Array()
    {
        for ( unsigned i = 0; i < capacity; i++ )
            destructPointer( data + i );

        IAllocator::release( data );
    }

    li_member( li_this& ) operator = ( const li_this& other )
    {
        load( other.data, other.capacity );
        return *this;
    }

    li_member( li_this& ) operator = ( li_this&& other )
    {
        resize( 0, false );

        data = other.data;
        capacity = other.capacity;
        other.data = nullptr;
        other.capacity = 0;

        return *this;
    }

    li_member( T* ) detachData()
    {
        T* ptr = data;

        data = nullptr;
        capacity = 0;

        return ptr;
    }

    li_member( T& ) get( TCapacity field )
    {
        if ( !( options & ArrayOptions::noBoundsChecking ) )
        {
            if ( field >= capacity )
                resize( std::max<TCapacity>( field * 4 / 3, 4 ) );
        }

        return data[field];
    }

    li_member( T* ) getPtr( TCapacity field )
    {
        if ( !( options & ArrayOptions::noBoundsChecking ) )
        {
            if ( field >= capacity )
                resize( std::max<TCapacity>( field * 4 / 3, 4 ) );
        }

        return &data[field];
    }

    li_member( const T& ) get( TCapacity field ) const
    {
        if ( !( options & ArrayOptions::noBoundsChecking ) )
        {
            if ( field >= capacity )
                throwException( "li.Array.get(const)", "IndexOutOfBounds", "Specified index is outside array bounds" );
        }

        return data[field];
    }

    li_member( void ) load( const T* source, TCapacity count, TCapacity offset )
    {
        resize( offset + count );
        IAllocator::move( data + offset, source, count );
    }

    li_member( void ) move( TCapacity destField, TCapacity srcField, TCapacity length )
    {
        if ( destField == srcField )
            return;

        // Clip the values to the array
        if ( std::max( destField + length, srcField + length ) > capacity )
            resize( std::max( destField + length, srcField + length ) );

        // Safely release all pointers in the destination area
        if ( destField < srcField )
            for ( TCapacity i = destField; i < destField + length && i < srcField; i++ )
                destructPointer( data + i );
        else
            for ( TCapacity i = destField + length; i > destField && i > srcField + length; i-- )
                destructPointer( data + i - 1 );

        // Move the data
        memmove( data + destField, data + srcField, length * sizeof( T ) );

        // Construct items in the source area (which aren't valid until this is done)
        if ( destField < srcField )
            for ( TCapacity i = srcField + length; i > srcField && i > destField + length; i-- )
                constructPointer( data + i - 1 );
        else
            for ( TCapacity i = srcField; i < srcField + length && i < destField; i++ )
                constructPointer( data + i );
    }

    li_member( void ) resize( TCapacity newCapacity, bool lazy )
    {
        if ( !lazy || newCapacity > capacity )
        {
            // Release when shrinking (newCapacity < capacity)
            // If data == 0, then capacity == 0 so this is safe.
            for ( TCapacity i = newCapacity; i < capacity; i++ )
                destructPointer( data + i );

            TCapacity oldCapacity = capacity;

            capacity = newCapacity;
            data = IAllocator::resize( data, capacity );

            // Initialize when growing (oldSize < capacity)
            // Again, if data == 0, then capacity == 0 so this is safe.
            for ( TCapacity i = oldCapacity; i < capacity; i++ )
                constructPointer( data + i );
        }
    }

    li_member( void ) set( TCapacity field, const T& value )
    {
        if ( !( options & ArrayOptions::noBoundsChecking ) )
        {
            if ( field >= capacity )
                resize( field + 1 );
        }

        data[field] = value;
    }

#undef li_member
#undef li_member_

#undef li_this
}

#pragma warning ( pop )
