/*
    Copyright (c) 2012 Xeatheran Minexew

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

#include <littl/Base.hpp>

#define iterateChain( iter_, chain_ ) for ( auto iter_ = ( chain_ ).getIterator(); iter_.isValid(); ++iter_ )

namespace li
{
    template <class Type>
    struct ChainLink
    {
        Type *prev, *next;

        ChainLink()
        {
            prev = nullptr;
            next = nullptr;
        }
    };

    template <class Type, ChainLink<Type> Type::* chainLink>
    class Chain
    {
        class Iterator
        {
            Chain<Type, chainLink>& chain;
            Type* link;

            public:
                Iterator( Chain<Type, chainLink>& chain ) : chain( chain )
                {
                    link = chain.head;
                }

                operator Type* () { return link; }
                Type* operator -> () { return link; }
                Type& operator * () { return *link; }
                void operator ++() { link = ( link->*chainLink ).next; }
                void operator --() { link = ( link->*chainLink ).prev; }

                bool isValid() const { return link != nullptr; }
        };

        Type *head, *tail;

        public:
            Chain()
            {
                head = nullptr;
                tail = nullptr;
            }
            
            void append( Type* link )
            {
                if ( tail == nullptr )
                    head = link;
                else
                    ( tail->*chainLink ).next = link;

                ( link->*chainLink ).prev = tail;
                ( link->*chainLink ).next = nullptr;
                tail = link;
            }

            Iterator getIterator()
            {
                return Iterator( *this );
            }

            void remove( Type* link )
            {
                Type *prev = ( tail->*chainLink ).prev, *next = ( tail->*chainLink ).next;

                if ( prev != nullptr )
                    ( prev->*chainLink ).next = next;
                else
                    head = next;

                if ( next != nullptr )
                    ( tail->*chainLink ).prev = prev;
                else
                    tail = prev;
            }
    };
}