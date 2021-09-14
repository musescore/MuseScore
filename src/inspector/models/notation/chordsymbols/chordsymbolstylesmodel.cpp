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
    // For refreshing the inspector view when style is changed through the styles dialog
    globalContext()->currentNotation()->style()->chordSymbolStyleChanged().onNotify(this, [this]() {
        initCurrentStyleIndex();
    });
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

void ChordSymbolStylesModel::setStyle(Ms::Sid id, QVariant val)
{
    globalContext()->currentNotation()->style()->setStyleValue(id, val);
}

QVariant ChordSymbolStylesModel::getDefVal(Ms::Sid id)
{
    return globalContext()->currentNotation()->style()->defaultStyleValue(id);
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
        setChordStyle(0);
    }

    globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::useChordSymbolPresets, m_styles.at(m_currentStyleIndex).usePresets);

    // Extract the selection history everytime because it could have been changed
    QString selectionHistory = globalContext()->currentNotation()->style()->styleValue(Ms::Sid::chordQualitySelectionHistory).toString();
    extractSelectionHistory(selectionHistory);

    emit currentStyleIndexChanged();
}

void ChordSymbolStylesModel::setChordStyle(int index)
{
    m_currentStyleIndex = index;
    QStringList chordStyles = { "Pop/Contemporary", "Jazz", "Symbols", "No preset style" };

    globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordStyle, m_styles.at(m_currentStyleIndex).styleName);
    globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordDescriptionFile, m_styles.at(m_currentStyleIndex).fileName);
    globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::useChordSymbolPresets, m_styles.at(m_currentStyleIndex).usePresets);
    if (!chordStyles.contains(m_styles.at(m_currentStyleIndex).styleName)) {
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordStyle, "custom");
    }

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
    // Get quality symbols
    QString descriptionFile = globalContext()->currentNotation()->style()->styleValue(Ms::Sid::chordDescriptionFile).toString();
    QHash<QString, QList<QualitySymbol> > qualitySymbols = styleManager->getQualitySymbols(descriptionFile);

    if (m_selectionHistory.find(currentStyle) != m_selectionHistory.end()) {
        // check if current style present in m_selectionHistory
        QString previousSelectedSymbol;
        int index = m_selectionHistory.value(currentStyle).value("maj7th").toInt();
        if (index != -1) {
            previousSelectedSymbol = qualitySymbols.value("major7th").at(index).qualitySymbol;
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityMajorSeventh, previousSelectedSymbol);
        } else {
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityMajorSeventh, "-1");
        }

        index = m_selectionHistory.value(currentStyle).value("half-dim").toInt();
        if (index != -1) {
            previousSelectedSymbol = qualitySymbols.value("half-diminished").at(index).qualitySymbol;
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityHalfDiminished, previousSelectedSymbol);
        } else {
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityHalfDiminished, "-1");
        }

        index = m_selectionHistory.value(currentStyle).value("min").toInt();
        if (index != -1) {
            previousSelectedSymbol = qualitySymbols.value("minor").at(index).qualitySymbol;
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityMinor, previousSelectedSymbol);
        } else {
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityMinor, "-1");
        }

        index = m_selectionHistory.value(currentStyle).value("aug").toInt();
        if (index != -1) {
            previousSelectedSymbol = qualitySymbols.value("augmented").at(index).qualitySymbol;
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityAugmented, previousSelectedSymbol);
        } else {
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityAugmented, "-1");
        }

        index = m_selectionHistory.value(currentStyle).value("dim").toInt();
        if (index != -1) {
            previousSelectedSymbol = qualitySymbols.value("diminished").at(index).qualitySymbol;
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityDiminished, previousSelectedSymbol);
        } else {
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityDiminished, "-1");
        }

        index = m_selectionHistory.value(currentStyle).value("sixNine").toInt();
        if (index != -1) {
            previousSelectedSymbol = qualitySymbols.value("sixNine").at(index).qualitySymbol;
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordExtensionSixNine, previousSelectedSymbol);
        } else {
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordExtensionSixNine, "-1");
        }

        index = m_selectionHistory.value(currentStyle).value("omit").toInt();
        if (index != -1) {
            previousSelectedSymbol = qualitySymbols.value("omit").at(index).qualitySymbol;
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordModifierOmit, previousSelectedSymbol);
        } else {
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordModifierOmit, "-1");
        }

        index = m_selectionHistory.value(currentStyle).value("sus").toInt();
        if (index != -1) {
            previousSelectedSymbol = qualitySymbols.value("suspension").at(index).qualitySymbol;
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordModifierSuspension, previousSelectedSymbol);
        } else {
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordModifierSuspension, "-1");
        }

        index = m_selectionHistory.value(currentStyle).value("bass").toInt();
        if (index != -1) {
            previousSelectedSymbol = qualitySymbols.value("bassNote").at(index).qualitySymbol;
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordBassNote, previousSelectedSymbol);
        } else {
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordBassNote, "-1");
        }
    } else {
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

        defaultSymbol = (qualitySymbols.value("bassNote").size() != 0) ? qualitySymbols.value("bassNote").at(0).qualitySymbol : "-1";
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordBassNote, defaultSymbol);
    }
}

