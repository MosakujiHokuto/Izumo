/* http/HTTPHeaders.hh -- HTTP header parsing and generation */

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

#ifndef IZUMO_HTTP_HTTPHEADERS_HH_
#define IZUMO_HTTP_HTTPHEADERS_HH_

#include <util/ByteArray.hh>

#include <optional>
#include <string>
#include <vector>

namespace izumo::http {

struct HeaderField {
    std::string name;
    std::string value;
};

using HeaderFields = std::vector<HeaderField>;

struct IncompleteInput {};

struct RequestHeader {
    std::string method, target, version;
    HeaderFields fields;

    util::ByteArrayView parse(util::ByteArrayView buf);
    // std::optional<util::ByteArrayView> write(util::ByteArrayView buf);
};

struct ResponseHeader {
    int code;
    std::string version, reason;
    HeaderFields fields;

    util::ByteArrayView parse(util::ByteArrayView buf);
    // std::optional<util::ByteArrayView> write(util::ByteArrayView buf);
};

} // namespace izumo::http

#endif /* IZUMO_HTTP_HTTPHEADERS_HH_ */
