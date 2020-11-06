// Copyright (c) 2019, Google Inc.
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
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE

#include <limits.h>
#include <stdio.h>

#include "processor/range_map-inl.h"

#include "breakpad_googletest_includes.h"
#include "processor/linked_ptr.h"
#include "processor/logging.h"

namespace {

using google_breakpad::linked_ptr;
using google_breakpad::MergeRangeStrategy;
using google_breakpad::RangeMap;

// A CountedObject holds an int.  A global (not thread safe!) count of
// allocated CountedObjects is maintained to help test memory management.
class CountedObject {
 public:
  explicit CountedObject(int id) : id_(id) { ++count_; }
  ~CountedObject() { --count_; }

  static int count() { return count_; }
  int id() const { return id_; }

 private:
  static int count_;
  int id_;
};

int CountedObject::count_;

typedef int AddressType;
typedef RangeMap<AddressType, linked_ptr<CountedObject>> TestMap;

// Same range cannot be stored wice.
TEST(RangeMapTruncateLower, SameRange) {
  TestMap range_map;
  range_map.SetMergeStrategy(MergeRangeStrategy::kTruncateLower);
  linked_ptr<CountedObject> object_1(new CountedObject(1));
  EXPECT_TRUE(
      range_map.StoreRange(0 /* base address */, 100 /* size */, object_1));

  // Same range cannot be stored wice.
  linked_ptr<CountedObject> object_2(new CountedObject(2));
  EXPECT_FALSE(
      range_map.StoreRange(0 /* base address */, 100 /* size */, object_2));
}

// If a range is completely contained by another range, then the larger range
// should be truncated.
TEST(RangeMapTruncateLower, CompletelyContained) {
  TestMap range_map;
  range_map.SetMergeStrategy(MergeRangeStrategy::kTruncateLower);
  // Larger range is added first.
  linked_ptr<CountedObject> object_1(new CountedObject(1));
  EXPECT_TRUE(
      range_map.StoreRange(0 /* base address */, 100 /* size */, object_1));
  // Smaller (contained) range is added second.
  linked_ptr<CountedObject> object_2(new CountedObject(2));
  EXPECT_TRUE(
      range_map.StoreRange(10 /* base address */, 80 /* size */, object_2));
  linked_ptr<CountedObject> object;
  AddressType retrieved_base = AddressType();
  AddressType retrieved_delta = AddressType();
  AddressType retrieved_size = AddressType();
  // The first range contains the second, so the first range should have been
  // shrunk to [0, 10].  Range [90, 99] should be free.
  EXPECT_FALSE(range_map.RetrieveRange(90, &object, &retrieved_base,
                                       &retrieved_delta, &retrieved_size));
  EXPECT_FALSE(range_map.RetrieveRange(99, &object, &retrieved_base,
                                       &retrieved_delta, &retrieved_size));
  EXPECT_TRUE(range_map.RetrieveRange(9, &object, &retrieved_base,
                                      &retrieved_delta, &retrieved_size));
  EXPECT_EQ(1, object->id());
  EXPECT_EQ(0, retrieved_base);
  EXPECT_EQ(0, retrieved_delta);
  EXPECT_EQ(10, retrieved_size);
  // Validate the properties of the smaller range (should be untouched).
  EXPECT_TRUE(range_map.RetrieveRange(10, &object, &retrieved_base,
                                      &retrieved_delta, &retrieved_size));
  EXPECT_EQ(2, object->id());
  EXPECT_EQ(10, retrieved_base);
  EXPECT_EQ(0, retrieved_delta);
  EXPECT_EQ(80, retrieved_size);
}

// Same as the previous test, however the larger range is added second.
TEST(RangeMapTruncateLower, CompletelyContained_LargerAddedSecond) {
  TestMap range_map;
  range_map.SetMergeStrategy(MergeRangeStrategy::kTruncateLower);
  // Smaller (contained) range is added first.
  linked_ptr<CountedObject> object_1(new CountedObject(1));
  EXPECT_TRUE(
      range_map.StoreRange(10 /* base address */, 80 /* size */, object_1));
  // Larger range is added second.
  linked_ptr<CountedObject> object_2(new CountedObject(2));
  EXPECT_TRUE(
      range_map.StoreRange(0 /* base address */, 100 /* size */, object_2));
  linked_ptr<CountedObject> object;
  AddressType retrieved_base = AddressType();
  AddressType retrieved_delta = AddressType();
  AddressType retrieved_size = AddressType();
  // The second range contains the first, so the second range should have been
  // truncated to [0, 9].  Range [90, 99] should be free.
  EXPECT_FALSE(range_map.RetrieveRange(90, &object, &retrieved_base,
                                       &retrieved_delta, &retrieved_size));
  EXPECT_FALSE(range_map.RetrieveRange(99, &object, &retrieved_base,
                                       &retrieved_delta, &retrieved_size));
  EXPECT_TRUE(range_map.RetrieveRange(9, &object, &retrieved_base,
                                      &retrieved_delta, &retrieved_size));
  EXPECT_EQ(2, object->id());
  EXPECT_EQ(0, retrieved_base);
  EXPECT_EQ(0, retrieved_delta);
  EXPECT_EQ(10, retrieved_size);
  // Validate the properties of the smaller range (should be untouched).
  EXPECT_TRUE(range_map.RetrieveRange(10, &object, &retrieved_base,
                                      &retrieved_delta, &retrieved_size));
  EXPECT_EQ(1, object->id());
  EXPECT_EQ(10, retrieved_base);
  EXPECT_EQ(0, retrieved_delta);
  EXPECT_EQ(80, retrieved_size);
}

TEST(RangeMapTruncateLower, PartialOverlap_AtBeginning) {
  TestMap range_map;
  range_map.SetMergeStrategy(MergeRangeStrategy::kTruncateLower);
  linked_ptr<CountedObject> object_1(new CountedObject(1));
  EXPECT_TRUE(
      range_map.StoreRange(0 /* base address */, 100 /* size */, object_1));

  // Partial overlap at the beginning of the new range.
  linked_ptr<CountedObject> object_2(new CountedObject(2));
  EXPECT_TRUE(
      range_map.StoreRange(90 /* base address */, 110 /* size */, object_2));

  linked_ptr<CountedObject> object;
  AddressType retrieved_base = AddressType();
  AddressType retrieved_delta = AddressType();
  AddressType retrieved_size = AddressType();
  // The first range should be truncated, so 99 should address the second range.
  EXPECT_TRUE(range_map.RetrieveRange(99, &object, &retrieved_base,
                                      &retrieved_delta, &retrieved_size));
  EXPECT_EQ(2, object->id());
  EXPECT_EQ(90, retrieved_base);
  EXPECT_EQ(0, retrieved_delta);
  EXPECT_EQ(110, retrieved_size);
  // Validate the properties of the truncated range.
  EXPECT_TRUE(range_map.RetrieveRange(89, &object, &retrieved_base,
                                      &retrieved_delta, &retrieved_size));
  EXPECT_EQ(1, object->id());
  EXPECT_EQ(0, retrieved_base);
  EXPECT_EQ(0, retrieved_delta);
  EXPECT_EQ(90, retrieved_size);
}

TEST(RangeMapTruncateLower, PartialOverlap_AtEnd) {
  TestMap range_map;
  range_map.SetMergeStrategy(MergeRangeStrategy::kTruncateLower);
  linked_ptr<CountedObject> object_1(new CountedObject(1));
  EXPECT_TRUE(
      range_map.StoreRange(50 /* base address */, 50 /* size */, object_1));

  // Partial overlap at the end of the new range.
  linked_ptr<CountedObject> object_2(new CountedObject(2));
  EXPECT_TRUE(
      range_map.StoreRange(0 /* base address */, 70 /* size */, object_2));

  linked_ptr<CountedObject> object;
  AddressType retrieved_base = AddressType();
  AddressType retrieved_delta = AddressType();
  AddressType retrieved_size = AddressType();
  // The second range should be truncated so 69 addresses the first range.
  EXPECT_TRUE(range_map.RetrieveRange(69, &object, &retrieved_base,
                                      &retrieved_delta, &retrieved_size));
  EXPECT_EQ(1, object->id());
  EXPECT_EQ(50, retrieved_base);
  EXPECT_EQ(0, retrieved_delta);
  EXPECT_EQ(50, retrieved_size);
  // Validate the properties of the truncated range.
  EXPECT_TRUE(range_map.RetrieveRange(49, &object, &retrieved_base,
                                      &retrieved_delta, &retrieved_size));
  EXPECT_EQ(2, object->id());
  EXPECT_EQ(0, retrieved_base);
  EXPECT_EQ(0, retrieved_delta);
  EXPECT_EQ(50, retrieved_size);
}

// A new range is overlapped at both ends.  The new range and the range
// that overlaps at the beginning should be truncated.  The range that overlaps
// at the end should be left untouched.
TEST(RangeMapTruncateLower, OverlapAtBothEnds) {
  TestMap range_map;
  range_map.SetMergeStrategy(MergeRangeStrategy::kTruncateLower);
  // This should overlap object_3 at the beginning.
  linked_ptr<CountedObject> object_1(new CountedObject(1));
  EXPECT_TRUE(
      range_map.StoreRange(0 /* base address */, 100 /* size */, object_1));

  // This should overlap object_3 at the end.
  linked_ptr<CountedObject> object_2(new CountedObject(2));
  EXPECT_TRUE(
      range_map.StoreRange(100 /* base address */, 100 /* size */, object_2));

  // This should be overlapped on both ends by object_1 and object_2.
  linked_ptr<CountedObject> object_3(new CountedObject(3));
  EXPECT_TRUE(
      range_map.StoreRange(50 /* base address */, 100 /* size */, object_3));

  linked_ptr<CountedObject> object;
  AddressType retrieved_base = AddressType();
  AddressType retrieved_delta = AddressType();
  AddressType retrieved_size = AddressType();
  // The first range should be truncated.
  EXPECT_TRUE(range_map.RetrieveRange(0, &object, &retrieved_base,
                                      &retrieved_delta, &retrieved_size));
  EXPECT_EQ(1, object->id());
  EXPECT_EQ(0, retrieved_base);
  EXPECT_EQ(0, retrieved_delta);
  EXPECT_EQ(50, retrieved_size);
  // The second range should be intact.
  EXPECT_TRUE(range_map.RetrieveRange(150, &object, &retrieved_base,
                                      &retrieved_delta, &retrieved_size));
  EXPECT_EQ(2, object->id());
  EXPECT_EQ(100, retrieved_base);
  EXPECT_EQ(0, retrieved_delta);
  EXPECT_EQ(100, retrieved_size);
  // The third range (in the middle) should be truncated.
  EXPECT_TRUE(range_map.RetrieveRange(99, &object, &retrieved_base,
                                      &retrieved_delta, &retrieved_size));
  EXPECT_EQ(3, object->id());
  EXPECT_EQ(50, retrieved_base);
  EXPECT_EQ(0, retrieved_delta);
  EXPECT_EQ(50, retrieved_size);
}

TEST(RangeMapTruncateLower, MultipleConflicts) {
  TestMap range_map;
  range_map.SetMergeStrategy(MergeRangeStrategy::kTruncateLower);
  // This should overlap with object_3.
  linked_ptr<CountedObject> object_1(new CountedObject(1));
  EXPECT_TRUE(
      range_map.StoreRange(10 /* base address */, 90 /* size */, object_1));

  // This should also overlap with object_3 but after object_1.
  linked_ptr<CountedObject> object_2(new CountedObject(2));
  EXPECT_TRUE(
      range_map.StoreRange(100 /* base address */, 100 /* size */, object_2));

  // This should be overlapped on both object_1 and object_2.
  linked_ptr<CountedObject> object_3(new CountedObject(3));
  EXPECT_TRUE(
      range_map.StoreRange(0 /* base address */, 300 /* size */, object_3));

  linked_ptr<CountedObject> object;
  AddressType retrieved_base = AddressType();
  AddressType retrieved_delta = AddressType();
  AddressType retrieved_size = AddressType();
  // The first range should be intact.
  EXPECT_TRUE(range_map.RetrieveRange(99, &object, &retrieved_base,
                                      &retrieved_delta, &retrieved_size));
  EXPECT_EQ(1, object->id());
  EXPECT_EQ(10, retrieved_base);
  EXPECT_EQ(0, retrieved_delta);
  EXPECT_EQ(90, retrieved_size);
  // The second range should be intact.
  EXPECT_TRUE(range_map.RetrieveRange(199, &object, &retrieved_base,
                                      &retrieved_delta, &retrieved_size));
  EXPECT_EQ(2, object->id());
  EXPECT_EQ(100, retrieved_base);
  EXPECT_EQ(0, retrieved_delta);
  EXPECT_EQ(100, retrieved_size);
  // The third range should be truncated.
  EXPECT_TRUE(range_map.RetrieveRange(9, &object, &retrieved_base,
                                      &retrieved_delta, &retrieved_size));
  EXPECT_EQ(3, object->id());
  EXPECT_EQ(0, retrieved_base);
  EXPECT_EQ(0, retrieved_delta);
  EXPECT_EQ(10, retrieved_size);
}

// Adding two ranges without overlap should succeed and the ranges should
// be left intact.
TEST(RangeMapTruncateLower, NoConflicts) {
  TestMap range_map;
  range_map.SetMergeStrategy(MergeRangeStrategy::kTruncateLower);
  // Adding range 1.
  linked_ptr<CountedObject> object_1(new CountedObject(1));
  EXPECT_TRUE(
      range_map.StoreRange(10 /* base address */, 90 /* size */, object_1));

  // Adding range 2 - no overlap with range 1.
  linked_ptr<CountedObject> object_2(new CountedObject(2));
  EXPECT_TRUE(
      range_map.StoreRange(110 /* base address */, 90 /* size */, object_2));

  linked_ptr<CountedObject> object;
  AddressType retrieved_base = AddressType();
  AddressType retrieved_delta = AddressType();
  AddressType retrieved_size = AddressType();
  // The first range should be intact.
  EXPECT_TRUE(range_map.RetrieveRange(99, &object, &retrieved_base,
                                      &retrieved_delta, &retrieved_size));
  EXPECT_EQ(1, object->id());
  EXPECT_EQ(10, retrieved_base);
  EXPECT_EQ(0, retrieved_delta);
  EXPECT_EQ(90, retrieved_size);
  // The second range should be intact.
  EXPECT_TRUE(range_map.RetrieveRange(199, &object, &retrieved_base,
                                      &retrieved_delta, &retrieved_size));
  EXPECT_EQ(2, object->id());
  EXPECT_EQ(110, retrieved_base);
  EXPECT_EQ(0, retrieved_delta);
  EXPECT_EQ(90, retrieved_size);
}

}  // namespace
