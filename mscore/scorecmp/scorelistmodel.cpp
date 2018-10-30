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

#include "scorelistmodel.h"

#include "scoretab.h"
#include "scoreview.h"

namespace Ms {

constexpr ScoreVersionIndex ScoreVersion::INDEX_CURRENT;
constexpr ScoreVersionIndex ScoreVersion::INDEX_LAST_SAVED;
constexpr ScoreVersionIndex ScoreVersion::INDEX_SESSION_START;

//---------------------------------------------------------
//   ScoreListModel::ScoreListModel
//---------------------------------------------------------

ScoreListModel::ScoreListModel(const QList<MasterScore*>* scoreList, ScoreTab* tab, QObject* parent)
   : QAbstractListModel(parent), _scoreList(scoreList), _tab(tab)
      {
      // Need to be synchronized with ScoreListModel::data()
      _usedRoles.push_back(Qt::DisplayRole);

      connect(tab, &ScoreTab::tabInserted, this, &ScoreListModel::tabInserted);
      connect(tab, &ScoreTab::tabRemoved, this, &ScoreListModel::tabRemoved);
      connect(tab, &ScoreTab::tabRenamed, this, &ScoreListModel::tabRenamed);
      connect(tab->getTab(), &QTabBar::tabMoved, this, &ScoreListModel::tabMoved);
      }

//---------------------------------------------------------
//   ScoreListModel::rowCount
//---------------------------------------------------------

int ScoreListModel::rowCount(const QModelIndex& parent) const
      {
      if (parent.isValid())
            return 0;
      return _scoreList->size();
      }

//---------------------------------------------------------
//   ScoreListModel::data
//---------------------------------------------------------

QVariant ScoreListModel::data(const QModelIndex& index, int role) const
      {
      const int row = index.row();
      if (row < 0 || row >= _scoreList->size())
            return QVariant();

      switch(role) {
            case Qt::DisplayRole:
                  return _tab->getTab()->tabText(row);
            default:
                  break;
            }
      return QVariant();
      }

//---------------------------------------------------------
//   ScoreListModel::getScore
//---------------------------------------------------------

Score* ScoreListModel::getScore(int idx) const
      {
      if (idx < 0 || idx >= _scoreList->size())
            return nullptr;
      return (*_scoreList)[idx];
      }

//---------------------------------------------------------
//   ScoreListModel::tabInserted
//---------------------------------------------------------

void ScoreListModel::tabInserted(int idx)
      {
      emit dataChanged(index(idx, 0), index(_scoreList->size() - 1, 0), _usedRoles);
      }

//---------------------------------------------------------
//   ScoreListModel::tabRemoved
//---------------------------------------------------------

void ScoreListModel::tabRemoved(int idx)
      {
      emit dataChanged(index(idx, 0), index(_scoreList->size(), 0), _usedRoles);
      }

//---------------------------------------------------------
//   ScoreListModel::tabRenamed
//---------------------------------------------------------

void ScoreListModel::tabRenamed(int idx)
      {
      QModelIndex tabIndex(index(idx, 0));
      emit dataChanged(tabIndex, tabIndex, _usedRoles);
      }

//---------------------------------------------------------
//   ScoreListModel::tabMoved
//---------------------------------------------------------

void ScoreListModel::tabMoved(int from, int to)
      {
      emit dataChanged(index(qMin(from, to), 0), index(qMax(from, to), 0), _usedRoles);
      }

//---------------------------------------------------------
//   ScoreVersionListModel::ScoreVersionListModel
//---------------------------------------------------------

ScoreVersionListModel::ScoreVersionListModel(MasterScore* score, QObject* parent)
   : QAbstractListModel(parent), _score(score)
      {
      update();
      }

//---------------------------------------------------------
//   ScoreVersionListModel::update
//---------------------------------------------------------

void ScoreVersionListModel::update()
      {
      beginResetModel();
      _versions.clear();
      if (_score) {
            _versions.emplace_back(_score, tr("Current version"), ScoreVersion::INDEX_CURRENT, /* recent */ true);
            if (_score->fileInfo() && _score->fileInfo()->isReadable())
                  _versions.emplace_back(_score, tr("Last saved version"), ScoreVersion::INDEX_LAST_SAVED, *_score->fileInfo(), false);
            if (_score->sessionStartBackupInfo().isReadable())
                  _versions.emplace_back(_score, tr("Session start"), ScoreVersion::INDEX_SESSION_START, _score->sessionStartBackupInfo(), false);
            }
      endResetModel();
      }

//---------------------------------------------------------
//   ScoreVersionListModel::setScore
//---------------------------------------------------------

void ScoreVersionListModel::setScore(MasterScore* s)
      {
      _score = s;
      update();
      }

//---------------------------------------------------------
//   ScoreVersionListModel::rowCount
//---------------------------------------------------------

int ScoreVersionListModel::rowCount(const QModelIndex& parent) const
      {
      if (parent.isValid())
            return 0;
      return _versions.size();
      }

//---------------------------------------------------------
//   ScoreVersionListModel::data
//---------------------------------------------------------

QVariant ScoreVersionListModel::data(const QModelIndex& index, int role) const
      {
      const int row = index.row();
      if (row < 0 || row >= int(_versions.size()))
            return QVariant();

      switch(role) {
            case Qt::DisplayRole:
                  return _versions[row].name;
            default:
                  break;
            }
      return QVariant();
      }

//---------------------------------------------------------
//   ScoreVersionListModel::getPosition
//    Returns a position of the version with the given
//    index in the versions list.
//    Returns -1 if there is no such a version.
//---------------------------------------------------------

int ScoreVersionListModel::getPosition(ScoreVersionIndex index) const
      {
      for (int i = 0; i < int(_versions.size()); ++i) {
            if (_versions[i].index == index)
                  return i;
            }
      return -1;
      }
}
