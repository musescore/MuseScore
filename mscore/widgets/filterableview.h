//=============================================================================
//  FilterableView
//
//  Copyright (C) 2018 Peter Jonas
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __FILTERABLEVIEW_H__
#define __FILTERABLEVIEW_H__

namespace Ms {

//---------------------------------------------------------
//   FilterableView
// Interface / abstract class to declare some methods that we would like
// to be able to use with item views. Later we will extend Qt's item view
// classes (QTreeView, etc.) to implement these virtual methods.
//---------------------------------------------------------

class FilterableView
      {

   public:
      virtual void selectFirst() = 0;
      virtual void selectNext() = 0;
      virtual void selectPrevious() = 0;
      virtual void toInitialState() = 0;
      virtual void toInitialState(const QModelIndex& node) = 0;
      virtual bool filter(const QString& searchString) = 0;
      };

}

#endif // __FILTERABLEVIEW_H__
