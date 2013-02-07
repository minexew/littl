/*
    Copyright (c) 2013 Xeatheran Minexew

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

/*
    littl/RawBuffers.hpp:   Low-lever helper classes to manipulate raw memory
                            stream buffers
*/

#pragma once

#include <littl/Base.hpp>

namespace li
{
    template <typename T> class BufferReaderBase
    {
        public:
            const T* pointer;
        
            BufferReaderBase(const T* buffer) : pointer(buffer)
            {
            }

            void read(T* buffer, size_t count)
            {
                memcpy(buffer, pointer, count * sizeof(T));
                pointer += count;
            }

            const T* readCustom(size_t count)
            {
                const T* ret_ptr = pointer;
                pointer += count;
                return ret_ptr;
            }
    };

    template <typename T> class BufferReader : public BufferReaderBase<T>
    {
        public:
            BufferReader(const T* buffer) : BufferReaderBase(buffer)
            {
            }
    };

    template <> class BufferReader<uint8_t> : public BufferReaderBase<uint8_t>
    {
        public:
            BufferReader(const uint8_t* buffer) : BufferReaderBase(buffer)
            {
            }

            using BufferReaderBase<uint8_t>::read;

            template <typename Type> Type read()
            {
                auto p_value = reinterpret_cast<const Type*>(pointer);
                pointer += sizeof(Type);
                return *p_value;
            }

            template <typename Type> const Type* readCustomAs(size_t count)
            {
                const Type* ret_ptr = reinterpret_cast<const Type*>(pointer);
                pointer += count * sizeof(Type);
                return reinterpret_cast<const Type*>(ret_ptr);
            }
    };

    template <typename T> class BufferWriterBase
    {
        public:
            T* pointer;
        
            BufferWriterBase(T* buffer) : pointer(buffer)
            {
            }

            void write(const T* buffer, size_t count)
            {
                memcpy(pointer, buffer, count * sizeof(T));
                pointer += count;
            }

            T* writeCustom(size_t count)
            {
                T* ret_ptr = pointer;
                pointer += count;
                return ret_ptr;
            }
    };

    template <typename T> class BufferWriter : public BufferWriterBase<T>
    {
        public:
            BufferWriter(T* buffer) : BufferWriterBase(buffer)
            {
            }
    };

    template <> class BufferWriter<uint8_t> : public BufferWriterBase<uint8_t>
    {
        public:
            BufferWriter(uint8_t* buffer) : BufferWriterBase(buffer)
            {
            }

            using BufferWriterBase<uint8_t>::write;

            template <typename Type> void write(const Type& value)
            {
                *reinterpret_cast<Type*>(pointer) = value;
                pointer += sizeof(Type);
            }

            template <typename Type> void writeAs(const Type* buffer, size_t count)
            {
                memcpy(pointer, buffer, count * sizeof(Type));
                pointer += count * sizeof(Type);
            }

            template <typename Type> Type* writeCustomAs(size_t count)
            {
                Type* ret_ptr = reinterpret_cast<Type*>(pointer);
                pointer += count * sizeof(Type);
                return reinterpret_cast<Type*>(ret_ptr);
            }
    };
}
