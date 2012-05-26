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

// Code known to compile and run with Qt 4.3 through Qt 4.7.
#include <QtCore>
#include "diff_match_patch.h"
#include "diff_match_patch_test.h"

int main(int argc, char **argv) {
  diff_match_patch_test dmp_test;
  qDebug("Starting diff_match_patch unit tests.");
  dmp_test.run_all_tests();
  qDebug("Done.");
  return 0;
  Q_UNUSED(argc)
  Q_UNUSED(argv)
}


diff_match_patch_test::diff_match_patch_test() {
}

void diff_match_patch_test::run_all_tests() {
  QTime t;
  t.start();
  try {
    testDiffCommonPrefix();
    testDiffCommonSuffix();
    testDiffCommonOverlap();
    testDiffHalfmatch();
    testDiffLinesToChars();
    testDiffCharsToLines();
    testDiffCleanupMerge();
    testDiffCleanupSemanticLossless();
    testDiffCleanupSemantic();
    testDiffCleanupEfficiency();
    testDiffPrettyHtml();
    testDiffText();
    testDiffDelta();
    testDiffXIndex();
    testDiffLevenshtein();
    testDiffBisect();
    testDiffMain();

    testMatchAlphabet();
    testMatchBitap();
    testMatchMain();

    testPatchObj();
    testPatchFromText();
    testPatchToText();
    testPatchAddContext();
    testPatchMake();
    testPatchSplitMax();
    testPatchAddPadding();
    testPatchApply();
    qDebug("All tests passed.");
  } catch (QString strCase) {
    qDebug("Test failed: %s", qPrintable(strCase));
  }
  qDebug("Total time: %d ms", t.elapsed());
}

//  DIFF TEST FUNCTIONS

void diff_match_patch_test::testDiffCommonPrefix() {
  // Detect any common prefix.
  assertEquals("diff_commonPrefix: Null case.", 0, dmp.diff_commonPrefix("abc", "xyz"));

  assertEquals("diff_commonPrefix: Non-null case.", 4, dmp.diff_commonPrefix("1234abcdef", "1234xyz"));

  assertEquals("diff_commonPrefix: Whole case.", 4, dmp.diff_commonPrefix("1234", "1234xyz"));
}

void diff_match_patch_test::testDiffCommonSuffix() {
  // Detect any common suffix.
  assertEquals("diff_commonSuffix: Null case.", 0, dmp.diff_commonSuffix("abc", "xyz"));

  assertEquals("diff_commonSuffix: Non-null case.", 4, dmp.diff_commonSuffix("abcdef1234", "xyz1234"));

  assertEquals("diff_commonSuffix: Whole case.", 4, dmp.diff_commonSuffix("1234", "xyz1234"));
}

void diff_match_patch_test::testDiffCommonOverlap() {
  // Detect any suffix/prefix overlap.
  assertEquals("diff_commonOverlap: Null case.", 0, dmp.diff_commonOverlap("", "abcd"));

  assertEquals("diff_commonOverlap: Whole case.", 3, dmp.diff_commonOverlap("abc", "abcd"));

  assertEquals("diff_commonOverlap: No overlap.", 0, dmp.diff_commonOverlap("123456", "abcd"));

  assertEquals("diff_commonOverlap: Overlap.", 3, dmp.diff_commonOverlap("123456xxx", "xxxabcd"));
}

void diff_match_patch_test::testDiffHalfmatch() {
  // Detect a halfmatch.
  dmp.Diff_Timeout = 1;
  assertEmpty("diff_halfMatch: No match #1.", dmp.diff_halfMatch("1234567890", "abcdef"));

  assertEmpty("diff_halfMatch: No match #2.", dmp.diff_halfMatch("12345", "23"));

  assertEquals("diff_halfMatch: Single Match #1.", QString("12,90,a,z,345678").split(","), dmp.diff_halfMatch("1234567890", "a345678z"));

  assertEquals("diff_halfMatch: Single Match #2.", QString("a,z,12,90,345678").split(","), dmp.diff_halfMatch("a345678z", "1234567890"));

  assertEquals("diff_halfMatch: Single Match #3.", QString("abc,z,1234,0,56789").split(","), dmp.diff_halfMatch("abc56789z", "1234567890"));

  assertEquals("diff_halfMatch: Single Match #4.", QString("a,xyz,1,7890,23456").split(","), dmp.diff_halfMatch("a23456xyz", "1234567890"));

  assertEquals("diff_halfMatch: Multiple Matches #1.", QString("12123,123121,a,z,1234123451234").split(","), dmp.diff_halfMatch("121231234123451234123121", "a1234123451234z"));

  assertEquals("diff_halfMatch: Multiple Matches #2.", QString(",-=-=-=-=-=,x,,x-=-=-=-=-=-=-=").split(","), dmp.diff_halfMatch("x-=-=-=-=-=-=-=-=-=-=-=-=", "xx-=-=-=-=-=-=-="));

  assertEquals("diff_halfMatch: Multiple Matches #3.", QString("-=-=-=-=-=,,,y,-=-=-=-=-=-=-=y").split(","), dmp.diff_halfMatch("-=-=-=-=-=-=-=-=-=-=-=-=y", "-=-=-=-=-=-=-=yy"));

  // Optimal diff would be -q+x=H-i+e=lloHe+Hu=llo-Hew+y not -qHillo+x=HelloHe-w+Hulloy
  assertEquals("diff_halfMatch: Non-optimal halfmatch.", QString("qHillo,w,x,Hulloy,HelloHe").split(","), dmp.diff_halfMatch("qHilloHelloHew", "xHelloHeHulloy"));

  dmp.Diff_Timeout = 0;
  assertEmpty("diff_halfMatch: Optimal no halfmatch.", dmp.diff_halfMatch("qHilloHelloHew", "xHelloHeHulloy"));
}

void diff_match_patch_test::testDiffLinesToChars() {
  // Convert lines down to characters.
  QStringList tmpVector;
  QList<QVariant> tmpVarList;
  tmpVector.append("");
  tmpVector.append("alpha\n");
  tmpVector.append("beta\n");
  tmpVarList << QVariant::fromValue(QString() + QChar((ushort)1) + QChar((ushort)2) + QChar((ushort)1));  //(("\u0001\u0002\u0001"));
  tmpVarList << QVariant::fromValue(QString() + QChar((ushort)2) + QChar((ushort)1) + QChar((ushort)2));  // (("\u0002\u0001\u0002"));
  tmpVarList << QVariant::fromValue(tmpVector);
  assertEquals("diff_linesToChars:", tmpVarList, dmp.diff_linesToChars("alpha\nbeta\nalpha\n", "beta\nalpha\nbeta\n"));

  tmpVector.clear();
  tmpVarList.clear();
  tmpVector.append("");
  tmpVector.append("alpha\r\n");
  tmpVector.append("beta\r\n");
  tmpVector.append("\r\n");
  tmpVarList << QVariant::fromValue(QString(""));
  tmpVarList << QVariant::fromValue(QString() + QChar((ushort)1) + QChar((ushort)2) + QChar((ushort)3) + QChar((ushort)3));  // (("\u0001\u0002\u0003\u0003"));
  tmpVarList << QVariant::fromValue(tmpVector);
  assertEquals("diff_linesToChars:", tmpVarList, dmp.diff_linesToChars("", "alpha\r\nbeta\r\n\r\n\r\n"));

  tmpVector.clear();
  tmpVarList.clear();
  tmpVector.append("");
  tmpVector.append("a");
  tmpVector.append("b");
  tmpVarList << QVariant::fromValue(QString() + QChar((ushort)1));  // (("\u0001"));
  tmpVarList << QVariant::fromValue(QString() + QChar((ushort)2));  // (("\u0002"));
  tmpVarList << QVariant::fromValue(tmpVector);
  assertEquals("diff_linesToChars:", tmpVarList, dmp.diff_linesToChars("a", "b"));

  // More than 256 to reveal any 8-bit limitations.
  int n = 300;
  tmpVector.clear();
  tmpVarList.clear();
  QString lines;
  QString chars;
  for (int x = 1; x < n + 1; x++) {
    tmpVector.append(QString::number(x) + "\n");
    lines += QString::number(x) + "\n";
    chars += QChar(static_cast<ushort>(x));
  }
  assertEquals("diff_linesToChars: More than 256 (setup).", n, tmpVector.size());
  assertEquals("diff_linesToChars: More than 256 (setup).", n, chars.length());
  tmpVector.prepend("");
  tmpVarList << QVariant::fromValue(chars);
  tmpVarList << QVariant::fromValue(QString(""));
  tmpVarList << QVariant::fromValue(tmpVector);
  assertEquals("diff_linesToChars: More than 256.", tmpVarList, dmp.diff_linesToChars(lines, ""));
}

