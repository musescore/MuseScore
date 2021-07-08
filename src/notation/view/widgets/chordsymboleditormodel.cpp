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
    initCurrentStyleIndex();
    initProperties();
    updatePropertyIndices();
    updateQualitySymbolsIndices(true);
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

QStringList ChordSymbolEditorModel::omitList() const
{
    return m_omitList;
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

int ChordSymbolEditorModel::stackModifiersIndex() const
{
    return m_stackModifiersIndex;
}

int ChordSymbolEditorModel::omitIndex() const
{
    return m_omitIndex;
}

qreal ChordSymbolEditorModel::qualityMag() const
{
    return m_qualityMag;
}

qreal ChordSymbolEditorModel::qualityAdjust() const
{
    return m_qualityAdjust;
}

qreal ChordSymbolEditorModel::extensionMag() const
{
    return m_extensionMag;
}

qreal ChordSymbolEditorModel::extensionAdjust() const
{
    return m_extensionAdjust;
}

qreal ChordSymbolEditorModel::modifierMag() const
{
    return m_modifierMag;
}

qreal ChordSymbolEditorModel::modifierAdjust() const
{
    return m_modifierAdjust;
}

qreal ChordSymbolEditorModel::harmonyFretDistance() const
{
    return m_harmonyFretDistance;
}

qreal ChordSymbolEditorModel::minHarmonyDistance() const
{
    return m_minHarmonyDistance;
}

qreal ChordSymbolEditorModel::maxHarmonyBarDistance() const
{
    return m_maxHarmonyBarDistance;
}

qreal ChordSymbolEditorModel::maxChordShiftAbove() const
{
    return m_maxChordShiftAbove;
}

qreal ChordSymbolEditorModel::maxChordShiftBelow() const
{
    return m_maxChordShiftBelow;
}

qreal ChordSymbolEditorModel::capoFretPosition() const
{
    return m_capoFretPosition;
}

qreal ChordSymbolEditorModel::autoCapitalization() const
{
    return m_autoCapitalization;
}

qreal ChordSymbolEditorModel::minorRootCapitalization() const
{
    return m_minorRootCapitalization;
}

qreal ChordSymbolEditorModel::qualitySymbolsCapitalization() const
{
    return m_qualitySymbolsCapitalization;
}

qreal ChordSymbolEditorModel::bassNotesCapitalization() const
{
    return m_bassNotesCapitalization;
}

qreal ChordSymbolEditorModel::solfegeNotesCapitalization() const
{
    return m_solfegeNotesCapitalization;
}

qreal ChordSymbolEditorModel::alterationsParentheses() const
{
    return m_alterationsParentheses;
}

qreal ChordSymbolEditorModel::suspensionsParentheses() const
{
    return m_suspensionsParentheses;
}

qreal ChordSymbolEditorModel::minMajParentheses() const
{
    return m_minMajParentheses;
}

qreal ChordSymbolEditorModel::addOmitParentheses() const
{
    return m_addOmitParentheses;
}

void ChordSymbolEditorModel::initProperties()
{
    m_chordSpellingList << "Standard" << "German" << "German Full" << "Solfege" << "French";
    m_qualityMag = globalContext()->currentNotation()->style()->styleValue(Ms::Sid::chordQualityMag).toReal();
    m_qualityAdjust = globalContext()->currentNotation()->style()->styleValue(Ms::Sid::chordQualityAdjust).toReal();
    m_extensionMag = globalContext()->currentNotation()->style()->styleValue(Ms::Sid::chordExtensionMag).toReal();
    m_extensionAdjust = globalContext()->currentNotation()->style()->styleValue(Ms::Sid::chordExtensionAdjust).toReal();
    m_modifierMag = globalContext()->currentNotation()->style()->styleValue(Ms::Sid::chordModifierMag).toReal();
    m_modifierAdjust = globalContext()->currentNotation()->style()->styleValue(Ms::Sid::chordModifierAdjust).toReal();
    m_harmonyFretDistance = globalContext()->currentNotation()->style()->styleValue(Ms::Sid::harmonyFretDist).toReal();
    m_minHarmonyDistance = globalContext()->currentNotation()->style()->styleValue(Ms::Sid::minHarmonyDistance).toReal();
    m_maxHarmonyBarDistance = globalContext()->currentNotation()->style()->styleValue(Ms::Sid::maxHarmonyBarDistance).toReal();
    m_maxChordShiftAbove = globalContext()->currentNotation()->style()->styleValue(Ms::Sid::maxChordShiftAbove).toReal();
    m_maxChordShiftBelow = globalContext()->currentNotation()->style()->styleValue(Ms::Sid::maxChordShiftBelow).toReal();
    m_capoFretPosition = globalContext()->currentNotation()->style()->styleValue(Ms::Sid::capoPosition).toReal();
    if (globalContext()->currentNotation()->style()->styleValue(Ms::Sid::stackModifiers).toBool()) {
        m_stackModifiersIndex = 0;
    } else {
        m_stackModifiersIndex = 1;
    }
    if (globalContext()->currentNotation()->style()->styleValue(Ms::Sid::automaticCapitalization).toBool()) {
        m_autoCapitalization = 1.0;
    } else {
        m_autoCapitalization = 0.0;
    }
    if (globalContext()->currentNotation()->style()->styleValue(Ms::Sid::lowerCaseMinorChords).toBool()) {
        m_minorRootCapitalization = 0.0;
    } else {
        m_minorRootCapitalization = 1.0;
    }
    if (globalContext()->currentNotation()->style()->styleValue(Ms::Sid::lowerCaseQualitySymbols).toBool()) {
        m_qualitySymbolsCapitalization = 0.0;
    } else {
        m_qualitySymbolsCapitalization = 1.0;
    }
    if (globalContext()->currentNotation()->style()->styleValue(Ms::Sid::lowerCaseBassNotes).toBool()) {
        m_bassNotesCapitalization = 0.0;
    } else {
        m_bassNotesCapitalization = 1.0;
    }
    if (globalContext()->currentNotation()->style()->styleValue(Ms::Sid::allCapsNoteNames).toBool()) {
        m_solfegeNotesCapitalization = 1.0;
    } else {
        m_solfegeNotesCapitalization = 0.0;
    }
    if (globalContext()->currentNotation()->style()->styleValue(Ms::Sid::chordAlterationsParentheses).toBool()) {
        m_alterationsParentheses = 1.0;
    } else {
        m_alterationsParentheses = 0.0;
    }
    if (globalContext()->currentNotation()->style()->styleValue(Ms::Sid::chordSuspensionsParentheses).toBool()) {
        m_suspensionsParentheses = 1.0;
    } else {
        m_suspensionsParentheses = 0.0;
    }
    if (globalContext()->currentNotation()->style()->styleValue(Ms::Sid::chordMinMajParentheses).toBool()) {
        m_minMajParentheses = 1.0;
    } else {
        m_minMajParentheses = 0.0;
    }
    if (globalContext()->currentNotation()->style()->styleValue(Ms::Sid::chordAddOmitParentheses).toBool()) {
        m_addOmitParentheses = 1.0;
    } else {
        m_addOmitParentheses = 0.0;
    }

    emit chordSpellingListChanged();
    emit qualityMagChanged();
    emit qualityAdjustChanged();
    emit extensionMagChanged();
    emit extensionAdjustChanged();
    emit modifierMagChanged();
    emit modifierAdjustChanged();
    emit harmonyFretDistanceChanged();
    emit minHarmonyDistanceChanged();
    emit maxHarmonyBarDistanceChanged();
    emit maxChordShiftAboveChanged();
    emit maxChordShiftBelowChanged();
    emit stackModifiersIndexChanged();
    emit autoCapitalizationChanged();
    emit minorRootCapitalizationChanged();
    emit qualitySymbolsCapitalizationChanged();
    emit bassNotesCapitalizationChanged();
    emit solfegeNotesCapitalizationChanged();
    emit alterationsParenthesesChanged();
    emit suspensionsParenthesesChanged();
    emit minMajParenthesesChanged();
    emit addOmitParenthesesChanged();
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

    // If the current style is not found in m_styles(i.e. no curresponding file),
    // set the first style available
    if (!foundCurrentStyle && (m_styles.size() > 0)) {
        setChordStyle(m_styles[0].styleName);
    }

    emit currentStyleIndexChanged();
}

void ChordSymbolEditorModel::updatePropertyIndices()
{
    for (auto& spelling: m_chordSpellingList) {
        Ms::Sid id = chordSpellingMap.value(spelling);
        bool isCurrentChordSpelling = globalContext()->currentNotation()->style()->styleValue(id).toBool();
        if (isCurrentChordSpelling) {
            m_chordSpellingIndex = m_chordSpellingList.indexOf(spelling);
        }
    }

    emit chordSpellingIndexChanged();
}

void ChordSymbolEditorModel::updateQualitySymbolsIndices(bool chordStyleChanged)
{
    // Steps/Checks for each quality/modifier
    // 1. Check if the current symbol is present in the current list. If so, set the index.
    // 2. Check if the selection history contains the symbol. If so, set the index.
    // 3. Set the first symbol. This condition occurs when that style is set for the first time.

    // Whenever the chord style is changed, skip 1st step
    // to preserve the selection history

    QString currentStyle = m_styles[m_currentStyleIndex].styleName;

    // Major Seventh
    Ms::Sid id = Ms::Sid::chordQualityMajorSeventh;
    QString currentSymbol = globalContext()->currentNotation()->style()->styleValue(id).toString();
    if (!chordStyleChanged && m_majorSeventhList.contains(currentSymbol)) {
        m_majorSeventhIndex = m_majorSeventhList.indexOf(currentSymbol);
    } else if (m_selectionHistory.find(currentStyle) != m_selectionHistory.end()) {
        // check if current style present in m_selectionHistory
        QString previousSelectedSymbol = m_selectionHistory.value(currentStyle).at(0);
        m_majorSeventhIndex = m_majorSeventhList.indexOf(previousSelectedSymbol);
        globalContext()->currentNotation()->style()->setStyleValue(id, previousSelectedSymbol);
    } else {
        //set the default
        m_majorSeventhIndex = 0;
        globalContext()->currentNotation()->style()->setStyleValue(id, m_majorSeventhList[0]);
    }

    // Half Diminished
    id = Ms::Sid::chordQualityHalfDiminished;
    currentSymbol = globalContext()->currentNotation()->style()->styleValue(id).toString();
    if (!chordStyleChanged && m_halfDiminishedList.contains(currentSymbol)) {
        m_halfDiminishedIndex = m_halfDiminishedList.indexOf(currentSymbol);
    } else if (m_selectionHistory.find(currentStyle) != m_selectionHistory.end()) {
        // check if current style present in m_selectionHistory
        QString previousSelectedSymbol = m_selectionHistory.value(currentStyle).at(1);
        m_halfDiminishedIndex = m_halfDiminishedList.indexOf(previousSelectedSymbol);
        globalContext()->currentNotation()->style()->setStyleValue(id, previousSelectedSymbol);
    } else {
        //set the default
        m_halfDiminishedIndex = 0;
        globalContext()->currentNotation()->style()->setStyleValue(id, m_halfDiminishedList[0]);
    }

    // Minor
    id = Ms::Sid::chordQualityMinor;
    currentSymbol = globalContext()->currentNotation()->style()->styleValue(id).toString();
    if (!chordStyleChanged && m_minorList.contains(currentSymbol)) {
        m_minorIndex = m_minorList.indexOf(currentSymbol);
    } else if (m_selectionHistory.find(currentStyle) != m_selectionHistory.end()) {
        // check if current style present in m_selectionHistory
        QString previousSelectedSymbol = m_selectionHistory.value(currentStyle).at(2);
        m_minorIndex = m_minorList.indexOf(previousSelectedSymbol);
        globalContext()->currentNotation()->style()->setStyleValue(id, previousSelectedSymbol);
    } else {
        //set the default
        m_minorIndex = 0;
        globalContext()->currentNotation()->style()->setStyleValue(id, m_minorList[0]);
    }

    // Augmented
    id = Ms::Sid::chordQualityAugmented;
    currentSymbol = globalContext()->currentNotation()->style()->styleValue(id).toString();
    if (!chordStyleChanged && m_augmentedList.contains(currentSymbol)) {
        m_augmentedIndex = m_augmentedList.indexOf(currentSymbol);
    } else if (m_selectionHistory.find(currentStyle) != m_selectionHistory.end()) {
        // check if current style present in m_selectionHistory
        QString previousSelectedSymbol = m_selectionHistory.value(currentStyle).at(3);
        m_augmentedIndex = m_augmentedList.indexOf(previousSelectedSymbol);
        globalContext()->currentNotation()->style()->setStyleValue(id, previousSelectedSymbol);
    } else {
        //set the default
        m_augmentedIndex = 0;
        globalContext()->currentNotation()->style()->setStyleValue(id, m_augmentedList[0]);
    }

    // Diminished
    id = Ms::Sid::chordQualityDiminished;
    currentSymbol = globalContext()->currentNotation()->style()->styleValue(id).toString();
    if (!chordStyleChanged && m_diminishedList.contains(currentSymbol)) {
        m_diminishedIndex = m_diminishedList.indexOf(currentSymbol);
    } else if (m_selectionHistory.find(currentStyle) != m_selectionHistory.end()) {
        // check if current style present in m_selectionHistory
        QString previousSelectedSymbol = m_selectionHistory.value(currentStyle).at(4);
        m_diminishedIndex = m_diminishedList.indexOf(previousSelectedSymbol);
        globalContext()->currentNotation()->style()->setStyleValue(id, previousSelectedSymbol);
    } else {
        //set the default
        m_diminishedIndex = 0;
        globalContext()->currentNotation()->style()->setStyleValue(id, m_diminishedList[0]);
    }

    //omit
    id = Ms::Sid::chordModifierOmit;
    currentSymbol = globalContext()->currentNotation()->style()->styleValue(id).toString();
    if (!chordStyleChanged && m_omitList.contains(currentSymbol)) {
        m_omitIndex = m_omitList.indexOf(currentSymbol);
    } else if (m_selectionHistory.find(currentStyle) != m_selectionHistory.end()) {
        // check if current style present in m_selectionHistory
        QString previousSelectedSymbol = m_selectionHistory.value(currentStyle).at(5);
        m_omitIndex = m_omitList.indexOf(previousSelectedSymbol);
        globalContext()->currentNotation()->style()->setStyleValue(id, previousSelectedSymbol);
    } else {
        //set the default
        m_omitIndex = 0;
        globalContext()->currentNotation()->style()->setStyleValue(id, m_omitList[0]);
    }

    updateSelectionHistory(currentStyle);

    emit majorSeventhIndexChanged();
    emit halfDiminishedIndexChanged();
    emit minorIndexChanged();
    emit augmentedIndexChanged();
    emit diminishedIndexChanged();
    emit omitIndexChanged();
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
    m_omitList = m_qualitySymbols["omit"];

    // Get and extract the selection History
    QString selectionHistory = globalContext()->currentNotation()->style()->styleValue(Ms::Sid::chordQualitySelectionHistory).toString();
    extractSelectionHistory(selectionHistory);

    // Notify QML ListViews about the change
    emit chordSpellingListChanged();
    emit majorSeventhListChanged();
    emit halfDiminishedListChanged();
    emit minorListChanged();
    emit augmentedListChanged();
    emit diminishedListChanged();
    emit omitListChanged();
}

void ChordSymbolEditorModel::setQualitySymbol(QString quality, QString symbol)
{
    QHash<QString, Ms::Sid> qualityToSid = {
        { "major7th", Ms::Sid::chordQualityMajorSeventh },
        { "half-diminished", Ms::Sid::chordQualityHalfDiminished },
        { "minor", Ms::Sid::chordQualityMinor },
        { "augmented", Ms::Sid::chordQualityAugmented },
        { "diminished", Ms::Sid::chordQualityDiminished },
        { "omit", Ms::Sid::chordModifierOmit }
    };

    Ms::Sid id = qualityToSid.value(quality);
    globalContext()->currentNotation()->style()->setStyleValue(id, symbol);
    updateQualitySymbolsIndices(false);
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
    updateQualitySymbolsIndices(true);

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

void ChordSymbolEditorModel::setProperty(QString property, qreal val)
{
    if (property == "QualityMag") {
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityMag, val);
        m_qualityMag = val;
        emit qualityMagChanged();
    } else if (property == "QualityAdjust") {
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityAdjust, val);
        m_qualityAdjust = val;
        emit qualityAdjustChanged();
    } else if (property == "ExtensionMag") {
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordExtensionMag, val);
        m_extensionMag = val;
        emit extensionMagChanged();
    } else if (property == "ExtensionAdjust") {
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordExtensionAdjust, val);
        m_extensionAdjust = val;
        emit extensionAdjustChanged();
    } else if (property == "ModifierMag") {
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordModifierMag, val);
        m_modifierMag= val;
        emit modifierMagChanged();
    } else if (property == "ModifierAdjust") {
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordModifierAdjust, val);
        m_modifierAdjust = val;
        emit modifierAdjustChanged();
    } else if (property == "HarmonyFretDistance") {
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::harmonyFretDist, val);
        m_harmonyFretDistance = val;
        emit harmonyFretDistanceChanged();
    } else if (property == "minHarmonyDistance") {
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::minHarmonyDistance, val);
        m_minHarmonyDistance = val;
        emit minHarmonyDistanceChanged();
    } else if (property == "maxHarmonyBarDistance") {
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::maxHarmonyBarDistance, val);
        m_maxHarmonyBarDistance = val;
        emit maxHarmonyBarDistanceChanged();
    } else if (property == "maxChordShiftAbove") {
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::maxChordShiftAbove, val);
        m_maxChordShiftAbove = val;
        emit maxChordShiftAboveChanged();
    } else if (property == "maxChordShiftBelow") {
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::maxChordShiftBelow, val);
        m_maxChordShiftBelow = val;
        emit maxChordShiftBelowChanged();
    } else if (property == "capoPosition") {
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::capoPosition, val);
        m_capoFretPosition = val;
        emit capoFretPositionChanged();
    } else if (property == "stackModifiers") {
        bool stackMod;

        if (val == 1) {
            stackMod = true;
        } else {
            stackMod = false;
        }

        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::stackModifiers, stackMod);

        if (stackMod) {
            m_stackModifiersIndex = 0;
        } else {
            m_stackModifiersIndex = 1;
        }

        emit stackModifiersIndexChanged();
    } else if (property == "autoCapitalization") {
        bool autoCapital = (val == 1);
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::automaticCapitalization, autoCapital);
        m_autoCapitalization = val;
        emit autoCapitalizationChanged();
    } else if (property == "minorRootCapitalization") {
        bool minorRootCapital = (val == 1);
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::lowerCaseMinorChords, !minorRootCapital);
        m_minorRootCapitalization = val;
        emit minorRootCapitalizationChanged();
    } else if (property == "qualitySymbolsCapitalization") {
        bool qualitySymbolsCapital = (val == 1);
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::lowerCaseQualitySymbols, !qualitySymbolsCapital);
        m_qualitySymbolsCapitalization = val;
        emit qualitySymbolsCapitalizationChanged();
    } else if (property == "bassNotesCapitalization") {
        bool bassCapital = (val == 1);
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::lowerCaseBassNotes, !bassCapital);
        m_bassNotesCapitalization = val;
        emit bassNotesCapitalizationChanged();
    } else if (property == "solfegeNotesCapitalization") {
        bool solfegeCapital = (val == 1);
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::allCapsNoteNames, solfegeCapital);
        m_solfegeNotesCapitalization = val;
        emit solfegeNotesCapitalizationChanged();
    } else if (property == "alterationsParentheses") {
        bool alterationsParentheses = (val == 1);
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordAlterationsParentheses, alterationsParentheses);
        m_alterationsParentheses = val;
        emit alterationsParenthesesChanged();
    } else if (property == "suspensionsParentheses") {
        bool suspensionsParentheses = (val == 1);
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordSuspensionsParentheses, suspensionsParentheses);
        m_suspensionsParentheses = val;
        emit suspensionsParenthesesChanged();
    } else if (property == "minMajParentheses") {
        bool minMajParentheses = (val == 1);
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordMinMajParentheses, minMajParentheses);
        m_minMajParentheses = val;
        emit minMajParenthesesChanged();
    } else if (property == "addOmitParentheses") {
        bool addOmitParentheses = (val == 1);
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordAddOmitParentheses, addOmitParentheses);
        m_addOmitParentheses = val;
        emit addOmitParenthesesChanged();
    }
}

