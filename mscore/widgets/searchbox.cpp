//=============================================================================
//  SearchBox
//
//  Copyright (C) 2018 Peter Jonas
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "searchbox.h"
#include "filterableview.h"

namespace Ms {

//---------------------------------------------------------
//   SearchBox
//---------------------------------------------------------

SearchBox::SearchBox(QWidget* parent)
   : QLineEdit(parent)
      {
      connect(this, &SearchBox::textChanged, this, &SearchBox::search);
      }

//---------------------------------------------------------
//   search
// Search the connected view.
//---------------------------------------------------------

void SearchBox::search(const QString& str)
      {
      Q_ASSERT(_view);
      _view->filter(str);
      }

//---------------------------------------------------------
//   setFilterableView
// Hook the searchbox up to the view you want to search.
//---------------------------------------------------------

void SearchBox::setFilterableView(FilterableView* view)
      {
      Q_ASSERT(view);
      _view = view;
      }

//---------------------------------------------------------
//   keyPressEvent
//---------------------------------------------------------

void SearchBox::keyPressEvent(QKeyEvent* event)
      {
      Q_ASSERT(_view);
      switch(event->key()) {
            case Qt::Key_Up:
                  _view->selectPrevious();
                  break;
            case Qt::Key_Down:
                  _view->selectNext();
                  break;
            default:
                  QLineEdit::keyPressEvent(event);
            }
      }

}
