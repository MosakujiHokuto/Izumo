/* tcp/TCPStream.hh -- TCP Stream implementation */

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

#ifndef IZUMO_TCP_TCPSTREAM_HH_
#define IZUMO_TCP_TCPSTREAM_HH_

#include <async/Async.hh>
#include <util/ByteArray.hh>

#include <functional>
#include <memory>
#include <optional>

namespace izumo::tcp {

using namespace async;

class TCPStreamBackendIfce {
  public:
    virtual ~TCPStreamBackendIfce() = default;

    virtual async::Async<std::size_t> read(util::ByteArrayView buf) = 0;
    virtual async::Async<std::size_t> write(util::ByteArrayView buf) = 0;
};

class TCPStream {
  private:
    std::unique_ptr<TCPStreamBackendIfce> m_backend = nullptr;

  public:
    TCPStream(std::unique_ptr<TCPStreamBackendIfce> backend)
        : m_backend(std::move(backend))
    {
    }
    TCPStream(const TCPStream&) = delete;
    TCPStream(TCPStream&& rhs) { std::swap(m_backend, rhs.m_backend); }
    ~TCPStream() = default;

    auto&
    operator=(TCPStream&& rhs)
    {
        m_backend = std::exchange(rhs.m_backend, nullptr);
        return *this;
    }

    Async<util::ByteArrayView> readAtMost(util::ByteArrayView buf);
    Async<util::ByteArrayView> readExact(util::ByteArrayView buf);

    Async<std::size_t> sendAtMost(util::ByteArrayView buf);
    Async<void> sendAll(util::ByteArrayView buf);
};

} // namespace izumo::tcp

#endif /* IZUMO_TCP_TCPSTREAM_HH_ */
