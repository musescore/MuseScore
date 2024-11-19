/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "docks_export.h"
#include "core/Window_p.h"
#include "core/WidgetResizeHandler_p.h"

#include <QObject>
#include <QAbstractNativeEventFilter>

namespace KDDockWidgets {

class DOCKS_EXPORT CustomFrameHelper : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    typedef WidgetResizeHandler::NativeFeatures (*ShouldUseCustomFrame)(Core::Window::Ptr);
    explicit CustomFrameHelper(ShouldUseCustomFrame shouldUseCustomFrameFunc,
                               QObject *parent = nullptr);
    ~CustomFrameHelper() override;

public Q_SLOTS:
    void applyCustomFrame(Core::Window::Ptr);

protected:
    bool nativeEventFilter(const QByteArray &eventType, void *message,
                           Qt5Qt6Compat::qintptr *result) override;

private:
    bool m_inDtor = false;
    ShouldUseCustomFrame m_shouldUseCustomFrameFunc = nullptr;
    bool m_recursionGuard = false;
};

}
