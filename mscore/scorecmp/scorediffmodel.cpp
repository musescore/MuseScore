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

#include "scorediffmodel.h"

#include "libmscore/scorediff.h"

namespace Ms {

//---------------------------------------------------------
//   RawScoreDiffModel::RawScoreDiffModel
//---------------------------------------------------------

RawScoreDiffModel::RawScoreDiffModel(ScoreDiff* d, bool skipEqual, QObject* parent)
   : QAbstractListModel(parent), _diff(d), _skipEqual(skipEqual)
      {
      update();
      }

//---------------------------------------------------------
//   RawScoreDiffModel::update
//---------------------------------------------------------

void RawScoreDiffModel::update() {
      beginResetModel();
      _textDiffs.clear();
      for (const TextDiff& diff : _diff->textDiffs()) {
            if (_skipEqual && diff.type == DiffType::EQUAL)
                  continue;
            if (diff.type == DiffType::REPLACE) {
                  // Push this item twice. First will be shown as DELETE,
                  // second as INSERT.
                  _textDiffs.push_back(&diff);
                  }
            _textDiffs.push_back(&diff);
            }
      endResetModel();
      }

//---------------------------------------------------------
//   RawScoreDiffModel::rowCount
//---------------------------------------------------------

int RawScoreDiffModel::rowCount(const QModelIndex& parent) const
      {
      if (parent.isValid())
            return 0;
      return int(_textDiffs.size());
      }

//---------------------------------------------------------
//   RawScoreDiffModel::data
//---------------------------------------------------------

QVariant RawScoreDiffModel::data(const QModelIndex& index, int role) const
      {
      const int row = index.row();
      if (row < 0 || row >= int(_textDiffs.size()))
            return QVariant();

      const TextDiff& diff = *_textDiffs[row];
      switch(role) {
            case Qt::DisplayRole:
                  {
                  DiffType type = diff.type;
                  if (diff.type == DiffType::REPLACE)
                        type = (row == 0 || _textDiffs[row - 1] != &diff) ? DiffType::DELETE : DiffType::INSERT;
                  return diff.toString(type);
                  }
            case Qt::BackgroundRole:
                  switch(diff.type) {
                        case DiffType::DELETE:
                              return QBrush(Qt::red);
                        case DiffType::INSERT:
                              return QBrush(Qt::green);
                        default:
                              return QVariant();
                        }
            }
      return QVariant();
      }

//---------------------------------------------------------
//   ScoreDiffModel::rowCount
//---------------------------------------------------------

int ScoreDiffModel::rowCount(const QModelIndex& parent) const
      {
      if (parent.isValid())
            return 0;
      return int(_diff->diffs().size());
      }

//---------------------------------------------------------
//   ScoreDiffModel::data
//---------------------------------------------------------

QVariant ScoreDiffModel::data(const QModelIndex& index, int role) const
      {
      const int row = index.row();
      if (row < 0 || row >= int(_diff->diffs().size()))
            return QVariant();

      const BaseDiff& diff = *_diff->diffs()[row];
      switch(role) {
            case Qt::DisplayRole:
                  return diff.toString();
            }
      return QVariant();
      }

//---------------------------------------------------------
//   ScoreDiffModel::diffItem
//---------------------------------------------------------

const BaseDiff* ScoreDiffModel::diffItem(const QModelIndex& index) const
      {
      const int row = index.row();
      if (row < 0 || row >= int(_diff->diffs().size()))
            return nullptr;
      return _diff->diffs()[row];
      }
}
