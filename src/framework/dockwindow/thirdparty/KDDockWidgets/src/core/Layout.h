/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

/**
 * @file
 * @brief A widget that supports an arbitrary number of splitters (called Separators) in any
 * combination of vertical/horizontal.
 *
 * This is a widget wrapper around the multisplitter layout (Core::Item)
 *
 * @author Sérgio Martins \<sergio.martins@kdab.com\>
 */

#ifndef KDDOCKWIDGETS_LAYOUT_P_H
#define KDDOCKWIDGETS_LAYOUT_P_H

#pragma once

#include "kddockwidgets/core/View.h"
#include "kddockwidgets/docks_export.h"
#include "kddockwidgets/KDDockWidgets.h"
#include "kddockwidgets/LayoutSaver.h"
#include "kddockwidgets/QtCompat_p.h"

namespace KDDockWidgets {

namespace Core {
class LayoutingHost;
class Group;
class FloatingWindow;
class DockWidget;
class MainWindow;
class Item;
class ItemContainer;
class Separator;

/**
 * @brief The widget (QWidget or QQuickItem) which holds a layout of dock widgets.
 *
 * Usually this would simply be MultiSplitter, but we've introduced this base class to support
 * different layouts, like MDI layouts, which are very different than traditional dock widget
 * layouts.
 *
 * This class makes the bridge between the GUI world (QWidget) and Core::Item world.
 * It's suitable to be set as a main window central widget for instance. The actual layouting is
 * then done by the root Item.
 */
class DOCKS_EXPORT Layout : public Controller
{
    Q_OBJECT
public:
    explicit Layout(ViewType, View *);
    ~Layout();

    /// @brief Returns whether this layout is in a MainWindow
    /// @param honourNesting If true, then we'll count DropAreas/MDIAreas which are nested into
    /// DropAreas/MDIAreas as inside the main window. otherwise, only direct parenting is considered
    bool isInMainWindow(bool honourNesting = false) const;

    Core::MainWindow *mainWindow(bool honourNesting = false) const;

    Core::FloatingWindow *floatingWindow() const;

    /**
     * @brief returns the layout's minimum size
     * @ref setLayoutMinimumSize
     */
    Size layoutMinimumSize() const;

    /**
     * @brief returns the layout's maximum size hint
     */
    Size layoutMaximumSizeHint() const;

    /**
     * @brief returns the contents width.
     * Usually it's the same width as the respective parent MultiSplitter.
     */
    int layoutWidth() const
    {
        return layoutSize().width();
    }

    /**
     * @brief returns the contents height.
     * Usually it's the same height as the respective parent MultiSplitter.
     */
    int layoutHeight() const
    {
        return layoutSize().height();
    }

    /**
     * @brief Returns the size of the contents
     */
    Size layoutSize() const;

    /// @brief Runs some sanity checks. Returns true if everything is OK
    bool checkSanity() const;

    /// @brief clears the layout
    void clearLayout();

    /// @brief dumps the layout to stderr
    void dumpLayout() const;

    /**
     * @brief setter for the contents size
     * The "contents size" is just the size() of this layout. However, since resizing
     * QWidgets is async and we need it to be sync. As sometimes adding widgets will increase
     * the MultiSplitter size (due to widget's min-size constraints).
     */
    void setLayoutSize(Size);


    /// @brief restores the dockwidget @p dw to its previous position
    void restorePlaceholder(Core::DockWidget *dw, Core::Item *, int tabIndex);

    /**
     * @brief The list of items in this layout.
     */
    Vector<Core::Item *> items() const;

    /**
     * @brief Returns true if this layout contains the specified item.
     */
    bool containsItem(const Core::Item *) const;

    /**
     * @brief  Returns true if this layout contains the specified group.
     */
    bool containsGroup(const Core::Group *) const;

    /**
     * @brief Returns the number of Item objects in this layout.
     * This includes non-visible (placeholder) Items too.
     * @sa visibleCount
     */
    int count() const;

    /**
     * @brief Returns the number of visible Items in this layout.
     * Which is @ref count minus @ref placeholderCount
     * @sa count
     */
    int visibleCount() const;

    /**
     * @brief Returns the number of placeholder items in this layout.
     * This is the same as @ref count minus @ref visibleCount
     * @sa count, visibleCount
     */
    int placeholderCount() const;

    /**
     * @brief returns the Item that holds @p group in this layout
     */
    Core::Item *itemForGroup(const Core::Group *group) const;

    /**
     * @brief Returns this list of Group objects contained in this layout
     */
    Vector<Core::Group *> groups() const;

    /// @brief Returns the list of dock widgets contained in this layout
    Vector<Core::DockWidget *> dockWidgets() const;

    /**
     * @brief Removes an item from this MultiSplitter.
     */
    void removeItem(Core::Item *item);

    /**
     * @brief Updates the min size of this layout.
     */
    void updateSizeConstraints();

    virtual bool deserialize(const LayoutSaver::MultiSplitter &);
    LayoutSaver::MultiSplitter serialize() const;

    Core::DropArea *asDropArea() const;
    Core::MDILayout *asMDILayout() const;

    void viewAboutToBeDeleted();

    Core::ItemContainer *rootItem() const;

    void onCloseEvent(CloseEvent *);

    LayoutingHost *asLayoutingHost() const;
    static Layout *fromLayoutingHost(LayoutingHost *);

    class Private;
    Layout::Private *d_ptr();

protected:
    void setRootItem(Core::ItemContainer *root);
    /**
     * @brief setter for the minimum size
     * @ref minimumSize
     */
    void setLayoutMinimumSize(Size);

    /**
     * @brief Removes unneeded placeholder items when adding new groups.
     *
     * A floating group A might have a placeholder in the main window (for example to remember its
     * position on the Left), but then the user might attach it to the right, so the left
     * placeholder is no longer need. Right before adding the group to the right we remove the left
     * placeholder, otherwise it's unrefed while we're adding causing a segfault. So what this does
     * is making the unrefing happen a bit earlier.
     */
    void unrefOldPlaceholders(const Vector<Core::Group *> &groupsBeingAdded) const;

    /**
     * @brief returns the groups contained in @p groupOrMultiSplitter-
     * If groupOrMultiSplitter- is a Group, it returns a list of 1 element, with that group
     * If groupOrMultiSplitter- is a MultiSplitter then it returns a list of all groups it contains
     */
    Vector<Core::Group *> groupsFrom(View *groupOrMultiSplitter) const;

private:
    Private *const d;
    bool onResize(Size newSize);
};

}

}

#endif
