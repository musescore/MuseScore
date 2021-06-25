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

using namespace mu::notation;
ChordSymbolEditorModel::ChordSymbolEditorModel(QObject* parent)
    : QAbstractListModel(parent)
{
    styleManager = new ChordSymbolStyleManager();
    m_styles = styleManager->getChordStyles();
    setQualityRepresentationsLists();
    initChordSpellingList();
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

QStringList ChordSymbolEditorModel::chordSpellingList() const
{
    return m_chordSpellingList;
}

QStringList ChordSymbolEditorModel::majorSeventhList() const
{
    return m_majorSeventhList;
}

QStringList ChordSymbolEditorModel::halfDiminishedList() const
{
    return m_halfDiminishedList;
}

QStringList ChordSymbolEditorModel::minorList() const
{
    return m_minorList;
}

QStringList ChordSymbolEditorModel::augmentedList() const
{
    return m_augmentedList;
}

QStringList ChordSymbolEditorModel::diminishedList() const
{
    return m_diminishedList;
}

void ChordSymbolEditorModel::initChordSpellingList()
{
    m_chordSpellingList << "Standard" << "German" << "German Full" << "Solfege" << "French";
}

void ChordSymbolEditorModel::setQualityRepresentationsLists()
{
    // Get the symbols from the file
    QString descriptionFile = globalContext()->currentNotation()->style()->styleValue(Ms::Sid::chordDescriptionFile).toString();
    m_qualitySymbols = styleManager->getQualitySymbols(descriptionFile);

    // Set the respective lists
    m_majorSeventhList = m_qualitySymbols["major7th"];
    m_halfDiminishedList = m_qualitySymbols["half-diminished"];
    m_minorList = m_qualitySymbols["minor"];
    m_augmentedList = m_qualitySymbols["augmented"];
    m_diminishedList = m_qualitySymbols["diminished"];

    // Notify QML ListViews about the change
    emit chordSpellingListChanged();
    emit majorSeventhListChanged();
    emit halfDiminishedListChanged();
    emit minorListChanged();
    emit augmentedListChanged();
    emit diminishedListChanged();
}

void ChordSymbolEditorModel::setQualitySymbol(QString quality, QString symbol)
{
    QHash<QString, Ms::Sid> qualityToSid = {
        { "major7th", Ms::Sid::chordQualityMajorSeventh },
        { "half-diminished", Ms::Sid::chordQualityHalfDiminished },
        { "minor", Ms::Sid::chordQualityMinor },
        { "augmented", Ms::Sid::chordQualityAugmented },
        { "diminished", Ms::Sid::chordQualityDiminished },
    };

    Ms::Sid id = qualityToSid.value(quality);
    globalContext()->currentNotation()->style()->setStyleValue(id, symbol);
    globalContext()->currentNotation()->score()->setUpQualitySymbols();

    emit chordSpellingListChanged();
    emit majorSeventhListChanged();
    emit halfDiminishedListChanged();
    emit minorListChanged();
    emit augmentedListChanged();
    emit diminishedListChanged();
}

void ChordSymbolEditorModel::setChordStyle(QString styleName)
{
    QString descriptionFileName = "chords_std.xml"; // Fall back

    for (auto& chordStyle: m_styles) {
        if (chordStyle.styleName == styleName) {
            descriptionFileName = chordStyle.fileName;
            break;
        }
    }

    globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordDescriptionFile, descriptionFileName);
    setQualityRepresentationsLists();
}

void ChordSymbolEditorModel::setChordSpelling(QString newSpelling)
{
    QHash<QString, Ms::Sid> chordSpellingMap = {
        { "Standard", Ms::Sid::useStandardNoteNames },
        { "German", Ms::Sid::useGermanNoteNames },
        { "German Full", Ms::Sid::useFullGermanNoteNames },
        { "Solfege", Ms::Sid::useSolfeggioNoteNames },
        { "French", Ms::Sid::useFrenchNoteNames }
    };

    for (auto i = chordSpellingMap.begin(); i != chordSpellingMap.end(); ++i) {
        QString spelling = i.key();
        Ms::Sid id = i.value();

        if (spelling == newSpelling) {
            globalContext()->currentNotation()->style()->setStyleValue(id, true);
        } else {
            globalContext()->currentNotation()->style()->setStyleValue(id, false);
        }
    }
}
