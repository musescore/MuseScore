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
    initCurrentStyleIndex();
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

int ChordSymbolStylesModel::currentStyleIndex() const
{
    return m_currentStyleIndex;
}

void ChordSymbolStylesModel::initCurrentStyleIndex()
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
    updateQualitySymbols();

    emit currentStyleIndexChanged();
}

void ChordSymbolStylesModel::setChordStyle(QString styleName)
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

    globalContext()->currentNotation()->style()->setStyleValue(StyleId::chordDescriptionFile, descriptionFileName);
    updateQualitySymbols();

    emit currentStyleIndexChanged();
}

void ChordSymbolStylesModel::updateQualitySymbols()
{
    QString currentStyle = m_styles[m_currentStyleIndex].styleName;

    // Get quality symbols
    QString descriptionFile = globalContext()->currentNotation()->style()->styleValue(Ms::Sid::chordDescriptionFile).toString();
    QHash<QString, QStringList> qualitySymbols = styleManager->getQualitySymbols(descriptionFile);

    // Extract the selection history everytime because it could have been changed
    QString selectionHistory = globalContext()->currentNotation()->style()->styleValue(Ms::Sid::chordQualitySelectionHistory).toString();
    extractSelectionHistory(selectionHistory);

    // Major Seventh
    Ms::Sid id = Ms::Sid::chordQualityMajorSeventh;
    if (m_selectionHistory.find(currentStyle) != m_selectionHistory.end()) {
        // check if current style present in m_selectionHistory
        QString previousSelectedSymbol = m_selectionHistory.value(currentStyle).at(0);
        globalContext()->currentNotation()->style()->setStyleValue(id, previousSelectedSymbol);
    } else {
        //set the default
        QString symMaj7 = qualitySymbols.value("major7th").at(0);
        globalContext()->currentNotation()->style()->setStyleValue(id, symMaj7);
    }

    // Half Diminished
    id = Ms::Sid::chordQualityHalfDiminished;
    if (m_selectionHistory.find(currentStyle) != m_selectionHistory.end()) {
        // check if current style present in m_selectionHistory
        QString previousSelectedSymbol = m_selectionHistory.value(currentStyle).at(1);
        globalContext()->currentNotation()->style()->setStyleValue(id, previousSelectedSymbol);
    } else {
        //set the default
        QString symHD = qualitySymbols.value("half-diminished").at(0);
        globalContext()->currentNotation()->style()->setStyleValue(id, symHD);
    }

    // Minor
    id = Ms::Sid::chordQualityMinor;
    if (m_selectionHistory.find(currentStyle) != m_selectionHistory.end()) {
        // check if current style present in m_selectionHistory
        QString previousSelectedSymbol = m_selectionHistory.value(currentStyle).at(2);
        globalContext()->currentNotation()->style()->setStyleValue(id, previousSelectedSymbol);
    } else {
        //set the default
        QString symMin = qualitySymbols.value("minor").at(0);
        globalContext()->currentNotation()->style()->setStyleValue(id, symMin);
    }

    // Augmented
    id = Ms::Sid::chordQualityAugmented;
    if (m_selectionHistory.find(currentStyle) != m_selectionHistory.end()) {
        // check if current style present in m_selectionHistory
        QString previousSelectedSymbol = m_selectionHistory.value(currentStyle).at(3);
        globalContext()->currentNotation()->style()->setStyleValue(id, previousSelectedSymbol);
    } else {
        //set the default
        QString symAug = qualitySymbols.value("augmented").at(0);
        globalContext()->currentNotation()->style()->setStyleValue(id, symAug);
    }

    // Diminished
    id = Ms::Sid::chordQualityDiminished;
    if (m_selectionHistory.find(currentStyle) != m_selectionHistory.end()) {
        // check if current style present in m_selectionHistory
        QString previousSelectedSymbol = m_selectionHistory.value(currentStyle).at(4);
        globalContext()->currentNotation()->style()->setStyleValue(id, previousSelectedSymbol);
    } else {
        //set the default
        QString symDim = qualitySymbols.value("diminished").at(0);
        globalContext()->currentNotation()->style()->setStyleValue(id, symDim);
    }

    //omit
    id = Ms::Sid::chordModifierOmit;
    if (m_selectionHistory.find(currentStyle) != m_selectionHistory.end()) {
        // check if current style present in m_selectionHistory
        QString previousSelectedSymbol = m_selectionHistory.value(currentStyle).at(5);
        globalContext()->currentNotation()->style()->setStyleValue(id, previousSelectedSymbol);
    } else {
        //set the default
        QString symOmit = qualitySymbols.value("omit").at(0);
        globalContext()->currentNotation()->style()->setStyleValue(id, symOmit);
    }
}

void ChordSymbolStylesModel::extractSelectionHistory(QString selectionHistory)
{
    m_selectionHistory.clear();
    QStringList selectionHistoryList = selectionHistory.split("\n");
    for (auto s: selectionHistoryList) {
        QStringList selectionHistoryOfStyle = s.split("|");
        m_selectionHistory.insert(selectionHistoryOfStyle[0], selectionHistoryOfStyle[1].split(","));
    }
}
