// Copyright 2020 The Crashpad Authors. All rights reserved.
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

#ifndef CRASHPAD_COMPAT_MAC_AVAILABILITYVERSIONS_H_
#define CRASHPAD_COMPAT_MAC_AVAILABILITYVERSIONS_H_

#if __has_include_next(<AvailabilityVersions.h>)
#include_next <AvailabilityVersions.h>
#endif

// 10.7 SDK

#ifndef __MAC_10_7
#define __MAC_10_7 1070
#endif

// 10.8 SDK

#ifndef __MAC_10_8
#define __MAC_10_8 1080
#endif

// 10.9 SDK

#ifndef __MAC_10_9
#define __MAC_10_9 1090
#endif

// 10.10 SDK

#ifndef __MAC_10_10
#define __MAC_10_10 101000
#endif

// 10.11 SDK

#ifndef __MAC_10_11
#define __MAC_10_11 101100
#endif

// 10.12 SDK

#ifndef __MAC_10_12
#define __MAC_10_12 101200
#endif

// 10.13 SDK

#ifndef __MAC_10_13
#define __MAC_10_13 101300
#endif

#ifndef __MAC_10_13_4
#define __MAC_10_13_4 101304
#endif

// 10.14 SDK

#ifndef __MAC_10_14
#define __MAC_10_14 101400
#endif

// 10.15 SDK

#ifndef __MAC_10_15
#define __MAC_10_15 101500
#endif

// 11.0 SDK

#ifndef __MAC_10_16
#define __MAC_10_16 101600
#endif

#ifndef __MAC_11_0
#define __MAC_11_0 110000
#endif

#endif  // CRASHPAD_COMPAT_MAC_AVAILABILITYVERSIONS_H_
