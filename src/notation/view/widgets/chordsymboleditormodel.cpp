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
    m_chordSpellingList << "Standard" << "German" << "German Full" << "Solfege" << "French";
    styleManager = new ChordSymbolStyleManager();
    m_styles = styleManager->getChordStyles();
    initCurrentStyleIndex();
    setQualitySymbolsLists();
    setQualitySymbolsOnStyleChange();
    setPropertiesOnStyleChange();
    updateSelectionHistory(m_styles[m_currentStyleIndex].styleName);
}

int ChordSymbolEditorModel::rowCount(const QModelIndex&) const
{
    return m_styles.count();
}

QHash<int, QByteArray> ChordSymbolEditorModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        { StyleNameRole, "styleName" },
        { FileRole, "fileName" },
        { UsePresetsRole, "usePresets" }
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
    case UsePresetsRole:
        return chordSymbolStyle.usePresets;
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
    QStringList qualitySymbolsList;
    for (auto qS: m_majorSeventhList) {
        if (chordSymbolStyles.contains(m_styles.at(m_currentStyleIndex).styleName)) {
            if (qS.qualMag != -1.0 && qS.qualMag != 1.0) {
                qualitySymbolsList << "s" + qS.qualitySymbol;
                continue;
            }
        }
        qualitySymbolsList << qS.qualitySymbol;
    }
    return qualitySymbolsList;
}

QStringList ChordSymbolEditorModel::halfDiminishedList() const
{
    QStringList qualitySymbolsList;
    for (auto qS: m_halfDiminishedList) {
        if (chordSymbolStyles.contains(m_styles.at(m_currentStyleIndex).styleName)) {
            if (qS.qualMag != -1.0 && qS.qualMag != 1.0) {
                qualitySymbolsList << "s" + qS.qualitySymbol;
                continue;
            }
        }
        qualitySymbolsList << qS.qualitySymbol;
    }
    return qualitySymbolsList;
}

QStringList ChordSymbolEditorModel::minorList() const
{
    QStringList qualitySymbolsList;
    for (auto qS: m_minorList) {
        if (chordSymbolStyles.contains(m_styles.at(m_currentStyleIndex).styleName)) {
            if (qS.qualMag != -1.0 && qS.qualMag != 1.0) {
                qualitySymbolsList << "s" + qS.qualitySymbol;
                continue;
            }
        }
        qualitySymbolsList << qS.qualitySymbol;
    }
    return qualitySymbolsList;
}

QStringList ChordSymbolEditorModel::augmentedList() const
{
    QStringList qualitySymbolsList;
    for (auto qS: m_augmentedList) {
        if (chordSymbolStyles.contains(m_styles.at(m_currentStyleIndex).styleName)) {
            if (qS.qualMag != -1.0 && qS.qualMag != 1.0) {
                qualitySymbolsList << "s" + qS.qualitySymbol;
                continue;
            }
        }
        qualitySymbolsList << qS.qualitySymbol;
    }
    return qualitySymbolsList;
}

QStringList ChordSymbolEditorModel::diminishedList() const
{
    QStringList qualitySymbolsList;
    for (auto qS: m_diminishedList) {
        if (chordSymbolStyles.contains(m_styles.at(m_currentStyleIndex).styleName)) {
            if (qS.qualMag != -1.0 && qS.qualMag != 1.0) {
                qualitySymbolsList << "s" + qS.qualitySymbol;
                continue;
            }
        }
        qualitySymbolsList << qS.qualitySymbol;
    }
    return qualitySymbolsList;
}

QStringList ChordSymbolEditorModel::sixNineList() const
{
    QStringList qualitySymbolsList;
    for (auto qS: m_sixNineList) {
        if (chordSymbolStyles.contains(m_styles.at(m_currentStyleIndex).styleName)) {
            if (qS.qualMag != -1.0 && qS.qualMag != 1.0) {
                qualitySymbolsList << "s" + qS.qualitySymbol;
                continue;
            }
        }
        qualitySymbolsList << qS.qualitySymbol;
    }
    return qualitySymbolsList;
}

QStringList ChordSymbolEditorModel::omitList() const
{
    QStringList qualitySymbolsList;
    for (auto qS: m_omitList) {
        if (chordSymbolStyles.contains(m_styles.at(m_currentStyleIndex).styleName)) {
            if (qS.qualMag != -1.0 && qS.qualMag != 1.0) {
                qualitySymbolsList << "s" + qS.qualitySymbol;
                continue;
            }
        }
        qualitySymbolsList << qS.qualitySymbol;
    }
    return qualitySymbolsList;
}

QStringList ChordSymbolEditorModel::suspensionList() const
{
    QStringList qualitySymbolsList;
    for (auto qS: m_suspensionList) {
        if (chordSymbolStyles.contains(m_styles.at(m_currentStyleIndex).styleName)) {
            if (qS.qualMag != -1.0 && qS.qualMag != 1.0) {
                qualitySymbolsList << "s" + qS.qualitySymbol;
                continue;
            }
        }
        qualitySymbolsList << qS.qualitySymbol;
    }
    return qualitySymbolsList;
}

QStringList ChordSymbolEditorModel::bassNoteList() const
{
    QStringList qualitySymbolsList;
    for (auto qS: m_bassNoteList) {
        if (chordSymbolStyles.contains(m_styles.at(m_currentStyleIndex).styleName)) {
            if (qS.qualMag != -1.0 && qS.qualMag != 1.0) {
                qualitySymbolsList << "s" + qS.qualitySymbol;
                continue;
            }
        }
        qualitySymbolsList << qS.qualitySymbol;
    }
    return qualitySymbolsList;
}

int ChordSymbolEditorModel::chordSpellingIndex() const
{
    return m_chordSpellingIndex;
}

int ChordSymbolEditorModel::currentStyleIndex() const
{
    return m_currentStyleIndex;
}

QString ChordSymbolEditorModel::styleDescription() const
{
    return m_styleDescription;
}

