/* async/Async.hh -- Async primitives implementation */

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

#ifndef IZUMO_ASYNC_ASYNC_HH_
#define IZUMO_ASYNC_ASYNC_HH_

#include <coroutine>
#include <exception>
#include <optional>
#include <utility>

namespace izumo::async {

template <typename Result>
class Async;

template <typename Result>
class AsyncPromise;

class AsyncPromiseBase {
  private:
    std::exception_ptr m_exception = nullptr;
    std::coroutine_handle<> m_awaiter;

  public:
    /* Custom utility interfaces */
    std::exception_ptr
    exception()
    {
        return m_exception;
    }

    /* Coroutine promise interfaces */
    std::suspend_never
    initial_suspend() noexcept
    {
        return {};
    }

    auto
    final_suspend() noexcept
    {
        class TransferAwaitable {
          private:
            std::coroutine_handle<> m_awaiter;

          public:
            TransferAwaitable(std::coroutine_handle<> awaiter)
                : m_awaiter(awaiter){};

            bool
            await_ready() const noexcept
            {
                return false;
            }

            std::coroutine_handle<>
            await_suspend(std::coroutine_handle<>) const noexcept
            {
                return m_awaiter ? m_awaiter : std::noop_coroutine();
            }

            void
            await_resume() const noexcept
            {
            }
        };

        return TransferAwaitable(m_awaiter);
    }

    void
    unhandled_exception()
    {
        m_exception = std::current_exception();
    }

    template <typename R>
    auto
    await_transform(Async<R> coro)
    {
        class AsyncAwaitable {
          private:
            std::coroutine_handle<AsyncPromise<R>> m_handle;

          public:
            AsyncAwaitable(std::coroutine_handle<AsyncPromise<R>> h)
                : m_handle(h)
            {
            }

            bool
            await_ready() const noexcept
            {
                return m_handle.promise().finished();
            }

            void
            await_suspend(std::coroutine_handle<> h) const noexcept
            {
                m_handle.promise().m_awaiter = h;
            }

            auto
            await_resume()
            {
                auto eptr = m_handle.promise().exception();
                if (eptr) {
                    std::rethrow_exception(eptr);
                }

                return m_handle.promise().result();
            }
        } ret{coro.handle()};

        return ret;
    }

    template <typename T>
    decltype(auto)
    await_transform(T&& expr)
    {
        /* return as-is */
        return std::forward<T>(expr);
    }
};

template <typename Result>
class AsyncPromise : public AsyncPromiseBase {
  private:
    std::optional<Result> m_result;

  public:
    /* Custom utility interfaces */
    bool
    finished() const noexcept
    {
        return m_result.has_value();
    }

    auto
    result()
    {
        auto ret = std::move(*m_result);
        return ret;
    }

    /* Coroutine promise interfaces */
    Async<Result>
    get_return_object()
    {
        return {
            std::coroutine_handle<AsyncPromise<Result>>::from_promise(*this)};
    }

    void
    return_value(Result ret)
    {
        m_result = std::move(ret);
    }
};

/* void specialization for AsyncPromise */
template <>
class AsyncPromise<void> : public AsyncPromiseBase {
  private:
    bool m_finished = false;

  public:
    /* Custom utility interfaces */
    bool
    finished() const noexcept
    {
        return m_finished;
    }

    void
    getResult()
    {
    }

    /* Coroutine promise interfaces */
    Async<void> get_return_object();

    void
    return_void()
    {
        m_finished = true;
    }
};

template <typename Result>
class Async {
  private:
    std::coroutine_handle<AsyncPromise<Result>> m_handle;

  public:
    /* for std::coroutine_traits */
    using promise_type = AsyncPromise<Result>;

  public:
    Async() = default;
    Async(std::coroutine_handle<AsyncPromise<Result>> handle) : m_handle(handle)
    {
    }
    Async(const Async& rhs) = delete;
    Async(Async&& rhs) { std::swap(m_handle, rhs.m_handle); }

    Async&
    operator=(Async&& rhs)
    {
        if (m_handle) {
            m_handle.destroy();
        }

        m_handle = std::exchange(rhs.m_handle, nullptr);
    };

    auto
    result()
    {
        return m_handle.promise().getResult();
    }

    auto
    handle()
    {
        return m_handle;
    }

    bool
    ready()
    {
        return m_handle && m_handle.finished();
    }
};

inline Async<void>
AsyncPromise<void>::get_return_object()
{
    return {std::coroutine_handle<AsyncPromise<void>>::from_promise(*this)};
}

} // namespace izumo::async

#endif /* IZUMO_ASYNC_ASYNC_HH_ */
