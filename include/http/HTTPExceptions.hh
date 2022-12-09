/* http/HTTPExceptions.hh -- HTTP exceptions */

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

#ifndef IZUMO_HTTP_HTTPEXCEPTIONS_HH_
#define IZUMO_HTTP_HTTPEXCEPTIONS_HH_

#include <stdexcept>

namespace izumo::http {

class HTTPParseError : public std::runtime_error {
  public:
    HTTPParseError(const std::string& str) : std::runtime_error(str) {}
    HTTPParseError(const char* str) : std::runtime_error(str) {}
};

} // namespace izumo::http

#endif /* IZUMO_HTTP_HTTPEXCEPTIONS_HH_ */
