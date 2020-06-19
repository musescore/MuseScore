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

#ifndef __STREEWIDGET_H__
#define __STREEWIDGET_H__

namespace Ms {
class Score;
class ScoreTreeView;

class ScoreTreeWidget : public QDockWidget
{
    Q_OBJECT

    ScoreTreeView * _treeView;

public:
    ScoreTreeWidget(QMainWindow* parent);
    ~ScoreTreeWidget();
    void setScore(Score* s);
};
} // namespace Ms

#endif // __STREEWIDGET_H__
