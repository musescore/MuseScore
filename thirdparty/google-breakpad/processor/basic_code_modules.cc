// Copyright (c) 2006, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// basic_code_modules.cc: Contains all of the CodeModule objects that
// were loaded into a single process.
//
// See basic_code_modules.h for documentation.
//
// Author: Mark Mentovai

#include "processor/basic_code_modules.h"

#include <assert.h>

#include <vector>

#include "google_breakpad/processor/code_module.h"
#include "processor/linked_ptr.h"
#include "processor/logging.h"
#include "processor/range_map-inl.h"

namespace google_breakpad {

using std::vector;

BasicCodeModules::BasicCodeModules(const CodeModules* that,
                                   MergeRangeStrategy strategy)
    : main_address_(0), map_() {
  BPLOG_IF(ERROR, !that) << "BasicCodeModules::BasicCodeModules requires "
                            "|that|";
  assert(that);

  map_.SetMergeStrategy(strategy);

  const CodeModule *main_module = that->GetMainModule();
  if (main_module)
    main_address_ = main_module->base_address();

  unsigned int count = that->module_count();
  for (unsigned int i = 0; i < count; ++i) {
    // Make a copy of the module and insert it into the map.  Use
    // GetModuleAtIndex because ordering is unimportant when slurping the
    // entire list, and GetModuleAtIndex may be faster than
    // GetModuleAtSequence.
    linked_ptr<const CodeModule> module(that->GetModuleAtIndex(i)->Copy());
    if (!map_.StoreRange(module->base_address(), module->size(), module)) {
      BPLOG(ERROR) << "Module " << module->code_file()
                   << " could not be stored";
    }
  }

  // Report modules with shrunk ranges.
  for (unsigned int i = 0; i < count; ++i) {
    linked_ptr<const CodeModule> module(that->GetModuleAtIndex(i)->Copy());
    uint64_t delta = 0;
    if (map_.RetrieveRange(module->base_address() + module->size() - 1,
                           &module, NULL /* base */, &delta, NULL /* size */) &&
        delta > 0) {
      BPLOG(INFO) << "The range for module " << module->code_file()
                  << " was shrunk down by " << HexString(delta) << " bytes.";
      linked_ptr<CodeModule> shrunk_range_module(module->Copy());
      shrunk_range_module->SetShrinkDownDelta(delta);
      shrunk_range_modules_.push_back(shrunk_range_module);
    }
  }

  // TODO(ivanpe): Report modules with conflicting ranges.  The list of such
  // modules should be copied from |that|.
}

BasicCodeModules::BasicCodeModules() : main_address_(0), map_() { }

BasicCodeModules::~BasicCodeModules() {
}

unsigned int BasicCodeModules::module_count() const {
  return map_.GetCount();
}

const CodeModule* BasicCodeModules::GetModuleForAddress(
    uint64_t address) const {
  linked_ptr<const CodeModule> module;
  if (!map_.RetrieveRange(address, &module, NULL /* base */, NULL /* delta */,
                          NULL /* size */)) {
    BPLOG(INFO) << "No module at " << HexString(address);
    return NULL;
  }

  return module.get();
}

const CodeModule* BasicCodeModules::GetMainModule() const {
  return GetModuleForAddress(main_address_);
}

const CodeModule* BasicCodeModules::GetModuleAtSequence(
    unsigned int sequence) const {
  linked_ptr<const CodeModule> module;
  if (!map_.RetrieveRangeAtIndex(sequence, &module, NULL /* base */,
                                 NULL /* delta */, NULL /* size */)) {
    BPLOG(ERROR) << "RetrieveRangeAtIndex failed for sequence " << sequence;
    return NULL;
  }

  return module.get();
}

const CodeModule* BasicCodeModules::GetModuleAtIndex(
    unsigned int index) const {
  // This class stores everything in a RangeMap, without any more-efficient
  // way to walk the list of CodeModule objects.  Implement GetModuleAtIndex
  // using GetModuleAtSequence, which meets all of the requirements, and
  // in addition, guarantees ordering.
  return GetModuleAtSequence(index);
}

const CodeModules* BasicCodeModules::Copy() const {
  return new BasicCodeModules(this, map_.GetMergeStrategy());
}

vector<linked_ptr<const CodeModule> >
BasicCodeModules::GetShrunkRangeModules() const {
  return shrunk_range_modules_;
}

}  // namespace google_breakpad