bool ChordSymbolEditorModel::usePresets() const
{
    return m_usePresets;
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

int ChordSymbolEditorModel::sixNineIndex() const
{
    return m_sixNineIndex;
}

int ChordSymbolEditorModel::omitIndex() const
{
    return m_omitIndex;
}

int ChordSymbolEditorModel::suspensionIndex() const
{
    return m_suspensionIndex;
}

int ChordSymbolEditorModel::bassNoteIndex() const
{
    return m_bassNoteIndex;
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

qreal ChordSymbolEditorModel::stackModifiers() const
{
    return m_stackModifiers;
}

qreal ChordSymbolEditorModel::autoCapitalization() const
{
    return m_autoCapitalization;
}

qreal ChordSymbolEditorModel::minorRootCapitalization() const
{
    return m_minorRootCapitalization;
}

qreal ChordSymbolEditorModel::qualityMajorCapitalization() const
{
    return m_qualityMajorCapitalization;
}

qreal ChordSymbolEditorModel::qualityMinorCapitalization() const
{
    return m_qualityMinorCapitalization;
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

qreal ChordSymbolEditorModel::chordSymbolScaling() const
{
    return m_chordSymbolScaling;
}

void ChordSymbolEditorModel::setStyle(Ms::Sid id, QVariant val)
{
    globalContext()->currentNotation()->style()->setStyleValue(id, val);
}

qreal ChordSymbolEditorModel::getDefValR(Ms::Sid id)
{
    return globalContext()->currentNotation()->style()->defaultStyleValue(id).toReal();
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
        setChordStyle(0);
    }

    globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::useChordSymbolPresets, m_styles.at(m_currentStyleIndex).usePresets);
    m_styleDescription = m_styles.at(m_currentStyleIndex).description;
    m_usePresets = m_styles.at(m_currentStyleIndex).usePresets;

    // Get and extract the selection History
    QString selectionHistory = globalContext()->currentNotation()->style()->styleValue(Ms::Sid::chordQualitySelectionHistory).toString();
    extractSelectionHistory(selectionHistory);

    emit currentStyleIndexChanged();
    emit styleDescriptionChanged();
    emit usePresetsChanged();
}

void ChordSymbolEditorModel::setQualitySymbolsOnStyleChange()
{
    if (!m_styles[m_currentStyleIndex].usePresets) {
        return;
    }
    // Do not worry about the quality, extension and modifier settings here
    QString currentStyle = m_styles[m_currentStyleIndex].styleName;

    if (m_selectionHistory.find(currentStyle) != m_selectionHistory.end()) {
        // Major 7th
        m_majorSeventhIndex = m_selectionHistory.value(currentStyle).value("maj7th").toInt();
        if (m_majorSeventhIndex != -1) {
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityMajorSeventh,
                                                                       m_majorSeventhList.at(m_majorSeventhIndex).qualitySymbol);
        } else {
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityMajorSeventh, "-1");
        }

        // Half-Diminished
        m_halfDiminishedIndex = m_selectionHistory.value(currentStyle).value("half-dim").toInt();
        if (m_halfDiminishedIndex != -1) {
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityHalfDiminished,
                                                                       m_halfDiminishedList.at(m_halfDiminishedIndex).qualitySymbol);
        } else {
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityHalfDiminished, "-1");
        }

        // Minor
        m_minorIndex= m_selectionHistory.value(currentStyle).value("min").toInt();
        if (m_minorIndex != -1) {
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityMinor, m_minorList.at(
                                                                           m_minorIndex).qualitySymbol);
        } else {
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityMinor, "-1");
        }

        // Augmented
        m_augmentedIndex = m_selectionHistory.value(currentStyle).value("aug").toInt();
        if (m_augmentedIndex != -1) {
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityAugmented, m_augmentedList.at(
                                                                           m_augmentedIndex).qualitySymbol);
        } else {
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityAugmented, "-1");
        }

        // Diminished
        m_diminishedIndex = m_selectionHistory.value(currentStyle).value("dim").toInt();
        if (m_diminishedIndex != -1) {
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityDiminished, m_diminishedList.at(
                                                                           m_diminishedIndex).qualitySymbol);
        } else {
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityDiminished, "-1");
        }

        // SixNine
        m_sixNineIndex = m_selectionHistory.value(currentStyle).value("sixNine").toInt();
        if (m_sixNineIndex != -1) {
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordExtensionSixNine, m_sixNineList.at(
                                                                           m_sixNineIndex).qualitySymbol);
        } else {
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordExtensionSixNine, "-1");
        }

        // Omit
        m_omitIndex = m_selectionHistory.value(currentStyle).value("omit").toInt();
        if (m_omitIndex != -1) {
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordModifierOmit,
                                                                       m_omitList.at(m_omitIndex).qualitySymbol);
        } else {
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordModifierOmit, "-1");
        }

        // Suspension
        m_suspensionIndex = m_selectionHistory.value(currentStyle).value("sus").toInt();
        if (m_suspensionIndex != -1) {
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordModifierSuspension,
                                                                       m_suspensionList.at(m_suspensionIndex).qualitySymbol);
        } else {
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordModifierSuspension, "-1");
        }

        // Bass Note
        m_bassNoteIndex = m_selectionHistory.value(currentStyle).value("bass").toInt();
        if (m_bassNoteIndex != -1) {
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordBassNote,
                                                                       m_bassNoteList.at(m_bassNoteIndex).qualitySymbol);
        } else {
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordBassNote, "-1");
        }
    } else {
        // Set the default values
        if (m_majorSeventhList.size() == 0) {
            m_majorSeventhIndex = -1;
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityMajorSeventh, "-1");
        } else {
            m_majorSeventhIndex = 0;
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityMajorSeventh,
                                                                       m_majorSeventhList[0].qualitySymbol);
        }

        // Half-Diminished
        if (m_halfDiminishedList.size() == 0) {
            m_halfDiminishedIndex = -1;
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityHalfDiminished, "-1");
        } else {
            m_halfDiminishedIndex = 0;
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityHalfDiminished,
                                                                       m_halfDiminishedList[0].qualitySymbol);
        }

        // Minor
        if (m_minorList.size() == 0) {
            m_minorIndex = -1;
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityMinor, "-1");
        } else {
            m_minorIndex = 0;
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityMinor, m_minorList[0].qualitySymbol);
        }

        // Augmented
        if (m_augmentedList.size() == 0) {
            m_augmentedIndex = -1;
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityAugmented, "-1");
        } else {
            m_augmentedIndex = 0;
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityAugmented, m_augmentedList[0].qualitySymbol);
        }

        // Diminished
        if (m_diminishedList.size() == 0) {
            m_diminishedIndex = -1;
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityDiminished, "-1");
        } else {
            m_diminishedIndex = 0;
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityDiminished, m_diminishedList[0].qualitySymbol);
        }

        // SixNine
        if (m_sixNineList.size() == 0) {
            m_sixNineIndex = -1;
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordExtensionSixNine, "-1");
        } else {
            m_sixNineIndex = 0;
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordExtensionSixNine, m_sixNineList[0].qualitySymbol);
        }

        // Omit
        if (m_omitList.size() == 0) {
            m_omitIndex = -1;
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordModifierOmit, "-1");
        } else {
            m_omitIndex = 0;
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordModifierOmit, m_omitList[0].qualitySymbol);
        }
        // Suspension
        if (m_suspensionList.size() == 0) {
            m_suspensionIndex = -1;
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordModifierSuspension, "-1");
        } else {
            m_suspensionIndex = 0;
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordModifierSuspension, m_suspensionList[0].qualitySymbol);
        }
        // Bass Note
        if (m_bassNoteList.size() == 0) {
            m_bassNoteIndex = -1;
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordBassNote, "-1");
        } else {
            m_bassNoteIndex = 0;
            globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordBassNote, m_bassNoteList[0].qualitySymbol);
        }
    }
    emit majorSeventhIndexChanged();
    emit halfDiminishedIndexChanged();
    emit minorIndexChanged();
    emit augmentedIndexChanged();
    emit diminishedIndexChanged();
    emit sixNineIndexChanged();
    emit omitIndexChanged();
    emit suspensionIndexChanged();
    emit bassNoteIndexChanged();
}

