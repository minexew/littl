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
    class ByteIOStream : public IOStream
    {
        public:
            ByteIOStream(bleb::ByteIO* bio = nullptr) : bio(bio) { pos = 0; }

            void set(bleb::ByteIO* bio) { this->bio = bio; }

            virtual bool finite() override { return true; }
            virtual bool seekable() override { return true; }

            virtual void flush() {}
            virtual const char* getErrorDesc() { return nullptr; }          // FIXME

            virtual bool eof() override { return pos >= bio->getSize(); }

            virtual uint64_t getPos() override { return pos; }
            virtual uint64_t getSize() override { return bio->getSize(); }
            virtual bool setPos(uint64_t pos) override { this->pos = pos; return true; }

            virtual size_t read(void* out, size_t length) override
            {
                length = (size_t) std::min<uint64_t>(length, bio->getSize());
                size_t got = bio->getBytesAt(pos, (uint8_t*) out, length) ? length : 0;
                pos += got;
                return got;
            }

            virtual size_t write(const void* in, size_t length) override
            {
                size_t written = bio->setBytesAt(pos, (const uint8_t*) in, length) ? length : 0;
                pos += written;
                return written;
            }

        private:
            bleb::ByteIO* bio;
            uint64_t pos;
};
}
