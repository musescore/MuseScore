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
    m_chordSpellingList << "Standard" << "German" << "German Full" << "Solfege" << "French";
    styleManager = new ChordSymbolStyleManager();
    m_styles = styleManager->getChordStyles();
    initCurrentStyleIndex();
    setQualitySymbolsOnStyleChange();
    setPropertiesOnStyleChange();
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

void ChordSymbolStylesModel::setStyleR(Ms::Sid id, qreal val)
{
    globalContext()->currentNotation()->style()->setStyleValue(id, val);
}

void ChordSymbolStylesModel::setStyleB(Ms::Sid id, bool val)
{
    globalContext()->currentNotation()->style()->setStyleValue(id, val);
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

    globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::useChordSymbolPresets, m_styles.at(m_currentStyleIndex).usePresets);

    // Extract the selection history everytime because it could have been changed
    QString selectionHistory = globalContext()->currentNotation()->style()->styleValue(Ms::Sid::chordQualitySelectionHistory).toString();
    extractSelectionHistory(selectionHistory);

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
    globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::useChordSymbolPresets, m_styles.at(m_currentStyleIndex).usePresets);

    // Extract the selection history everytime because it could have been changed
    QString selectionHistory = globalContext()->currentNotation()->style()->styleValue(Ms::Sid::chordQualitySelectionHistory).toString();
    extractSelectionHistory(selectionHistory);

    setQualitySymbolsOnStyleChange();
    setPropertiesOnStyleChange();

    emit currentStyleIndexChanged();
}

void ChordSymbolStylesModel::setQualitySymbolsOnStyleChange()
{
    if (!m_styles[m_currentStyleIndex].usePresets) {
        return;
    }
    QString currentStyle = m_styles[m_currentStyleIndex].styleName;

    if (m_selectionHistory.find(currentStyle) != m_selectionHistory.end()) {
        // check if current style present in m_selectionHistory
        QString previousSelectedSymbol = m_selectionHistory.value(currentStyle).value("maj7th").toString();
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityMajorSeventh, previousSelectedSymbol);

        previousSelectedSymbol = m_selectionHistory.value(currentStyle).value("half-dim").toString();
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityHalfDiminished, previousSelectedSymbol);

        previousSelectedSymbol = m_selectionHistory.value(currentStyle).value("min").toString();
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityMinor, previousSelectedSymbol);

        previousSelectedSymbol = m_selectionHistory.value(currentStyle).value("aug").toString();
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityAugmented, previousSelectedSymbol);

        previousSelectedSymbol = m_selectionHistory.value(currentStyle).value("dim").toString();
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityDiminished, previousSelectedSymbol);

        previousSelectedSymbol = m_selectionHistory.value(currentStyle).value("sixNine").toString();
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordExtensionSixNine, previousSelectedSymbol);

        previousSelectedSymbol = m_selectionHistory.value(currentStyle).value("omit").toString();
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordModifierOmit, previousSelectedSymbol);

        previousSelectedSymbol = m_selectionHistory.value(currentStyle).value("sus").toString();
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordModifierSuspension, previousSelectedSymbol);
    } else {
        // Get quality symbols
        QString descriptionFile = globalContext()->currentNotation()->style()->styleValue(Ms::Sid::chordDescriptionFile).toString();
        QHash<QString, QList<QualitySymbol> > qualitySymbols = styleManager->getQualitySymbols(descriptionFile);

        //set the default
        QString defaultSymbol
            = (qualitySymbols.value("major7th").size() != 0) ? qualitySymbols.value("major7th").at(0).qualitySymbol : "-1";
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityMajorSeventh, defaultSymbol);

        defaultSymbol
            = (qualitySymbols.value("half-diminished").size() != 0) ? qualitySymbols.value("half-diminished").at(0).qualitySymbol : "-1";
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityHalfDiminished, defaultSymbol);

        defaultSymbol = (qualitySymbols.value("minor").size() != 0) ? qualitySymbols.value("minor").at(0).qualitySymbol : "-1";
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityMinor, defaultSymbol);

        defaultSymbol = (qualitySymbols.value("augmented").size() != 0) ? qualitySymbols.value("augmented").at(0).qualitySymbol : "-1";
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityAugmented, defaultSymbol);

        defaultSymbol = (qualitySymbols.value("diminished").size() != 0) ? qualitySymbols.value("diminished").at(0).qualitySymbol : "-1";
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityDiminished, defaultSymbol);

        defaultSymbol = (qualitySymbols.value("sixNine").size() != 0) ? qualitySymbols.value("sixNine").at(0).qualitySymbol : "-1";
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordExtensionSixNine, defaultSymbol);

        defaultSymbol = (qualitySymbols.value("omit").size() != 0) ? qualitySymbols.value("omit").at(0).qualitySymbol : "-1";
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordModifierOmit, defaultSymbol);

        defaultSymbol = (qualitySymbols.value("suspension").size() != 0) ? qualitySymbols.value("suspension").at(0).qualitySymbol : "-1";
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordModifierSuspension, defaultSymbol);
    }
}

