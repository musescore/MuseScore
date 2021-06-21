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

#include "chordsymboleditormodel.h"

ChordSymbolEditorModel::ChordSymbolEditorModel(QObject* parent)
    : QAbstractListModel(parent)
{
    styleManager = new ChordSymbolStyleManager();
    m_styles = styleManager->getChordStyles();
    // Testing
    m_chordSpellingList = {"Standard","Solfege"};
}

int ChordSymbolEditorModel::rowCount(const QModelIndex&) const
{
    return m_styles.count();
}

QHash<int, QByteArray> ChordSymbolEditorModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        { StyleNameRole, "styleName" },
        { FileRole, "fileName" }
    };

    return roles;
}

QVariant ChordSymbolEditorModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount() || m_styles.isEmpty()) {
        return QVariant();
    }

    Ms::ChordSymbolStyle chordSymbolStyle = m_styles.at(index.row());

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

QList<QString> ChordSymbolEditorModel::chordSpellingList() const{
    return m_chordSpellingList;
}
QList<QString> ChordSymbolEditorModel::majorSeventhList() const{
    return m_majorSeventhList;
}
QList<QString> ChordSymbolEditorModel::halfDiminishedList() const{
    return m_halfDiminishedList;
}
QList<QString> ChordSymbolEditorModel::minorList() const{
    return m_minorList;
}
QList<QString> ChordSymbolEditorModel::augmentedList() const{
    return m_augmentedList;
}
QList<QString> ChordSymbolEditorModel::diminishedList() const{
    return m_diminishedList;
}

void ChordSymbolEditorModel::setChordStyle(QString styleName) const
{
    QString descriptionFileName = "chords_std.xml"; // Fall back

    for (auto& chordStyle: m_styles) {
        if (chordStyle.styleName == styleName) {
            descriptionFileName = chordStyle.fileName;
            break;
        }
    }

    globalContext()->currentNotation()->style()->setStyleValue(mu::notation::StyleId::chordDescriptionFile,descriptionFileName);
}