void diff_match_patch_test::testDiffCharsToLines() {
  // First check that Diff equality works.
  assertTrue("diff_charsToLines:", Diff(EQUAL, "a") == Diff(EQUAL, "a"));

  assertEquals("diff_charsToLines:", Diff(EQUAL, "a"), Diff(EQUAL, "a"));

  // Convert chars up to lines.
  QList<Diff> diffs;
  diffs << Diff(EQUAL, QString() + QChar((ushort)1) + QChar((ushort)2) + QChar((ushort)1));  // ("\u0001\u0002\u0001");
  diffs << Diff(INSERT, QString() + QChar((ushort)2) + QChar((ushort)1) + QChar((ushort)2));  // ("\u0002\u0001\u0002");
  QStringList tmpVector;
  tmpVector.append("");
  tmpVector.append("alpha\n");
  tmpVector.append("beta\n");
  dmp.diff_charsToLines(diffs, tmpVector);
  assertEquals("diff_charsToLines:", diffList(Diff(EQUAL, "alpha\nbeta\nalpha\n"), Diff(INSERT, "beta\nalpha\nbeta\n")), diffs);

  // More than 256 to reveal any 8-bit limitations.
  int n = 300;
  tmpVector.clear();
  QList<QVariant> tmpVarList;
  QString lines;
  QString chars;
  for (int x = 1; x < n + 1; x++) {
    tmpVector.append(QString::number(x) + "\n");
    lines += QString::number(x) + "\n";
    chars += QChar(static_cast<ushort>(x));
  }
  assertEquals("diff_linesToChars: More than 256 (setup).", n, tmpVector.size());
  assertEquals("diff_linesToChars: More than 256 (setup).", n, chars.length());
  tmpVector.prepend("");
  diffs = diffList(Diff(DELETE, chars));
  dmp.diff_charsToLines(diffs, tmpVector);
  assertEquals("diff_charsToLines: More than 256.", diffList(Diff(DELETE, lines)), diffs);
}

void diff_match_patch_test::testDiffCleanupMerge() {
  // Cleanup a messy diff.
  QList<Diff> diffs;
  dmp.diff_cleanupMerge(diffs);
  assertEquals("diff_cleanupMerge: Null case.", diffList(), diffs);

  diffs = diffList(Diff(EQUAL, "a"), Diff(DELETE, "b"), Diff(INSERT, "c"));
  dmp.diff_cleanupMerge(diffs);
  assertEquals("diff_cleanupMerge: No change case.", diffList(Diff(EQUAL, "a"), Diff(DELETE, "b"), Diff(INSERT, "c")), diffs);

  diffs = diffList(Diff(EQUAL, "a"), Diff(EQUAL, "b"), Diff(EQUAL, "c"));
  dmp.diff_cleanupMerge(diffs);
  assertEquals("diff_cleanupMerge: Merge equalities.", diffList(Diff(EQUAL, "abc")), diffs);

  diffs = diffList(Diff(DELETE, "a"), Diff(DELETE, "b"), Diff(DELETE, "c"));
  dmp.diff_cleanupMerge(diffs);
  assertEquals("diff_cleanupMerge: Merge deletions.", diffList(Diff(DELETE, "abc")), diffs);

  diffs = diffList(Diff(INSERT, "a"), Diff(INSERT, "b"), Diff(INSERT, "c"));
  dmp.diff_cleanupMerge(diffs);
  assertEquals("diff_cleanupMerge: Merge insertions.", diffList(Diff(INSERT, "abc")), diffs);

  diffs = diffList(Diff(DELETE, "a"), Diff(INSERT, "b"), Diff(DELETE, "c"), Diff(INSERT, "d"), Diff(EQUAL, "e"), Diff(EQUAL, "f"));
  dmp.diff_cleanupMerge(diffs);
  assertEquals("diff_cleanupMerge: Merge interweave.", diffList(Diff(DELETE, "ac"), Diff(INSERT, "bd"), Diff(EQUAL, "ef")), diffs);

  diffs = diffList(Diff(DELETE, "a"), Diff(INSERT, "abc"), Diff(DELETE, "dc"));
  dmp.diff_cleanupMerge(diffs);
  assertEquals("diff_cleanupMerge: Prefix and suffix detection.", diffList(Diff(EQUAL, "a"), Diff(DELETE, "d"), Diff(INSERT, "b"), Diff(EQUAL, "c")), diffs);

  diffs = diffList(Diff(EQUAL, "x"), Diff(DELETE, "a"), Diff(INSERT, "abc"), Diff(DELETE, "dc"), Diff(EQUAL, "y"));
  dmp.diff_cleanupMerge(diffs);
  assertEquals("diff_cleanupMerge: Prefix and suffix detection with equalities.", diffList(Diff(EQUAL, "xa"), Diff(DELETE, "d"), Diff(INSERT, "b"), Diff(EQUAL, "cy")), diffs);

  diffs = diffList(Diff(EQUAL, "a"), Diff(INSERT, "ba"), Diff(EQUAL, "c"));
  dmp.diff_cleanupMerge(diffs);
  assertEquals("diff_cleanupMerge: Slide edit left.", diffList(Diff(INSERT, "ab"), Diff(EQUAL, "ac")), diffs);

  diffs = diffList(Diff(EQUAL, "c"), Diff(INSERT, "ab"), Diff(EQUAL, "a"));
  dmp.diff_cleanupMerge(diffs);
  assertEquals("diff_cleanupMerge: Slide edit right.", diffList(Diff(EQUAL, "ca"), Diff(INSERT, "ba")), diffs);

  diffs = diffList(Diff(EQUAL, "a"), Diff(DELETE, "b"), Diff(EQUAL, "c"), Diff(DELETE, "ac"), Diff(EQUAL, "x"));
  dmp.diff_cleanupMerge(diffs);
  assertEquals("diff_cleanupMerge: Slide edit left recursive.", diffList(Diff(DELETE, "abc"), Diff(EQUAL, "acx")), diffs);

  diffs = diffList(Diff(EQUAL, "x"), Diff(DELETE, "ca"), Diff(EQUAL, "c"), Diff(DELETE, "b"), Diff(EQUAL, "a"));
  dmp.diff_cleanupMerge(diffs);
  assertEquals("diff_cleanupMerge: Slide edit right recursive.", diffList(Diff(EQUAL, "xca"), Diff(DELETE, "cba")), diffs);
}

void diff_match_patch_test::testDiffCleanupSemanticLossless() {
  // Slide diffs to match logical boundaries.
  QList<Diff> diffs = diffList();
  dmp.diff_cleanupSemanticLossless(diffs);
  assertEquals("diff_cleanupSemantic: Null case.", diffList(), diffs);

  diffs = diffList(Diff(EQUAL, "AAA\r\n\r\nBBB"), Diff(INSERT, "\r\nDDD\r\n\r\nBBB"), Diff(EQUAL, "\r\nEEE"));
  dmp.diff_cleanupSemanticLossless(diffs);
  assertEquals("diff_cleanupSemanticLossless: Blank lines.", diffList(Diff(EQUAL, "AAA\r\n\r\n"), Diff(INSERT, "BBB\r\nDDD\r\n\r\n"), Diff(EQUAL, "BBB\r\nEEE")), diffs);

  diffs = diffList(Diff(EQUAL, "AAA\r\nBBB"), Diff(INSERT, " DDD\r\nBBB"), Diff(EQUAL, " EEE"));
  dmp.diff_cleanupSemanticLossless(diffs);
  assertEquals("diff_cleanupSemanticLossless: Line boundaries.", diffList(Diff(EQUAL, "AAA\r\n"), Diff(INSERT, "BBB DDD\r\n"), Diff(EQUAL, "BBB EEE")), diffs);

  diffs = diffList(Diff(EQUAL, "The c"), Diff(INSERT, "ow and the c"), Diff(EQUAL, "at."));
  dmp.diff_cleanupSemanticLossless(diffs);
  assertEquals("diff_cleanupSemantic: Word boundaries.", diffList(Diff(EQUAL, "The "), Diff(INSERT, "cow and the "), Diff(EQUAL, "cat.")), diffs);

  diffs = diffList(Diff(EQUAL, "The-c"), Diff(INSERT, "ow-and-the-c"), Diff(EQUAL, "at."));
  dmp.diff_cleanupSemanticLossless(diffs);
  assertEquals("diff_cleanupSemantic: Alphanumeric boundaries.", diffList(Diff(EQUAL, "The-"), Diff(INSERT, "cow-and-the-"), Diff(EQUAL, "cat.")), diffs);

  diffs = diffList(Diff(EQUAL, "a"), Diff(DELETE, "a"), Diff(EQUAL, "ax"));
  dmp.diff_cleanupSemanticLossless(diffs);
  assertEquals("diff_cleanupSemantic: Hitting the start.", diffList(Diff(DELETE, "a"), Diff(EQUAL, "aax")), diffs);

  diffs = diffList(Diff(EQUAL, "xa"), Diff(DELETE, "a"), Diff(EQUAL, "a"));
  dmp.diff_cleanupSemanticLossless(diffs);
  assertEquals("diff_cleanupSemantic: Hitting the end.", diffList(Diff(EQUAL, "xaa"), Diff(DELETE, "a")), diffs);
}

