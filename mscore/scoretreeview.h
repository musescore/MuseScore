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

#ifndef __STREEVIEW_H__
#define __STREEVIEW_H__

#include "scoreitemmodel.h"

namespace Ms {
class ScoreTreeView : public QTreeView
{
    Q_OBJECT

public:
    ScoreTreeView(QWidget* parent);
    ~ScoreTreeView();

    void setModel(QAbstractItemModel* model);
    ScoreItemModel* model() const;

    void currentChanged(const QModelIndex& current, const QModelIndex& prev) override;

private:
    void selectElementAtIndex(const QModelIndex& index);
};
}  // namespace Ms

#endif  // __STREEVIEW_H__
