/*
    Copyright (c) 2008-2011 Xeatheran Minexew

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

namespace li
{
    template<typename T, typename TCapacity = size_t, class IAllocator = Allocator<T> >
    class Array
    {
        T* data;
        TCapacity capacity;

        public:
            Array( TCapacity initialCapacity = 0 );
            Array( Array<T>&& other );
            Array( const Array<T>& other );
            Array<T>& operator = ( const Array<T>& );
            virtual ~Array();

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

#define __li_member( type ) template<typename T, typename TCapacity, class IAllocator> type Array<T, TCapacity, IAllocator>::
#define __li_member_ template<typename T, typename TCapacity, class IAllocator> Array<T, TCapacity, IAllocator>::

    __li_member_ Array( TCapacity initialCapacity ) : data( 0 ), capacity( initialCapacity )
    {
        data = ( T* )IAllocator::allocate( capacity );

        for ( unsigned i = 0; i < capacity; i++ )
            constructPointer( data + i );
    }

    __li_member_ Array( Array<T>&& other ) : data( other.data ), capacity( other.capacity )
    {
        other.data = nullptr;
        other.capacity = 0;
    }

    __li_member_ Array( const Array<T>& other ) : data( 0 ), capacity( 0 )
    {
        load( other.data, other.capacity );
    }

    __li_member( Array<T>& ) operator = ( const Array<T>& other )
    {
        load( other.data, other.capacity );
        return *this;
    }

    __li_member_ ~Array()
    {
        for ( unsigned i = 0; i < capacity; i++ )
            destructPointer( data + i );

        IAllocator::release( data );
    }

    __li_member( T* ) detachData()
    {
        T* ptr = data;

        data = nullptr;

        return ptr;
    }

    __li_member( T& ) get( TCapacity field )
    {
        if ( field >= capacity )
            resize( field * 2 + 1 );

        return data[field];
    }

    __li_member( T* ) getPtr( TCapacity field )
    {
        if ( field >= capacity )
            resize( field * 2 + 1 );

        return data + field;
    }

    __li_member( const T& ) get( TCapacity field ) const
    {
        if ( field >= capacity )
            throwException("li.Array.get(const)", "IndexOutOfBounds", "Specified index is outside array bounds");

        return data[field];
    }

    __li_member( void ) load( const T* source, TCapacity count, TCapacity offset )
    {
        resize( offset + count );
        IAllocator::move( data + offset, source, count );
    }

    __li_member( void ) move( TCapacity destField, TCapacity srcField, TCapacity length )
    {
        if ( destField == srcField )
            return;

        // Clip the values to the array
        if ( maximum( destField + length, srcField + length ) > capacity )
            resize( maximum( destField + length, srcField + length ) );

        // Safely release all pointers in the destination area
        if ( destField < srcField )
            for ( TCapacity i = destField; i < destField + length && i < srcField; i++ )
                destructPointer( data + i );
        else
            for ( TCapacity i = destField + length; i > destField && i > srcField + length; i-- )
                destructPointer( data + i );

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

    __li_member( void ) resize( TCapacity newCapacity, bool lazy )
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

    __li_member( void ) set( TCapacity field, const T& value )
    {
        if ( field >= capacity )
            resize( field + 1 );

        data[field] = value;
    }

#undef __li_member
#undef __li_member_
}