void diff_match_patch_test::testDiffCleanupSemantic() {
  // Cleanup semantically trivial equalities.
  QList<Diff> diffs = diffList();
  dmp.diff_cleanupSemantic(diffs);
  assertEquals("diff_cleanupSemantic: Null case.", diffList(), diffs);

  diffs = diffList(Diff(DELETE, "ab"), Diff(INSERT, "cd"), Diff(EQUAL, "12"), Diff(DELETE, "e"));
  dmp.diff_cleanupSemantic(diffs);
  assertEquals("diff_cleanupSemantic: No elimination #1.", diffList(Diff(DELETE, "ab"), Diff(INSERT, "cd"), Diff(EQUAL, "12"), Diff(DELETE, "e")), diffs);

  diffs = diffList(Diff(DELETE, "abc"), Diff(INSERT, "ABC"), Diff(EQUAL, "1234"), Diff(DELETE, "wxyz"));
  dmp.diff_cleanupSemantic(diffs);
  assertEquals("diff_cleanupSemantic: No elimination #2.", diffList(Diff(DELETE, "abc"), Diff(INSERT, "ABC"), Diff(EQUAL, "1234"), Diff(DELETE, "wxyz")), diffs);

  diffs = diffList(Diff(DELETE, "a"), Diff(EQUAL, "b"), Diff(DELETE, "c"));
  dmp.diff_cleanupSemantic(diffs);
  assertEquals("diff_cleanupSemantic: Simple elimination.", diffList(Diff(DELETE, "abc"), Diff(INSERT, "b")), diffs);

  diffs = diffList(Diff(DELETE, "ab"), Diff(EQUAL, "cd"), Diff(DELETE, "e"), Diff(EQUAL, "f"), Diff(INSERT, "g"));
  dmp.diff_cleanupSemantic(diffs);
  assertEquals("diff_cleanupSemantic: Backpass elimination.", diffList(Diff(DELETE, "abcdef"), Diff(INSERT, "cdfg")), diffs);

  diffs = diffList(Diff(INSERT, "1"), Diff(EQUAL, "A"), Diff(DELETE, "B"), Diff(INSERT, "2"), Diff(EQUAL, "_"), Diff(INSERT, "1"), Diff(EQUAL, "A"), Diff(DELETE, "B"), Diff(INSERT, "2"));
  dmp.diff_cleanupSemantic(diffs);
  assertEquals("diff_cleanupSemantic: Multiple elimination.", diffList(Diff(DELETE, "AB_AB"), Diff(INSERT, "1A2_1A2")), diffs);

  diffs = diffList(Diff(EQUAL, "The c"), Diff(DELETE, "ow and the c"), Diff(EQUAL, "at."));
  dmp.diff_cleanupSemantic(diffs);
  assertEquals("diff_cleanupSemantic: Word boundaries.", diffList(Diff(EQUAL, "The "), Diff(DELETE, "cow and the "), Diff(EQUAL, "cat.")), diffs);

  diffs = diffList(Diff(DELETE, "abcxx"), Diff(INSERT, "xxdef"));
  dmp.diff_cleanupSemantic(diffs);
  assertEquals("diff_cleanupSemantic: Overlap elimination #1.", diffList(Diff(DELETE, "abc"), Diff(EQUAL, "xx"), Diff(INSERT, "def")), diffs);

  diffs = diffList(Diff(DELETE, "abcxx"), Diff(INSERT, "xxdef"), Diff(DELETE, "ABCXX"), Diff(INSERT, "XXDEF"));
  dmp.diff_cleanupSemantic(diffs);
  assertEquals("diff_cleanupSemantic: Overlap elimination #2.", diffList(Diff(DELETE, "abc"), Diff(EQUAL, "xx"), Diff(INSERT, "def"), Diff(DELETE, "ABC"), Diff(EQUAL, "XX"), Diff(INSERT, "DEF")), diffs);
}

void diff_match_patch_test::testDiffCleanupEfficiency() {
  // Cleanup operationally trivial equalities.
  dmp.Diff_EditCost = 4;
  QList<Diff> diffs = diffList();
  dmp.diff_cleanupEfficiency(diffs);
  assertEquals("diff_cleanupEfficiency: Null case.", diffList(), diffs);

  diffs = diffList(Diff(DELETE, "ab"), Diff(INSERT, "12"), Diff(EQUAL, "wxyz"), Diff(DELETE, "cd"), Diff(INSERT, "34"));
  dmp.diff_cleanupEfficiency(diffs);
  assertEquals("diff_cleanupEfficiency: No elimination.", diffList(Diff(DELETE, "ab"), Diff(INSERT, "12"), Diff(EQUAL, "wxyz"), Diff(DELETE, "cd"), Diff(INSERT, "34")), diffs);

  diffs = diffList(Diff(DELETE, "ab"), Diff(INSERT, "12"), Diff(EQUAL, "xyz"), Diff(DELETE, "cd"), Diff(INSERT, "34"));
  dmp.diff_cleanupEfficiency(diffs);
  assertEquals("diff_cleanupEfficiency: Four-edit elimination.", diffList(Diff(DELETE, "abxyzcd"), Diff(INSERT, "12xyz34")), diffs);

  diffs = diffList(Diff(INSERT, "12"), Diff(EQUAL, "x"), Diff(DELETE, "cd"), Diff(INSERT, "34"));
  dmp.diff_cleanupEfficiency(diffs);
  assertEquals("diff_cleanupEfficiency: Three-edit elimination.", diffList(Diff(DELETE, "xcd"), Diff(INSERT, "12x34")), diffs);

  diffs = diffList(Diff(DELETE, "ab"), Diff(INSERT, "12"), Diff(EQUAL, "xy"), Diff(INSERT, "34"), Diff(EQUAL, "z"), Diff(DELETE, "cd"), Diff(INSERT, "56"));
  dmp.diff_cleanupEfficiency(diffs);
  assertEquals("diff_cleanupEfficiency: Backpass elimination.", diffList(Diff(DELETE, "abxyzcd"), Diff(INSERT, "12xy34z56")), diffs);

  dmp.Diff_EditCost = 5;
  diffs = diffList(Diff(DELETE, "ab"), Diff(INSERT, "12"), Diff(EQUAL, "wxyz"), Diff(DELETE, "cd"), Diff(INSERT, "34"));
  dmp.diff_cleanupEfficiency(diffs);
  assertEquals("diff_cleanupEfficiency: High cost elimination.", diffList(Diff(DELETE, "abwxyzcd"), Diff(INSERT, "12wxyz34")), diffs);
  dmp.Diff_EditCost = 4;
}

void diff_match_patch_test::testDiffPrettyHtml() {
  // Pretty print.
  QList<Diff> diffs = diffList(Diff(EQUAL, "a\n"), Diff(DELETE, "<B>b</B>"), Diff(INSERT, "c&d"));
  assertEquals("diff_prettyHtml:", "<span>a&para;<br></span><del style=\"background:#ffe6e6;\">&lt;B&gt;b&lt;/B&gt;</del><ins style=\"background:#e6ffe6;\">c&amp;d</ins>", dmp.diff_prettyHtml(diffs));
}

void diff_match_patch_test::testDiffText() {
  // Compute the source and destination texts.
  QList<Diff> diffs = diffList(Diff(EQUAL, "jump"), Diff(DELETE, "s"), Diff(INSERT, "ed"), Diff(EQUAL, " over "), Diff(DELETE, "the"), Diff(INSERT, "a"), Diff(EQUAL, " lazy"));
  assertEquals("diff_text1:", "jumps over the lazy", dmp.diff_text1(diffs));
  assertEquals("diff_text2:", "jumped over a lazy", dmp.diff_text2(diffs));
}

