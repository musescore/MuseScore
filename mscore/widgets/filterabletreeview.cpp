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

#include "filterabletreeview.h"

namespace Ms {

//---------------------------------------------------------
//   FilterableTreeViewTemplate
//---------------------------------------------------------

template <typename T>
FilterableTreeViewTemplate<T>::FilterableTreeViewTemplate(QWidget* parent)
   : T(parent)
      {
      // expand branches when clicked unless the branch is selectable, in which case it will just be selected as usual
      FilterableTreeViewTemplate<T>::connect(this, &FilterableTreeViewTemplate<T>::clicked, this, &FilterableTreeViewTemplate<T>::toggleExpandedForUnselectable);
      // expand current branch when activated (usually when double-clicked, or when selected and Enter key is pressed)
      FilterableTreeViewTemplate<T>::connect(this, &FilterableTreeViewTemplate<T>::activated, this, &FilterableTreeViewTemplate<T>::toggleExpanded);
      }

// to avoid linker errors, we must explicity instantiate constructors for the types we need
template FilterableTreeViewTemplate<QTreeView>::FilterableTreeViewTemplate(QWidget* parent);
template FilterableTreeViewTemplate<QTreeWidget>::FilterableTreeViewTemplate(QWidget* parent);

//---------------------------------------------------------
//   keyPressEvent
//---------------------------------------------------------

template <typename T>
void FilterableTreeViewTemplate<T>::keyPressEvent(QKeyEvent* event)
      {
      switch(event->key()) {
            case Qt::Key_Space:
                  // expand current branch when spacebar is pressed
                  toggleExpanded(FilterableTreeViewTemplate<T>::currentIndex());
                  break;
            default:
                  T::keyPressEvent(event);
            }
      }

//---------------------------------------------------------
//   toggleExpanded
// Expand or collapse a branch in the tree view.
//---------------------------------------------------------

template <typename T>
void FilterableTreeViewTemplate<T>::toggleExpanded(const QModelIndex& node)
      {
      FilterableTreeViewTemplate<T>::setExpanded(node, !FilterableTreeViewTemplate<T>::isExpanded(node));
      }

//---------------------------------------------------------
//   toggleExpandedForUnselectable
// Expand or collapse a branch only if it cannot be selected.
//---------------------------------------------------------

template <typename T>
void FilterableTreeViewTemplate<T>::toggleExpandedForUnselectable(const QModelIndex& node)
      {
      if (!(node.flags() & Qt::ItemIsSelectable))
            toggleExpanded(node);
      }

//---------------------------------------------------------
//   selectFirst
// Select the first selectable node in the tree. Make sure you have disabled
// the Qt::ItemIsSelectable flag for any items that shouldn't be selectable.
// Often you will want leaf nodes to be selectable but not branch nodes.
//---------------------------------------------------------

template <typename T>
void FilterableTreeViewTemplate<T>::selectFirst()
      {
      const auto lambdaNodeIsSelectable = [](QModelIndex idx) -> bool {
            return idx.flags() & Qt::ItemIsSelectable;
            };
      QModelIndex node = recurse(lambdaNodeIsSelectable);
      FilterableTreeViewTemplate<T>::setCurrentIndex(node);
      }

//---------------------------------------------------------
//   selectNext
// Select the next selectable node in the tree after the current one.
//---------------------------------------------------------

template <typename T>
void FilterableTreeViewTemplate<T>::selectNext()
      {
      QModelIndex current = FilterableTreeViewTemplate<T>::currentIndex();
//      if (!current.isValid())
//            selectFirst();
      const auto lambdaNodeIsVisibleSelectable = [this](QModelIndex idx) -> bool {
            return idx.flags() & Qt::ItemIsSelectable && !FilterableTreeViewTemplate<T>::isIndexHidden(idx);
            };
      // try decendents of current index
      QModelIndex node = recurseUnder(lambdaNodeIsVisibleSelectable, current);
      if (node.isValid()) {
            // found a suitable decendant of current index
            FilterableTreeViewTemplate<T>::setCurrentIndex(node);
            return;
            }
      // otherwise keep looking
      for (; current.isValid(); current = current.parent()) {
            for (
                 QModelIndex youngerSibling = current.sibling(current.row() + 1, 0);
                 youngerSibling.isValid();
                 youngerSibling = youngerSibling.sibling(youngerSibling.row() + 1, 0)
                 ) {
                  // try sibling and its decendants
                  node = recurse(lambdaNodeIsVisibleSelectable, youngerSibling);
                  if (node.isValid()) {
                        // found a suitable decendant of sibling
                        FilterableTreeViewTemplate<T>::setCurrentIndex(node);
                        return;
                        }
                  }
            // otherwise go up a level and check parent's younger siblings
            }
      }

//---------------------------------------------------------
//   selectPrevious
// Select the previous selectable node in the tree after the current one.
//---------------------------------------------------------

template <typename T>
void FilterableTreeViewTemplate<T>::selectPrevious()
      {
      QModelIndex current = FilterableTreeViewTemplate<T>::currentIndex();
//      if (!current.isValid())
//            selectFirst();
      const auto lambdaNodeIsVisibleSelectable = [this](QModelIndex idx) -> bool {
            return idx.flags() & Qt::ItemIsSelectable && !FilterableTreeViewTemplate<T>::isIndexHidden(idx);
            };
      while (current.isValid()) {
            for (
                 QModelIndex olderSibling = current.sibling(current.row() - 1, 0);
                 olderSibling.isValid();
                 olderSibling = olderSibling.sibling(olderSibling.row() - 1, 0)
                 ) {
                  // try sibling and its decendants in reverse order
                  QModelIndex node = recurse(lambdaNodeIsVisibleSelectable, olderSibling, true);
                  if (node.isValid()) {
                        // found a suitable decendant of sibling
                        FilterableTreeViewTemplate<T>::setCurrentIndex(node);
                        return;
                        }
                  }
            // otherwise check parent (don't check decendants)
            current = current.parent();
            if (lambdaNodeIsVisibleSelectable(current)) {
                  // parent is suitable
                  FilterableTreeViewTemplate<T>::setCurrentIndex(current);
                  return;
                  }
            // otherwise loop to check parent's older siblings
            }
      }

//---------------------------------------------------------
//   toInitialState
//    Make all items visible but collapse all branches
//---------------------------------------------------------

template <typename T>
void FilterableTreeViewTemplate<T>::toInitialState(const QModelIndex& node)
      {
      const auto lambda = [this](QModelIndex idx) -> bool {
            FilterableTreeViewTemplate<T>::setRowHidden(idx.row(), idx.parent(), false);
            FilterableTreeViewTemplate<T>::setExpanded(idx, false);
            return false;
            };
      recurse(lambda, node);
      }

//---------------------------------------------------------
//   filter
// Show leaves and branches that match a search string. Hide the rest.
//---------------------------------------------------------

template <typename T>
bool FilterableTreeViewTemplate<T>::filter(const QString& searchString, const QModelIndex& node)
      {
      if (searchString.isEmpty()) {
            toInitialState();
            return true;
            }
      bool found = false;
      const auto lambdaShowNode = [this](QModelIndex idx) -> bool {
            FilterableTreeViewTemplate<T>::setRowHidden(idx.row(), idx.parent(), false);
            FilterableTreeViewTemplate<T>::setExpanded(idx, false);
            return false;
            };
      QAbstractItemModel* mod = FilterableTreeViewTemplate<T>::model();
      for (int row = 0; row < mod->rowCount(node); row++) {
            const QModelIndex child = mod->index(row, 0, node);
            bool match = matches(child, searchString);
            if (match)
                  recurse(lambdaShowNode, child); // branch matches so show all decendants
            else
                  match = filter(searchString, child); // check decendants
            FilterableTreeViewTemplate<T>::setExpanded(child, match);
            FilterableTreeViewTemplate<T>::setRowHidden(row, node, !match);
            if (match)
                  found = true;
            }
      return found;
      }

//---------------------------------------------------------
//   recurseUnder
// Apply a callable (e.g. a function or lambda) to all decendants of a node,
// but not the node itself (use `recurse` to include the original node). If
// the callable returns true for any decendant then recursing stops
// immediately and that decendant's index is returned.
//
// If you want to do something to all decendents simply provide a callable
// expression that always returns false.
//---------------------------------------------------------

template <typename T>
template <typename F>
QModelIndex FilterableTreeViewTemplate<T>::recurseUnder(const F& func, const QModelIndex& node, const bool backwards)
      {
      const QAbstractItemModel* mod = FilterableTreeViewTemplate<T>::model();
      const int start = backwards ? mod->rowCount(node) - 1 : 0;
      const int step = backwards ? -1 : 1;
      for (
           QModelIndex child = mod->index(start, 0, node);
           child.isValid();
           child = child.sibling(child.row() + step, 0)
           ) {
            if (func(child)) // test the child (call the callable on it)
                  return child; // callable returned true for child
            // otherwise test the decendents of child
            QModelIndex trueNode = recurseUnder(func, child, backwards);
            if (trueNode.isValid())
                  return trueNode; // callable returned true for a decendant
            }
      // callable returned false for all decendants
      return QModelIndex(); // return invalid index
      }
}
