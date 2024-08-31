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

#include "client/crashpad_client.h"

#import <Foundation/Foundation.h>

#include <vector>

#include "gtest/gtest.h"
#include "testing/platform_test.h"

namespace crashpad {
namespace test {
namespace {

using CrashpadIOSClient = PlatformTest;

TEST_F(CrashpadIOSClient, DumpWithoutCrash) {
  CrashpadClient client;
  client.StartCrashpadInProcessHandler();

  NativeCPUContext context;
  CaptureContext(&context);
  client.DumpWithoutCrash(&context);
}

// This test is covered by a similar XCUITest, but for development purposes it's
// sometimes easier and faster to run in Google Test.  However, there's no way
// to correctly run this in Google Test. Leave the test here, disabled, for use
// during development only.
TEST_F(CrashpadIOSClient, DISABLED_ThrowNSException) {
  CrashpadClient client;
  client.StartCrashpadInProcessHandler();
  [NSException raise:@"GoogleTestNSException" format:@"ThrowException"];
}

// This test is covered by a similar XCUITest, but for development purposes it's
// sometimes easier and faster to run in Google Test.  However, there's no way
// to correctly run this in Google Test. Leave the test here, disabled, for use
// during development only.
TEST_F(CrashpadIOSClient, DISABLED_ThrowException) {
  CrashpadClient client;
  client.StartCrashpadInProcessHandler();
  std::vector<int> empty_vector;
  empty_vector.at(42);
}

}  // namespace
}  // namespace test
}  // namespace crashpad
