//=============================================================================
//  PaletteListView
//
//  Copyright (C) 2020 Peter Jonas
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "palettelistview.h"

#include <QKeyEvent>

#include "palettemodel.h"

namespace Ms {
//---------------------------------------------------------
//   PaletteListView::PaletteListView
//---------------------------------------------------------

PaletteListView::PaletteListView(PalettePanel* panel, QWidget* parent)
    : QListView(parent)
{
    setViewMode(QListView::IconMode);
    setMovement(QListView::Static);
    setResizeMode(QListView::Adjust);
    const QSize gridSize = panel->scaledGridSize();
    setIconSize(gridSize);
    setGridSize(gridSize);
    setSpacing(-5);   // zero spacing still has a large gap between icons

    PaletteTreePtr tree = std::make_shared<PaletteTree>();
    tree->append(panel);

    PaletteTreeModel* model = new PaletteTreeModel(tree);
    QModelIndex parentCategory = model->index(0, 0, QModelIndex());

    setModel(model);
    setRootIndex(parentCategory);
}

//---------------------------------------------------------
//   PaletteListView::currentCell
//---------------------------------------------------------

const PaletteCell* PaletteListView::currentCell() const
{
    return model()->data(currentIndex(), PaletteTreeModel::PaletteCellRole).value<const PaletteCell*>();
}

//---------------------------------------------------------
//   PaletteListView::currentElement
//---------------------------------------------------------

Element* PaletteListView::currentElement() const
{
    return currentCell()->element.get();
}

//---------------------------------------------------------
//   PaletteListView::focusNextMatchingCell
//---------------------------------------------------------

void PaletteListView::focusNextMatchingCell(const QString& str)
{
    const int nextRow = (currentRow() == count() - 1) ? 0 : currentRow() + 1;
    const QModelIndex nextIndex = model()->index(nextRow, 0, rootIndex());
    const auto matchedIndexList = model()->match(nextIndex, Qt::ToolTipRole, str);
    if (!matchedIndexList.isEmpty()) {
        setCurrentIndex(matchedIndexList.first());
    }
}

//---------------------------------------------------------
//   onlyContainsVisibleCharacters
/// Return true if string is non-empty and contains no whitespace
/// or control characters, otherwise false.
//---------------------------------------------------------

static bool onlyContainsVisibleCharacters(const QString& str)
{
    constexpr auto options = QRegularExpression::UseUnicodePropertiesOption;
    const QRegularExpression pattern("^[[:graph:]]+$", options);
    return pattern.match(str).hasMatch();
}

//---------------------------------------------------------
//   PaletteListView::keyPressEvent
//---------------------------------------------------------

void PaletteListView::keyPressEvent(QKeyEvent* event)
{
    const int key = event->key();
    switch (key) {
    case Qt::Key_Down:
    case Qt::Key_Right:
        incrementCurrentRow();
        break;
    case Qt::Key_Up:
    case Qt::Key_Left:
        decrementCurrentRow();
        break;
    default:
        if (onlyContainsVisibleCharacters(event->text())) {
            focusNextMatchingCell(event->text());
        } else {
            QListView::keyPressEvent(event);
        }
    }
}
} // namespace Ms
