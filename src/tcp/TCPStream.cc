/* tcp/TCPStream.cc -- TCP Stream implementation */

/*
 * Copyright 2022 Youkou Tenhouin <youkou@tenhou.in>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <tcp/TCPStream.hh>

namespace izumo::tcp {

Async<util::ByteArrayView>
TCPStream::readAtMost(util::ByteArrayView buf)
{
    std::size_t bytesRead = co_await m_backend->read(buf);
    co_return buf.subView(bytesRead);
}

Async<util::ByteArrayView>
TCPStream::readExact(util::ByteArrayView buf)
{
    while (buf.size()) {
	std::size_t bytesRead = co_await m_backend->read(buf);
	buf = buf.subView(bytesRead, buf.size());
    }

    co_return buf;
}

Async<std::size_t>
TCPStream::sendAtMost(util::ByteArrayView buf)
{
    return m_backend->write(buf);
}

Async<void>
TCPStream::sendAll(util::ByteArrayView buf)
{
    while (buf.size()) {
	std::size_t bytesSent = co_await m_backend->write(buf);
	buf = buf.subView(bytesSent, buf.size());
    }

    co_return;
}


} // namespace izumo::tcp
