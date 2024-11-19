/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

/**
 * @file
 * @brief FocusScope
 *
 * @author Sérgio Martins \<sergio.martins@kdab.com\>
 */

#ifndef KD_DOCKWIDGETS_FOCUSSCOPE_H
#define KD_DOCKWIDGETS_FOCUSSCOPE_H

#include "kddockwidgets/docks_export.h"
#include "kddockwidgets/KDDockWidgets.h"

namespace KDDockWidgets::Core {

class View;

///@brief Allows to implement a similar functionality to QtQuick's FocusScope item, in QtWidgets
class DOCKS_EXPORT FocusScope
{
    KDDW_DELETE_COPY_CTOR(FocusScope)
public:
    ///@brief constructor
    explicit FocusScope(View *thisView);
    virtual ~FocusScope();

    ///@brief Returns true if this FocusScope is focused.
    /// This is similar to the QWidget::hasFocus(), except that it counts with the children being
    /// focused too. i.e: If any child is focused then this FocusScope has focus too.
    bool isFocused() const;

    ///@brief Sets focus on this scope.
    ///
    /// This will call QWidget::focus() on the last QWidget that was focused in this scope.
    void focus(Qt::FocusReason = Qt::OtherFocusReason);

protected:
    ///@brief reimplement in the 1st QObject derived class
    virtual void isFocusedChangedCallback() = 0;
    virtual void focusedWidgetChangedCallback() = 0;

private:
    class Private;
    Private *const d;
};
}

#endif
