/* http/HTTPHeaders.cc -- HTTP Header parsing and generating */

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

#include <http/HTTPExceptions.hh>
#include <http/HTTPHeaders.hh>

namespace izumo::http {

bool
contains(const std::string& table, char c)
{
    for (auto i : table) {
        if (c == i)
            return true;
    }
    return false;
}

bool
exact(const std::string& str, char* p, char* end)
{
    if (end - p < str.size()) {
        return false;
    }

    return str == std::string(p, str.size());
}

bool
isVChar(char c)
{
    return c >= 0x20 && c < 0x7f;
}

bool
isVCharObs(char c)
{
    return !(c < 0x20 || c == 0x7f);
}

bool
isAlpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool
isDigit(char c)
{
    return (c >= '0' && c <= '9');
}

bool
isTChar(char c)
{
    return isAlpha(c) || isDigit(c) || contains("!#$%&'*+-.^_`|~", c);
}

static char*
parseToken(char* begin, char* end)
{
    char* p = begin;
    while (p != end) {
        if (!isTChar(*p)) {
            return p;
        }
        ++p;
    }

    return p;
}

static char*
parseTarget(char* p, char* end)
{
    /* XXX proper uri parsing */
    while (p != end) {
        if (*p == ' ') {
            return p;
        }
        ++p;
    }

    return p;
}

const std::size_t HTTPVER_PREFIXLEN = 7;
const std::size_t HTTPVER_REQUIREDLEN = 8;

static char*
parseHTTPVer(char* begin, char* end)
{
    if (!exact("HTTP/1.", begin, end)) {
        return nullptr;
    }

    char* p = begin + HTTPVER_PREFIXLEN;
    if (p == end) {
        return nullptr;
    }
    if (*p != '0' && *p != '1') {
        return nullptr;
    }

    return p + 1;
}

static util::ByteArrayView
parseCRLF(util::ByteArrayView buf)
{
    auto begin = reinterpret_cast<char*>(buf.begin());
    auto end = reinterpret_cast<char*>(buf.end());

    auto p = begin;

    if (p == end) {
        /* Incomplete */
        throw IncompleteInput();
    }

    if (*p == '\r') {
        ++p;
        if (p == end) {
            /* Incomplete */
            throw IncompleteInput();
        }
    }

    if (*p != '\n') {
        /* Invalid CRLF */
        throw HTTPParseError("Invalid CRLF");
    }

    return buf.subView(p - begin, buf.size());
}

static util::ByteArrayView
parseRequestLine(util::ByteArrayView buf, std::string& method,
                 std::string& target, std::string& httpver)
{
    auto begin = reinterpret_cast<char*>(buf.begin());
    auto end = reinterpret_cast<char*>(buf.end());

    auto methodEnd = parseToken(begin, end);
    if (methodEnd == end) {
        /* Incomplete */
        throw IncompleteInput();
    }

    if (*methodEnd != ' ') {
        /* Invalid separator */
        throw HTTPParseError("Invalid separator following method");
    }

    auto targetBegin = methodEnd + 1;
    if (targetBegin == end) {
        /* Incomplete */
        throw IncompleteInput();
    }

    auto targetEnd = parseTarget(targetBegin, end);
    if (targetEnd == end) {
        /* Incomplete */
        throw IncompleteInput();
    }

    if (*targetEnd != ' ') {
        /* Invalid separator */
        throw HTTPParseError("Invalid separator following target");
    }

    auto httpverBegin = targetEnd + 1;
    if (end - httpverBegin < HTTPVER_REQUIREDLEN) {
        /* Incomplete */
        throw IncompleteInput();
    }

    auto httpverEnd = parseHTTPVer(httpverBegin, end);
    if (!httpverEnd) {
        /* Invalid http version */
        throw HTTPParseError("Invalid http version");
    }

    if (httpverEnd == end) {
        /* Incomplete */
        throw IncompleteInput();
    }

    char* eolp = httpverEnd + 1;
    if (eolp == end) {
        /* Incomplete */
        throw IncompleteInput();
    }

    if (*eolp == '\r') {
        ++eolp;
        if (eolp == end) {
            /* Incomplete */
            throw IncompleteInput();
        }
    }

    if (*eolp != '\n') {
        /* Invalid CRLF */
        throw HTTPParseError("Invalid CRLF in request line");
    }

    ++eolp;

    auto consumed = eolp - begin;
    method = std::string{begin, methodEnd};
    target = std::string{targetBegin, targetEnd};
    httpver = std::string{httpverBegin, httpverEnd};

    return buf.subView(consumed, buf.size());
}

const std::size_t STATUSCODE_LEN = 3;

static util::ByteArrayView
parseResponseLine(util::ByteArrayView buf, std::string& httpver, int& code,
                  std::string& reason)
{
    auto begin = reinterpret_cast<char*>(buf.begin());
    auto end = reinterpret_cast<char*>(buf.end());

    if (end - begin < HTTPVER_REQUIREDLEN) {
        /* Incomplete */
        throw IncompleteInput();
    }

    auto httpverEnd = parseHTTPVer(begin, end);
    if (httpverEnd == end) {
        /* Incomplete */
        throw IncompleteInput();
    }

    if (*httpverEnd != ' ') {
        /* Invalid separator */
        throw HTTPParseError("Invalid separator after http version");
    }

    auto codep = httpverEnd + 1;
    if (end - codep < STATUSCODE_LEN + 1) {
        /* Incomplete */
        throw IncompleteInput();
    }

    int _code = 0;
    for (std::size_t i = 0; i < STATUSCODE_LEN; ++i) {
        if (!isDigit(codep[i])) {
            /* Invalid status code */
            throw HTTPParseError("Invalid status code");
        }
        _code *= 10;
        _code += codep[i] - '0';
    }

    auto codeEnd = codep + STATUSCODE_LEN;
    if (codeEnd == end) {
        /* Incomplete */
        throw IncompleteInput();
    }

    if (*codeEnd != ' ') {
        /* Invalid separator */
        throw HTTPParseError("Invalid separator after status code");
    }

    auto reasonBegin = codeEnd + 1;
    auto reasonEnd = reasonBegin;
    while (true) {
        if (reasonEnd == end) {
            /* Incomplete */
            throw IncompleteInput();
        }

        if (*reasonEnd == '\r' || *reasonEnd == '\n') {
            break;
        }

        if (!isVCharObs(*reasonEnd)) {
            /* Invalid character */
            throw HTTPParseError("Invalid character in reason");
        }

        ++reasonEnd;
    }

    auto eolp = reasonEnd;
    assert(eolp < end);

    if (*eolp == '\r') {
        ++eolp;
        if (eolp == end) {
            /* Incomplete */
            throw IncompleteInput();
        }
    }

    if (*eolp != '\n') {
        /* Invalid CRLF */
        throw HTTPParseError("Invalid CRLF in response line");
    }

    ++eolp;

    auto consumed = eolp - begin;
    httpver = std::string{begin, httpverEnd};
    code = _code;
    reason = std::string{reasonBegin, reasonEnd};

    return buf.subView(consumed, buf.size());
}

util::ByteArrayView
parseField(util::ByteArrayView buf, HeaderFields& fields)
{
    auto begin = reinterpret_cast<char*>(buf.begin());
    auto end = reinterpret_cast<char*>(buf.end());

    auto nameEnd = parseToken(begin, end);
    if (nameEnd == end) {
        /* Incomplete */
        throw IncompleteInput();
    }

    if (*nameEnd != ':') {
        /* Invalid separator */
        throw HTTPParseError("Invalid separator after field name");
    }

    auto valueBegin = nameEnd + 1;
    /* skip OWS */
    while (true) {
        if (valueBegin == end) {
            /* Incomplete */
            throw IncompleteInput();
        }

        if (*valueBegin != ' ') {
            break;
        }

        ++valueBegin;
    }

    auto valueEnd = valueBegin;

    while (true) {
        if (valueEnd == end) {
            /* Incomplete */
            throw IncompleteInput();
        }

        if (*valueEnd == '\r' || *valueEnd == '\n') {
            break;
        }

        if (!isVCharObs(*valueEnd)) {
            /* Invalid character in field value */
            throw HTTPParseError("Invalid character in field value");
        }

        ++valueEnd;
    }

    auto eolp = valueEnd + 1;
    if (*eolp == '\r') {
        ++eolp;
        if (eolp == end) {
            /* Incomplete */
            throw IncompleteInput();
        }
    }

    if (*eolp != '\n') {
        /* Invalid CRLF */
        throw HTTPParseError("Invalid CRLF in field");
    }

    ++eolp;

    auto consumed = eolp - begin;
    std::string name{begin, nameEnd};
    std::string value{valueBegin, valueEnd};
    fields.emplace_back(std::move(name), std::move(value));

    return buf.subView(consumed, buf.size());
}

util::ByteArrayView
parseFields(util::ByteArrayView buf, HeaderFields& fields)
{
    while (true) {
        if (buf.size() == 0) {
            /* Incomplete */
            throw IncompleteInput();
        }

        if (*(buf.begin()) == '\r' || *(buf.begin()) == '\n') {
            return buf;
        }

        buf = parseField(buf, fields);
    }
}

util::ByteArrayView
RequestHeader::parse(util::ByteArrayView buf)
{
    buf = parseRequestLine(buf, method, target, version);
    buf = parseFields(buf, fields);
    return parseCRLF(buf);
}

util::ByteArrayView
ResponseHeader::parse(util::ByteArrayView buf)
{
    buf = parseResponseLine(buf, version, code, reason);
    buf = parseFields(buf, fields);
    return parseCRLF(buf);
}

} // namespace izumo::http
