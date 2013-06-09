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

// TODO: The validity of accessing items where `index >= length` should be considered

#pragma once

#include <littl/Array.hpp>

#define iterate2( iter_, list_ ) for ( auto iter_ = ( list_ ).getIterator(); iter_.isValid(); ++iter_ )
#define reverse_iterate2( iter_, list_ ) for ( auto iter_ = ( list_ ).getIterator( true ); iter_.isValid(); --iter_ )

namespace li
{
#define li_base Array<T, TLength, IAllocator, options>
#define li_this List<T, TLength, IAllocator, options>

    template<typename T, typename TLength = size_t, class IAllocator = Allocator<T>, int options = 0>
    class List : public li_base
    {
        protected:
        public:
            TLength length;

            class Iterator
            {
                li_this& list;
                intptr_t i;

                Iterator& operator = ( const Iterator& );

                public:
                    Iterator( li_this& list, intptr_t i ) : list( list ), i( i ) {}

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
                const li_this& list;
                intptr_t i;

                public:
                    ConstIterator( const li_this& list ) : list( list ), i( 0 ) {}

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
                : li_base( initial ), length( 0 )
            {
            }

            List( li_this&& other )
                : li_base( ( li_base&& ) other ), length( other.length )
            {
            }

            List( const List<T>& other )
                : li_base( other.getLength() ), length( 0 )
            {
                load( other );
            }

            TLength add( T&& item )
            {
                this->resize( length + 1, true );
                this->getUnsafe( length ) = ( T&& ) item;
                return length++;
            }

            TLength add( const T& item )
            {
                this->resize( length + 1, true );
                this->getUnsafe( length ) = item;
                return length++;
            }

            T& addEmpty()
            {
                this->resize( length + 1, true );
                return this->get( length++ );
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

            void deleteAllItems()
            {
                for ( TLength i = 0; i < length; i++ )
                    delete this->getUnsafe( i );

                setLength( 0, false );
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

            TLength findZero() const
            {
                TLength i;

                for ( i = 0; i < length; i++ )
                    if ( this->getUnsafe( i ) == 0 )
                        break;

                return i;
            }

            T& getFromEnd( TLength field = 0 )
            {
                // FIXME: Protection against negative calculated field
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
                    this->getUnsafe( field ) = item;
                    length++;
                    return field;
                }
                else
                    return add( item );
            }

            T& insertEmpty( TLength field )
            {
                if ( field < length )
                {
                    this->move( field + 1, field, length - field );
                    length++;
                    return this->getUnsafe( field );
                }
                else
                    return addEmpty();
            }

            bool isEmpty() const
            {
                return length == 0;
            }

            void load( const T* source, TLength count, TLength offset = 0 )
            {
                if ( count )
                {
                    li_base::load( source, count, offset );
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

            void setLength( size_t length, bool lazy = false )
            {
                this->resize(length, lazy);
                this->length = length;
            }

            void setLengthUnsafe( size_t length )
            {
                this->length = length;
            }

    	    li_this& operator = ( const li_this& other )
    	    {
                // TODO: Do this more efficiently

    	        clear();

    	        for ( unsigned i = 0; i < other.getLength(); i++ )
    	            add( other[i] );

                return *this;
    	    }

            li_this& operator = ( li_this&& other )
    	    {
                li_base::operator = ( ( li_base&& ) other );

                length = other.length;
                other.length = 0;

                return *this;
    	    }
    };

#undef li_base
#undef li_this

#define li_base List<Reference<T>, TLength, IAllocator, options>

    template<typename T, typename TLength = size_t, class IAllocator = Allocator<Reference<T> >, int options = 0>
    class ReferenceList : public li_base
    {
        public:
            TLength add( T* item )
            {
                this->resize( this->length + 1, true );
                this->getUnsafe( this->length ) = item;
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

#undef li_base

#define li_base List<T, TLength, IAllocator, options | ArrayOptions::noBoundsChecking>
#define li_this UncheckedList<T, TLength, IAllocator, options>

    template<typename T, typename TLength = size_t, class IAllocator = Allocator<T>, int options = 0>
    class UncheckedList : public li_base
    {
        public:
            li_this& operator = ( li_this&& other )
    	    {
                li_base::operator = ( ( li_base&& ) other );

                return *this;
    	    }
    };

#undef li_base
#undef li_this
}
