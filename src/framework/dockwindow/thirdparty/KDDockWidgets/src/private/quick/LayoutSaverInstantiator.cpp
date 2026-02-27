/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019-2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

/**
 * @file
 * @brief The GUI counterpart of Frame.
 *
 * @author Sérgio Martins \<sergio.martins@kdab.com\>
 */

#include "LayoutSaverInstantiator_p.h"

#include <QQmlContext>
#include <QQmlEngine>

using namespace KDDockWidgets;

LayoutSaverInstantiator::LayoutSaverInstantiator(QObject *parent)
    : QObject(parent)
{
}

LayoutSaverInstantiator::~LayoutSaverInstantiator()
{
    delete m_saver;
}

void LayoutSaverInstantiator::componentComplete() 
{
    QQmlContext* qmlCtx = qmlContext(this);
    assert(qmlCtx);
    const int ctx = qmlCtx->contextProperty(QStringLiteral("_kddw_context")).value<int>();

    m_saver = new LayoutSaver(ctx, RestoreOption_None);
}

bool LayoutSaverInstantiator::saveToFile(const QString &jsonFilename)
{
    return m_saver->saveToFile(jsonFilename);
}

bool LayoutSaverInstantiator::restoreFromFile(const QString &jsonFilename)
{
    return m_saver->restoreFromFile(jsonFilename);
}
