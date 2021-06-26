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
    setQualitySymbolsLists();
    initChordSpellingList();
    initCurrentStyleIndex();
    updatePropertyIndices();
    updateQualitySymbolsIndices();
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

int ChordSymbolEditorModel::chordSpellingIndex() const
{
    return m_chordSpellingIndex;
}

int ChordSymbolEditorModel::currentStyleIndex() const
{
    return m_currentStyleIndex;
}

int ChordSymbolEditorModel::majorSeventhIndex() const
{
    return m_majorSeventhIndex;
}

int ChordSymbolEditorModel::halfDiminishedIndex() const
{
    return m_halfDiminishedIndex;
}

int ChordSymbolEditorModel::minorIndex() const
{
    return m_minorIndex;
}

int ChordSymbolEditorModel::augmentedIndex() const
{
    return m_augmentedIndex;
}

int ChordSymbolEditorModel::diminishedIndex() const
{
    return m_diminishedIndex;
}

void ChordSymbolEditorModel::initChordSpellingList()
{
    m_chordSpellingList << "Standard" << "German" << "German Full" << "Solfege" << "French";
    emit chordSpellingListChanged();
}

void ChordSymbolEditorModel::initCurrentStyleIndex()
{
    int index = 0;
    bool foundCurrentStyle = false;
    QString descriptionFile = globalContext()->currentNotation()->style()->styleValue(Ms::Sid::chordDescriptionFile).toString();

    for (auto& chordStyle: m_styles) {
        if (chordStyle.fileName == descriptionFile) {
            m_currentStyleIndex = index;
            foundCurrentStyle = true;
            break;
        }
        index++;
    }

    if (!foundCurrentStyle && (m_styles.size() > 0)) {
        setChordStyle(m_styles[0].styleName);
    }

    emit currentStyleIndexChanged();
}

void ChordSymbolEditorModel::updatePropertyIndices()
{
    // Will include extension, scaling and stuff in the future
    for (auto& spelling: m_chordSpellingList) {
        Ms::Sid id = chordSpellingMap.value(spelling);
        bool isCurrentChordSpelling = globalContext()->currentNotation()->style()->styleValue(id).toBool();
        if (isCurrentChordSpelling) {
            m_chordSpellingIndex = m_chordSpellingList.indexOf(spelling);
        }
    }

    emit chordSpellingIndexChanged();
}

void ChordSymbolEditorModel::updateQualitySymbolsIndices()
{
    QHash<QString, Ms::Sid> qualityToSid = {
        { "major7th", Ms::Sid::chordQualityMajorSeventh },
        { "half-diminished", Ms::Sid::chordQualityHalfDiminished },
        { "minor", Ms::Sid::chordQualityMinor },
        { "augmented", Ms::Sid::chordQualityAugmented },
        { "diminished", Ms::Sid::chordQualityDiminished },
    };

    // Major Seventh
    Ms::Sid id = qualityToSid.value("major7th");
    QString currentSymbol = globalContext()->currentNotation()->style()->styleValue(id).toString();
    if (m_majorSeventhList.contains(currentSymbol)) {
        m_majorSeventhIndex = m_majorSeventhList.indexOf(currentSymbol);
    } else {
        //set the default
        m_majorSeventhIndex = 0;
        globalContext()->currentNotation()->style()->setStyleValue(id, m_majorSeventhList[0]);
    }

    // Half Diminished
    id = qualityToSid.value("half-diminished");
    currentSymbol = globalContext()->currentNotation()->style()->styleValue(id).toString();
    if (m_halfDiminishedList.contains(currentSymbol)) {
        m_halfDiminishedIndex = m_halfDiminishedList.indexOf(currentSymbol);
    } else {
        //set the default
        m_halfDiminishedIndex = 0;
        globalContext()->currentNotation()->style()->setStyleValue(id, m_halfDiminishedList[0]);
    }

    // Minor
    id = qualityToSid.value("minor");
    currentSymbol = globalContext()->currentNotation()->style()->styleValue(id).toString();
    if (m_minorList.contains(currentSymbol)) {
        m_minorIndex = m_minorList.indexOf(currentSymbol);
    } else {
        //set the default
        m_minorIndex = 0;
        globalContext()->currentNotation()->style()->setStyleValue(id, m_minorList[0]);
    }

    // Augmented
    id = qualityToSid.value("augmented");
    currentSymbol = globalContext()->currentNotation()->style()->styleValue(id).toString();
    if (m_augmentedList.contains(currentSymbol)) {
        m_augmentedIndex = m_augmentedList.indexOf(currentSymbol);
    } else {
        //set the default
        m_augmentedIndex = 0;
        globalContext()->currentNotation()->style()->setStyleValue(id, m_augmentedList[0]);
    }

    // Diminished
    id = qualityToSid.value("diminished");
    currentSymbol = globalContext()->currentNotation()->style()->styleValue(id).toString();
    if (m_diminishedList.contains(currentSymbol)) {
        m_diminishedIndex = m_diminishedList.indexOf(currentSymbol);
    } else {
        //set the default
        m_diminishedIndex = 0;
        globalContext()->currentNotation()->style()->setStyleValue(id, m_diminishedList[0]);
    }

    globalContext()->currentNotation()->score()->setUpQualitySymbols();

    emit majorSeventhIndexChanged();
    emit halfDiminishedIndexChanged();
    emit minorIndexChanged();
    emit augmentedIndexChanged();
    emit diminishedIndexChanged();
}

void ChordSymbolEditorModel::refreshChordSymbols()
{
    // Temporary hack to get the chord symbols to refresh
    globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordDescriptionFile, "dummy");
    globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordDescriptionFile, m_styles[m_currentStyleIndex].fileName);
}

void ChordSymbolEditorModel::setQualitySymbolsLists()
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
    updateQualitySymbolsIndices();
    refreshChordSymbols();
}

void ChordSymbolEditorModel::setChordStyle(QString styleName)
{
    QString descriptionFileName = "chords_std.xml"; // Fall back

    int index = 0;
    for (auto& chordStyle: m_styles) {
        if (chordStyle.styleName == styleName) {
            descriptionFileName = chordStyle.fileName;
            m_currentStyleIndex = index;
            break;
        }
        index++;
    }

    globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordDescriptionFile, descriptionFileName);
    setQualitySymbolsLists();
    updateQualitySymbolsIndices();
    refreshChordSymbols();

    emit currentStyleIndexChanged();
}

void ChordSymbolEditorModel::setChordSpelling(QString newSpelling)
{
    for (auto& spelling: m_chordSpellingList) {
        Ms::Sid id = chordSpellingMap.value(spelling);

        if (spelling == newSpelling) {
            globalContext()->currentNotation()->style()->setStyleValue(id, true);
        } else {
            globalContext()->currentNotation()->style()->setStyleValue(id, false);
        }
    }

    updatePropertyIndices();
}
