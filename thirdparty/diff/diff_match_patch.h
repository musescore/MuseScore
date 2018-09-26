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
 * Diff Match and Patch
 * http://code.google.com/p/google-diff-match-patch/
 */

#ifndef DIFF_MATCH_PATCH_H
#define DIFF_MATCH_PATCH_H

#include <ctime>

/*
 * Functions for diff, match and patch.
 * Computes the difference between two texts to create a patch.
 * Applies the patch onto another text, allowing for errors.
 *
 * @author fraser@google.com (Neil Fraser)
 *
 * Qt/C++ port by mikeslemmer@gmail.com (Mike Slemmer):
 *
 * Code known to compile and run with Qt 4.3 through Qt 4.7.
 *
 * Here is a trivial sample program which works properly when linked with this
 * library:
 *

 #include <QtCore>
 #include <QString>
 #include <QList>
 #include <QMap>
 #include <QVariant>
 #include "diff_match_patch.h"
 int main(int argc, char **argv) {
   diff_match_patch dmp;
   QString str1 = QString("First string in diff");
   QString str2 = QString("Second string in diff");

   QString strPatch = dmp.patch_toText(dmp.patch_make(str1, str2));
   QPair<QString, QVector<bool> > out
       = dmp.patch_apply(dmp.patch_fromText(strPatch), str1);
   QString strResult = out.first;

   // here, strResult will equal str2 above.
   return 0;
 }

 */


/**-
* The data structure representing a diff is a Linked list of Diff objects:
* {Diff(Operation.DELETE, "Hello"), Diff(Operation.INSERT, "Goodbye"),
*  Diff(Operation.EQUAL, " world.")}
* which means: delete "Hello", add "Goodbye" and keep " world."
*/
#undef DELETE
#undef INSERT
#undef EQUAL
enum Operation {
  DELETE, INSERT, EQUAL
};


/**
* Class representing one diff operation.
*/
class Diff {
 public:
  Operation operation;
  // One of: INSERT, DELETE or EQUAL.
  QString text;
  // The text associated with this diff operation.

  /**
   * Constructor.  Initializes the diff with the provided values.
   * @param operation One of INSERT, DELETE or EQUAL.
   * @param text The text being applied.
   */
  Diff(Operation _operation, const QString &_text);
  Diff();
  inline bool isNull() const;
  QString toString() const;
  bool operator==(const Diff &d) const;
  bool operator!=(const Diff &d) const;

  static QString strOperation(Operation op);
};


/**
* Class representing one patch operation.
*/
class Patch {
 public:
  QList<Diff> diffs;
  int start1;
  int start2;
  int length1;
  int length2;

  /**
   * Constructor.  Initializes with an empty list of diffs.
   */
  Patch();
  bool isNull() const;
  QString toString();
};


/**
 * Class containing the diff, match and patch methods.
 * Also contains the behaviour settings.
 */
class diff_match_patch {

  friend class diff_match_patch_test;

 public:
  // Defaults.
  // Set these on your diff_match_patch instance to override the defaults.

  // Number of seconds to map a diff before giving up (0 for infinity).
  float Diff_Timeout;
  // Cost of an empty edit operation in terms of edit characters.
  short Diff_EditCost;
  // At what point is no match declared (0.0 = perfection, 1.0 = very loose).
  float Match_Threshold;
  // How far to search for a match (0 = exact location, 1000+ = broad match).
  // A match this many characters away from the expected location will add
  // 1.0 to the score (0.0 is a perfect match).
  int Match_Distance;
  // When deleting a large block of text (over ~64 characters), how close does
  // the contents have to match the expected contents. (0.0 = perfection,
  // 1.0 = very loose).  Note that Match_Threshold controls how closely the
  // end points of a delete need to match.
  float Patch_DeleteThreshold;
  // Chunk size for context length.
  short Patch_Margin;

  // The number of bits in an int.
  short Match_MaxBits;


 public:

  diff_match_patch();

  //  DIFF FUNCTIONS


  /**
   * Find the differences between two texts.
   * Run a faster slightly less optimal diff.
   * This method allows the 'checklines' of diff_main() to be optional.
   * Most of the time checklines is wanted, so default to true.
   * @param text1 Old string to be diffed.
   * @param text2 New string to be diffed.
   * @return Linked List of Diff objects.
   */
  QList<Diff> diff_main(const QString &text1, const QString &text2);

  /**
   * Find the differences between two texts.
   * @param text1 Old string to be diffed.
   * @param text2 New string to be diffed.
   * @param checklines Speedup flag.  If false, then don't run a
   *     line-level diff first to identify the changed areas.
   *     If true, then run a faster slightly less optimal diff.
   * @return Linked List of Diff objects.
   */
  QList<Diff> diff_main(const QString &text1, const QString &text2, bool checklines);