void ChordSymbolStylesModel::setPropertiesOnStyleChange()
{
    QString currentStyle = m_styles[m_currentStyleIndex].styleName;

    if (m_selectionHistory.find(currentStyle) != m_selectionHistory.end()) {
        int chordSpellingIndex = m_selectionHistory.value(currentStyle).value("chrdSpell").toInt();
        setChordSpelling(m_chordSpellingList[chordSpellingIndex]);
        setStyleR(Ms::Sid::chordQualityMag, m_selectionHistory.value(currentStyle).value("qualMag").toReal());
        setStyleR(Ms::Sid::chordQualityAdjust, m_selectionHistory.value(currentStyle).value("qualAdj").toReal());
        setStyleR(Ms::Sid::chordExtensionMag, m_selectionHistory.value(currentStyle).value("extMag").toReal());
        setStyleR(Ms::Sid::chordExtensionAdjust, m_selectionHistory.value(currentStyle).value("extAdj").toReal());
        setStyleR(Ms::Sid::chordModifierMag, m_selectionHistory.value(currentStyle).value("modMag").toReal());
        setStyleR(Ms::Sid::chordModifierAdjust, m_selectionHistory.value(currentStyle).value("modAdj").toReal());

        setStyleR(Ms::Sid::harmonyFretDist, m_selectionHistory.value(currentStyle).value("hFretDist").toReal());
        setStyleR(Ms::Sid::minHarmonyDistance, m_selectionHistory.value(currentStyle).value("mnHDist").toReal());
        setStyleR(Ms::Sid::maxHarmonyBarDistance, m_selectionHistory.value(currentStyle).value("mxHBarDist").toReal());
        setStyleR(Ms::Sid::maxChordShiftAbove, m_selectionHistory.value(currentStyle).value("mxSftAbv").toReal());
        setStyleR(Ms::Sid::maxChordShiftBelow, m_selectionHistory.value(currentStyle).value("mxSftBlw").toReal());
        setStyleR(Ms::Sid::capoPosition, m_selectionHistory.value(currentStyle).value("cpFretPos").toReal());

        setStyleB(Ms::Sid::stackModifiers, (m_selectionHistory.value(currentStyle).value("stkMod").toReal() == 1));

        setStyleB(Ms::Sid::automaticCapitalization, (m_selectionHistory.value(currentStyle).value("autoCap").toReal() == 1));
        setStyleB(Ms::Sid::lowerCaseMinorChords, !(m_selectionHistory.value(currentStyle).value("minRtCap").toReal() == 1));
        setStyleB(Ms::Sid::lowerCaseQualitySymbols, !(m_selectionHistory.value(currentStyle).value("qualCap").toReal() == 1));
        setStyleB(Ms::Sid::lowerCaseBassNotes, !(m_selectionHistory.value(currentStyle).value("bsNtCap").toReal() == 1));
        setStyleB(Ms::Sid::allCapsNoteNames, (m_selectionHistory.value(currentStyle).value("solNtCap").toReal() == 1));

        setStyleB(Ms::Sid::chordAlterationsParentheses, (m_selectionHistory.value(currentStyle).value("altParen").toReal() == 1));
        setStyleB(Ms::Sid::chordSuspensionsParentheses, (m_selectionHistory.value(currentStyle).value("susParen").toReal() == 1));
        setStyleB(Ms::Sid::chordMinMajParentheses, (m_selectionHistory.value(currentStyle).value("minMajParen").toReal() == 1));
        setStyleB(Ms::Sid::chordAddOmitParentheses, (m_selectionHistory.value(currentStyle).value("addOmitParen").toReal() == 1));
    } else {
        // Set default values
        setChordSpelling(m_chordSpellingList[0]);

        setStyleR(Ms::Sid::chordQualityMag, 1.0);
        setStyleR(Ms::Sid::chordQualityAdjust, 0.0);
        setStyleR(Ms::Sid::chordExtensionMag, 1.0);
        setStyleR(Ms::Sid::chordExtensionAdjust, 0.0);
        setStyleR(Ms::Sid::chordModifierMag, 1.0);
        setStyleR(Ms::Sid::chordModifierAdjust, 0.0);

        setStyleR(Ms::Sid::harmonyFretDist, 1.0);
        setStyleR(Ms::Sid::minHarmonyDistance, 0.5);
        setStyleR(Ms::Sid::maxHarmonyBarDistance, 3.0);
        setStyleR(Ms::Sid::maxChordShiftAbove, 0.0);
        setStyleR(Ms::Sid::maxChordShiftBelow, 0.0);
        setStyleR(Ms::Sid::capoPosition, 0.0);

        setStyleB(Ms::Sid::stackModifiers, true);

        setStyleB(Ms::Sid::automaticCapitalization, true);
        setStyleB(Ms::Sid::lowerCaseMinorChords, false);
        setStyleB(Ms::Sid::lowerCaseQualitySymbols, false);
        setStyleB(Ms::Sid::lowerCaseBassNotes, false);
        setStyleB(Ms::Sid::allCapsNoteNames, false);

        setStyleB(Ms::Sid::chordAlterationsParentheses, true);
        setStyleB(Ms::Sid::chordSuspensionsParentheses, true);
        setStyleB(Ms::Sid::chordMinMajParentheses, true);
        setStyleB(Ms::Sid::chordAddOmitParentheses, true);
    }
}

