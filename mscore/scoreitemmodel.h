//=============================================================================
//  ScoreItemModel
//
//  Copyright (C) 2020 Peter Jonas
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __SCOREITEMMODEL_H__
#define __SCOREITEMMODEL_H__

#include "libmscore/score.h"
#include "libmscore/scoreElement.h"

namespace Ms {
//---------------------------------------------------------
//   ScoreItemModel
//---------------------------------------------------------

class ScoreItemModel : public QAbstractItemModel
{
    Q_OBJECT

private:
    Score* _scoreRoot;

public:
    ScoreItemModel(Score* score, QObject* parent = nullptr);

    Score* score() const { return _scoreRoot; }

    // compulsory overrides
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override { Q_UNUSED(parent) return 1; }
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex& child) const override;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    // optional overrides
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    ScoreElement* scoreElementFromIndex(const QModelIndex& index) const;
};
}  // namespace Ms

#endif  // __SCOREITEMMODEL_H__
