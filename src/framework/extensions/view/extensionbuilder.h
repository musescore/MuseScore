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
#ifndef MUSE_EXTENSIONS_EXTENSIONBUILDER_H
#define MUSE_EXTENSIONS_EXTENSIONBUILDER_H

#include <QObject>
#include <QQmlComponent>
#include <QQuickItem>

#include "modularity/ioc.h"
#include "../iextensionsprovider.h"
#include "../iextensionsuiengine.h"

#include "global/async/asyncable.h"

namespace muse::extensions {
class ExtensionBuilder : public QObject, public Injectable, public async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(QString title READ title NOTIFY titleChanged FINAL)
    Q_PROPERTY(QQuickItem * contentItem READ contentItem NOTIFY contentItemChanged FINAL)

    Inject<IExtensionsProvider> provider = { this };
    Inject<IExtensionsUiEngine> engine = { this };

public:
    ExtensionBuilder(QObject* parent = nullptr);

    QString title() const;
    QQuickItem* contentItem() const;

    Q_INVOKABLE void load(const QString& uriQuery, QObject* itemParent);

signals:
    void titleChanged();
    void contentItemChanged();

    void closeRequested();

private:

    void setTitle(QString title);

    QString m_title;
    QQuickItem* m_contentItem = nullptr;
};
}

#endif // MUSE_EXTENSIONS_EXTENSIONBUILDER_H
