/* backends/epoll.cc -- Epoll backend */

/*
 * Copyright 2022 Symbol’s value as variable is void: my-yas-author
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

#include <backends/epoll.hh>
#include <tcp/TCPExceptions.hh>
#include <util/Exceptions.hh>

#include <arpa/inet.h>
#include <cstring>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>

namespace izumo::backends {

EpollListener::EpollListener(int fd, EpollService& drv) : m_srv(drv), m_fd(fd)
{
    m_srv.addListener(*this);
}

EpollListener::~EpollListener()
{
    m_srv.removeListener(*this);
    ::close(m_fd);
}

Async<std::unique_ptr<tcp::TCPStreamBackendIfce>>
EpollTCPAcceptor::accept()
{
    int clientFd;
    while (true) {
        clientFd = ::accept4(m_listener->fd(), nullptr, nullptr, SOCK_NONBLOCK);
        if (clientFd != -1)
            break;

        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            /* Wait until ready */
            co_await m_listener->waitEvent();
            continue;
        } else if (errno == EINTR) {
            continue;
        } else {
            throw util::SystemException("accept");
        }
    }

    co_return std::make_unique<EpollTCPStream>(clientFd, m_listener->service());
}

Async<std::size_t>
EpollTCPStream::read(util::ByteArrayView buf)
{
    while (true) {
        ::ssize_t ret = ::recv(m_listener->fd(), buf.begin(), buf.size(),
                               MSG_NOSIGNAL | MSG_DONTWAIT);
        if (ret < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                /* Wait until ready */
                co_await m_listener->waitEvent();
                continue;
            } else {
                throw util::SystemException("recv");
            }
        } else if (ret == 0) {
            throw tcp::EndOfStreamException();
        }

        co_return ret;
    }
}

Async<std::size_t>
EpollTCPStream::write(util::ByteArrayView buf)
{
    while (true) {
        ::size_t ret = ::send(m_listener->fd(), buf.begin(), buf.size(),
                              MSG_NOSIGNAL | MSG_DONTWAIT);
        if (ret < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                /* Wait until ready */
                co_await m_listener->waitEvent();
                continue;
            } else {
                throw util::SystemException("send");
            }
        } else if (ret == 0) {
            throw tcp::EndOfStreamException();
        }

        co_return ret;
    }
}

static int
bindSocket(const char* addr, std::uint16_t port)
{
    int fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (fd < 0) {
        throw util::SystemException("socket");
    }

    int val = 1;
    if (::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0) {
        ::close(fd);
        throw util::SystemException("SO_REUSEADDR");
    }

    val = 1;
    if (::setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &val, sizeof(val)) < 0) {
        ::close(fd);
        throw util::SystemException("SO_REUSEPORT");
    }

    struct sockaddr_in sockaddr;

    if (!::inet_aton(addr, &sockaddr.sin_addr)) {
        ::close(fd);
        throw std::runtime_error("inet_aton error");
    }

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = ::htons(port);
    std::memset(&sockaddr.sin_zero, 0, sizeof(sockaddr.sin_zero));

    if (::bind(fd, reinterpret_cast<struct sockaddr*>(&sockaddr),
               sizeof(struct sockaddr_in)) < 0) {
        ::close(fd);
        throw util::SystemException("bind");
    }

    if (::listen(fd, 128) < 0) {
        ::close(fd);
        throw util::SystemException("listen");
    }

    return fd;
}

EpollTCPServer::EpollTCPServer(const char* addr, std::uint16_t port,
                               EpollService& srv)
    : TCPServer(tcp::TCPAcceptor(
          std::make_unique<EpollTCPAcceptor>(bindSocket(addr, port), srv)))
{
}

EpollService::EpollService()
{
    int epfd = epoll_create1(0);
    if (epfd < 0) {
        throw util::SystemException("epoll_create1");
    }

    m_epfd = epfd;
}

EpollService::~EpollService() { ::close(m_epfd); }

EpollService&
EpollService::instance()
{
    static thread_local EpollService ins;

    return ins;
}

void
EpollService::addListener(EpollListener& listener)
{
    struct epoll_event ev = {
        .events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET,
        .data = {.ptr = &listener},
    };
    int ret = ::epoll_ctl(m_epfd, EPOLL_CTL_ADD, listener.fd(), &ev);
    if (ret < 0) {
        throw util::SystemException("EPOLL_CTL_ADD");
    }
}

void
EpollService::removeListener(EpollListener& listener)
{
    int ret = ::epoll_ctl(m_epfd, EPOLL_CTL_DEL, listener.fd(), nullptr);
    if (ret < 0) {
        throw util::SystemException("EPOLL_CTL_DEL");
    }
}

void
EpollService::start()
{
    /* TODO timeout */
    struct epoll_event ev[backlog];
    while (true) {
        int ret = ::epoll_wait(m_epfd, ev, backlog, -1);

        if (ret < 0) {
            switch (errno) {
            case EINTR:
                /* Interrupted by a signal, ignore */
                break;
            default:
                throw util::SystemException("epoll_wait");
            }
        }

        for (int i = 0; i < ret; ++i) {
            EpollListener* l = static_cast<EpollListener*>(ev[i].data.ptr);
            l->onEvent();
        }
    }
}

} // namespace izumo::backends