void diff_match_patch_test::testDiffDelta() {
  // Convert a diff into delta string.
  QList<Diff> diffs = diffList(Diff(EQUAL, "jump"), Diff(DELETE, "s"), Diff(INSERT, "ed"), Diff(EQUAL, " over "), Diff(DELETE, "the"), Diff(INSERT, "a"), Diff(EQUAL, " lazy"), Diff(INSERT, "old dog"));
  QString text1 = dmp.diff_text1(diffs);
  assertEquals("diff_text1: Base text.", "jumps over the lazy", text1);

  QString delta = dmp.diff_toDelta(diffs);
  assertEquals("diff_toDelta:", "=4\t-1\t+ed\t=6\t-3\t+a\t=5\t+old dog", delta);

  // Convert delta string into a diff.
  assertEquals("diff_fromDelta: Normal.", diffs, dmp.diff_fromDelta(text1, delta));

  // Generates error (19 < 20).
  try {
    dmp.diff_fromDelta(text1 + "x", delta);
    assertFalse("diff_fromDelta: Too long.", true);
  } catch (QString ex) {
    // Exception expected.
  }

  // Generates error (19 > 18).
  try {
    dmp.diff_fromDelta(text1.mid(1), delta);
    assertFalse("diff_fromDelta: Too short.", true);
  } catch (QString ex) {
    // Exception expected.
  }

  // Generates error (%c3%xy invalid Unicode).
  /* This test does not work because QUrl::fromPercentEncoding("%xy") -> "?"
  try {
    dmp.diff_fromDelta("", "+%c3%xy");
    assertFalse("diff_fromDelta: Invalid character.", true);
  } catch (QString ex) {
    // Exception expected.
  }
  */

  // Test deltas with special characters.
  diffs = diffList(Diff(EQUAL, QString::fromWCharArray((const wchar_t*) L"\u0680 \000 \t %", 7)), Diff(DELETE, QString::fromWCharArray((const wchar_t*) L"\u0681 \001 \n ^", 7)), Diff(INSERT, QString::fromWCharArray((const wchar_t*) L"\u0682 \002 \\ |", 7)));
  text1 = dmp.diff_text1(diffs);
  assertEquals("diff_text1: Unicode text.", QString::fromWCharArray((const wchar_t*) L"\u0680 \000 \t %\u0681 \001 \n ^", 14), text1);

  delta = dmp.diff_toDelta(diffs);
  assertEquals("diff_toDelta: Unicode.", "=7\t-7\t+%DA%82 %02 %5C %7C", delta);

  assertEquals("diff_fromDelta: Unicode.", diffs, dmp.diff_fromDelta(text1, delta));

  // Verify pool of unchanged characters.
  diffs = diffList(Diff(INSERT, "A-Z a-z 0-9 - _ . ! ~ * ' ( ) ; / ? : @ & = + $ , # "));
  QString text2 = dmp.diff_text2(diffs);
  assertEquals("diff_text2: Unchanged characters.", "A-Z a-z 0-9 - _ . ! ~ * \' ( ) ; / ? : @ & = + $ , # ", text2);

  delta = dmp.diff_toDelta(diffs);
  assertEquals("diff_toDelta: Unchanged characters.", "+A-Z a-z 0-9 - _ . ! ~ * \' ( ) ; / ? : @ & = + $ , # ", delta);

  // Convert delta string into a diff.
  assertEquals("diff_fromDelta: Unchanged characters.", diffs, dmp.diff_fromDelta("", delta));
}

void diff_match_patch_test::testDiffXIndex() {
  // Translate a location in text1 to text2.
  QList<Diff> diffs = diffList(Diff(DELETE, "a"), Diff(INSERT, "1234"), Diff(EQUAL, "xyz"));
  assertEquals("diff_xIndex: Translation on equality.", 5, dmp.diff_xIndex(diffs, 2));

  diffs = diffList(Diff(EQUAL, "a"), Diff(DELETE, "1234"), Diff(EQUAL, "xyz"));
  assertEquals("diff_xIndex: Translation on deletion.", 1, dmp.diff_xIndex(diffs, 3));
}

void diff_match_patch_test::testDiffLevenshtein() {
  QList<Diff> diffs = diffList(Diff(DELETE, "abc"), Diff(INSERT, "1234"), Diff(EQUAL, "xyz"));
  assertEquals("diff_levenshtein: Trailing equality.", 4, dmp.diff_levenshtein(diffs));

  diffs = diffList(Diff(EQUAL, "xyz"), Diff(DELETE, "abc"), Diff(INSERT, "1234"));
  assertEquals("diff_levenshtein: Leading equality.", 4, dmp.diff_levenshtein(diffs));

  diffs = diffList(Diff(DELETE, "abc"), Diff(EQUAL, "xyz"), Diff(INSERT, "1234"));
  assertEquals("diff_levenshtein: Middle equality.", 7, dmp.diff_levenshtein(diffs));
}

void diff_match_patch_test::testDiffBisect() {
  // Normal.
  QString a = "cat";
  QString b = "map";
  // Since the resulting diff hasn't been normalized, it would be ok if
  // the insertion and deletion pairs are swapped.
  // If the order changes, tweak this test as required.
  QList<Diff> diffs = diffList(Diff(DELETE, "c"), Diff(INSERT, "m"), Diff(EQUAL, "a"), Diff(DELETE, "t"), Diff(INSERT, "p"));
  assertEquals("diff_bisect: Normal.", diffs, dmp.diff_bisect(a, b, std::numeric_limits<clock_t>::max()));

  // Timeout.
  diffs = diffList(Diff(DELETE, "cat"), Diff(INSERT, "map"));
  assertEquals("diff_bisect: Timeout.", diffs, dmp.diff_bisect(a, b, 0));
}

