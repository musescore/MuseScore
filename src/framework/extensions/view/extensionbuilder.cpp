/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "extensionbuilder.h"

#include <QQmlEngine>

#include "log.h"

using namespace mu::extensions;

ExtensionBuilder::ExtensionBuilder(QObject* parent)
    : QObject(parent)
{}

void ExtensionBuilder::load(const QString& uri, QObject* itemParent)
{
    Manifest m = provider()->manifest(Uri(uri.toStdString()));

    setTitle(m.title);

    QQmlEngine* e = new QQmlEngine(this);
    //QQmlEngine* e = qmlEngine(this);

    QQmlComponent component = QQmlComponent(e, m.qmlFilePath.toQString());
    if (!component.isReady()) {
        LOGE() << "Failed to load QML file: " << m.qmlFilePath << ", from extension: " << uri;
        LOGE() << component.errorString();
    }

    m_contentItem = component.createWithInitialProperties({ { "parent", QVariant::fromValue(itemParent) } });

    emit contentItemChanged();
}

QObject* ExtensionBuilder::contentItem() const
{
    return m_contentItem;
}

void ExtensionBuilder::setTitle(QString title)
{
    if (m_title == title) {
        return;
    }
    m_title = title;
    emit titleChanged();
}

QString ExtensionBuilder::title() const
{
    return m_title;
}