void ChordSymbolEditorModel::setPropertiesOnStyleChange()
{
    QString currentStyle = m_styles[m_currentStyleIndex].styleName;

    if (m_selectionHistory.find(currentStyle) != m_selectionHistory.end()) {
        m_chordSpellingIndex = m_selectionHistory.value(currentStyle).value("chrdSpell").toInt();

        m_qualityMag = m_selectionHistory.value(currentStyle).value("qualMag").toReal();
        m_qualityAdjust = m_selectionHistory.value(currentStyle).value("qualAdj").toReal();
        m_extensionMag = m_selectionHistory.value(currentStyle).value("extMag").toReal();
        m_extensionAdjust = m_selectionHistory.value(currentStyle).value("extAdj").toReal();
        m_modifierMag = m_selectionHistory.value(currentStyle).value("modMag").toReal();
        m_modifierAdjust = m_selectionHistory.value(currentStyle).value("modAdj").toReal();

        m_harmonyFretDistance = m_selectionHistory.value(currentStyle).value("hFretDist").toReal();
        m_minHarmonyDistance = m_selectionHistory.value(currentStyle).value("mnHDist").toReal();
        m_maxHarmonyBarDistance = m_selectionHistory.value(currentStyle).value("mxHBarDist").toReal();
        m_maxChordShiftAbove = m_selectionHistory.value(currentStyle).value("mxSftAbv").toReal();
        m_maxChordShiftBelow = m_selectionHistory.value(currentStyle).value("mxSftBlw").toReal();
        m_capoFretPosition = m_selectionHistory.value(currentStyle).value("cpFretPos").toReal();

        m_stackModifiers = m_selectionHistory.value(currentStyle).value("stkMod").toReal();

        m_autoCapitalization = m_selectionHistory.value(currentStyle).value("autoCap").toReal();
        m_minorRootCapitalization = m_selectionHistory.value(currentStyle).value("minRtCap").toReal();
        m_qualityMajorCapitalization = m_selectionHistory.value(currentStyle).value("qualMajCap").toReal();
        m_qualityMinorCapitalization = m_selectionHistory.value(currentStyle).value("qualMinCap").toReal();
        m_bassNotesCapitalization = m_selectionHistory.value(currentStyle).value("bsNtCap").toReal();
        m_solfegeNotesCapitalization = m_selectionHistory.value(currentStyle).value("solNtCap").toReal();

        m_alterationsParentheses = m_selectionHistory.value(currentStyle).value("altParen").toReal();
        m_suspensionsParentheses = m_selectionHistory.value(currentStyle).value("susParen").toReal();
        m_minMajParentheses = m_selectionHistory.value(currentStyle).value("minMajParen").toReal();
        m_addOmitParentheses = m_selectionHistory.value(currentStyle).value("addOmitParen").toReal();

        m_chordSymbolScaling = m_selectionHistory.value(currentStyle).value("chordSymbolScaling").toReal();
    } else {
        // Set the defaults
        m_chordSpellingIndex = 0; // Standard

        m_qualityMag = getDefValR(Ms::Sid::chordQualityMag);
        m_qualityAdjust = getDefValR(Ms::Sid::chordQualityAdjust);
        m_extensionMag = getDefValR(Ms::Sid::chordExtensionMag);
        m_extensionAdjust = getDefValR(Ms::Sid::chordExtensionAdjust);
        m_modifierMag = getDefValR(Ms::Sid::chordModifierMag);
        m_modifierAdjust = getDefValR(Ms::Sid::chordModifierAdjust);

        m_harmonyFretDistance = getDefValR(Ms::Sid::harmonyFretDist);
        m_minHarmonyDistance = getDefValR(Ms::Sid::minHarmonyDistance);
        m_maxHarmonyBarDistance = getDefValR(Ms::Sid::maxHarmonyBarDistance);
        m_maxChordShiftAbove = getDefValR(Ms::Sid::maxChordShiftAbove);
        m_maxChordShiftBelow = getDefValR(Ms::Sid::maxChordShiftBelow);
        m_capoFretPosition = getDefValR(Ms::Sid::capoPosition);

        m_stackModifiers = getDefValR(Ms::Sid::stackModifiers);

        m_autoCapitalization = getDefValR(Ms::Sid::automaticCapitalization);
        m_minorRootCapitalization = !getDefValR(Ms::Sid::lowerCaseMinorChords);
        m_qualityMajorCapitalization = !getDefValR(Ms::Sid::lowerCaseMajorSymbols);
        m_qualityMinorCapitalization = !getDefValR(Ms::Sid::lowerCaseMinorChords);
        m_bassNotesCapitalization = !getDefValR(Ms::Sid::lowerCaseBassNotes);
        m_solfegeNotesCapitalization = getDefValR(Ms::Sid::allCapsNoteNames);

        m_alterationsParentheses = getDefValR(Ms::Sid::chordAlterationsParentheses);
        m_suspensionsParentheses = getDefValR(Ms::Sid::chordSuspensionsParentheses);
        m_minMajParentheses = getDefValR(Ms::Sid::chordMinMajParentheses);
        m_addOmitParentheses = getDefValR(Ms::Sid::chordAddOmitParentheses);

        m_chordSymbolScaling = 100;
    }
    setChordSpelling(m_chordSpellingList[m_chordSpellingIndex]);

    setStyle(Ms::Sid::chordQualityMag, m_qualityMag);
    setStyle(Ms::Sid::chordQualityAdjust, m_qualityAdjust);
    setStyle(Ms::Sid::chordExtensionMag, m_extensionMag);
    setStyle(Ms::Sid::chordExtensionAdjust, m_extensionAdjust);
    setStyle(Ms::Sid::chordModifierMag, m_modifierMag);
    setStyle(Ms::Sid::chordModifierAdjust, m_modifierAdjust);

    setStyle(Ms::Sid::harmonyFretDist, m_harmonyFretDistance);
    setStyle(Ms::Sid::minHarmonyDistance, m_minHarmonyDistance);
    setStyle(Ms::Sid::maxHarmonyBarDistance, m_maxHarmonyBarDistance);
    setStyle(Ms::Sid::maxChordShiftAbove, m_maxChordShiftAbove);
    setStyle(Ms::Sid::maxChordShiftBelow, m_maxChordShiftBelow);
    setStyle(Ms::Sid::capoPosition, m_capoFretPosition);

    setStyle(Ms::Sid::stackModifiers, (m_stackModifiers == 1));

    setStyle(Ms::Sid::automaticCapitalization, (m_autoCapitalization == 1));
    setStyle(Ms::Sid::lowerCaseMinorChords, !(m_minorRootCapitalization == 1));
    setStyle(Ms::Sid::lowerCaseMajorSymbols, !(m_qualityMajorCapitalization == 1));
    setStyle(Ms::Sid::lowerCaseMinorSymbols, !(m_qualityMinorCapitalization == 1));
    setStyle(Ms::Sid::lowerCaseBassNotes, !(m_bassNotesCapitalization == 1));
    setStyle(Ms::Sid::allCapsNoteNames, (m_solfegeNotesCapitalization == 1));

    setStyle(Ms::Sid::chordAlterationsParentheses, (m_alterationsParentheses == 1));
    setStyle(Ms::Sid::chordSuspensionsParentheses, (m_suspensionsParentheses == 1));
    setStyle(Ms::Sid::chordMinMajParentheses, (m_minMajParentheses == 1));
    setStyle(Ms::Sid::chordAddOmitParentheses, (m_addOmitParentheses == 1));

    int defaultSize = globalContext()->currentNotation()->style()->defaultStyleValue(Ms::Sid::chordSymbolAFontSize).toInt();
    setStyle(Ms::Sid::chordSymbolAFontSize, (m_chordSymbolScaling * defaultSize) / 100);

    emit chordSpellingIndexChanged();

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
    emit capoFretPositionChanged();

    emit stackModifiersChanged();

    emit autoCapitalizationChanged();
    emit minorRootCapitalizationChanged();
    emit qualityMajorCapitalizationChanged();
    emit qualityMinorCapitalizationChanged();
    emit bassNotesCapitalizationChanged();
    emit solfegeNotesCapitalizationChanged();

    emit alterationsParenthesesChanged();
    emit suspensionsParenthesesChanged();
    emit minMajParenthesesChanged();
    emit addOmitParenthesesChanged();

    emit chordSymbolScalingChanged();
}

