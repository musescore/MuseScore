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

#ifndef __SCOREDIFFMODEL_H__
#define __SCOREDIFFMODEL_H__

namespace Ms {

struct BaseDiff;
class ScoreDiff;
struct TextDiff;

//---------------------------------------------------------
//   RawScoreDiffModel
//---------------------------------------------------------

class RawScoreDiffModel : public QAbstractListModel {
      Q_OBJECT

      ScoreDiff* _diff;
      std::vector<const TextDiff*> _textDiffs;
      bool _skipEqual;

   public:
      RawScoreDiffModel(ScoreDiff* d, bool skipEqual = true, QObject* parent = nullptr);

      int rowCount(const QModelIndex& parent = QModelIndex()) const override;
      QVariant data(const QModelIndex& index, int role) const override;

   public slots:
      void update();
      };

//---------------------------------------------------------
//   ScoreDiffModel
//---------------------------------------------------------

class ScoreDiffModel : public QAbstractListModel {
      Q_OBJECT

      ScoreDiff* _diff;

   public:
      ScoreDiffModel(ScoreDiff* d, QObject* parent = nullptr) : QAbstractListModel(parent), _diff(d) {}

      int rowCount(const QModelIndex& parent = QModelIndex()) const override;
      QVariant data(const QModelIndex& index, int role) const override;

      const BaseDiff* diffItem(const QModelIndex& index) const;

   public slots:
      void diffAboutToBeUpdated() { beginResetModel(); }
      void diffUpdated() { endResetModel(); }
      };

}     // namespace Ms
#endif
