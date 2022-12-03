/* ByteArray.hh -- Fixed-size binary data container */

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

#ifndef IZUMO_UTIL_BYTEARRAY_HH_
#define IZUMO_UTIL_BYTEARRAY_HH_

#include <cassert>
#include <cstdint>
#include <utility>

namespace izumo::util {

typedef std::uint8_t Byte;

/*
 * A non-owning view into a ByteArray
 */
class ByteArrayView {
  private:
    Byte* m_data_ref = nullptr;
    std::size_t m_len = 0;

  public:
    ByteArrayView() = default;
    ByteArrayView(Byte* begin, Byte* end)
        : m_data_ref(begin), m_len(end - begin)
    {
        assert(end >= begin);
    }

    ByteArrayView(const ByteArrayView&) = default;

    Byte*
    begin() const noexcept
    {
        return m_data_ref;
    }

    Byte*
    end() const noexcept
    {
        return m_data_ref + m_len;
    }

    ByteArrayView
    subView(std::size_t end)
    {
	return subView(0, end);
    }

    ByteArrayView
    subView(std::size_t begin, std::size_t end)
    {
	assert(begin <= end);
	assert(end <= m_len);

	return ByteArrayView(m_data_ref + begin, m_data_ref + end);
    }

    std::size_t size() const noexcept
    {
	return m_len;
    }

    Byte&
    operator[](std::size_t idx)
    {
        return m_data_ref[idx];
    }
};

/*
 * Fixed-size container for holding binary data
 */
class ByteArray {
  private:
    std::size_t m_len = 0;
    Byte* m_data = nullptr;

  public:
    ByteArray() = default;
    ByteArray(std::size_t size) : m_len(size) { m_data = new Byte[size]; }
    ByteArray(const ByteArray&) = delete;
    ByteArray(ByteArray&& rhs) { this->swap(rhs); }

    ~ByteArray() { delete m_data; }

    Byte*
    data() const noexcept
    {
        return m_data;
    }

    std::size_t
    size() const noexcept
    {
        return m_len;
    }

    void
    swap(ByteArray& rhs) noexcept
    {
        std::swap(m_data, rhs.m_data);
        std::swap(m_len, rhs.m_len);
    }

    ByteArrayView
    toView() const noexcept
    {
	return toView(0, size());
    }

    ByteArrayView
    toView(std::size_t end) const noexcept
    {
	return toView(0, end);
    }

    ByteArrayView
    toView(std::size_t begin, std::size_t end) const noexcept
    {
        assert(end <= m_len);
        assert(begin <= end);
        return ByteArrayView(m_data + begin, m_data + end);
    }

};

} // namespace izumo::util

#endif /* IZUMO_UTIL_BYTEARRAY_HH_ */