void ChordSymbolEditorModel::setQualitySymbolsLists()
{
    if (m_styles.at(m_currentStyleIndex).usePresets) {
        m_qualitySymbols = styleManager->getQualitySymbols(m_styles.at(m_currentStyleIndex).fileName);
    } else {
        QString descFileWithPresets
            = globalContext()->currentNotation()->style()->styleValue(Ms::Sid::chordDescriptionFileWithPresets).toString();
        m_qualitySymbols = styleManager->getQualitySymbols(descFileWithPresets);
    }

    m_majorSeventhList = m_qualitySymbols["major7th"];
    m_halfDiminishedList = m_qualitySymbols["half-diminished"];
    m_minorList = m_qualitySymbols["minor"];
    m_augmentedList = m_qualitySymbols["augmented"];
    m_diminishedList = m_qualitySymbols["diminished"];
    m_sixNineList = m_qualitySymbols["sixNine"];
    m_omitList = m_qualitySymbols["omit"];
    m_suspensionList = m_qualitySymbols["suspension"];
    m_bassNoteList = m_qualitySymbols["bassNote"];

    // Notify QML ListViews about the change

    emit majorSeventhListChanged();
    emit halfDiminishedListChanged();
    emit minorListChanged();
    emit augmentedListChanged();
    emit diminishedListChanged();
    emit sixNineListChanged();
    emit omitListChanged();
    emit suspensionListChanged();
    emit bassNoteListChanged();
}

