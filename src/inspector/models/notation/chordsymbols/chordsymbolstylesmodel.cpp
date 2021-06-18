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
#include "chordsymbolstylesmodel.h"

using namespace mu::inspector;
using namespace mu::notation;

ChordSymbolStylesModel::ChordSymbolStylesModel(QObject* parent)
    : QAbstractListModel(parent)
{
    styleManager = new ChordSymbolStyleManager();
    m_styles = styleManager->getChordStyles();
}

int ChordSymbolStylesModel::rowCount(const QModelIndex&) const
{
    return m_styles.count();
}

QHash<int, QByteArray> ChordSymbolStylesModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        { StyleNameRole, "styleName" },
        { FileRole, "fileName" }
    };

    return roles;
}

QVariant ChordSymbolStylesModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount() || m_styles.isEmpty()) {
        return QVariant();
    }

    ChordSymbolStyle chordSymbolStyle = m_styles.at(index.row());

    switch (role) {
    case StyleNameRole:
        return chordSymbolStyle.styleName;
    case FileRole:
        return chordSymbolStyle.fileName;
    default:
        break;
    }

    return QVariant();
}

void ChordSymbolStylesModel::setChordStyle(QString styleName) const
{
    QString descriptionFileName = "chords_std.xml"; // Fall back

    for (auto& chordStyle: m_styles) {
        if (chordStyle.styleName == styleName) {
            descriptionFileName = chordStyle.fileName;
            break;
        }
    }

    globalContext()->currentNotation()->style()->setStyleValue(StyleId::chordDescriptionFile, descriptionFileName);
}
