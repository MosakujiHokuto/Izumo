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
#include <http/HTTPHeaders.hh>
#include <tcp/TCPServer.hh>

#include <cstring>
#include <iostream>

using izumo::async::Async;
using izumo::backends::EpollService;
using izumo::backends::EpollTCPServer;
using izumo::http::RequestHeader;
using izumo::http::ResponseHeader;
using izumo::tcp::TCPStream;
using izumo::util::ByteArray;

class TestHTTPServer : public EpollTCPServer {
  protected:
    Async<void> handleRequest(TCPStream stream) override;

  public:
    TestHTTPServer(const char* addr, std::uint16_t port)
        : EpollTCPServer(addr, port)
    {
    }
};

const char RESPONSE[] = "HTTP/1.1 200 Established\r\n"
                        "Server: Izumo\r\n"
                        "Content-Type: text/plain\r\n"
                        "Content-Length: 11\r\n\r\n"
                        "IZUMO DEMO\n";

Async<void>
TestHTTPServer::handleRequest(TCPStream stream)
{
    ByteArray buffer(4096);

    RequestHeader req;

    std::size_t bytesRead = 0;

    while (true) {
        bytesRead += (co_await stream.readAtMost(
                          buffer.toView(bytesRead, buffer.size())))
                         .size();

        try {
            req.parse(buffer.toView(bytesRead));
        } catch (izumo::http::IncompleteInput) {
            continue;
        }

	break;
    }

    std::cout << "method: " << req.method << std::endl;
    std::cout << "target: " << req.target << std::endl;
    std::cout << "version: " << req.version << std::endl;
    std::cout << "fields:" << std::endl;

    for (auto& f : req.fields) {
        std::cout << "  name: " << f.name << std::endl;
        std::cout << "  value: " << f.value << std::endl << std::endl;
    }

    std::memcpy(buffer.data(), RESPONSE, sizeof(RESPONSE));
    co_await stream.sendAll(buffer.toView(sizeof(RESPONSE)));
}

int
main()
{
    TestHTTPServer srv("127.0.0.1", 1551);
    srv.start();
    EpollService::instance().start();
}
