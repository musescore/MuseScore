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
#include "devextensionslistmodel.h"

using namespace muse::extensions;

DevExtensionsListModel::DevExtensionsListModel(QObject* parent)
    : QObject(parent), Injectable(muse::iocCtxForQmlObject(this))
{}

QVariantList DevExtensionsListModel::extensionsList()
{
    QVariantList list;
    ManifestList manifests = provider()->manifestList();
    for (const Manifest& m : manifests) {
        QVariantMap obj;
        obj["uri"] = QString::fromStdString(m.uri.toString());
        obj["type"] = QString::fromStdString(typeToString(m.type));
        obj["title"] = m.title.toQString();

        list << obj;
    }

    return list;
}

void DevExtensionsListModel::clicked(const QString& uri_)
{
    UriQuery q(uri_.toStdString());
    provider()->perform(q);
}
