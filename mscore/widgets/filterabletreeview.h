//=============================================================================
//  FilterableTreeView
//
//  Copyright (C) 2018 Peter Jonas
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __FILTERABLETREEVIEW_H__
#define __FILTERABLETREEVIEW_H__

#include "filterableview.h"

namespace Ms {

//---------------------------------------------------------
//   FilterableTreeViewTemplate
// We use this class template to create a class called FilterableTreeView
// that extends QTreeView, and a class called FilterableTreeWidget that
// extends QTreeWidget. The template allows us to create both at once without
// encountering inheritance problems or resorting to preprocessor hacks.
//---------------------------------------------------------

template <typename T>
class FilterableTreeViewTemplate : public T, public FilterableView
      {

   private:
      void toggleExpanded(const QModelIndex& node);
      void toggleExpandedForUnselectable(const QModelIndex& node);
      template <typename F> inline QModelIndex recurse(const F& func, const bool backwards = false) {
            return recurseUnder(func, FilterableTreeViewTemplate<T>::rootIndex(), backwards);
            }
      template <typename F> inline QModelIndex recurse(const F& func, const QModelIndex& node, const bool backwards = false) {
            if (node.isValid() && func(node))
                  return node; // test the node unless it is the root (root is invalid but can have decendants)
            return recurseUnder(func, node, backwards); // test decendants of node
            }
      template <typename F> QModelIndex recurseUnder(const F& func, const QModelIndex& node, const bool backwards = false);

   protected:
      virtual void keyPressEvent(QKeyEvent* event) override;

   public:
      FilterableTreeViewTemplate(QWidget* parent = nullptr);
      virtual void selectFirst() override;
      virtual void selectNext() override;
      virtual void selectPrevious() override;
      inline virtual void toInitialState() override {
            toInitialState(FilterableTreeViewTemplate<T>::rootIndex());
            }
      virtual void toInitialState(const QModelIndex& node) override;
      inline virtual bool filter(const QString& searchString) override {
            return filter(searchString, FilterableTreeViewTemplate<T>::rootIndex());
            }
      virtual bool filter(const QString& searchString, const QModelIndex& node);
      };

// Define aliases to hide scary template syntax
using FilterableTreeView = FilterableTreeViewTemplate<QTreeView>;
using FilterableTreeWidget = FilterableTreeViewTemplate<QTreeWidget>;

// Now you can promote QTreeView to FilterableTreeView and QTreeWidget to FilterableTreeWidget

}

#endif // __FILTERABLETREEVIEW_H__
