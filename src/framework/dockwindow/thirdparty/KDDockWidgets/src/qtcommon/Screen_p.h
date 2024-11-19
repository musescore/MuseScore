/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Waqar Ahmed <waqar.ahmed@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "../core/Screen_p.h"

#include <QPointer>

QT_BEGIN_NAMESPACE
class QScreen;
QT_END_NAMESPACE

namespace KDDockWidgets {

class Screen_qt final : public Core::Screen
{
public:
    Screen_qt(QScreen *);

    QString name() const override;

    QSize size() const override;
    QRect geometry() const override;

    qreal devicePixelRatio() const override;

    QSize availableSize() const override;
    QRect availableGeometry() const override;

    QSize virtualSize() const override;
    QRect virtualGeometry() const override;

    QScreen *qtScreen() const;

    bool equals(std::shared_ptr<Screen> other) const override;

public:
    QPointer<QScreen> m_screen;
    Q_DISABLE_COPY(Screen_qt)
};

}
