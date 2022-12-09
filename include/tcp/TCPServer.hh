/* tcp/TCPServer.hh -- TCP Server implementation */

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

#ifndef IZUMO_TCP_TCPSERVER_HH_
#define IZUMO_TCP_TCPSERVER_HH_

#include <async/Async.hh>
#include <tcp/TCPStream.hh>

#include <memory>

namespace izumo::tcp {

using async::Async;

class TCPAcceptorBackendIfce {
  public:
    virtual ~TCPAcceptorBackendIfce() = default;

    virtual Async<std::unique_ptr<TCPStreamBackendIfce>> accept() = 0;
};

class TCPAcceptor {
  private:
    std::unique_ptr<TCPAcceptorBackendIfce> m_backend = nullptr;

  public:
    TCPAcceptor(std::unique_ptr<TCPAcceptorBackendIfce> backend)
        : m_backend(std::move(backend))
    {
    }
    TCPAcceptor(TCPAcceptor&& rhs) { std::swap(m_backend, rhs.m_backend); }

    auto&
    operator=(TCPAcceptor&& rhs)
    {
        m_backend = std::exchange(rhs.m_backend, nullptr);
        return *this;
    }

    Async<TCPStream> accept();
};

class TCPServer {
  private:
    TCPAcceptor m_acceptor;

  protected:
    virtual Async<void> handleRequest(TCPStream stream) = 0;

  public:
    TCPServer(TCPAcceptor acceptor) : m_acceptor(std::move(acceptor)) {}
    virtual ~TCPServer() = default;

    Async<void> start();
};

} // namespace izumo::tcp

#endif /* IZUMO_TCP_TCPSERVER_HH_ */
