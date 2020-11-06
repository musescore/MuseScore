// Copyright (c) 2017, Google Inc.
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

#include <algorithm>
#include <string>

#include "breakpad_googletest_includes.h"
#include "common/long_string_dictionary.h"

namespace google_breakpad {

using std::string;

TEST(LongStringDictionary, LongStringDictionary) {
  // Make a new dictionary
  LongStringDictionary dict;

  // Set three distinct values on three keys
  dict.SetKeyValue("key1", "value1");
  dict.SetKeyValue("key2", "value2");
  dict.SetKeyValue("key3", "value3");

  EXPECT_EQ("value1", dict.GetValueForKey("key1"));
  EXPECT_EQ("value2", dict.GetValueForKey("key2"));
  EXPECT_EQ("value3", dict.GetValueForKey("key3"));
  EXPECT_EQ(3u, dict.GetCount());
  // try an unknown key
  EXPECT_EQ("", dict.GetValueForKey("key4"));

  // Remove a key
  dict.RemoveKey("key3");

  // Now make sure it's not there anymore
  EXPECT_EQ("", dict.GetValueForKey("key3"));

  // Remove by setting value to NULL
  dict.SetKeyValue("key2", NULL);

  // Now make sure it's not there anymore
  EXPECT_EQ("", dict.GetValueForKey("key2"));
}

// Add a bunch of values to the dictionary, remove some entries in the middle,
// and then add more.
TEST(LongStringDictionary, Iterator) {
  LongStringDictionary* dict = new LongStringDictionary();
  ASSERT_TRUE(dict);

  char key[LongStringDictionary::key_size];
  char value[LongStringDictionary::value_size];

  const int kDictionaryCapacity = LongStringDictionary::num_entries;
  const int kPartitionIndex = kDictionaryCapacity - 5;

  // We assume at least this size in the tests below
  ASSERT_GE(kDictionaryCapacity, 64);

  // We'll keep track of the number of key/value pairs we think should
  // be in the dictionary
  int expectedDictionarySize = 0;

  // Set a bunch of key/value pairs like key0/value0, key1/value1, ...
  for (int i = 0; i < kPartitionIndex; ++i) {
    sprintf(key, "key%d", i);
    sprintf(value, "value%d", i);
    dict->SetKeyValue(key, value);
  }
  expectedDictionarySize = kPartitionIndex;

  // set a couple of the keys twice (with the same value) - should be nop
  dict->SetKeyValue("key2", "value2");
  dict->SetKeyValue("key4", "value4");
  dict->SetKeyValue("key15", "value15");

  // Remove some random elements in the middle
  dict->RemoveKey("key7");
  dict->RemoveKey("key18");
  dict->RemoveKey("key23");
  dict->RemoveKey("key31");
  expectedDictionarySize -= 4; // we just removed four key/value pairs

  // Set some more key/value pairs like key59/value59, key60/value60, ...
  for (int i = kPartitionIndex; i < kDictionaryCapacity; ++i) {
    sprintf(key, "key%d", i);
    sprintf(value, "value%d", i);
    dict->SetKeyValue(key, value);
  }
  expectedDictionarySize += kDictionaryCapacity - kPartitionIndex;

  // Now create an iterator on the dictionary
  SimpleStringDictionary::Iterator iter(*dict);

  // We then verify that it iterates through exactly the number of
  // key/value pairs we expect, and that they match one-for-one with what we
  // would expect.  The ordering of the iteration does not matter...

  // used to keep track of number of occurrences found for key/value pairs
  int count[kDictionaryCapacity];
  memset(count, 0, sizeof(count));

  int totalCount = 0;

  const SimpleStringDictionary::Entry* entry;
  while ((entry = iter.Next())) {
    totalCount++;

    // Extract keyNumber from a string of the form key<keyNumber>
    int keyNumber;
    sscanf(entry->key, "key%d", &keyNumber);

    // Extract valueNumber from a string of the form value<valueNumber>
    int valueNumber;
    sscanf(entry->value, "value%d", &valueNumber);

    // The value number should equal the key number since that's how we set them
    EXPECT_EQ(keyNumber, valueNumber);

    // Key and value numbers should be in proper range:
    // 0 <= keyNumber < kDictionaryCapacity
    bool isKeyInGoodRange = (keyNumber >= 0 && keyNumber < kDictionaryCapacity);
    bool isValueInGoodRange =
        (valueNumber >= 0 && valueNumber < kDictionaryCapacity);
    EXPECT_TRUE(isKeyInGoodRange);
    EXPECT_TRUE(isValueInGoodRange);

    if (isKeyInGoodRange && isValueInGoodRange) {
      ++count[keyNumber];
    }
  }

  // Make sure each of the key/value pairs showed up exactly one time, except
  // for the ones which we removed.
  for (size_t i = 0; i < kDictionaryCapacity; ++i) {
    // Skip over key7, key18, key23, and key31, since we removed them
    if (!(i == 7 || i == 18 || i == 23 || i == 31)) {
      EXPECT_EQ(count[i], 1);
    }
  }

  // Make sure the number of iterations matches the expected dictionary size.
  EXPECT_EQ(totalCount, expectedDictionarySize);
}

TEST(LongStringDictionary, AddRemove) {
  LongStringDictionary dict;
  dict.SetKeyValue("rob", "ert");
  dict.SetKeyValue("mike", "pink");
  dict.SetKeyValue("mark", "allays");

  EXPECT_EQ(3u, dict.GetCount());
  EXPECT_EQ("ert", dict.GetValueForKey("rob"));
  EXPECT_EQ("pink", dict.GetValueForKey("mike"));
  EXPECT_EQ("allays", dict.GetValueForKey("mark"));

  dict.RemoveKey("mike");

  EXPECT_EQ(2u, dict.GetCount());
  EXPECT_EQ("", dict.GetValueForKey("mike"));

  dict.SetKeyValue("mark", "mal");
  EXPECT_EQ(2u, dict.GetCount());
  EXPECT_EQ("mal", dict.GetValueForKey("mark"));

  dict.RemoveKey("mark");
  EXPECT_EQ(1u, dict.GetCount());
  EXPECT_EQ("", dict.GetValueForKey("mark"));
}

TEST(LongStringDictionary, AddRemoveLongValue) {
  LongStringDictionary dict;

  string long_value = string(256, 'x');
  dict.SetKeyValue("rob", long_value.c_str());

  EXPECT_EQ(2u, dict.GetCount());

  string long_value_part_1 = string(255, 'x');

  EXPECT_EQ(long_value_part_1, dict.GetValueForKey("rob__1"));
  EXPECT_EQ("x", dict.GetValueForKey("rob__2"));

  EXPECT_EQ(long_value, dict.GetValueForKey("rob"));

  dict.RemoveKey("rob");
  EXPECT_EQ(0u, dict.GetCount());
}

TEST(LongStringDictionary, AddRemoveSuperLongValue) {
  LongStringDictionary dict;

  string long_value = string(255 * 10, 'x');
  dict.SetKeyValue("rob", long_value.c_str());

  EXPECT_EQ(10u, dict.GetCount());

  string long_value_part = string(255, 'x');

  EXPECT_EQ(long_value_part, dict.GetValueForKey("rob__1"));
  EXPECT_EQ(long_value_part, dict.GetValueForKey("rob__2"));
  EXPECT_EQ(long_value_part, dict.GetValueForKey("rob__3"));
  EXPECT_EQ(long_value_part, dict.GetValueForKey("rob__4"));
  EXPECT_EQ(long_value_part, dict.GetValueForKey("rob__5"));
  EXPECT_EQ(long_value_part, dict.GetValueForKey("rob__6"));
  EXPECT_EQ(long_value_part, dict.GetValueForKey("rob__7"));
  EXPECT_EQ(long_value_part, dict.GetValueForKey("rob__8"));
  EXPECT_EQ(long_value_part, dict.GetValueForKey("rob__9"));
  EXPECT_EQ(long_value_part, dict.GetValueForKey("rob__10"));
  EXPECT_EQ(10u, dict.GetCount());

  EXPECT_EQ(long_value, dict.GetValueForKey("rob"));

  dict.RemoveKey("rob");
  EXPECT_EQ(0u, dict.GetCount());
}

TEST(LongStringDictionary, TruncateSuperLongValue) {
  LongStringDictionary dict;

  string long_value = string(255 * 11, 'x');
  dict.SetKeyValue("rob", long_value.c_str());

  EXPECT_EQ(10u, dict.GetCount());

  string long_value_part = string(255, 'x');

  EXPECT_EQ(long_value_part, dict.GetValueForKey("rob__1"));
  EXPECT_EQ(long_value_part, dict.GetValueForKey("rob__2"));
  EXPECT_EQ(long_value_part, dict.GetValueForKey("rob__3"));
  EXPECT_EQ(long_value_part, dict.GetValueForKey("rob__4"));
  EXPECT_EQ(long_value_part, dict.GetValueForKey("rob__5"));
  EXPECT_EQ(long_value_part, dict.GetValueForKey("rob__6"));
  EXPECT_EQ(long_value_part, dict.GetValueForKey("rob__7"));
  EXPECT_EQ(long_value_part, dict.GetValueForKey("rob__8"));
  EXPECT_EQ(long_value_part, dict.GetValueForKey("rob__9"));
  EXPECT_EQ(long_value_part, dict.GetValueForKey("rob__10"));
  EXPECT_EQ(10u, dict.GetCount());

  string expected_long_value = string(255 * 10, 'x');
  EXPECT_EQ(expected_long_value, dict.GetValueForKey("rob"));

  dict.RemoveKey("rob");
  EXPECT_EQ(0u, dict.GetCount());
}

TEST(LongStringDictionary, OverrideLongValue) {
  LongStringDictionary dict;

  string long_value = string(255 * 10, 'x');
  dict.SetKeyValue("rob", long_value.c_str());

  EXPECT_EQ(10u, dict.GetCount());
  EXPECT_EQ(long_value, dict.GetValueForKey("rob"));

  dict.SetKeyValue("rob", "short_value");

  EXPECT_EQ(1u, dict.GetCount());
  EXPECT_EQ("short_value", dict.GetValueForKey("rob"));
}

TEST(LongStringDictionary, OverrideShortValue) {
  LongStringDictionary dict;

  dict.SetKeyValue("rob", "short_value");

  EXPECT_EQ(1u, dict.GetCount());
  EXPECT_EQ("short_value", dict.GetValueForKey("rob"));

  string long_value = string(255 * 10, 'x');
  dict.SetKeyValue("rob", long_value.c_str());

  EXPECT_EQ(10u, dict.GetCount());
  EXPECT_EQ(long_value, dict.GetValueForKey("rob"));
}

} // namespace google_breakpad
