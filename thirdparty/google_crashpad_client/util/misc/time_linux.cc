// Copyright 2019 The Crashpad Authors. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "util/misc/time.h"

#include "base/logging.h"

namespace crashpad {

bool GetBootTime(timespec* boot_time) {
  timespec uptime;
  if (clock_gettime(CLOCK_BOOTTIME, &uptime) != 0) {
    PLOG(ERROR) << "clock_gettime";
    return false;
  }

  timespec current_time;
  if (clock_gettime(CLOCK_REALTIME, &current_time) != 0) {
    PLOG(ERROR) << "clock_gettime";
    return false;
  }

  SubtractTimespec(current_time, uptime, boot_time);
  return true;
}

}  // namespace crashpad
