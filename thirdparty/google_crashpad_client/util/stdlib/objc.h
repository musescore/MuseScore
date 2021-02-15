// Copyright 2014 The Crashpad Authors. All rights reserved.
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

#ifndef CRASHPAD_UTIL_STDLIB_OBJC_H_
#define CRASHPAD_UTIL_STDLIB_OBJC_H_

#include <Availability.h>
#include <objc/objc.h>

#if __MAC_OS_X_VERSION_MAX_ALLOWED < __MAC_10_8

// In order for the @NO and @YES literals to work, NO and YES must be defined as
// __objc_no and __objc_yes. See
// https://clang.llvm.org/docs/ObjectiveCLiterals.html.
//
// NO and YES are defined properly for this purpose in the 10.8 SDK, but not in
// earlier SDKs. Because this code is never expected to be compiled with a
// compiler that does not understand the modern forms of these boolean
// constants, but it may be built with an older SDK, replace the outdated SDK
// definitions unconditionally.
#undef NO
#undef YES
#define NO __objc_no
#define YES __objc_yes

#endif

#endif  // CRASHPAD_UTIL_STDLIB_OBJC_H_
