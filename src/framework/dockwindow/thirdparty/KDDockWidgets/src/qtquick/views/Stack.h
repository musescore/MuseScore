/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

/**
 * @file
 * @brief The QQuickItem counter part of TabWidgetQuick. Handles GUI while TabWidget handles state.
 *
 * @author Sérgio Martins \<sergio.martins@kdab.com\>
 */

#ifndef KDTABWIDGETQUICK_P_H
#define KDTABWIDGETQUICK_P_H
#pragma once

#include "View.h"
#include "kddockwidgets/core/views/StackViewInterface.h"

#include <QVector>

namespace KDDockWidgets {

namespace Core {
class Stack;
}

namespace QtQuick {

class DOCKS_EXPORT Stack : public QtQuick::View, public Core::StackViewInterface
{
    Q_OBJECT
public:
    explicit Stack(Core::Stack *controller, QQuickItem *parent = nullptr);

private:
    Q_DISABLE_COPY(Stack)
};

}
}

#endif
