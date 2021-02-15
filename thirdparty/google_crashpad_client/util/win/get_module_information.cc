// Copyright 2016 The Crashpad Authors. All rights reserved.
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

#include "util/win/get_module_information.h"

#include "util/win/get_function.h"

namespace crashpad {

BOOL CrashpadGetModuleInformation(HANDLE process,
                                  HMODULE module,
                                  MODULEINFO* module_info,
                                  DWORD cb) {
#if PSAPI_VERSION == 1
  static const auto get_module_information =
    GET_FUNCTION_REQUIRED(L"psapi.dll", GetModuleInformation);
  return get_module_information(process, module, module_info, cb);
#elif PSAPI_VERSION == 2
  return GetModuleInformation(process, module, module_info, cb);
#endif
}

}  // namespace crashpad
