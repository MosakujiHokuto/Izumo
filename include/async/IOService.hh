/* async/IOService.hh -- IO Service interface */

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

#ifndef IZUMO_ASYNC_IOSERVICE_HH_
#define IZUMO_ASYNC_IOSERVICE_HH_

namespace izumo::async {
class IOService {
  public:
    virtual ~IOService() = default;
    virtual void start() = 0;
};
} // namespace izumo::async

#endif /* IZUMO_ASYNC_IOSERVICE_HH_ */