void ChordSymbolStylesModel::setPropertiesOnStyleChange()
{
    QString currentStyle = m_styles[m_currentStyleIndex].styleName;

    if (m_selectionHistory.find(currentStyle) != m_selectionHistory.end()) {
        int chordSpellingIndex = m_selectionHistory.value(currentStyle).value("chrdSpell").toInt();
        setChordSpelling(m_chordSpellingList[chordSpellingIndex]);
        setStyle(Ms::Sid::chordQualityMag, m_selectionHistory.value(currentStyle).value("qualMag").toReal());
        setStyle(Ms::Sid::chordQualityAdjust, m_selectionHistory.value(currentStyle).value("qualAdj").toReal());
        setStyle(Ms::Sid::chordExtensionMag, m_selectionHistory.value(currentStyle).value("extMag").toReal());
        setStyle(Ms::Sid::chordExtensionAdjust, m_selectionHistory.value(currentStyle).value("extAdj").toReal());
        setStyle(Ms::Sid::chordModifierMag, m_selectionHistory.value(currentStyle).value("modMag").toReal());
        setStyle(Ms::Sid::chordModifierAdjust, m_selectionHistory.value(currentStyle).value("modAdj").toReal());

        setStyle(Ms::Sid::harmonyFretDist, m_selectionHistory.value(currentStyle).value("hFretDist").toReal());
        setStyle(Ms::Sid::minHarmonyDistance, m_selectionHistory.value(currentStyle).value("mnHDist").toReal());
        setStyle(Ms::Sid::maxHarmonyBarDistance, m_selectionHistory.value(currentStyle).value("mxHBarDist").toReal());
        setStyle(Ms::Sid::maxChordShiftAbove, m_selectionHistory.value(currentStyle).value("mxSftAbv").toReal());
        setStyle(Ms::Sid::maxChordShiftBelow, m_selectionHistory.value(currentStyle).value("mxSftBlw").toReal());
        setStyle(Ms::Sid::capoPosition, m_selectionHistory.value(currentStyle).value("cpFretPos").toReal());

        setStyle(Ms::Sid::stackModifiers, (m_selectionHistory.value(currentStyle).value("stkMod").toReal() == 1));

        setStyle(Ms::Sid::automaticCapitalization, (m_selectionHistory.value(currentStyle).value("autoCap").toReal() == 1));
        setStyle(Ms::Sid::lowerCaseMinorChords, !(m_selectionHistory.value(currentStyle).value("minRtCap").toReal() == 1));
        setStyle(Ms::Sid::lowerCaseMajorSymbols, !(m_selectionHistory.value(currentStyle).value("qualMajCap").toReal() == 1));
        setStyle(Ms::Sid::lowerCaseMinorSymbols, !(m_selectionHistory.value(currentStyle).value("qualMinCap").toReal() == 1));
        setStyle(Ms::Sid::lowerCaseBassNotes, !(m_selectionHistory.value(currentStyle).value("bsNtCap").toReal() == 1));
        setStyle(Ms::Sid::allCapsNoteNames, (m_selectionHistory.value(currentStyle).value("solNtCap").toReal() == 1));

        setStyle(Ms::Sid::chordAlterationsParentheses, (m_selectionHistory.value(currentStyle).value("altParen").toReal() == 1));
        setStyle(Ms::Sid::chordSuspensionsParentheses, (m_selectionHistory.value(currentStyle).value("susParen").toReal() == 1));
        setStyle(Ms::Sid::chordMinMajParentheses, (m_selectionHistory.value(currentStyle).value("minMajParen").toReal() == 1));
        setStyle(Ms::Sid::chordAddOmitParentheses, (m_selectionHistory.value(currentStyle).value("addOmitParen").toReal() == 1));
    } else {
        // Set default values
        setChordSpelling(m_chordSpellingList[0]);

        setStyle(Ms::Sid::chordQualityMag, getDefVal(Ms::Sid::chordQualityMag));
        setStyle(Ms::Sid::chordQualityAdjust, getDefVal(Ms::Sid::chordQualityAdjust));
        setStyle(Ms::Sid::chordExtensionMag, getDefVal(Ms::Sid::chordExtensionMag));
        setStyle(Ms::Sid::chordExtensionAdjust, getDefVal(Ms::Sid::chordExtensionAdjust));
        setStyle(Ms::Sid::chordModifierMag, getDefVal(Ms::Sid::chordModifierMag));
        setStyle(Ms::Sid::chordModifierAdjust, getDefVal(Ms::Sid::chordModifierAdjust));

        setStyle(Ms::Sid::harmonyFretDist, getDefVal(Ms::Sid::harmonyFretDist));
        setStyle(Ms::Sid::minHarmonyDistance, getDefVal(Ms::Sid::minHarmonyDistance));
        setStyle(Ms::Sid::maxHarmonyBarDistance, getDefVal(Ms::Sid::maxHarmonyBarDistance));
        setStyle(Ms::Sid::maxChordShiftAbove, getDefVal(Ms::Sid::maxChordShiftAbove));
        setStyle(Ms::Sid::maxChordShiftBelow, getDefVal(Ms::Sid::maxChordShiftBelow));
        setStyle(Ms::Sid::capoPosition, getDefVal(Ms::Sid::capoPosition));

        setStyle(Ms::Sid::stackModifiers, getDefVal(Ms::Sid::stackModifiers));

        setStyle(Ms::Sid::automaticCapitalization, getDefVal(Ms::Sid::automaticCapitalization));
        setStyle(Ms::Sid::lowerCaseMinorChords, getDefVal(Ms::Sid::lowerCaseMinorChords));
        setStyle(Ms::Sid::lowerCaseMajorSymbols, getDefVal(Ms::Sid::lowerCaseMajorSymbols));
        setStyle(Ms::Sid::lowerCaseMinorSymbols, getDefVal(Ms::Sid::lowerCaseMinorSymbols));
        setStyle(Ms::Sid::lowerCaseBassNotes, getDefVal(Ms::Sid::lowerCaseBassNotes));
        setStyle(Ms::Sid::allCapsNoteNames, getDefVal(Ms::Sid::allCapsNoteNames));

        setStyle(Ms::Sid::chordAlterationsParentheses, getDefVal(Ms::Sid::chordAlterationsParentheses));
        setStyle(Ms::Sid::chordSuspensionsParentheses, getDefVal(Ms::Sid::chordSuspensionsParentheses));
        setStyle(Ms::Sid::chordMinMajParentheses, getDefVal(Ms::Sid::chordMinMajParentheses));
        setStyle(Ms::Sid::chordAddOmitParentheses, getDefVal(Ms::Sid::chordAddOmitParentheses));
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
