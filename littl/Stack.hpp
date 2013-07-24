#pragma once

#include <littl/Array.hpp>
#include <littl/Exception.hpp>

namespace li
{
#define __li_base Array<T, THeight, IAllocator>

    template<typename T, typename THeight = size_t, class IAllocator = Allocator<T> >
    class Stack : protected __li_base
    {
        THeight height;

        public:
            Stack( THeight initial = 5 )
                : Array<T, THeight, IAllocator>( initial ), height( 0 )
            {
            }

            void clear()
            {
                __li_base::resize( 4 );
                height = 0;
            }

            void deleteAllItems()
            {
                while ( !isEmpty() )
                    delete pop();
            }

            const THeight getHeight() const
            {
                return height;
            }

            bool isEmpty() const
            {
                return height == 0;
            }

            void push( T item )
            {
                __li_base::get( height++ ) = item;
            }

            T& pushEmpty()
            {
                return __li_base::get( height++ );
            }

            T& pop()
            {
                if ( height == 0 )
                    throw Exception( "li.Stack.pop", "littl.StackUnderflowException", "Stack underflow in li.Stack.pop()" );
                else
                    return __li_base::get( --height );
            }

            T* popPtr()
            {
                if ( height == 0 )
                    throw Exception( "li.Stack.popPtr", "littl.StackUnderflowException", "Stack underflow in li.Stack.popPtr()" );
                else
                    return __li_base::getPtr( --height );
            }

            T& top()
            {
                if ( height == 0 )
                    throw Exception( "li.Stack.top", "littl.StackUnderflowException", "Stack underflow in li.Stack.top()" );
                else
                    return __li_base::get( height - 1 );
            }

            const T& top() const
            {
                if ( height == 0 )
                    throw Exception( "li.Stack.top", "littl.StackUnderflowException", "Stack underflow in const li.Stack.top()" );
                else
                    return __li_base::get( height - 1 );
            }
    };
}
