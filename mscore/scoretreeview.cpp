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

#include "scoretreeview.h"

namespace Ms {
//---------------------------------------------------------
//   selectElementAndChildren
//---------------------------------------------------------

static void selectElementAndChildren(Element* el)
{
    el->score()->selection().add(el);
    for (ScoreElement* child : (*el)) {
        if (child) {
            selectElementAndChildren(toElement(child));
        }
    }
}

//---------------------------------------------------------
//   ScoreTreeView
//---------------------------------------------------------

ScoreTreeView::ScoreTreeView(QWidget* parent)
    : QTreeView(parent)
{
}

//---------------------------------------------------------
//   ~ScoreTreeView
//---------------------------------------------------------

ScoreTreeView::~ScoreTreeView()
{
}

//---------------------------------------------------------
//   setModel
//---------------------------------------------------------

void ScoreTreeView::setModel(QAbstractItemModel* model)
{
    Q_ASSERT(model);  // ensure there is a model
    Q_ASSERT(dynamic_cast<ScoreItemModel*>(model));  // ensure model is correct type
    QTreeView::setModel(model);
}

//---------------------------------------------------------
//   model
//---------------------------------------------------------

ScoreItemModel* ScoreTreeView::model() const
{
    return static_cast<ScoreItemModel*>(QTreeView::model());
}

//---------------------------------------------------------
//   currentChanged
//---------------------------------------------------------

void ScoreTreeView::currentChanged(const QModelIndex& current, const QModelIndex& previous)
{
    selectElementAtIndex(current);
    QTreeView::currentChanged(current, previous);
}

//---------------------------------------------------------
//   selectElementAtIndex
/// Select the element represented by index and all its children
/// in the score, so they are highlighted in the ScoreView.
//---------------------------------------------------------

void ScoreTreeView::selectElementAtIndex(const QModelIndex& index)
{
    ScoreElement* el = static_cast<ScoreElement*>(index.internalPointer());

    if (!el || !el->isElement()) {
        return;
    }

    Score* cs = model()->score();
    cs->selection().deselectAll();
    cs->selection().setState(SelState::LIST);

    // This function recursively adds all the children of the selected element
    // to the selection in the score. This is necessary because some non leaf node
    // elements cannot be selected by themselves (Segments, Measures) etc.
    selectElementAndChildren(toElement(el));

    cs->setSelectionChanged(true);
    cs->update();
}
}  // namespace Ms