  /**
   * Find the differences between two texts.  Simplifies the problem by
   * stripping any common prefix or suffix off the texts before diffing.
   * @param text1 Old string to be diffed.
   * @param text2 New string to be diffed.
   * @param checklines Speedup flag.  If false, then don't run a
   *     line-level diff first to identify the changed areas.
   *     If true, then run a faster slightly less optimal diff.
   * @param deadline Time when the diff should be complete by.  Used
   *     internally for recursive calls.  Users should set DiffTimeout instead.
   * @return Linked List of Diff objects.
   */
 private:
  QList<Diff> diff_main(const QString &text1, const QString &text2, bool checklines, clock_t deadline);

  /**
   * Find the differences between two texts.  Assumes that the texts do not
   * have any common prefix or suffix.
   * @param text1 Old string to be diffed.
   * @param text2 New string to be diffed.
   * @param checklines Speedup flag.  If false, then don't run a
   *     line-level diff first to identify the changed areas.
   *     If true, then run a faster slightly less optimal diff.
   * @param deadline Time when the diff should be complete by.
   * @return Linked List of Diff objects.
   */
 private:
  QList<Diff> diff_compute(QString text1, QString text2, bool checklines, clock_t deadline);

  /**
   * Do a quick line-level diff on both strings, then rediff the parts for
   * greater accuracy.
   * This speedup can produce non-minimal diffs.
   * @param text1 Old string to be diffed.
   * @param text2 New string to be diffed.
   * @param deadline Time when the diff should be complete by.
   * @return Linked List of Diff objects.
   */
 private:
  QList<Diff> diff_lineMode(QString text1, QString text2, clock_t deadline);

  /**
   * Find the 'middle snake' of a diff, split the problem in two
   * and return the recursively constructed diff.
   * See Myers 1986 paper: An O(ND) Difference Algorithm and Its Variations.
   * @param text1 Old string to be diffed.
   * @param text2 New string to be diffed.
   * @return Linked List of Diff objects.
   */
 protected:
  QList<Diff> diff_bisect(const QString &text1, const QString &text2, clock_t deadline);

  /**
   * Given the location of the 'middle snake', split the diff in two parts
   * and recurse.
   * @param text1 Old string to be diffed.
   * @param text2 New string to be diffed.
   * @param x Index of split point in text1.
   * @param y Index of split point in text2.
   * @param deadline Time at which to bail if not yet complete.
   * @return LinkedList of Diff objects.
   */
 private:
  QList<Diff> diff_bisectSplit(const QString &text1, const QString &text2, int x, int y, clock_t deadline);

  /**
   * Split two texts into a list of strings.  Reduce the texts to a string of
   * hashes where each Unicode character represents one line.
   * @param text1 First string.
   * @param text2 Second string.
   * @return Three element Object array, containing the encoded text1, the
   *     encoded text2 and the List of unique strings.  The zeroth element
   *     of the List of unique strings is intentionally blank.
   */
 protected:
  QList<QVariant> diff_linesToChars(const QString &text1, const QString &text2); // return elems 0 and 1 are QString, elem 2 is QStringList

  /**
   * Split a text into a list of strings.  Reduce the texts to a string of
   * hashes where each Unicode character represents one line.
   * @param text String to encode.
   * @param lineArray List of unique strings.
   * @param lineHash Map of strings to indices.
   * @return Encoded string.
   */
 private:
  QString diff_linesToCharsMunge(const QString &text, QStringList &lineArray,
                                 QMap<QString, int> &lineHash);

  /**
   * Rehydrate the text in a diff from a string of line hashes to real lines of
   * text.
   * @param diffs LinkedList of Diff objects.
   * @param lineArray List of unique strings.
   */
 protected: // changed for usage in scorediff
  void diff_charsToLines(QList<Diff> &diffs, const QStringList &lineArray);

  /**
   * Determine the common prefix of two strings.
   * @param text1 First string.
   * @param text2 Second string.
   * @return The number of characters common to the start of each string.
   */
 public:
  int diff_commonPrefix(const QString &text1, const QString &text2);

  /**
   * Determine the common suffix of two strings.
   * @param text1 First string.
   * @param text2 Second string.
   * @return The number of characters common to the end of each string.
   */
 public:
  int diff_commonSuffix(const QString &text1, const QString &text2);

  /**
   * Determine if the suffix of one string is the prefix of another.
   * @param text1 First string.
   * @param text2 Second string.
   * @return The number of characters common to the end of the first
   *     string and the start of the second string.
   */
 protected:
  int diff_commonOverlap(const QString &text1, const QString &text2);

  /**
   * Do the two texts share a substring which is at least half the length of
   * the longer text?
   * This speedup can produce non-minimal diffs.
   * @param text1 First string.
   * @param text2 Second string.
   * @return Five element String array, containing the prefix of text1, the
   *     suffix of text1, the prefix of text2, the suffix of text2 and the
   *     common middle.  Or null if there was no match.
   */
 protected:
  QStringList diff_halfMatch(const QString &text1, const QString &text2);

