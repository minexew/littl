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

#include <iterator>

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

            template <typename TT, typename This>
            class generic_iterator : public std::iterator<std::input_iterator_tag, TT>
            {
                This& list;
                intptr_t i;

                public:
                    generic_iterator(This& list, intptr_t i) : list(list), i(i) {}

                    TT* operator -> () { return list.getPtrUnsafe( i ); }
                    TT& operator * () { return list.getUnsafe( i ); }
                    void operator ++() { i++; }

                    bool operator == (const generic_iterator<TT, This>& other) const { return &list == &other.list && i == other.i; }
                    bool operator != (const generic_iterator<TT, This>& other) const { return &list != &other.list || i != other.i; }

                    size_t getIndex() const { return i; }
                    bool isValid() const { return i >= 0 && (size_t)i < list.getLength(); }
            };

            typedef generic_iterator<const T, const li_this> const_iterator;
            typedef generic_iterator<T, li_this> iterator;

        public:
            List( TLength initialCapacity = 0 )
                : li_base( initialCapacity ), length( 0 )
            {
            }

            List( li_this&& other )
                : li_base( ( li_base&& ) other ), length( other.length )
            {
                other.length = 0;
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

            T* detachData()
            {
                T* ptr = li_base::detachData();

                length = 0;

                return ptr;
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

            T* getPtrFromEnd2( TLength field )
            {
                return this->getPtr( length - field );
            }

            [[deprecated]] Iterator getIterator( bool reverse = false )
            {
                return Iterator( *this, reverse ? length - 1 : 0 );
            }

            [[deprecated]] ConstIterator getIterator() const
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
                    length = std::max( length, offset + count );
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

            void removeFromEnd2( TLength field, TLength count = 1 )
            {
                if ( field > length )
                    return;

                if ( field > count )
                    count = field;

                remove( length - field, count );
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

            TLength size() const
            {
                return length;
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

            // iterators
            const const_iterator begin() const
            {
                return const_iterator( *this, 0 );
            }

            const const_iterator end() const
            {
                return const_iterator( *this, getLength() );
            }

            const iterator begin()
            {
                return iterator( *this, 0 );
            }

            const iterator end()
            {
                return iterator( *this, getLength() );
            }
    };

#undef li_base
#undef li_this
}