void ChordSymbolEditorModel::setPropertiesOfQualitySymbol(QualitySymbol qS)
{
    if (qS.qualMag != -1) {
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityMag, qS.qualMag);
        m_qualityMag = qS.qualMag;
        emit qualityMagChanged();
    }

    if (qS.qualAdjust != 999) {
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityAdjust, qS.qualAdjust);
        m_qualityAdjust = qS.qualAdjust;
        emit qualityAdjustChanged();
    }

    if (qS.extMag != -1) {
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordExtensionMag, qS.extMag);
        m_extensionMag = qS.extMag;
        emit extensionMagChanged();
    }

    if (qS.extAdjust != 999) {
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordExtensionAdjust, qS.extAdjust);
        m_extensionAdjust = qS.extAdjust;
        emit extensionAdjustChanged();
    }

    if (qS.modMag != -1) {
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordModifierMag, qS.modMag);
        m_modifierMag= qS.modMag;
        emit modifierMagChanged();
    }

    if (qS.modAdjust != 999) {
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordModifierAdjust, qS.modAdjust);
        m_modifierAdjust = qS.modAdjust;
        emit modifierAdjustChanged();
    }
}

void ChordSymbolEditorModel::setQualitySymbol(QString quality, int index)
{
    if (quality == "major7th") {
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityMajorSeventh,
                                                                   m_majorSeventhList.at(index).qualitySymbol);
        m_majorSeventhIndex = index;
        setPropertiesOfQualitySymbol(m_majorSeventhList.at(index));
        emit majorSeventhIndexChanged();
    } else if (quality == "half-diminished") {
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityHalfDiminished,
                                                                   m_halfDiminishedList.at(index).qualitySymbol);
        m_halfDiminishedIndex = index;
        setPropertiesOfQualitySymbol(m_halfDiminishedList.at(index));
        emit halfDiminishedIndexChanged();
    } else if (quality == "minor") {
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityMinor, m_minorList.at(index).qualitySymbol);
        m_minorIndex = index;
        setPropertiesOfQualitySymbol(m_minorList.at(index));
        emit minorIndexChanged();
    } else if (quality == "augmented") {
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityAugmented, m_augmentedList.at(index).qualitySymbol);
        m_augmentedIndex = index;
        setPropertiesOfQualitySymbol(m_augmentedList.at(index));
        emit augmentedIndexChanged();
    } else if (quality == "diminished") {
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualityDiminished,
                                                                   m_diminishedList.at(index).qualitySymbol);
        m_diminishedIndex = index;
        setPropertiesOfQualitySymbol(m_diminishedList.at(index));
        emit diminishedIndexChanged();
    } else if (quality == "sixNine") {
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordExtensionSixNine, m_sixNineList.at(index).qualitySymbol);
        m_sixNineIndex = index;
        setPropertiesOfQualitySymbol(m_sixNineList.at(index));
        emit sixNineIndexChanged();
    } else if (quality == "omit") {
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordModifierOmit, m_omitList.at(index).qualitySymbol);
        m_omitIndex = index;
        setPropertiesOfQualitySymbol(m_omitList.at(index));
        emit omitIndexChanged();
    } else if (quality == "suspension") {
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordModifierSuspension, m_suspensionList.at(
                                                                       index).qualitySymbol);
        m_suspensionIndex = index;
        setPropertiesOfQualitySymbol(m_suspensionList.at(index));
        emit suspensionIndexChanged();
    } else if (quality == "bassNote") {
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordBassNote, m_bassNoteList.at(index).qualitySymbol);
        m_bassNoteIndex = index;
        setPropertiesOfQualitySymbol(m_bassNoteList.at(index));
        emit bassNoteIndexChanged();
    }
    updateSelectionHistory(m_styles[m_currentStyleIndex].styleName);
}

void ChordSymbolEditorModel::setChordStyle(int index)
{
    m_currentStyleIndex = index;
    QStringList chordStyles = { "Pop/Contemporary", "Jazz", "Symbols", "No preset style" };

    globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordStyle, m_styles.at(m_currentStyleIndex).styleName);
    globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordDescriptionFile, m_styles.at(m_currentStyleIndex).fileName);
    globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::useChordSymbolPresets, m_styles.at(m_currentStyleIndex).usePresets);
    if (!chordStyles.contains(m_styles.at(m_currentStyleIndex).styleName)) {
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordStyle, "custom");
    }
    globalContext()->currentNotation()->style()->chordSymbolStyleChanged().notify();
    m_styleDescription = m_styles.at(m_currentStyleIndex).description;
    m_usePresets = m_styles.at(m_currentStyleIndex).usePresets;

    if (m_styles.at(m_currentStyleIndex).usePresets) {
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordDescriptionFileWithPresets, m_styles.at(
                                                                       m_currentStyleIndex).fileName);
    }

    // Get and extract the selection History
    QString selectionHistory = globalContext()->currentNotation()->style()->styleValue(Ms::Sid::chordQualitySelectionHistory).toString();
    extractSelectionHistory(selectionHistory);

    setQualitySymbolsLists();
    setQualitySymbolsOnStyleChange();
    setPropertiesOnStyleChange();
    updateSelectionHistory(m_styles[m_currentStyleIndex].styleName);

    emit currentStyleIndexChanged();
    emit styleDescriptionChanged();
    emit usePresetsChanged();
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

    m_chordSpellingIndex = m_chordSpellingList.indexOf(newSpelling);
    updateSelectionHistory(m_styles[m_currentStyleIndex].styleName);
    emit chordSpellingIndexChanged();
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
        bool stackMod = (val == 1);
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::stackModifiers, stackMod);
        m_stackModifiers = val;
        emit stackModifiersChanged();
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
    } else if (property == "qualityMajorCapitalization") {
        bool qualityMajorCapital = (val == 1);
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::lowerCaseMajorSymbols, !qualityMajorCapital);
        m_qualityMajorCapitalization = val;
        emit qualityMajorCapitalizationChanged();
    } else if (property == "qualityMinorCapitalization") {
        bool qualityMinorCapital = (val == 1);
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::lowerCaseMinorSymbols, !qualityMinorCapital);
        m_qualityMinorCapitalization = val;
        emit qualityMinorCapitalizationChanged();
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
    } else if (property == "chordSymbolScaling") {
        // TODO: What about chordSymbolBFontSize???
        int defaultSize = globalContext()->currentNotation()->style()->defaultStyleValue(Ms::Sid::chordSymbolAFontSize).toInt();
        globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordSymbolAFontSize, (defaultSize * val) / 100);
        m_chordSymbolScaling = val;
        emit chordSymbolScalingChanged();
    }
    updateSelectionHistory(m_styles[m_currentStyleIndex].styleName);
}