  /**
   * Does a substring of shorttext exist within longtext such that the
   * substring is at least half the length of longtext?
   * @param longtext Longer string.
   * @param shorttext Shorter string.
   * @param i Start index of quarter length substring within longtext.
   * @return Five element String array, containing the prefix of longtext, the
   *     suffix of longtext, the prefix of shorttext, the suffix of shorttext
   *     and the common middle.  Or null if there was no match.
   */
 private:
  QStringList diff_halfMatchI(const QString &longtext, const QString &shorttext, int i);

  /**
   * Reduce the number of edits by eliminating semantically trivial equalities.
   * @param diffs LinkedList of Diff objects.
   */
 public:
  void diff_cleanupSemantic(QList<Diff> &diffs);

  /**
   * Look for single edits surrounded on both sides by equalities
   * which can be shifted sideways to align the edit to a word boundary.
   * e.g: The c<ins>at c</ins>ame. -> The <ins>cat </ins>came.
   * @param diffs LinkedList of Diff objects.
   */
 public:
  void diff_cleanupSemanticLossless(QList<Diff> &diffs);

  /**
   * Given two strings, compute a score representing whether the internal
   * boundary falls on logical boundaries.
   * Scores range from 5 (best) to 0 (worst).
   * @param one First string.
   * @param two Second string.
   * @return The score.
   */
 private:
  int diff_cleanupSemanticScore(const QString &one, const QString &two);

  /**
   * Reduce the number of edits by eliminating operationally trivial equalities.
   * @param diffs LinkedList of Diff objects.
   */
 public:
  void diff_cleanupEfficiency(QList<Diff> &diffs);

  /**
   * Reorder and merge like edit sections.  Merge equalities.
   * Any edit section can move as long as it doesn't cross an equality.
   * @param diffs LinkedList of Diff objects.
   */
 public:
  void diff_cleanupMerge(QList<Diff> &diffs);

  /**
   * loc is a location in text1, compute and return the equivalent location in
   * text2.
   * e.g. "The cat" vs "The big cat", 1->1, 5->8
   * @param diffs LinkedList of Diff objects.
   * @param loc Location within text1.
   * @return Location within text2.
   */
 public:
  int diff_xIndex(const QList<Diff> &diffs, int loc);

  /**
   * Convert a Diff list into a pretty HTML report.
   * @param diffs LinkedList of Diff objects.
   * @return HTML representation.
   */
 public:
  QString diff_prettyHtml(const QList<Diff> &diffs);

  /**
   * Compute and return the source text (all equalities and deletions).
   * @param diffs LinkedList of Diff objects.
   * @return Source text.
   */
 public:
  QString diff_text1(const QList<Diff> &diffs);

  /**
   * Compute and return the destination text (all equalities and insertions).
   * @param diffs LinkedList of Diff objects.
   * @return Destination text.
   */
 public:
  QString diff_text2(const QList<Diff> &diffs);

  /**
   * Compute the Levenshtein distance; the number of inserted, deleted or
   * substituted characters.
   * @param diffs LinkedList of Diff objects.
   * @return Number of changes.
   */
 public:
  int diff_levenshtein(const QList<Diff> &diffs);

  /**
   * Crush the diff into an encoded string which describes the operations
   * required to transform text1 into text2.
   * E.g. =3\t-2\t+ing  -> Keep 3 chars, delete 2 chars, insert 'ing'.
   * Operations are tab-separated.  Inserted text is escaped using %xx notation.
   * @param diffs Array of diff tuples.
   * @return Delta text.
   */
 public:
  QString diff_toDelta(const QList<Diff> &diffs);

  /**
   * Given the original text1, and an encoded string which describes the
   * operations required to transform text1 into text2, compute the full diff.
   * @param text1 Source string for the diff.
   * @param delta Delta text.
   * @return Array of diff tuples or null if invalid.
   * @throws QString If invalid input.
   */
 public:
  QList<Diff> diff_fromDelta(const QString &text1, const QString &delta);


  //  MATCH FUNCTIONS


  /**
   * Locate the best instance of 'pattern' in 'text' near 'loc'.
   * Returns -1 if no match found.
   * @param text The text to search.
   * @param pattern The pattern to search for.
   * @param loc The location to search around.
   * @return Best match index or -1.
   */
 public:
  int match_main(const QString &text, const QString &pattern, int loc);

  /**
   * Locate the best instance of 'pattern' in 'text' near 'loc' using the
   * Bitap algorithm.  Returns -1 if no match found.
   * @param text The text to search.
   * @param pattern The pattern to search for.
   * @param loc The location to search around.
   * @return Best match index or -1.
   */
 protected:
  int match_bitap(const QString &text, const QString &pattern, int loc);

