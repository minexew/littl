/*
    Copyright (C) 2011, 2012, 2013 Xeatheran Minexew

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

#include <littl/Allocator.hpp>

#include <cinttypes>
#include <utility>

namespace li
{
#define li_this HashMap<Key, Value, Hash, getHash, Size>

    template<typename Key, typename Value, typename Hash = size_t, Hash ( *getHash )( const Key& ) = Key::getHash, typename Size = uint32_t>
    class HashMap
    {
        protected:
            struct Pair;

            class Iterator
            {
                HashMap<Key, Value, Hash, getHash, Size>& hashMap;
                Size bucket, i;

                void adjust()
                {
                    while ( bucket < hashMap.numBuckets && i >= hashMap.buckets[bucket].numEntries )
                    {
                        i = 0;
                        bucket++;
                    }
                }

                Iterator operator = ( const Iterator& );

                public:
                    Iterator( HashMap<Key, Value, Hash, getHash, Size>& hashMap ) : hashMap( hashMap ), bucket( 0 ), i( 0 )
                    {
                        adjust();
                    }

                    operator Pair&() { return hashMap.buckets[bucket].entries[i]; }
                    Value& operator -> () { return hashMap.buckets[bucket].entries[i].value; }
                    Pair& operator * () { return hashMap.buckets[bucket].entries[i]; }
                    void operator ++() { i++; adjust(); }

                    bool isValid() const { return bucket < hashMap.numBuckets && i < hashMap.buckets[bucket].numEntries; }
            };

            struct Pair
            {
                Key key;
                Value value;
                Hash hash;
            };

            struct Bucket
            {
                Pair* entries;
                Size numEntries, capacity;
            };

            Bucket* buckets;
            Size numBuckets, numEntries, shiftAmount;

            Pair* findPair( const Key& key );
            bool getOrSetWithoutInitialization( Key&& key, Pair*& pair_out );

            static void releaseBuckets( Bucket* buckets, Size numBuckets );

            HashMap( const HashMap<Key, Value, Hash, getHash, Size>& other );
            HashMap<Key, Value, Hash, getHash, Size>& operator =( const HashMap<Key, Value, Hash, getHash, Size>& other );

        public:
            HashMap( Size shiftAmount = 2 );
            HashMap( HashMap<Key, Value, Hash, getHash, Size>&& other );
            ~HashMap();
            HashMap<Key, Value, Hash, getHash, Size>& operator =( HashMap<Key, Value, Hash, getHash, Size>&& other );

            void clear();
            Value* find( const Key& key );
            Value get( const Key& key ) const;
            Iterator getIterator() { return Iterator( *this ); }
            void printStatistics();
            void resize( Size shiftAmount, bool lazy = false );
            Value& setEmpty( Key&& key );
            Value& set( Key&& key, Value&& value );
            bool unset( Key&& key );
    };

#define li_member( type ) template<typename Key, typename Value, typename Hash, Hash ( *getHash )( const Key& ), typename Size> type li_this::
#define li_member_ template<typename Key, typename Value, typename Hash, Hash ( *getHash )( const Key& ), typename Size> li_this::

    li_member_ HashMap( Size shiftAmount )
            : buckets( nullptr ), numBuckets( 0 ), numEntries( 0 ), shiftAmount( 0 )
    {
        resize( shiftAmount );
    }

    li_member_ HashMap( HashMap<Key, Value, Hash, getHash, Size>&& other )
            : buckets( other.buckets ), numBuckets( other.numBuckets ), numEntries( other.numEntries ), shiftAmount( other.shiftAmount )
    {
        other.buckets = nullptr;
        other.numBuckets = 0;
        other.numEntries = 0;
        other.shiftAmount = 0;
    }

    li_member_ ~HashMap()
    {
        clear();
    }

    template<typename Key, typename Value, typename Hash, Hash ( *getHash )( const Key& ), typename Size>
    HashMap<Key, Value, Hash, getHash, Size>& li_this::operator =( HashMap<Key, Value, Hash, getHash, Size>&& other )
    {
        clear();

        buckets = other.buckets;
        numBuckets = other.numBuckets;
        numEntries = other.numEntries;
        shiftAmount = other.shiftAmount;

        other.buckets = nullptr;
        other.numBuckets = 0;
        other.numEntries = 0;
        other.shiftAmount = 0;

        return *this;
    }

    li_member( void ) clear()
    {
        releaseBuckets( buckets, numBuckets );

        buckets = nullptr;
        numBuckets = 0;
        numEntries = 0;
        shiftAmount = 0;
    }

    li_member( Value* ) find( const Key& key )
    {
        Pair* pair = findPair( key );

        if ( pair != nullptr )
            return &pair->value;
        else
            return nullptr;
    }

    li_member( typename li_this::Pair* ) findPair( const Key& key )
    {
        Hash hash = getHash( key );
        Bucket& bucket = buckets[hash & ( numBuckets - 1 )];

        for ( Size i = 0; i < bucket.numEntries; i++ )
            if ( bucket.entries[i].hash == hash && bucket.entries[i].key == key )
                return &bucket.entries[i];

        return nullptr;
    }

    li_member( Value ) get( const Key& key ) const
    {
        Hash hash = getHash( key );
        Bucket& bucket = buckets[hash & ( numBuckets - 1 )];

        for ( Size i = 0; i < bucket.numEntries; i++ )
            if ( bucket.entries[i].hash == hash && bucket.entries[i].key == key )
                return bucket.entries[i].value;

        return 0;
    }

    li_member( bool ) getOrSetWithoutInitialization( Key&& key, Pair*& pair_out )
    {
        //printf( "HashMap.set(): %u entries, %u buckets (%u/%u)\n", numEntries, numBuckets, numEntries, numBuckets * 4 );

        pair_out = findPair( key );

        if ( pair_out != nullptr )
            return false;

        if ( numEntries + 1 > numBuckets * 4 )
            resize( shiftAmount + 2, false );

        Hash hash = getHash( key );
        Bucket& bucket = buckets[hash & ( numBuckets - 1 )];

        //printf( "HashMap.add(): Hash = %08X, bucket = %u\n", hash, hash & ( numBuckets - 1 ) );

        if ( bucket.numEntries + 1 >= bucket.capacity )
        {
            Size newCapacity = ( bucket.capacity == 0 ) ? 2 : bucket.capacity * 2;

            //printf( "HashMap.add(): resizing bucket %u -> %u\n", bucket.capacity, newCapacity );

            bucket.entries = Allocator<Pair>::resize( bucket.entries, newCapacity );
            bucket.capacity = newCapacity;
        }

        bucket.numEntries++;
        numEntries++;

        constructPointer( &bucket.entries[bucket.numEntries - 1].key, std::forward<Key>( key ) );
        constructPointer( &bucket.entries[bucket.numEntries - 1].hash, std::forward<Hash>( hash ) );

        pair_out = &bucket.entries[bucket.numEntries - 1];
        return true;
    }

    li_member( void ) printStatistics()
    {
        printf( "HashMap: %" PRIuPTR " buckets (shift = %" PRIuPTR "); %" PRIuPTR " entries\n", ( size_t ) this->numBuckets, ( size_t ) this->shiftAmount, ( size_t ) this->numEntries );

        for ( Size i = 0; i < numBuckets; i++ )
        {
            const Bucket& bucket = buckets[i];

            printf( "  - bucket #%04X (%" PRIuPTR "/%" PRIuPTR " entries)\n", ( uint16_t ) i, ( size_t ) bucket.numEntries, ( size_t ) bucket.capacity );

            for ( Size j = 0; j < bucket.numEntries; j++ )
                printf( "      - entry #%" PRIuPTR " (hash = %08X)\n", ( size_t ) j, bucket.entries[j].hash );
        }
    }

    li_member( void ) releaseBuckets( Bucket* buckets, Size numBuckets )
    {
        for ( Size i = 0; i < numBuckets; i++ )
        {
            Bucket& bucket = buckets[i];

            for ( Size j = 0; j < bucket.numEntries; j++ )
            {
                destructPointer( &bucket.entries[j].key );
                destructPointer( &bucket.entries[j].value );
                destructPointer( &bucket.entries[j].hash );
            }

            Allocator<Pair>::release( bucket.entries );
        }

        Allocator<Bucket>::release( buckets );
    }

    li_member( void ) resize( Size shiftAmount, bool lazy )
    {
        if ( shiftAmount == this->shiftAmount )
            return;

        Size numBuckets = ( 1 << shiftAmount );

        if ( lazy && numBuckets < this->numBuckets )
            return;

        if ( buckets == nullptr )
        {
            buckets = Allocator<Bucket>::allocate( numBuckets );

            for ( Size i = 0; i < numBuckets; i++ )
            {
                buckets[i].entries = nullptr;
                buckets[i].numEntries = 0;
                buckets[i].capacity = 0;
            }

            this->numBuckets = numBuckets;
            this->shiftAmount = shiftAmount;
        }
        else
        {
            // Allocate new bucket array
            Bucket* buckets = Allocator<Bucket>::allocate( numBuckets );

            // Init the new bucket array (we're starting from scratch)
            for ( Size i = 0; i < numBuckets; i++ )
            {
                buckets[i].entries = nullptr;
                buckets[i].numEntries = 0;
                buckets[i].capacity = 0;
            }

            // Now re-insert the original entries using the already computed hashes
            for ( Size i = 0; i < this->numBuckets; i++ )
            {
                Bucket& sourceBucket = this->buckets[i];

                for ( Size j = 0; j < sourceBucket.numEntries; j++ )
                {
                    Hash hash = sourceBucket.entries[j].hash;
                    Bucket& bucket = buckets[hash & ( numBuckets - 1 )];

                    if ( bucket.numEntries + 1 >= bucket.capacity )
                    {
                        Size newCapacity = ( bucket.capacity == 0 ) ? 2 : bucket.capacity * 2;
                        bucket.entries = Allocator<Pair>::resize( bucket.entries, newCapacity );
                        bucket.capacity = newCapacity;
                    }

                    constructPointer( &bucket.entries[bucket.numEntries].key,   std::move( sourceBucket.entries[j].key ) );
                    constructPointer( &bucket.entries[bucket.numEntries].value, std::move( sourceBucket.entries[j].value ) );
                    constructPointer( &bucket.entries[bucket.numEntries].hash,  std::move( sourceBucket.entries[j].hash ) );

                    bucket.numEntries++;
                }
            }

            releaseBuckets( this->buckets, this->numBuckets );

            this->buckets = buckets;
            this->numBuckets = numBuckets;
            this->shiftAmount = shiftAmount;
        }
    }

    li_member( Value& ) set( Key&& key, Value&& value )
    {
        Pair* pair;

        if ( !getOrSetWithoutInitialization( std::forward<Key>( key ), pair ) )
        {
            pair->value = std::move( value );
            return pair->value;
        }

        constructPointer( &pair->value, std::move( value ) );
        return pair->value;
    }

    li_member( Value& ) setEmpty( Key&& key )
    {
        Pair* pair;
        
        if ( !getOrSetWithoutInitialization( std::forward<Key>( key ), pair ) )
            return pair->value;

        constructPointer( &pair->value );
        return pair->value;
    }

    li_member( bool ) unset( Key&& key )
    {
        Hash hash = getHash( key );
        Bucket& bucket = buckets[hash & ( numBuckets - 1 )];
        
        for ( Size i = 0; i < bucket.numEntries; i++ )
            if ( bucket.entries[i].hash == hash && bucket.entries[i].key == key )
            {
                destructPointer( &bucket.entries[i].key );
                destructPointer( &bucket.entries[i].value );
                destructPointer( &bucket.entries[i].hash );
                
                memmove(&bucket.entries[i], &bucket.entries[i + 1], (bucket.numEntries - i - 1) * sizeof(Pair));
                
                bucket.numEntries--;
                numEntries--;
                
                return true;
            }
        
        return false;
    }
    
#undef li_member
#undef li_member_

#undef li_this
}
