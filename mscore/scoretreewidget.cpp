//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "scoretreewidget.h"

#include "libmscore/score.h"
#include "scoreitemmodel.h"
#include "scoretreeview.h"

namespace Ms {
//---------------------------------------------------------
//   ScoreTreeWidget
//---------------------------------------------------------

ScoreTreeWidget::ScoreTreeWidget(QMainWindow* parent)
    : QDockWidget("Score Tree", parent)
{
    _treeView = new ScoreTreeView(this);
    setWidget(_treeView);
    setAllowedAreas(Qt::RightDockWidgetArea);
    parent->addDockWidget(Qt::RightDockWidgetArea, this);
}

//---------------------------------------------------------
//   ~ScoreTreeWidget
//---------------------------------------------------------

ScoreTreeWidget::~ScoreTreeWidget()
{
}

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void ScoreTreeWidget::setScore(Score* s)
{
    QAbstractItemModel* oldModel = _treeView->model();
    if (s) {
        _treeView->setModel(new ScoreItemModel(s, _treeView));
    }
    delete oldModel;
}
} // namespace Ms
