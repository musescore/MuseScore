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

#ifndef CRASHPAD_UTIL_IOS_EXCEPTION_PROCESSOR_H_
#define CRASHPAD_UTIL_IOS_EXCEPTION_PROCESSOR_H_

namespace crashpad {

//! \brief Installs the Objective-C exception preprocessor.
//!
//! When code raises an Objective-C exception, unwind the stack looking for
//! any exception handlers. If an exception handler is encountered, test to
//! see if it is a function known to be a catch-and-rethrow 'sinkhole' exception
//! handler. Various routines in UIKit do this, and they obscure the
//! crashing stack, since the original throw location is no longer present
//! on the stack (just the re-throw) when Crashpad captures the crash
//! report. In the case of sinkholes, trigger an immediate exception to
//! capture the original stack.
//!
//! This should be installed at the same time the CrashpadClient installs the
//! signal handler. It should only be installed once.
void InstallObjcExceptionPreprocessor();

}  // namespace crashpad

#endif  // CRASHPAD_UTIL_IOS_EXCEPTION_PROCESSOR_H_
