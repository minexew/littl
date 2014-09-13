/*
    Copyright (c) 2014 Xeatheran Minexew

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

#include <littl/Stream.hpp>
#include <bleb/byteio.hpp>

namespace li
{
    class InputStreamByteIO : public bleb::ByteIO
    {
        public:
            InputStreamByteIO(InputStream* is = nullptr) : is(is) {}

            void set(InputStream* is) { this->is = is; }

            virtual void close() override {}
            virtual uint64_t getSize() override { return is->getSize(); }

            virtual bool getBytesAt(uint64_t pos, uint8_t* buffer, size_t count) override
            {
                return is->setPos(pos) && is->read(buffer, count) == count;
            }

            virtual bool setBytesAt(uint64_t pos, const uint8_t* buffer, size_t count) override
            {
                return false;
            }

            virtual bool clearBytesAt(uint64_t pos, uint64_t count) override
            {
                return false;
            }

        private:
            InputStream* is;
    };

    class StreamByteIO : public bleb::ByteIO
    {
        public:
            StreamByteIO(IOStream* ios = nullptr) : ios(ios) {}

            void set(IOStream* ios) { this->ios = ios; }

            virtual void close() override {}
            virtual uint64_t getSize() override { return ios->getSize(); }

            virtual bool getBytesAt(uint64_t pos, uint8_t* buffer, size_t count) override
            {
                return ios->setPos(pos) && ios->read(buffer, count) == count;
            }

            virtual bool setBytesAt(uint64_t pos, const uint8_t* buffer, size_t count) override
            {
                return ios->setPos(pos) && ios->write(buffer, count) == count;
            }

            virtual bool clearBytesAt(uint64_t pos, uint64_t count) override
            {
                static const uint8_t empty[256] = {};

                while (count > sizeof(empty)) {
                    if (!setBytesAt(pos, empty, sizeof(empty)))
                        return false;

                    pos += sizeof(empty);
                    count -= sizeof(empty);
                }

                if (!setBytesAt(pos, empty, (size_t) count))
                    return false;

                return true;
            }

        private:
            IOStream* ios;
    };
}