void diff_match_patch_test::testDiffMain() {
  // Perform a trivial diff.
  QList<Diff> diffs = diffList();
  assertEquals("diff_main: Null case.", diffs, dmp.diff_main("", "", false));

  diffs = diffList(Diff(EQUAL, "abc"));
  assertEquals("diff_main: Equality.", diffs, dmp.diff_main("abc", "abc", false));

  diffs = diffList(Diff(EQUAL, "ab"), Diff(INSERT, "123"), Diff(EQUAL, "c"));
  assertEquals("diff_main: Simple insertion.", diffs, dmp.diff_main("abc", "ab123c", false));

  diffs = diffList(Diff(EQUAL, "a"), Diff(DELETE, "123"), Diff(EQUAL, "bc"));
  assertEquals("diff_main: Simple deletion.", diffs, dmp.diff_main("a123bc", "abc", false));

  diffs = diffList(Diff(EQUAL, "a"), Diff(INSERT, "123"), Diff(EQUAL, "b"), Diff(INSERT, "456"), Diff(EQUAL, "c"));
  assertEquals("diff_main: Two insertions.", diffs, dmp.diff_main("abc", "a123b456c", false));

  diffs = diffList(Diff(EQUAL, "a"), Diff(DELETE, "123"), Diff(EQUAL, "b"), Diff(DELETE, "456"), Diff(EQUAL, "c"));
  assertEquals("diff_main: Two deletions.", diffs, dmp.diff_main("a123b456c", "abc", false));

  // Perform a real diff.
  // Switch off the timeout.
  dmp.Diff_Timeout = 0;
  diffs = diffList(Diff(DELETE, "a"), Diff(INSERT, "b"));
  assertEquals("diff_main: Simple case #1.", diffs, dmp.diff_main("a", "b", false));

  diffs = diffList(Diff(DELETE, "Apple"), Diff(INSERT, "Banana"), Diff(EQUAL, "s are a"), Diff(INSERT, "lso"), Diff(EQUAL, " fruit."));
  assertEquals("diff_main: Simple case #2.", diffs, dmp.diff_main("Apples are a fruit.", "Bananas are also fruit.", false));

  diffs = diffList(Diff(DELETE, "a"), Diff(INSERT, QString::fromWCharArray((const wchar_t*) L"\u0680", 1)), Diff(EQUAL, "x"), Diff(DELETE, "\t"), Diff(INSERT, QString::fromWCharArray((const wchar_t*) L"\000", 1)));
  assertEquals("diff_main: Simple case #3.", diffs, dmp.diff_main("ax\t", QString::fromWCharArray((const wchar_t*) L"\u0680x\000", 3), false));

  diffs = diffList(Diff(DELETE, "1"), Diff(EQUAL, "a"), Diff(DELETE, "y"), Diff(EQUAL, "b"), Diff(DELETE, "2"), Diff(INSERT, "xab"));
  assertEquals("diff_main: Overlap #1.", diffs, dmp.diff_main("1ayb2", "abxab", false));

  diffs = diffList(Diff(INSERT, "xaxcx"), Diff(EQUAL, "abc"), Diff(DELETE, "y"));
  assertEquals("diff_main: Overlap #2.", diffs, dmp.diff_main("abcy", "xaxcxabc", false));

  diffs = diffList(Diff(DELETE, "ABCD"), Diff(EQUAL, "a"), Diff(DELETE, "="), Diff(INSERT, "-"), Diff(EQUAL, "bcd"), Diff(DELETE, "="), Diff(INSERT, "-"), Diff(EQUAL, "efghijklmnopqrs"), Diff(DELETE, "EFGHIJKLMNOefg"));
  assertEquals("diff_main: Overlap #3.", diffs, dmp.diff_main("ABCDa=bcd=efghijklmnopqrsEFGHIJKLMNOefg", "a-bcd-efghijklmnopqrs", false));

  diffs = diffList(Diff(INSERT, " "), Diff(EQUAL, "a"), Diff(INSERT, "nd"), Diff(EQUAL, " [[Pennsylvania]]"), Diff(DELETE, " and [[New"));
  assertEquals("diff_main: Large equality.", diffs, dmp.diff_main("a [[Pennsylvania]] and [[New", " and [[Pennsylvania]]", false));

  dmp.Diff_Timeout = 0.1f;  // 100ms
  // This test may 'fail' on extremely fast computers.  If so, just increase the text lengths.
  QString a = "`Twas brillig, and the slithy toves\nDid gyre and gimble in the wabe:\nAll mimsy were the borogoves,\nAnd the mome raths outgrabe.\n";
  QString b = "I am the very model of a modern major general,\nI've information vegetable, animal, and mineral,\nI know the kings of England, and I quote the fights historical,\nFrom Marathon to Waterloo, in order categorical.\n";
  // Increase the text lengths by 1024 times to ensure a timeout.
  for (int x = 0; x < 10; x++) {
    a = a + a;
    b = b + b;
  }
  clock_t startTime = clock();
  dmp.diff_main(a, b);
  clock_t endTime = clock();
  // Test that we took at least the timeout period.
  assertTrue("diff_main: Timeout min.", dmp.Diff_Timeout * CLOCKS_PER_SEC <= endTime - startTime);
  // Test that we didn't take forever (be forgiving).
  // Theoretically this test could fail very occasionally if the
  // OS task swaps or locks up for a second at the wrong moment.
  // Java seems to overrun by ~80% (compared with 10% for other languages).
  // Therefore use an upper limit of 0.5s instead of 0.2s.
  assertTrue("diff_main: Timeout max.", dmp.Diff_Timeout * CLOCKS_PER_SEC * 2 > endTime - startTime);
  dmp.Diff_Timeout = 0;

  // Test the linemode speedup.
  // Must be long to pass the 100 char cutoff.
  a = "1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n";
  b = "abcdefghij\nabcdefghij\nabcdefghij\nabcdefghij\nabcdefghij\nabcdefghij\nabcdefghij\nabcdefghij\nabcdefghij\nabcdefghij\nabcdefghij\nabcdefghij\nabcdefghij\n";
  assertEquals("diff_main: Simple line-mode.", dmp.diff_main(a, b, true), dmp.diff_main(a, b, false));

  a = "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890";
  b = "abcdefghijabcdefghijabcdefghijabcdefghijabcdefghijabcdefghijabcdefghijabcdefghijabcdefghijabcdefghijabcdefghijabcdefghijabcdefghij";
  assertEquals("diff_main: Single line-mode.", dmp.diff_main(a, b, true), dmp.diff_main(a, b, false));

  a = "1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n";
  b = "abcdefghij\n1234567890\n1234567890\n1234567890\nabcdefghij\n1234567890\n1234567890\n1234567890\nabcdefghij\n1234567890\n1234567890\n1234567890\nabcdefghij\n";
  QStringList texts_linemode = diff_rebuildtexts(dmp.diff_main(a, b, true));
  QStringList texts_textmode = diff_rebuildtexts(dmp.diff_main(a, b, false));
  assertEquals("diff_main: Overlap line-mode.", texts_textmode, texts_linemode);

  // Test null inputs.
  try {
    dmp.diff_main(NULL, NULL);
    assertFalse("diff_main: Null inputs.", true);
  } catch (const char* ex) {
    // Exception expected.
  }
}


//  MATCH TEST FUNCTIONS


void diff_match_patch_test::testMatchAlphabet() {
  // Initialise the bitmasks for Bitap.
  QMap<QChar, int> bitmask;
  bitmask.insert('a', 4);
  bitmask.insert('b', 2);
  bitmask.insert('c', 1);
  assertEquals("match_alphabet: Unique.", bitmask, dmp.match_alphabet("abc"));

  bitmask = QMap<QChar, int>();
  bitmask.insert('a', 37);
  bitmask.insert('b', 18);
  bitmask.insert('c', 8);
  assertEquals("match_alphabet: Duplicates.", bitmask, dmp.match_alphabet("abcaba"));
}

void diff_match_patch_test::testMatchBitap() {
  // Bitap algorithm.
  dmp.Match_Distance = 100;
  dmp.Match_Threshold = 0.5f;
  assertEquals("match_bitap: Exact match #1.", 5, dmp.match_bitap("abcdefghijk", "fgh", 5));

  assertEquals("match_bitap: Exact match #2.", 5, dmp.match_bitap("abcdefghijk", "fgh", 0));

  assertEquals("match_bitap: Fuzzy match #1.", 4, dmp.match_bitap("abcdefghijk", "efxhi", 0));

  assertEquals("match_bitap: Fuzzy match #2.", 2, dmp.match_bitap("abcdefghijk", "cdefxyhijk", 5));

  assertEquals("match_bitap: Fuzzy match #3.", -1, dmp.match_bitap("abcdefghijk", "bxy", 1));

  assertEquals("match_bitap: Overflow.", 2, dmp.match_bitap("123456789xx0", "3456789x0", 2));

  assertEquals("match_bitap: Before start match.", 0, dmp.match_bitap("abcdef", "xxabc", 4));

  assertEquals("match_bitap: Beyond end match.", 3, dmp.match_bitap("abcdef", "defyy", 4));

  assertEquals("match_bitap: Oversized pattern.", 0, dmp.match_bitap("abcdef", "xabcdefy", 0));

  dmp.Match_Threshold = 0.4f;
  assertEquals("match_bitap: Threshold #1.", 4, dmp.match_bitap("abcdefghijk", "efxyhi", 1));

  dmp.Match_Threshold = 0.3f;
  assertEquals("match_bitap: Threshold #2.", -1, dmp.match_bitap("abcdefghijk", "efxyhi", 1));

  dmp.Match_Threshold = 0.0f;
  assertEquals("match_bitap: Threshold #3.", 1, dmp.match_bitap("abcdefghijk", "bcdef", 1));

  dmp.Match_Threshold = 0.5f;
  assertEquals("match_bitap: Multiple select #1.", 0, dmp.match_bitap("abcdexyzabcde", "abccde", 3));

  assertEquals("match_bitap: Multiple select #2.", 8, dmp.match_bitap("abcdexyzabcde", "abccde", 5));

  dmp.Match_Distance = 10;  // Strict location.
  assertEquals("match_bitap: Distance test #1.", -1, dmp.match_bitap("abcdefghijklmnopqrstuvwxyz", "abcdefg", 24));

  assertEquals("match_bitap: Distance test #2.", 0, dmp.match_bitap("abcdefghijklmnopqrstuvwxyz", "abcdxxefg", 1));

  dmp.Match_Distance = 1000;  // Loose location.
  assertEquals("match_bitap: Distance test #3.", 0, dmp.match_bitap("abcdefghijklmnopqrstuvwxyz", "abcdefg", 24));
}

void diff_match_patch_test::testMatchMain() {
  // Full match.
  assertEquals("match_main: Equality.", 0, dmp.match_main("abcdef", "abcdef", 1000));

  assertEquals("match_main: Null text.", -1, dmp.match_main("", "abcdef", 1));

  assertEquals("match_main: Null pattern.", 3, dmp.match_main("abcdef", "", 3));

  assertEquals("match_main: Exact match.", 3, dmp.match_main("abcdef", "de", 3));

  dmp.Match_Threshold = 0.7f;
  assertEquals("match_main: Complex match.", 4, dmp.match_main("I am the very model of a modern major general.", " that berry ", 5));
  dmp.Match_Threshold = 0.5f;

  // Test null inputs.
  try {
    dmp.match_main(NULL, NULL, 0);
    assertFalse("match_main: Null inputs.", true);
  } catch (const char* ex) {
    // Exception expected.
  }
}