void ChordSymbolStylesModel::setChordSpelling(QString newSpelling)
{
    QHash<QString, Ms::Sid> chordSpellingMap = {
        { "Standard", Ms::Sid::useStandardNoteNames },
        { "German", Ms::Sid::useGermanNoteNames },
        { "German Full", Ms::Sid::useFullGermanNoteNames },
        { "Solfege", Ms::Sid::useSolfeggioNoteNames },
        { "French", Ms::Sid::useFrenchNoteNames }
    };

    for (auto& spelling: m_chordSpellingList) {
        Ms::Sid id = chordSpellingMap.value(spelling);

        if (spelling == newSpelling) {
            globalContext()->currentNotation()->style()->setStyleValue(id, true);
        } else {
            globalContext()->currentNotation()->style()->setStyleValue(id, false);
        }
    }
}

void ChordSymbolStylesModel::extractSelectionHistory(QString selectionHistory)
{
    if (selectionHistory == "") {
        return;
    }
    m_selectionHistory.clear();
    QStringList selectionHistoryList = selectionHistory.split("\n");
    for (auto style: selectionHistoryList) {
        QStringList selectionHistoryOfStyle = style.split("|"); // { styleName, comma-separated properties }
        QStringList properties = selectionHistoryOfStyle[1].split(";");
        QHash<QString, QVariant> propHash;
        for (auto prop: properties) {
            QStringList keyValue = prop.split(":"); // {propValue, value}
            propHash.insert(keyValue[0], keyValue[1]);
        }
        m_selectionHistory.insert(selectionHistoryOfStyle[0], propHash);
    }
}
