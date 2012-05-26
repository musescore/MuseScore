/*
 * Copyright 2008 Google Inc. All Rights Reserved.
 * Author: fraser@google.com (Neil Fraser)
 * Author: mikeslemmer@gmail.com (Mike Slemmer)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Diff Match and Patch -- Test Harness
 * http://code.google.com/p/google-diff-match-patch/
 */

#ifndef DIFF_MATCH_PATCH_TEST_H
#define DIFF_MATCH_PATCH_TEST_H

class diff_match_patch_test {
 public:
  diff_match_patch_test();
  void run_all_tests();

  //  DIFF TEST FUNCTIONS
  void testDiffCommonPrefix();
  void testDiffCommonSuffix();
  void testDiffCommonOverlap();
  void testDiffHalfmatch();
  void testDiffLinesToChars();
  void testDiffCharsToLines();
  void testDiffCleanupMerge();
  void testDiffCleanupSemanticLossless();
  void testDiffCleanupSemantic();
  void testDiffCleanupEfficiency();
  void testDiffPrettyHtml();
  void testDiffText();
  void testDiffDelta();
  void testDiffXIndex();
  void testDiffLevenshtein();
  void testDiffBisect();
  void testDiffMain();

  //  MATCH TEST FUNCTIONS
  void testMatchAlphabet();
  void testMatchBitap();
  void testMatchMain();

  //  PATCH TEST FUNCTIONS
  void testPatchObj();
  void testPatchFromText();
  void testPatchToText();
  void testPatchAddContext();
  void testPatchMake();
  void testPatchSplitMax();
  void testPatchAddPadding();
  void testPatchApply();

 private:
  diff_match_patch dmp;

  // Define equality.
  void assertEquals(const QString &strCase, int n1, int n2);
  void assertEquals(const QString &strCase, const QString &s1, const QString &s2);
  void assertEquals(const QString &strCase, const Diff &d1, const Diff &d2);
  void assertEquals(const QString &strCase, const QList<Diff> &list1, const QList<Diff> &list2);
  void assertEquals(const QString &strCase, const QList<QVariant> &list1, const QList<QVariant> &list2);
  void assertEquals(const QString &strCase, const QVariant &var1, const QVariant &var2);
  void assertEquals(const QString &strCase, const QMap<QChar, int> &m1, const QMap<QChar, int> &m2);
  void assertEquals(const QString &strCase, const QStringList &list1, const QStringList &list2);
  void assertTrue(const QString &strCase, bool value);
  void assertFalse(const QString &strCase, bool value);
  void assertEmpty(const QString &strCase, const QStringList &list);

  // Construct the two texts which made up the diff originally.
  QStringList diff_rebuildtexts(QList<Diff> diffs);
  // Private function for quickly building lists of diffs.
  QList<Diff> diffList(
      // Diff(INSERT, NULL) is invalid and thus is used as the default argument.
      Diff d1 = Diff(INSERT, NULL), Diff d2 = Diff(INSERT, NULL),
      Diff d3 = Diff(INSERT, NULL), Diff d4 = Diff(INSERT, NULL),
      Diff d5 = Diff(INSERT, NULL), Diff d6 = Diff(INSERT, NULL),
      Diff d7 = Diff(INSERT, NULL), Diff d8 = Diff(INSERT, NULL),
      Diff d9 = Diff(INSERT, NULL), Diff d10 = Diff(INSERT, NULL));
};

#endif // DIFF_MATCH_PATCH_TEST_H