//  PATCH TEST FUNCTIONS


void diff_match_patch_test::testPatchObj() {
  // Patch Object.
  Patch p;
  p.start1 = 20;
  p.start2 = 21;
  p.length1 = 18;
  p.length2 = 17;
  p.diffs = diffList(Diff(EQUAL, "jump"), Diff(DELETE, "s"), Diff(INSERT, "ed"), Diff(EQUAL, " over "), Diff(DELETE, "the"), Diff(INSERT, "a"), Diff(EQUAL, "\nlaz"));
  QString strp = "@@ -21,18 +22,17 @@\n jump\n-s\n+ed\n  over \n-the\n+a\n %0Alaz\n";
  assertEquals("Patch: toString.", strp, p.toString());
}

void diff_match_patch_test::testPatchFromText() {
  assertTrue("patch_fromText: #0.", dmp.patch_fromText("").isEmpty());

  QString strp = "@@ -21,18 +22,17 @@\n jump\n-s\n+ed\n  over \n-the\n+a\n %0Alaz\n";
  assertEquals("patch_fromText: #1.", strp, dmp.patch_fromText(strp).value(0).toString());

  assertEquals("patch_fromText: #2.", "@@ -1 +1 @@\n-a\n+b\n", dmp.patch_fromText("@@ -1 +1 @@\n-a\n+b\n").value(0).toString());

  assertEquals("patch_fromText: #3.", "@@ -1,3 +0,0 @@\n-abc\n", dmp.patch_fromText("@@ -1,3 +0,0 @@\n-abc\n").value(0).toString());

  assertEquals("patch_fromText: #4.", "@@ -0,0 +1,3 @@\n+abc\n", dmp.patch_fromText("@@ -0,0 +1,3 @@\n+abc\n").value(0).toString());

  // Generates error.
  try {
    dmp.patch_fromText("Bad\nPatch\n");
    assertFalse("patch_fromText: #5.", true);
  } catch (QString ex) {
    // Exception expected.
  }
}

void diff_match_patch_test::testPatchToText() {
  QString strp = "@@ -21,18 +22,17 @@\n jump\n-s\n+ed\n  over \n-the\n+a\n  laz\n";
  QList<Patch> patches;
  patches = dmp.patch_fromText(strp);
  assertEquals("patch_toText: Single", strp, dmp.patch_toText(patches));

  strp = "@@ -1,9 +1,9 @@\n-f\n+F\n oo+fooba\n@@ -7,9 +7,9 @@\n obar\n-,\n+.\n  tes\n";
  patches = dmp.patch_fromText(strp);
  assertEquals("patch_toText: Dual", strp, dmp.patch_toText(patches));
}

void diff_match_patch_test::testPatchAddContext() {
  dmp.Patch_Margin = 4;
  Patch p;
  p = dmp.patch_fromText("@@ -21,4 +21,10 @@\n-jump\n+somersault\n").value(0);
  dmp.patch_addContext(p, "The quick brown fox jumps over the lazy dog.");
  assertEquals("patch_addContext: Simple case.", "@@ -17,12 +17,18 @@\n fox \n-jump\n+somersault\n s ov\n", p.toString());

  p = dmp.patch_fromText("@@ -21,4 +21,10 @@\n-jump\n+somersault\n").value(0);
  dmp.patch_addContext(p, "The quick brown fox jumps.");
  assertEquals("patch_addContext: Not enough trailing context.", "@@ -17,10 +17,16 @@\n fox \n-jump\n+somersault\n s.\n", p.toString());

  p = dmp.patch_fromText("@@ -3 +3,2 @@\n-e\n+at\n").value(0);
  dmp.patch_addContext(p, "The quick brown fox jumps.");
  assertEquals("patch_addContext: Not enough leading context.", "@@ -1,7 +1,8 @@\n Th\n-e\n+at\n  qui\n", p.toString());

  p = dmp.patch_fromText("@@ -3 +3,2 @@\n-e\n+at\n").value(0);
  dmp.patch_addContext(p, "The quick brown fox jumps.  The quick brown fox crashes.");
  assertEquals("patch_addContext: Ambiguity.", "@@ -1,27 +1,28 @@\n Th\n-e\n+at\n  quick brown fox jumps. \n", p.toString());
}

void diff_match_patch_test::testPatchMake() {
  QList<Patch> patches;
  patches = dmp.patch_make("", "");
  assertEquals("patch_make: Null case", "", dmp.patch_toText(patches));

  QString text1 = "The quick brown fox jumps over the lazy dog.";
  QString text2 = "That quick brown fox jumped over a lazy dog.";
  QString expectedPatch = "@@ -1,8 +1,7 @@\n Th\n-at\n+e\n  qui\n@@ -21,17 +21,18 @@\n jump\n-ed\n+s\n  over \n-a\n+the\n  laz\n";
  // The second patch must be "-21,17 +21,18", not "-22,17 +21,18" due to rolling context.
  patches = dmp.patch_make(text2, text1);
  assertEquals("patch_make: Text2+Text1 inputs", expectedPatch, dmp.patch_toText(patches));

  expectedPatch = "@@ -1,11 +1,12 @@\n Th\n-e\n+at\n  quick b\n@@ -22,18 +22,17 @@\n jump\n-s\n+ed\n  over \n-the\n+a\n  laz\n";
  patches = dmp.patch_make(text1, text2);
  assertEquals("patch_make: Text1+Text2 inputs", expectedPatch, dmp.patch_toText(patches));

  QList<Diff> diffs = dmp.diff_main(text1, text2, false);
  patches = dmp.patch_make(diffs);
  assertEquals("patch_make: Diff input", expectedPatch, dmp.patch_toText(patches));

  patches = dmp.patch_make(text1, diffs);
  assertEquals("patch_make: Text1+Diff inputs", expectedPatch, dmp.patch_toText(patches));

  patches = dmp.patch_make(text1, text2, diffs);
  assertEquals("patch_make: Text1+Text2+Diff inputs (deprecated)", expectedPatch, dmp.patch_toText(patches));

  patches = dmp.patch_make("`1234567890-=[]\\;',./", "~!@#$%^&*()_+{}|:\"<>?");
  assertEquals("patch_toText: Character encoding.", "@@ -1,21 +1,21 @@\n-%601234567890-=%5B%5D%5C;',./\n+~!@#$%25%5E&*()_+%7B%7D%7C:%22%3C%3E?\n", dmp.patch_toText(patches));

  diffs = diffList(Diff(DELETE, "`1234567890-=[]\\;',./"), Diff(INSERT, "~!@#$%^&*()_+{}|:\"<>?"));
  assertEquals("patch_fromText: Character decoding.", diffs, dmp.patch_fromText("@@ -1,21 +1,21 @@\n-%601234567890-=%5B%5D%5C;',./\n+~!@#$%25%5E&*()_+%7B%7D%7C:%22%3C%3E?\n").value(0).diffs);

  text1 = "";
  for (int x = 0; x < 100; x++) {
    text1 += "abcdef";
  }
  text2 = text1 + "123";
  expectedPatch = "@@ -573,28 +573,31 @@\n cdefabcdefabcdefabcdefabcdef\n+123\n";
  patches = dmp.patch_make(text1, text2);
  assertEquals("patch_make: Long string with repeats.", expectedPatch, dmp.patch_toText(patches));

  // Test null inputs.
  try {
    dmp.patch_make(NULL, NULL);
    assertFalse("patch_make: Null inputs.", true);
  } catch (const char* ex) {
    // Exception expected.
  }
}