void ChordSymbolEditorModel::resetProperties()
{
    // Set the defaults

    m_qualityMag = getDefValR(Ms::Sid::chordQualityMag);
    m_qualityAdjust = getDefValR(Ms::Sid::chordQualityAdjust);
    m_extensionMag = getDefValR(Ms::Sid::chordExtensionMag);
    m_extensionAdjust = getDefValR(Ms::Sid::chordExtensionAdjust);
    m_modifierMag = getDefValR(Ms::Sid::chordModifierMag);
    m_modifierAdjust = getDefValR(Ms::Sid::chordModifierAdjust);

    m_harmonyFretDistance = getDefValR(Ms::Sid::harmonyFretDist);
    m_minHarmonyDistance = getDefValR(Ms::Sid::minHarmonyDistance);
    m_maxHarmonyBarDistance = getDefValR(Ms::Sid::maxHarmonyBarDistance);
    m_maxChordShiftAbove = getDefValR(Ms::Sid::maxChordShiftAbove);
    m_maxChordShiftBelow = getDefValR(Ms::Sid::maxChordShiftBelow);
    m_capoFretPosition = getDefValR(Ms::Sid::capoPosition);

    m_stackModifiers = getDefValR(Ms::Sid::stackModifiers);

    m_autoCapitalization = getDefValR(Ms::Sid::automaticCapitalization);
    m_minorRootCapitalization = !getDefValR(Ms::Sid::lowerCaseMinorChords);
    m_qualityMajorCapitalization = !getDefValR(Ms::Sid::lowerCaseMajorSymbols);
    m_qualityMinorCapitalization = !getDefValR(Ms::Sid::lowerCaseMinorChords);
    m_bassNotesCapitalization = !getDefValR(Ms::Sid::lowerCaseBassNotes);
    m_solfegeNotesCapitalization = getDefValR(Ms::Sid::allCapsNoteNames);

    m_alterationsParentheses = getDefValR(Ms::Sid::chordAlterationsParentheses);
    m_suspensionsParentheses = getDefValR(Ms::Sid::chordSuspensionsParentheses);
    m_minMajParentheses = getDefValR(Ms::Sid::chordMinMajParentheses);
    m_addOmitParentheses = getDefValR(Ms::Sid::chordAddOmitParentheses);

    m_chordSymbolScaling = 100;

    setStyle(Ms::Sid::chordQualityMag, m_qualityMag);
    setStyle(Ms::Sid::chordQualityAdjust, m_qualityAdjust);
    setStyle(Ms::Sid::chordExtensionMag, m_extensionMag);
    setStyle(Ms::Sid::chordExtensionAdjust, m_extensionAdjust);
    setStyle(Ms::Sid::chordModifierMag, m_modifierMag);
    setStyle(Ms::Sid::chordModifierAdjust, m_modifierAdjust);

    setStyle(Ms::Sid::harmonyFretDist, m_harmonyFretDistance);
    setStyle(Ms::Sid::minHarmonyDistance, m_minHarmonyDistance);
    setStyle(Ms::Sid::maxHarmonyBarDistance, m_maxHarmonyBarDistance);
    setStyle(Ms::Sid::maxChordShiftAbove, m_maxChordShiftAbove);
    setStyle(Ms::Sid::maxChordShiftBelow, m_maxChordShiftBelow);
    setStyle(Ms::Sid::capoPosition, m_capoFretPosition);

    setStyle(Ms::Sid::stackModifiers, (m_stackModifiers == 1));

    setStyle(Ms::Sid::automaticCapitalization, (m_autoCapitalization == 1));
    setStyle(Ms::Sid::lowerCaseMinorChords, !(m_minorRootCapitalization == 1));
    setStyle(Ms::Sid::lowerCaseMajorSymbols, !(m_qualityMajorCapitalization == 1));
    setStyle(Ms::Sid::lowerCaseMinorSymbols, !(m_qualityMinorCapitalization == 1));
    setStyle(Ms::Sid::lowerCaseBassNotes, !(m_bassNotesCapitalization == 1));
    setStyle(Ms::Sid::allCapsNoteNames, (m_solfegeNotesCapitalization == 1));

    setStyle(Ms::Sid::chordAlterationsParentheses, (m_alterationsParentheses == 1));
    setStyle(Ms::Sid::chordSuspensionsParentheses, (m_suspensionsParentheses == 1));
    setStyle(Ms::Sid::chordMinMajParentheses, (m_minMajParentheses == 1));
    setStyle(Ms::Sid::chordAddOmitParentheses, (m_addOmitParentheses == 1));

    int defaultSize = globalContext()->currentNotation()->style()->defaultStyleValue(Ms::Sid::chordSymbolAFontSize).toInt();
    setStyle(Ms::Sid::chordSymbolAFontSize, (m_chordSymbolScaling * defaultSize) / 100);

    updateSelectionHistory(m_styles[m_currentStyleIndex].styleName);

    emit chordSpellingIndexChanged();

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
    emit capoFretPositionChanged();

    emit stackModifiersChanged();

    emit autoCapitalizationChanged();
    emit minorRootCapitalizationChanged();
    emit qualityMajorCapitalizationChanged();
    emit qualityMinorCapitalizationChanged();
    emit bassNotesCapitalizationChanged();
    emit solfegeNotesCapitalizationChanged();

    emit alterationsParenthesesChanged();
    emit suspensionsParenthesesChanged();
    emit minMajParenthesesChanged();
    emit addOmitParenthesesChanged();

    emit chordSymbolScalingChanged();
}

void ChordSymbolEditorModel::stringifyAndSaveSelectionHistory()
{
    QString selectionHist = "";
    QStringList selectionHistoryList;
    for (auto i = m_selectionHistory.begin(); i != m_selectionHistory.end(); i++) {
        QStringList propStringList = {};
        for (auto j = i.value().begin(); j != i.value().end(); j++) {
            propStringList += j.key() + ":" + j.value().toString();
        }
        selectionHistoryList << i.key() + "|" + propStringList.join(";");
    }
    selectionHist = selectionHistoryList.join("\n");
    globalContext()->currentNotation()->style()->setStyleValue(Ms::Sid::chordQualitySelectionHistory, selectionHist);
}

