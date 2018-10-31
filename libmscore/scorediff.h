//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2018 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __SCOREDIFF_H__
#define __SCOREDIFF_H__

#include "score.h"

#include <vector>

namespace Ms {

enum class Pid;
class ScoreElement;

//---------------------------------------------------------
//   ItemType
//---------------------------------------------------------

enum class ItemType {
      ELEMENT,
      PROPERTY,
      MARKUP,
      CONTEXTCHANGE
      };

//---------------------------------------------------------
//   DiffType
//---------------------------------------------------------
#undef DELETE
enum class DiffType {
      EQUAL,
      INSERT,
      DELETE,
      REPLACE
      };

//---------------------------------------------------------
//   TextDiff
//    A structure similar to Diff from diff_match_patch
//    but contains info on line numbers of the diff
//---------------------------------------------------------

struct TextDiff {
      DiffType type;
      QString text[2];
      int start[2]; // starting line numbers in both texts
      int end[2];   // ending line numbers in both texts

      void merge(const TextDiff& other); // merge other diff into this one
      QString toString(DiffType type, bool prefixLines = false) const;
      QString toString(bool prefixLines = false) const { return toString(type, prefixLines); }
      };

//---------------------------------------------------------
//   BaseDiff
//---------------------------------------------------------

struct BaseDiff {
      DiffType type;
      const TextDiff* textDiff;
      const ScoreElement* ctx[2];
      const ScoreElement* before[2];

      virtual ~BaseDiff() = default;

      virtual ItemType itemType() const = 0;
      virtual bool sameItem(const BaseDiff&) const;
      virtual Fraction afrac(int score) const;
      virtual QString toString() const = 0;
      };

//---------------------------------------------------------
//   ContextChange
//    Dummy Diff for temporary storing information on
//    context changes.
//---------------------------------------------------------

struct ContextChange : public BaseDiff {
      ItemType itemType() const override { return ItemType::CONTEXTCHANGE; }
      QString toString() const override;
      };

//---------------------------------------------------------
//   ElementDiff
//---------------------------------------------------------

struct ElementDiff : public BaseDiff {
      const ScoreElement* el[2];

      ItemType itemType() const override { return ItemType::ELEMENT; }
      bool sameItem(const BaseDiff&) const override;
      Fraction afrac(int score) const override;
      QString toString() const override;
      };

//---------------------------------------------------------
//   PropertyDiff
//---------------------------------------------------------

struct PropertyDiff : public BaseDiff {
      Pid pid;

      ItemType itemType() const override { return ItemType::PROPERTY; }
      bool sameItem(const BaseDiff&) const override;
      QString toString() const override;
      };

//---------------------------------------------------------
//   MarkupDiff
//---------------------------------------------------------

struct MarkupDiff : public BaseDiff {
      QString name;
      QVariant info;

      ItemType itemType() const override { return ItemType::MARKUP; }
      bool sameItem(const BaseDiff&) const override;
      QString toString() const override;
      };

//---------------------------------------------------------
//   ScoreDiff
//---------------------------------------------------------

class ScoreDiff {
      std::vector<TextDiff> _textDiffs; // raw diff between MSCX code for scores
      std::vector<const TextDiff*> _mergedTextDiffs; // extra diff items created after merging score diff items
      std::vector<BaseDiff*> _diffs;
      Score* _s1;
      Score* _s2;
      ScoreContentState _scoreState1;
      ScoreContentState _scoreState2;

      bool _textDiffOnly;

      void processMarkupDiffs();
      void mergeInsertDeleteDiffs();
      void mergeElementDiffs();
      void editPropertyDiffs();

   public:
      ScoreDiff(Score* s1, Score* s2, bool textDiffOnly = false);
      ScoreDiff(const ScoreDiff&) = delete;
      ~ScoreDiff();

      void update();
      bool updated() const { return _scoreState1 == _s1->state() && _scoreState2 == _s2->state(); }

      std::vector<BaseDiff*>& diffs() { return _diffs; }
      const std::vector<TextDiff>& textDiffs() const { return _textDiffs; }

      const Score* score1() const { return _s1; }
      const Score* score2() const { return _s2; }

      QString rawDiff(bool skipEqual = true) const;
      QString userDiff() const;
      };

}     // namespace Ms
#endif
