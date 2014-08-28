/*
    Boost Software License - Version 1.0 - August 17, 2003

    Permission is hereby granted, free of charge, to any person or organization
    obtaining a copy of the software and accompanying documentation covered by
    this license (the "Software") to use, reproduce, display, distribute,
    execute, and transmit the Software, and to prepare derivative works of the
    Software, and to permit third-parties to whom the Software is furnished to
    do so, all subject to the following:

    The copyright notices in the Software and this entire statement, including
    the above license grant, this restriction and the following disclaimer,
    must be included in all copies of the Software, in whole or in part, and
    all derivative works of the Software, unless such copies or derivative
    works are solely in the form of machine-executable object code generated by
    a source language processor.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
    SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
    FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
    ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "ArrayIOStreamWithSRW.hpp"

#include <littl/TcpSocket.hpp>

#include <reflection/api.hpp>
#include <reflection/rpc.hpp>

#include <memory>

namespace li {
namespace tcp_rpc_client {
std::unique_ptr<TcpSocket> socket_(TcpSocket::create());
ArrayIOStreamWithSRW bufferIn, bufferOut;
bool rpcReturnsValue;
}

bool tcpRpcConnect(const char* hostname, int port) {
    using namespace li::tcp_rpc_client;

	return socket_->connect(hostname, port, true);
}
}

namespace rpc {
bool beginRPC(const char* functionName, bool returnsValue, IWriter*& writer_out, IReader*& reader_out) {
    using namespace li::tcp_rpc_client;

	assert(reflection::reflectSerialize(std::string(functionName), &bufferOut));
    rpcReturnsValue = returnsValue;

	writer_out = &bufferOut;
	reader_out = &bufferIn;

    return true;
}

bool invokeRPC() {
	using namespace li::tcp_rpc_client;

	assert(socket_->send(bufferOut));

    if (rpcReturnsValue)
	   assert(socket_->receive(bufferIn, li::Timeout()));

    return true;
}

void endRPC() {
    using namespace li::tcp_rpc_client;

	bufferIn.clear(true);
	bufferOut.clear(true);
}
}