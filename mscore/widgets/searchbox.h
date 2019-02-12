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

#ifndef __SEARCHBOX_H__
#define __SEARCHBOX_H__

namespace Ms {

class FilterableView;

//---------------------------------------------------------
//   SearchBox
// Extends QLineEdit to provide methods to search an item view.
//---------------------------------------------------------

class SearchBox : public QLineEdit
      {
      Q_OBJECT

      FilterableView* _view = nullptr;

   protected:
      virtual void keyPressEvent(QKeyEvent* event) override;

   public:
      SearchBox(QWidget* parent = nullptr);
      void setFilterableView(FilterableView* view);
      void search(const QString& str);
      };

}

#endif // __SEARCHBOX_H__
