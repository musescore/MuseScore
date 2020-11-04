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

#ifndef __SCORELISTMODEL_H__
#define __SCORELISTMODEL_H__

namespace Ms {

class MasterScore;
class Score;
class ScoreTab;

//---------------------------------------------------------
//   ScoreListModel
//---------------------------------------------------------

class ScoreListModel : public QAbstractListModel {
      Q_OBJECT

      QVector<int> _usedRoles;
      const QList<MasterScore*>* _scoreList;
      ScoreTab* _tab; // currently used for tracking changes in score list

   public:
      ScoreListModel(const QList<MasterScore*>* scoreList, ScoreTab* tab, QObject* parent = nullptr);

      int rowCount(const QModelIndex& parent = QModelIndex()) const override;
      QVariant data(const QModelIndex& index, int role) const override;

      Score* getScore(int idx) const;

   public slots:
      void tabInserted(int idx);
      void tabRemoved(int idx);
      void tabRenamed(int idx);
      void tabMoved(int from, int to);
      };

//---------------------------------------------------------
//   ScoreVersion
//---------------------------------------------------------

typedef int ScoreVersionIndex;

struct ScoreVersion {
      Score* score;
      QString name;
      ScoreVersionIndex index;
      QFileInfo fileInfo;
      bool recent;

      constexpr static ScoreVersionIndex INDEX_CURRENT = -1;
      constexpr static ScoreVersionIndex INDEX_LAST_SAVED = -2;
      constexpr static ScoreVersionIndex INDEX_SESSION_START = -3;

      ScoreVersion(Score* score, const QString& name, ScoreVersionIndex index, const QFileInfo& fileInfo, bool recent)
         : score(score), name(name), index(index), fileInfo(fileInfo), recent(recent) {}
      ScoreVersion(Score* score, const QString& name, ScoreVersionIndex index, bool recent)
         : score(score), name(name), index(index), recent(recent) {}
      };

//---------------------------------------------------------
//   ScoreVersionListModel
//---------------------------------------------------------

class ScoreVersionListModel : public QAbstractListModel {
      Q_OBJECT

      MasterScore* _score;
      std::vector<ScoreVersion> _versions;

   public:
      ScoreVersionListModel(MasterScore* score, QObject* parent = nullptr);

      int rowCount(const QModelIndex& parent = QModelIndex()) const override;
      QVariant data(const QModelIndex& index, int role) const override;

      const MasterScore* score() const { return _score; }
      const ScoreVersion& getScoreVersion(int idx) const { return _versions[idx]; }
      int getPosition(ScoreVersionIndex index) const;

   public slots:
      void update();
      void setScore(MasterScore* s);
      };

}     // namespace Ms
#endif
