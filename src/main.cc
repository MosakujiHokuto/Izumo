/* main.cc -- Izumo entry */

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

/* XXX temporary implementation */

#include <backends/epoll.hh>
#include <tcp/TCPServer.hh>

using izumo::async::Async;
using izumo::backends::EpollTCPServer;
using izumo::backends::EpollService;
using izumo::tcp::TCPStream;
using izumo::util::ByteArray;

class EchoServer : public EpollTCPServer {
  protected:
    Async<void> handleRequest(TCPStream stream) override;

  public:
    EchoServer(const char* addr, std::uint16_t port)
        : EpollTCPServer(addr, port)
    {
    }
};

Async<void>
EchoServer::handleRequest(TCPStream stream)
{
    ByteArray buffer(4096);

    while (true) {
        auto incoming = co_await stream.readAtMost(buffer.toView());
        co_await stream.sendAll(incoming);
    }
}

int
main()
{
    EchoServer srv("127.0.0.1", 1551);
    srv.start();
    EpollService::instance().start();
}
