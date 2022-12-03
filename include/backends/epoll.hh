/* backends/epoll.hh -- Epoll backend */

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

#ifndef IZUMO_BACKENDS_EPOLL_HH_
#define IZUMO_BACKENDS_EPOLL_HH_

#include <async/IOService.hh>
#include <tcp/TCPServer.hh>
#include <tcp/TCPStream.hh>
#include <util/ByteArray.hh>

#include <coroutine>

namespace izumo::backends {

using async::Async;

class EpollService;
class EpollListener {
  private:
    EpollService& m_srv;
    int m_fd;

    std::coroutine_handle<> m_awaiter;

  public:
    EpollListener(int fd, EpollService& srv);
    EpollListener(const EpollListener&) = delete;
    EpollListener(EpollListener&&) = delete;
    virtual ~EpollListener();

    int
    fd()
    {
        return m_fd;
    }

    EpollService&
    service()
    {
        return m_srv;
    }

    auto
    waitEvent()
    {
        class Awaitable {
          private:
            EpollListener& m_listener;

          public:
            Awaitable(EpollListener& listener) : m_listener(listener) {}

            /* std awaitable interfaces */
            bool
            await_ready() const noexcept
            {
                return false;
            }

            void
            await_suspend(std::coroutine_handle<> awaiter) noexcept
            {
                assert(!m_listener.m_awaiter);
                m_listener.m_awaiter = awaiter;
            }

            void
            await_resume() const noexcept
            {
            }

        } ret{*this};
        return ret;
    }

    void
    onEvent()
    {
	auto awaiter = std::exchange(m_awaiter, nullptr);
	if (awaiter) {
	    awaiter.resume();
	}
    }
};

class EpollService : public async::IOService {
  private:
    int m_epfd = -1;

    constexpr inline static int backlog = 256;

  public:
    EpollService();
    ~EpollService();

    void start() override;

    static EpollService& instance();

    void addListener(EpollListener& listener);
    void removeListener(EpollListener& listener);
};

class EpollTCPAcceptor : public tcp::TCPAcceptorBackendIfce {
  private:
    std::unique_ptr<EpollListener> m_listener = nullptr;

  public:
    EpollTCPAcceptor(int fd, EpollService& srv)
        : m_listener(std::make_unique<EpollListener>(fd, srv))
    {
    }
    EpollTCPAcceptor(const EpollTCPAcceptor&) = delete;
    EpollTCPAcceptor(EpollTCPAcceptor&&) = delete;

    Async<std::unique_ptr<tcp::TCPStreamBackendIfce>> accept() override;
};

class EpollTCPStream : public tcp::TCPStreamBackendIfce {
  private:
    std::unique_ptr<EpollListener> m_listener = nullptr;

  public:
    EpollTCPStream(int fd, EpollService& srv)
        : m_listener(std::make_unique<EpollListener>(fd, srv))
    {
    }
    EpollTCPStream(const EpollTCPStream&) = delete;
    EpollTCPStream(EpollTCPStream&&) = delete;

    Async<std::size_t> read(util::ByteArrayView buf) override;
    Async<std::size_t> write(util::ByteArrayView buf) override;
};

class EpollTCPServer : public tcp::TCPServer {
  private:
  public:
    EpollTCPServer(const char* addr, std::uint16_t port,
                   EpollService& srv = EpollService::instance());
};

} // namespace izumo::backends

#endif /* IZUMO_TCP_BACKENDS_EPOLL_HH_ */
