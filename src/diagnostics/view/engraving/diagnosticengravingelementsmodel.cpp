/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#include "diagnosticengravingelementsmodel.h"

#include <QTextStream>

#include "engraving/libmscore/scoreElement.h"

using namespace mu::diagnostics;

DiagnosticEngravingElementsModel::DiagnosticEngravingElementsModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

QVariant DiagnosticEngravingElementsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount()) {
        return QVariant();
    }

    const Item& item = m_items.at(index.row());
    switch (role) {
    case rItemData: return item.toQVariant();
    }
    return QVariant();
}

int DiagnosticEngravingElementsModel::rowCount(const QModelIndex&) const
{
    return m_items.count();
}

QHash<int, QByteArray> DiagnosticEngravingElementsModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        { rItemData, "itemData" },
    };

    return roles;
}

void DiagnosticEngravingElementsModel::init()
{
}

void DiagnosticEngravingElementsModel::reload()
{
    beginResetModel();

    m_items.clear();

    auto els = engravingRegister()->elements();
    for (Ms::ScoreElement* el : els) {
        Item it;
        it.el = el;
        m_items.append(it);
    }

    endResetModel();

    updateInfo();
}

void DiagnosticEngravingElementsModel::updateInfo()
{
    QHash<QString, int> els;
    for (const Item& it : m_items) {
        els[it.el->name()] += 1;
    }

    m_info.clear();
    QTextStream stream(&m_info);
    for (auto it = els.constBegin(); it != els.constEnd(); ++it) {
        stream << it.key() << ": " << it.value() << "\n";
    }
    stream << "----------------------\n";
    stream << "Total: " << m_items.count();
    stream.flush();

    emit infoChanged();
}

QString DiagnosticEngravingElementsModel::info() const
{
    return m_info;
}

// Item ===============

QVariant DiagnosticEngravingElementsModel::Item::toQVariant() const
{
    QVariantMap data;
    data["name"] = el->name();
    return data;
}
