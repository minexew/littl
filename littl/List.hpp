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

#include <littl/Array.hpp>

#define each_in_list( list_, iter_ ) ( size_t iter_ = 0; iter_ < ( list_ ).getLength(); iter_++ )
#define each_in_list_ptr( list_, iter_ ) ( size_t iter_ = 0; ( list_ ) && iter_ < ( list_ )->getLength(); iter_++ )
#define reverse_each( list_, iter_ ) ( intptr_t iter_ = ( list_ ).getLength() - 1; iter_ >= 0; iter_-- )

#define iterate( list_ ) for ( ( list_ ).begin(); ( list_ ).iter() < ( int )( list_ ).iterableGetLength(); ( list_ ).next() )
#define reverse_iterate( list_ ) for ( ( list_ ).end(); ( list_ ).iter() >= 0; ( list_ ).prev() )

#define li_List_getLength_safe( list_ ) ( ( list_ ) ? ( list_ )->getLength() : 0 )

#define iterate2( iter_, list_ ) for ( auto iter_ = ( list_ ).getIterator(); iter_.isValid(); ++iter_ )
#define reverse_iterate2( iter_, list_ ) for ( auto iter_ = ( list_ ).getIterator( true ); iter_.isValid(); --iter_ )

namespace li
{
#define __li_base Array<T, TLength, IAllocator>

    template <typename T, typename TLength> class Iterable
    {
        intptr_t iterator;

        public:
            virtual TLength iterableGetLength() const = 0;
            virtual T iterableGetItem( TLength index ) = 0;

            void begin() { iterator = 0; }
    	    void end() { iterator = iterableGetLength() - 1; }
    	    void prev() { --iterator; }
    	    void next() { ++iterator; }
    	    intptr_t iter() const { return iterator; }
    	    T current() { return iterableGetItem( iterator ); }
    };

    template<typename T, typename TLength = size_t, class IAllocator = Allocator<T> >
    class List : public __li_base, public Iterable<T&, TLength>
    {
        protected:
        public:
            TLength length;

            class Iterator
            {
                List<T, TLength, IAllocator>& list;
                intptr_t i;

                public:
                    Iterator( List<T, TLength, IAllocator>& list, intptr_t i ) : list( list ), i( i ) {}

                    operator T&() { return list.getUnsafe(i); }
                    T& operator -> () { return list.getUnsafe(i); }
                    T& operator * () { return list.getUnsafe(i); }
                    void operator ++() { i++; }
                    void operator --() { i--; }

                    size_t getIndex() const { return i; }
                    bool isValid() const { return i >= 0 && ( size_t ) i < list.getLength(); }
                    void remove() { list.remove(i--); }

                    Iterator& operator = ( T&& value )
                    {
                        list[i] = value;
                        return *this;
                    }
            };

            class ConstIterator
            {
                const List<T, TLength, IAllocator>& list;
                intptr_t i;

                public:
                    ConstIterator( const List<T, TLength, IAllocator>& list ) : list( list ), i( 0 ) {}

                    operator const T&() { return list[i]; }
                    const T& operator -> () { return list[i]; }
                    const T& operator * () { return list[i]; }
                    void operator ++() { i++; }
                    void operator --() { i--; }

                    size_t getIndex() const { return i; }
                    bool isValid() const { return i >= 0 && ( size_t ) i < list.getLength(); }
            };

        public:
            List( TLength initial = 5 )
                : __li_base( initial ), length( 0 )
            {
            }

            List( List<T>&& other )
                : __li_base( ( __li_base&& ) other ), length( other.length )
            {
            }

            List( const List<T>& other )
                : __li_base( other.getLength() ), length( 0 )
            {
                load( other );
            }

            TLength add( T&& item )
            {
                this->get( length ) = ( T&& ) item;
                return length++;
            }

            TLength add( const T& item )
            {
                this->get( length ) = item;
                return length++;
            }

            TLength addUnsafe( const T& item )
            {
                this->getUnsafe( length ) = item;
                return length++;
            }

            void clear( bool lazy = false )
            {
                this->resize( 0, lazy );
                length = 0;
            }

            template <typename T2>
            intptr_t find( const T2& value )
            {
                TLength i;

                for ( i = 0; i < length; i++ )
                    if ( this->getUnsafe( i ) == value )
                        return i;

                return -1;
            }

            TLength findEmpty() const
            {
                TLength i;

                for ( i = 0; i < length; i++ )
                    if ( this->getUnsafe( i ) == 0 )
                        break;

                return i;
            }

            T& getFromEnd( TLength field = 0 )
            {
                return this->get( length - field - 1 );
            }

            Iterator getIterator( bool reverse = false )
            {
                return Iterator( *this, reverse ? length - 1 : 0 );
            }

            ConstIterator getIterator() const
            {
                return ConstIterator( *this );
            }

            TLength getLength() const
            {
                return length;
            }

            TLength insert( const T& item, TLength field )
            {
                if ( field < length )
                {
                    this->move( field + 1, field, length - field );
                    this->get( field ) = item;
                    length++;
                    return field;
                }
                else
                    return add( item );
            }

            bool isEmpty() const
            {
                return length == 0;
            }

            virtual TLength iterableGetLength() const
            {
                return getLength();
            }

            virtual T& iterableGetItem( TLength index )
            {
                return __li_base::get( index );
            }

            void load( const T* source, TLength count, TLength offset = 0 )
            {
                if ( count )
                {
                    __li_base::load( source, count, offset );
                    length = maximum( length, offset + count );
                }
            }

            void load( const List<T>& source, TLength offset = 0 )
            {
                load( source.getPtrUnsafe(), source.getLength(), offset );
            }

            void remove( TLength field, TLength count = 1 )
            {
                if ( field + count <= length )
                {
                    this->move( field, field + count, length - field - count );
                    length -= count;
                }
            }

            void removeFromEnd( TLength field, TLength count = 1 )
            {
                if ( field + count <= length )
                    remove( length - field - count, count );
            }

            bool removeItem( const T& item )
            {
                for ( TLength i = 0; i < length; i++ )
                    if ( this->getUnsafe( i ) == item )
                    {
                        remove( i );
                        return true;
                    }

                return false;
            }

    	    List<T>& operator = ( const List<T>& other )
    	    {
    	        clear();

    	        for ( unsigned i = 0; i < other.getLength(); i++ )
    	            add( other[i] );

                return *this;
    	    }
    };

    #undef __li_base

    #define __li_base List<Reference<T>, TLength, IAllocator>

    template<typename T, typename TLength = size_t, class IAllocator = Allocator<Reference<T> > >
    class ReferenceList : public __li_base
    {
        public:
            TLength add( T* item )
            {
                this->get( this->length ) = item;
                return this->length++;
            }

            TLength insert( T* item, TLength field )
            {
                if ( field < this->length )
                {
                    this->move( field + 1, field, this->length - field );
                    this->get( field ) = item;
                    this->length++;
                    return field;
                }
                else
                    return add( item );
            }

            bool removeItem( T* item )
            {
                for ( TLength i = 0; i < this->length; i++ )
                    if ( this->getUnsafe( i ) == item )
                    {
                        this->remove( i );
                        return true;
                    }

                return false;
            }
    };

    #undef __li_base
}