void diff_match_patch_test::testPatchSplitMax() {
  // Assumes that Match_MaxBits is 32.
  QList<Patch> patches;
  patches = dmp.patch_make("abcdefghijklmnopqrstuvwxyz01234567890", "XabXcdXefXghXijXklXmnXopXqrXstXuvXwxXyzX01X23X45X67X89X0");
  dmp.patch_splitMax(patches);
  assertEquals("patch_splitMax: #1.", "@@ -1,32 +1,46 @@\n+X\n ab\n+X\n cd\n+X\n ef\n+X\n gh\n+X\n ij\n+X\n kl\n+X\n mn\n+X\n op\n+X\n qr\n+X\n st\n+X\n uv\n+X\n wx\n+X\n yz\n+X\n 012345\n@@ -25,13 +39,18 @@\n zX01\n+X\n 23\n+X\n 45\n+X\n 67\n+X\n 89\n+X\n 0\n", dmp.patch_toText(patches));

  patches = dmp.patch_make("abcdef1234567890123456789012345678901234567890123456789012345678901234567890uvwxyz", "abcdefuvwxyz");
  QString oldToText = dmp.patch_toText(patches);
  dmp.patch_splitMax(patches);
  assertEquals("patch_splitMax: #2.", oldToText, dmp.patch_toText(patches));

  patches = dmp.patch_make("1234567890123456789012345678901234567890123456789012345678901234567890", "abc");
  dmp.patch_splitMax(patches);
  assertEquals("patch_splitMax: #3.", "@@ -1,32 +1,4 @@\n-1234567890123456789012345678\n 9012\n@@ -29,32 +1,4 @@\n-9012345678901234567890123456\n 7890\n@@ -57,14 +1,3 @@\n-78901234567890\n+abc\n", dmp.patch_toText(patches));

  patches = dmp.patch_make("abcdefghij , h : 0 , t : 1 abcdefghij , h : 0 , t : 1 abcdefghij , h : 0 , t : 1", "abcdefghij , h : 1 , t : 1 abcdefghij , h : 1 , t : 1 abcdefghij , h : 0 , t : 1");
  dmp.patch_splitMax(patches);
  assertEquals("patch_splitMax: #4.", "@@ -2,32 +2,32 @@\n bcdefghij , h : \n-0\n+1\n  , t : 1 abcdef\n@@ -29,32 +29,32 @@\n bcdefghij , h : \n-0\n+1\n  , t : 1 abcdef\n", dmp.patch_toText(patches));
}

void diff_match_patch_test::testPatchAddPadding() {
  QList<Patch> patches;
  patches = dmp.patch_make("", "test");
  assertEquals("patch_addPadding: Both edges full.", "@@ -0,0 +1,4 @@\n+test\n", dmp.patch_toText(patches));
  dmp.patch_addPadding(patches);
  assertEquals("patch_addPadding: Both edges full.", "@@ -1,8 +1,12 @@\n %01%02%03%04\n+test\n %01%02%03%04\n", dmp.patch_toText(patches));

  patches = dmp.patch_make("XY", "XtestY");
  assertEquals("patch_addPadding: Both edges partial.", "@@ -1,2 +1,6 @@\n X\n+test\n Y\n", dmp.patch_toText(patches));
  dmp.patch_addPadding(patches);
  assertEquals("patch_addPadding: Both edges partial.", "@@ -2,8 +2,12 @@\n %02%03%04X\n+test\n Y%01%02%03\n", dmp.patch_toText(patches));

  patches = dmp.patch_make("XXXXYYYY", "XXXXtestYYYY");
  assertEquals("patch_addPadding: Both edges none.", "@@ -1,8 +1,12 @@\n XXXX\n+test\n YYYY\n", dmp.patch_toText(patches));
  dmp.patch_addPadding(patches);
  assertEquals("patch_addPadding: Both edges none.", "@@ -5,8 +5,12 @@\n XXXX\n+test\n YYYY\n", dmp.patch_toText(patches));
}

void diff_match_patch_test::testPatchApply() {
  dmp.Match_Distance = 1000;
  dmp.Match_Threshold = 0.5f;
  dmp.Patch_DeleteThreshold = 0.5f;
  QList<Patch> patches;
  patches = dmp.patch_make("", "");
  QPair<QString, QVector<bool> > results = dmp.patch_apply(patches, "Hello world.");
  QVector<bool> boolArray = results.second;

  QString resultStr = QString("%1\t%2").arg(results.first).arg(boolArray.count());
  assertEquals("patch_apply: Null case.", "Hello world.\t0", resultStr);

  patches = dmp.patch_make("The quick brown fox jumps over the lazy dog.", "That quick brown fox jumped over a lazy dog.");
  results = dmp.patch_apply(patches, "The quick brown fox jumps over the lazy dog.");
  boolArray = results.second;
  resultStr = results.first + "\t" + (boolArray[0] ? "true" : "false") + "\t" + (boolArray[1] ? "true" : "false");
  assertEquals("patch_apply: Exact match.", "That quick brown fox jumped over a lazy dog.\ttrue\ttrue", resultStr);

  results = dmp.patch_apply(patches, "The quick red rabbit jumps over the tired tiger.");
  boolArray = results.second;
  resultStr = results.first + "\t" + (boolArray[0] ? "true" : "false") + "\t" + (boolArray[1] ? "true" : "false");
  assertEquals("patch_apply: Partial match.", "That quick red rabbit jumped over a tired tiger.\ttrue\ttrue", resultStr);

  results = dmp.patch_apply(patches, "I am the very model of a modern major general.");
  boolArray = results.second;
  resultStr = results.first + "\t" + (boolArray[0] ? "true" : "false") + "\t" + (boolArray[1] ? "true" : "false");
  assertEquals("patch_apply: Failed match.", "I am the very model of a modern major general.\tfalse\tfalse", resultStr);

  patches = dmp.patch_make("x1234567890123456789012345678901234567890123456789012345678901234567890y", "xabcy");
  results = dmp.patch_apply(patches, "x123456789012345678901234567890-----++++++++++-----123456789012345678901234567890y");
  boolArray = results.second;
  resultStr = results.first + "\t" + (boolArray[0] ? "true" : "false") + "\t" + (boolArray[1] ? "true" : "false");
  assertEquals("patch_apply: Big delete, small change.", "xabcy\ttrue\ttrue", resultStr);

  patches = dmp.patch_make("x1234567890123456789012345678901234567890123456789012345678901234567890y", "xabcy");
  results = dmp.patch_apply(patches, "x12345678901234567890---------------++++++++++---------------12345678901234567890y");
  boolArray = results.second;
  resultStr = results.first + "\t" + (boolArray[0] ? "true" : "false") + "\t" + (boolArray[1] ? "true" : "false");
  assertEquals("patch_apply: Big delete, large change 1.", "xabc12345678901234567890---------------++++++++++---------------12345678901234567890y\tfalse\ttrue", resultStr);

  dmp.Patch_DeleteThreshold = 0.6f;
  patches = dmp.patch_make("x1234567890123456789012345678901234567890123456789012345678901234567890y", "xabcy");
  results = dmp.patch_apply(patches, "x12345678901234567890---------------++++++++++---------------12345678901234567890y");
  boolArray = results.second;
  resultStr = results.first + "\t" + (boolArray[0] ? "true" : "false") + "\t" + (boolArray[1] ? "true" : "false");
  assertEquals("patch_apply: Big delete, large change 2.", "xabcy\ttrue\ttrue", resultStr);
  dmp.Patch_DeleteThreshold = 0.5f;

  dmp.Match_Threshold = 0.0f;
  dmp.Match_Distance = 0;
  patches = dmp.patch_make("abcdefghijklmnopqrstuvwxyz--------------------1234567890", "abcXXXXXXXXXXdefghijklmnopqrstuvwxyz--------------------1234567YYYYYYYYYY890");
  results = dmp.patch_apply(patches, "ABCDEFGHIJKLMNOPQRSTUVWXYZ--------------------1234567890");
  boolArray = results.second;
  resultStr = results.first + "\t" + (boolArray[0] ? "true" : "false") + "\t" + (boolArray[1] ? "true" : "false");
  assertEquals("patch_apply: Compensate for failed patch.", "ABCDEFGHIJKLMNOPQRSTUVWXYZ--------------------1234567YYYYYYYYYY890\tfalse\ttrue", resultStr);
  dmp.Match_Threshold = 0.5f;
  dmp.Match_Distance = 1000;

  patches = dmp.patch_make("", "test");
  QString patchStr = dmp.patch_toText(patches);
  dmp.patch_apply(patches, "");
  assertEquals("patch_apply: No side effects.", patchStr, dmp.patch_toText(patches));

  patches = dmp.patch_make("The quick brown fox jumps over the lazy dog.", "Woof");
  patchStr = dmp.patch_toText(patches);
  dmp.patch_apply(patches, "The quick brown fox jumps over the lazy dog.");
  assertEquals("patch_apply: No side effects with major delete.", patchStr, dmp.patch_toText(patches));

  patches = dmp.patch_make("", "test");
  results = dmp.patch_apply(patches, "");
  boolArray = results.second;
  resultStr = results.first + "\t" + (boolArray[0] ? "true" : "false");
  assertEquals("patch_apply: Edge exact match.", "test\ttrue", resultStr);

  patches = dmp.patch_make("XY", "XtestY");
  results = dmp.patch_apply(patches, "XY");
  boolArray = results.second;
  resultStr = results.first + "\t" + (boolArray[0] ? "true" : "false");
  assertEquals("patch_apply: Near edge exact match.", "XtestY\ttrue", resultStr);

  patches = dmp.patch_make("y", "y123");
  results = dmp.patch_apply(patches, "x");
  boolArray = results.second;
  resultStr = results.first + "\t" + (boolArray[0] ? "true" : "false");
  assertEquals("patch_apply: Edge partial match.", "x123\ttrue", resultStr);
}