void ChordSymbolEditorModel::extractSelectionHistory(QString selectionHistory)
{
    if (selectionHistory == "") {
        return;
    }
    // The selection history is of the format
    // "StyleName1|maj7th:xx,half-dim:xx,min:xx,aug:xx,dim:xx,omit:xx,.....\nStyleName2...."
    m_selectionHistory.clear();
    QStringList selectionHistoryList = selectionHistory.split("\n");
    for (auto style: selectionHistoryList) {
        QStringList selectionHistoryOfStyle = style.split("|"); // { styleName, comma-separated properties }
        QStringList properties = selectionHistoryOfStyle[1].split(";");
        QHash<QString, QVariant> propMap;
        for (auto prop: properties) {
            QStringList keyValue = prop.split(":");
            propMap.insert(keyValue[0], keyValue[1]);
        }
        m_selectionHistory.insert(selectionHistoryOfStyle[0], propMap);
    }
}

void ChordSymbolEditorModel::updateSelectionHistory(QString currentStyle)
{
    m_selectionHistory.remove(currentStyle);
    QHash<QString, QVariant> propMap;
    // Chord Symbols
    if (m_styles[m_currentStyleIndex].usePresets) {
        if (m_majorSeventhIndex == -1 || m_majorSeventhList.size() == 0) {
            propMap.insert("maj7th", QVariant("-1"));
        } else {
            propMap.insert("maj7th", QVariant(m_majorSeventhIndex));
        }
        if (m_halfDiminishedIndex == -1 || m_halfDiminishedList.size() == 0) {
            propMap.insert("half-dim", QVariant("-1"));
        } else {
            propMap.insert("half-dim", QVariant(m_halfDiminishedIndex));
        }
        if (m_minorIndex == -1 || m_minorList.size() == 0) {
            propMap.insert("min", QVariant("-1"));
        } else {
            propMap.insert("min", QVariant(m_minorIndex));
        }
        if (m_augmentedIndex == -1 || m_augmentedList.size() == 0) {
            propMap.insert("aug", QVariant("-1"));
        } else {
            propMap.insert("aug", QVariant(m_augmentedIndex));
        }
        if (m_diminishedIndex == -1 || m_diminishedList.size() == 0) {
            propMap.insert("dim", QVariant("-1"));
        } else {
            propMap.insert("dim", QVariant(m_diminishedIndex));
        }
        if (m_sixNineIndex == -1 || m_sixNineList.size() == 0) {
            propMap.insert("sixNine", QVariant("-1"));
        } else {
            propMap.insert("sixNine", QVariant(m_sixNineIndex));
        }
        if (m_omitIndex == -1 || m_omitList.size() == 0) {
            propMap.insert("omit", QVariant("-1"));
        } else {
            propMap.insert("omit", QVariant(m_omitIndex));
        }
        if (m_suspensionIndex == -1 || m_suspensionList.size() == 0) {
            propMap.insert("sus", QVariant("-1"));
        } else {
            propMap.insert("sus", QVariant(m_suspensionIndex));
        }
        if (m_bassNoteIndex == -1 || m_bassNoteList.size() == 0) {
            propMap.insert("bass", QVariant("-1"));
        } else {
            propMap.insert("bass", QVariant(m_bassNoteIndex));
        }
    } else {
        propMap.insert("maj7th", QVariant(""));
        propMap.insert("half-dim", QVariant(""));
        propMap.insert("min", QVariant(""));
        propMap.insert("aug", QVariant(""));
        propMap.insert("dim", QVariant(""));
        propMap.insert("sixNine", QVariant(""));
        propMap.insert("omit", QVariant(""));
        propMap.insert("sus", QVariant(""));
        propMap.insert("bass", QVariant(""));
    }

    // Properties
    propMap.insert("chrdSpell", QVariant(m_chordSpellingIndex));

    propMap.insert("qualMag", QVariant(m_qualityMag));
    propMap.insert("qualAdj", QVariant(m_qualityAdjust));
    propMap.insert("extMag", QVariant(m_extensionMag));
    propMap.insert("extAdj", QVariant(m_extensionAdjust));
    propMap.insert("modMag", QVariant(m_modifierMag));
    propMap.insert("modAdj", QVariant(m_modifierAdjust));

    propMap.insert("hFretDist", QVariant(m_harmonyFretDistance));
    propMap.insert("mnHDist", QVariant(m_minHarmonyDistance));
    propMap.insert("mxHBarDist", QVariant(m_maxHarmonyBarDistance));
    propMap.insert("mxSftAbv", QVariant(m_maxChordShiftAbove));
    propMap.insert("mxSftBlw", QVariant(m_maxChordShiftBelow));
    propMap.insert("cpFretPos", QVariant(m_capoFretPosition));

    propMap.insert("stkMod", QVariant(m_stackModifiers));

    propMap.insert("autoCap", QVariant(m_autoCapitalization));
    propMap.insert("minRtCap", QVariant(m_minorRootCapitalization));
    propMap.insert("qualMajCap", QVariant(m_qualityMajorCapitalization));
    propMap.insert("qualMinCap", QVariant(m_qualityMinorCapitalization));
    propMap.insert("bsNtCap", QVariant(m_bassNotesCapitalization));
    propMap.insert("solNtCap", QVariant(m_solfegeNotesCapitalization));

    propMap.insert("altParen", QVariant(m_alterationsParentheses));
    propMap.insert("susParen", QVariant(m_suspensionsParentheses));
    propMap.insert("minMajParen", QVariant(m_minMajParentheses));
    propMap.insert("addOmitParen", QVariant(m_addOmitParentheses));

    propMap.insert("chordSymbolScaling", QVariant(m_chordSymbolScaling));

    m_selectionHistory.insert(currentStyle, propMap);
    stringifyAndSaveSelectionHistory();
}

