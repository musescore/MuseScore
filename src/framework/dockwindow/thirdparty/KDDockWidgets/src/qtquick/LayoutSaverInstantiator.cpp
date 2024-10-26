/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "LayoutSaverInstantiator.h"
#include "core/LayoutSaver_p.h"

using namespace KDDockWidgets;

LayoutSaverInstantiator::LayoutSaverInstantiator(QObject *parent)
    : QObject(parent)
{
}

LayoutSaverInstantiator::~LayoutSaverInstantiator()
{
}

bool LayoutSaverInstantiator::saveToFile(const QString &jsonFilename)
{
    return LayoutSaver::saveToFile(jsonFilename);
}

bool LayoutSaverInstantiator::restoreFromFile(const QString &jsonFilename)
{
    return LayoutSaver::restoreFromFile(jsonFilename);
}

QVector<QString> LayoutSaverInstantiator::affinities() const
{
    return dptr()->m_affinityNames;
}

void LayoutSaverInstantiator::setAffinities(const QVector<QString> &affinities)
{
    setAffinityNames(affinities);
    Q_EMIT affinitiesChanged();
}