void ChordSymbolEditorModel::stringifyAndSaveSelectionHistory()
{
    QString selectionHist = "";
    QStringList selectionHistoryList;
    for (auto i = m_selectionHistory.begin(); i != m_selectionHistory.end(); i++) {
        QString currentSelection = i.value().join(",");
        selectionHistoryList << i.key() + "|" + currentSelection;
    }
    selectionHist = selectionHistoryList.join("\n");
    globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualitySelectionHistory, selectionHist);
}

void ChordSymbolEditorModel::extractSelectionHistory(QString selectionHistory)
{
    // The selection history is of the format
    // "StyleName1|maj7,half-dim,min,aug,dim,omit\nStyleName2...."
    m_selectionHistory.clear();
    QStringList selectionHistoryList = selectionHistory.split("\n");
    for (auto s: selectionHistoryList) {
        QStringList selectionHistoryOfStyle = s.split("|");
        m_selectionHistory.insert(selectionHistoryOfStyle[0], selectionHistoryOfStyle[1].split(","));
    }
}

void ChordSymbolEditorModel::updateSelectionHistory(QString currentStyle)
{
    m_selectionHistory.remove(currentStyle);
    QStringList qualitySymbolsList;
    qualitySymbolsList << m_majorSeventhList[m_majorSeventhIndex];
    qualitySymbolsList << m_halfDiminishedList[m_halfDiminishedIndex];
    qualitySymbolsList << m_minorList[m_minorIndex];
    qualitySymbolsList << m_augmentedList[m_augmentedIndex];
    qualitySymbolsList << m_diminishedList[m_diminishedIndex];
    qualitySymbolsList << m_omitList[m_omitIndex];
    m_selectionHistory.insert(currentStyle, qualitySymbolsList);
    stringifyAndSaveSelectionHistory();
}