void diff_match_patch_test::assertEquals(const QString &strCase, int n1, int n2) {
  if (n1 != n2) {
    qDebug("%s FAIL\nExpected: %d\nActual: %d", qPrintable(strCase), n1, n2);
    throw strCase;
  }
  qDebug("%s OK", qPrintable(strCase));
}

void diff_match_patch_test::assertEquals(const QString &strCase, const QString &s1, const QString &s2) {
  if (s1 != s2) {
    qDebug("%s FAIL\nExpected: %s\nActual: %s",
           qPrintable(strCase), qPrintable(s1), qPrintable(s2));
    throw strCase;
  }
  qDebug("%s OK", qPrintable(strCase));
}

void diff_match_patch_test::assertEquals(const QString &strCase, const Diff &d1, const Diff &d2) {
  if (d1 != d2) {
    qDebug("%s FAIL\nExpected: %s\nActual: %s", qPrintable(strCase),
        qPrintable(d1.toString()), qPrintable(d2.toString()));
    throw strCase;
  }
  qDebug("%s OK", qPrintable(strCase));
}

void diff_match_patch_test::assertEquals(const QString &strCase, const QList<Diff> &list1, const QList<Diff> &list2) {
  bool fail = false;
  if (list1.count() == list2.count()) {
    int i = 0;
    foreach(Diff d1, list1) {
      Diff d2 = list2.value(i);
      if (d1 != d2) {
        fail = true;
        break;
      }
      i++;
    }
  } else {
    fail = true;
  }

  if (fail) {
    // Build human readable description of both lists.
    QString listString1 = "(";
    bool first = true;
    foreach(Diff d1, list1) {
      if (!first) {
        listString1 += ", ";
      }
      listString1 += d1.toString();
      first = false;
    }
    listString1 += ")";
    QString listString2 = "(";
    first = true;
    foreach(Diff d2, list2) {
      if (!first) {
        listString2 += ", ";
      }
      listString2 += d2.toString();
      first = false;
    }
    listString2 += ")";
    qDebug("%s FAIL\nExpected: %s\nActual: %s",
        qPrintable(strCase), qPrintable(listString1), qPrintable(listString2));
    throw strCase;
  }
  qDebug("%s OK", qPrintable(strCase));
}

void diff_match_patch_test::assertEquals(const QString &strCase, const QList<QVariant> &list1, const QList<QVariant> &list2) {
  bool fail = false;
  if (list1.count() == list2.count()) {
    int i = 0;
    foreach(QVariant q1, list1) {
      QVariant q2 = list2.value(i);
      if (q1 != q2) {
        fail = true;
        break;
      }
      i++;
    }
  } else {
    fail = true;
  }

  if (fail) {
    // Build human readable description of both lists.
    QString listString1 = "(";
    bool first = true;
    foreach(QVariant q1, list1) {
      if (!first) {
        listString1 += ", ";
      }
      listString1 += q1.toString();
      first = false;
    }
    listString1 += ")";
    QString listString2 = "(";
    first = true;
    foreach(QVariant q2, list2) {
      if (!first) {
        listString2 += ", ";
      }
      listString2 += q2.toString();
      first = false;
    }
    listString2 += ")";
    qDebug("%s FAIL\nExpected: %s\nActual: %s",
        qPrintable(strCase), qPrintable(listString1), qPrintable(listString2));
    throw strCase;
  }
  qDebug("%s OK", qPrintable(strCase));
}

void diff_match_patch_test::assertEquals(const QString &strCase, const QVariant &var1, const QVariant &var2) {
  if (var1 != var2) {
    qDebug("%s FAIL\nExpected: %s\nActual: %s", qPrintable(strCase),
        qPrintable(var1.toString()), qPrintable(var2.toString()));
    throw strCase;
  }
  qDebug("%s OK", qPrintable(strCase));
}

void diff_match_patch_test::assertEquals(const QString &strCase, const QMap<QChar, int> &m1, const QMap<QChar, int> &m2) {
  QMapIterator<QChar, int> i1(m1), i2(m2);

  while (i1.hasNext() && i2.hasNext()) {
    i1.next();
    i2.next();
    if (i1.key() != i2.key() || i1.value() != i2.value()) {
      qDebug("%s FAIL\nExpected: (%c, %d)\nActual: (%c, %d)", qPrintable(strCase),
          i1.key().toAscii(), i1.value(), i2.key().toAscii(), i2.value());
      throw strCase;
    }
  }

  if (i1.hasNext()) {
    i1.next();
    qDebug("%s FAIL\nExpected: (%c, %d)\nActual: none",
        qPrintable(strCase), i1.key().toAscii(), i1.value());
    throw strCase;
  }
  if (i2.hasNext()) {
    i2.next();
    qDebug("%s FAIL\nExpected: none\nActual: (%c, %d)",
        qPrintable(strCase), i2.key().toAscii(), i2.value());
    throw strCase;
  }
  qDebug("%s OK", qPrintable(strCase));
}

void diff_match_patch_test::assertEquals(const QString &strCase, const QStringList &list1, const QStringList &list2) {
  if (list1 != list2) {
    qDebug("%s FAIL\nExpected: %s\nActual: %s", qPrintable(strCase),
        qPrintable(list1.join(",")), qPrintable(list2.join(",")));
    throw strCase;
  }
  qDebug("%s OK", qPrintable(strCase));
}

void diff_match_patch_test::assertTrue(const QString &strCase, bool value) {
  if (!value) {
    qDebug("%s FAIL\nExpected: true\nActual: false", qPrintable(strCase));
    throw strCase;
  }
  qDebug("%s OK", qPrintable(strCase));
}

void diff_match_patch_test::assertFalse(const QString &strCase, bool value) {
  if (value) {
    qDebug("%s FAIL\nExpected: false\nActual: true", qPrintable(strCase));
    throw strCase;
  }
  qDebug("%s OK", qPrintable(strCase));
}


// Construct the two texts which made up the diff originally.
QStringList diff_match_patch_test::diff_rebuildtexts(QList<Diff> diffs) {
  QStringList text;
  text << QString("") << QString("");
  foreach (Diff myDiff, diffs) {
    if (myDiff.operation != INSERT) {
      text[0] += myDiff.text;
    }
    if (myDiff.operation != DELETE) {
      text[1] += myDiff.text;
    }
  }
  return text;
}

void diff_match_patch_test::assertEmpty(const QString &strCase, const QStringList &list) {
  if (!list.isEmpty()) {
    throw strCase;
  }
}


// Private function for quickly building lists of diffs.
QList<Diff> diff_match_patch_test::diffList(Diff d1, Diff d2, Diff d3, Diff d4, Diff d5,
  Diff d6, Diff d7, Diff d8, Diff d9, Diff d10) {
  // Diff(INSERT, NULL) is invalid and thus is used as the default argument.
  QList<Diff> listRet;
  if (d1.operation == INSERT && d1.text == NULL) {
    return listRet;
  }
  listRet << d1;

  if (d2.operation == INSERT && d2.text == NULL) {
    return listRet;
  }
  listRet << d2;

  if (d3.operation == INSERT && d3.text == NULL) {
    return listRet;
  }
  listRet << d3;

  if (d4.operation == INSERT && d4.text == NULL) {
    return listRet;
  }
  listRet << d4;

  if (d5.operation == INSERT && d5.text == NULL) {
    return listRet;
  }
  listRet << d5;

  if (d6.operation == INSERT && d6.text == NULL) {
    return listRet;
  }
  listRet << d6;

  if (d7.operation == INSERT && d7.text == NULL) {
    return listRet;
  }
  listRet << d7;

  if (d8.operation == INSERT && d8.text == NULL) {
    return listRet;
  }
  listRet << d8;

  if (d9.operation == INSERT && d9.text == NULL) {
    return listRet;
  }
  listRet << d9;

  if (d10.operation == INSERT && d10.text == NULL) {
    return listRet;
  }
  listRet << d10;

  return listRet;
}


/*
Compile instructions for MinGW and QT4 on Windows:
qmake -project
qmake
mingw32-make
g++ -o diff_match_patch_test debug\diff_match_patch_test.o debug\diff_match_patch.o \qt4\lib\libQtCore4.a
diff_match_patch_test.exe

Compile insructions for OS X:
qmake -spec macx-g++
make
./diff_match_patch
*/