  /**
   * Compute and return the score for a match with e errors and x location.
   * @param e Number of errors in match.
   * @param x Location of match.
   * @param loc Expected location of match.
   * @param pattern Pattern being sought.
   * @return Overall score for match (0.0 = good, 1.0 = bad).
   */
 private:
  double match_bitapScore(int e, int x, int loc, const QString &pattern);

  /**
   * Initialise the alphabet for the Bitap algorithm.
   * @param pattern The text to encode.
   * @return Hash of character locations.
   */
 protected:
  QMap<QChar, int> match_alphabet(const QString &pattern);


 //  PATCH FUNCTIONS


  /**
   * Increase the context until it is unique,
   * but don't let the pattern expand beyond Match_MaxBits.
   * @param patch The patch to grow.
   * @param text Source text.
   */
 protected:
  void patch_addContext(Patch &patch, const QString &text);

  /**
   * Compute a list of patches to turn text1 into text2.
   * A set of diffs will be computed.
   * @param text1 Old text.
   * @param text2 New text.
   * @return LinkedList of Patch objects.
   */
 public:
  QList<Patch> patch_make(const QString &text1, const QString &text2);

  /**
   * Compute a list of patches to turn text1 into text2.
   * text1 will be derived from the provided diffs.
   * @param diffs Array of diff tuples for text1 to text2.
   * @return LinkedList of Patch objects.
   */
 public:
  QList<Patch> patch_make(const QList<Diff> &diffs);

  /**
   * Compute a list of patches to turn text1 into text2.
   * text2 is ignored, diffs are the delta between text1 and text2.
   * @param text1 Old text.
   * @param text2 Ignored.
   * @param diffs Array of diff tuples for text1 to text2.
   * @return LinkedList of Patch objects.
   * @deprecated Prefer patch_make(const QString &text1, const QList<Diff> &diffs).
   */
 public:
  QList<Patch> patch_make(const QString &text1, const QString &text2, const QList<Diff> &diffs);

  /**
   * Compute a list of patches to turn text1 into text2.
   * text2 is not provided, diffs are the delta between text1 and text2.
   * @param text1 Old text.
   * @param diffs Array of diff tuples for text1 to text2.
   * @return LinkedList of Patch objects.
   */
 public:
  QList<Patch> patch_make(const QString &text1, const QList<Diff> &diffs);

  /**
   * Given an array of patches, return another array that is identical.
   * @param patches Array of patch objects.
   * @return Array of patch objects.
   */
 public:
  QList<Patch> patch_deepCopy(QList<Patch> &patches);

  /**
   * Merge a set of patches onto the text.  Return a patched text, as well
   * as an array of true/false values indicating which patches were applied.
   * @param patches Array of patch objects.
   * @param text Old text.
   * @return Two element Object array, containing the new text and an array of
   *      boolean values.
   */
 public:
  QPair<QString,QVector<bool> > patch_apply(QList<Patch> &patches, const QString &text);

  /**
   * Add some padding on text start and end so that edges can match something.
   * Intended to be called only from within patch_apply.
   * @param patches Array of patch objects.
   * @return The padding string added to each side.
   */
 public:
  QString patch_addPadding(QList<Patch> &patches);

  /**
   * Look through the patches and break up any which are longer than the
   * maximum limit of the match algorithm.
   * Intended to be called only from within patch_apply.
   * @param patches LinkedList of Patch objects.
   */
 public:
  void patch_splitMax(QList<Patch> &patches);

  /**
   * Take a list of patches and return a textual representation.
   * @param patches List of Patch objects.
   * @return Text representation of patches.
   */
 public:
  QString patch_toText(const QList<Patch> &patches);

  /**
   * Parse a textual representation of patches and return a List of Patch
   * objects.
   * @param textline Text representation of patches.
   * @return List of Patch objects.
   * @throws QString If invalid input.
   */
 public:
  QList<Patch> patch_fromText(const QString &textline);

  /**
   * A safer version of QString.mid(pos).  This one returns "" instead of
   * null when the postion equals the string length.
   * @param str String to take a substring from.
   * @param pos Position to start the substring from.
   * @return Substring.
   */
 private:
  static inline QString safeMid(const QString &str, int pos) {
    return (pos == str.length()) ? QString("") : str.mid(pos);
  }

  /**
   * A safer version of QString.mid(pos, len).  This one returns "" instead of
   * null when the postion equals the string length.
   * @param str String to take a substring from.
   * @param pos Position to start the substring from.
   * @param len Length of substring.
   * @return Substring.
   */
 private:
  static inline QString safeMid(const QString &str, int pos, int len) {
    return (pos == str.length()) ? QString("") : str.mid(pos, len);
  }
};

#endif // DIFF_MATCH_PATCH_H