// Temporary implementation
int ChordSymbolEditorModel::getIconFromText(QString qualSym)
{
    QHash<QString, QHash<QString, mu::ui::IconCode::Code> > stringToIconcode = {
        { "Pop/Contemporary", {
              { "maj 7", mu::ui::IconCode::Code::POP_MAJ7_MAJ7 },
              { "ma 7", mu::ui::IconCode::Code::POP_MAJ7_MA7 },
              { "m", mu::ui::IconCode::Code::POP_MINOR_M },
              { "min", mu::ui::IconCode::Code::POP_MINOR_MIN },
              { "aug", mu::ui::IconCode::Code::POP_AUG_AUG },
              { "+", mu::ui::IconCode::Code::POP_AUG_PLUS },
              { "+ 5", mu::ui::IconCode::Code::POP_AUG_PLUS5 },
              { "dim", mu::ui::IconCode::Code::POP_DIM_DIM },
              { "circle", mu::ui::IconCode::Code::POP_DIM_DEGREE },
              { "omit", mu::ui::IconCode::Code::POP_OMIT_OMIT },
              { "no", mu::ui::IconCode::Code::POP_OMIT_NO },
              { "Stacked", mu::ui::IconCode::Code::POP_STACKED_YES },
              { "Non-Stacked", mu::ui::IconCode::Code::POP_STACKED_NO }
          } },
        { "Jazz", {
              { "MA 7", mu::ui::IconCode::Code::JAZZ_MAJ7_SCMA7 },
              { "ma 7", mu::ui::IconCode::Code::JAZZ_MAJ7_MA7 },
              { "maj 7", mu::ui::IconCode::Code::JAZZ_MAJ7_MAJ7 },
              { "MI", mu::ui::IconCode::Code::JAZZ_MINOR_SCMI },
              { "Mi", mu::ui::IconCode::Code::JAZZ_MINOR_MI },
              { "m", mu::ui::IconCode::Code::JAZZ_MINOR_M },
              { "min", mu::ui::IconCode::Code::JAZZ_MINOR_MIN },
              { "-", mu::ui::IconCode::Code::JAZZ_MINOR_MINUS },
              { "s+", mu::ui::IconCode::Code::JAZZ_AUG_SPLUS },
              { "aug", mu::ui::IconCode::Code::JAZZ_AUG_SAUG },
              { "Aug", mu::ui::IconCode::Code::JAZZ_AUG_AUG },
              { "dim", mu::ui::IconCode::Code::JAZZ_DIM_DIM },
              { "degree", mu::ui::IconCode::Code::JAZZ_DIM_DEGREE },
              { "6/9", mu::ui::IconCode::Code::JAZZ_69_69SLASH },
              { "69", mu::ui::IconCode::Code::JAZZ_69_69STACKED },
              { "omit", mu::ui::IconCode::Code::JAZZ_OMIT_OMIT },
              { "no", mu::ui::IconCode::Code::JAZZ_OMIT_NO },
              { "baseline", mu::ui::IconCode::Code::JAZZ_SUS_BASELINE },
              { "raised", mu::ui::IconCode::Code::JAZZ_SUS_RAISED },
              { "Stacked", mu::ui::IconCode::Code::JAZZ_STACKED_YES },
              { "Non-Stacked", mu::ui::IconCode::Code::JAZZ_STACKED_NO }
          } },
        { "Symbols", {
              { "/lowered", mu::ui::IconCode::Code::SYMBOLS_BASS_TILTED },
              { "/stacked", mu::ui::IconCode::Code::SYMBOLS_BASS_STACKED },
              { "/center", mu::ui::IconCode::Code::SYMBOLS_BASS_BASELINE },
              { "triangle", mu::ui::IconCode::Code::SYMBOLS_MAJ7_TRIANGLE },
              { "striangle", mu::ui::IconCode::Code::SYMBOLS_MAJ7_STRIANGLE },
              { "striangle 7", mu::ui::IconCode::Code::SYMBOLS_MAJ7_STRIANGLE7 },
              { "triangle 7", mu::ui::IconCode::Code::SYMBOLS_MAJ7_TRIANGLE7 },
              { "+", mu::ui::IconCode::Code::SYMBOLS_AUG_PLUS },
              { "+ 5", mu::ui::IconCode::Code::SYMBOLS_AUG_PLUS5 },
              { "soslash 7", mu::ui::IconCode::Code::SYMBOLS_HDIM_SOSLASH7 },
              { "oslash 7", mu::ui::IconCode::Code::SYMBOLS_HDIM_OSLASH7 },
              { "scircle", mu::ui::IconCode::Code::SYMBOLS_DIM_SCIRCLE7 },
              { "circle", mu::ui::IconCode::Code::SYMBOLS_DIM_CIRCLE7 },
              { "6/9", mu::ui::IconCode::Code::SYMBOLS_69_69SLASH },
              { "69", mu::ui::IconCode::Code::SYMBOLS_69_69STACKED },
              { "6-9", mu::ui::IconCode::Code::SYMBOLS_69_69FRACTION },
              { "omit", mu::ui::IconCode::Code::SYMBOLS_OMIT_OMIT },
              { "no", mu::ui::IconCode::Code::SYMBOLS_OMIT_NO },
              { "Stacked", mu::ui::IconCode::Code::SYMBOLS_STACKED_YES },
              { "Non-Stacked", mu::ui::IconCode::Code::SYMBOLS_STACKED_NO }
          } }
    };
    mu::ui::IconCode::Code iconCode = mu::ui::IconCode::Code::NONE;
    QString currentStyle = m_styles.at(m_currentStyleIndex).styleName;
    if (stringToIconcode.find(currentStyle) != stringToIconcode.end()) {
        iconCode = stringToIconcode[currentStyle][qualSym];
    } else {
        QString descFileWithPresets
            = globalContext()->currentNotation()->style()->styleValue(Ms::Sid::chordDescriptionFileWithPresets).toString();
        for (auto style: m_styles) {
            if (style.fileName == descFileWithPresets) {
                if (stringToIconcode.find(style.styleName) != stringToIconcode.end()) {
                    iconCode = stringToIconcode[currentStyle][qualSym];
                }
            }
        }
    }

    return static_cast<int>(iconCode);
}
