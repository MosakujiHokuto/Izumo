/* util/Exceptions.hh -- Common exceptions */

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

#ifndef IZUMO_UTIL_EXCEPTIONS_HH_
#define IZUMO_UTIL_EXCEPTIONS_HH_

#include <system_error>

#include <cerrno>

namespace izumo::util {

class SystemException : public std::system_error {
  public:
    SystemException() = delete;
    SystemException(int error = errno)
        : std::system_error(error, std::generic_category())
    {
    }
    SystemException(const char* str, int error = errno)
        : std::system_error(error, std::generic_category(), str){};
};

} // namespace izumo::util

#endif /* IZUMO_UTIL_EXCEPTIONS_HH_ */
