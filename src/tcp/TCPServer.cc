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

#include <tcp/TCPServer.hh>
#include <tcp/TCPStream.hh>

namespace izumo::tcp {

Async<TCPStream>
TCPAcceptor::accept()
{
    auto backend = co_await m_backend->accept();

    co_return TCPStream(std::move(backend));
}

Async<void>
TCPServer::start()
{
    while (true) {
	auto stream = co_await m_acceptor.accept();
	this->handleRequest(std::move(stream));
    }
}

}
