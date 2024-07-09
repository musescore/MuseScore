/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include "scoreaccessibilitysettingsmodel.h"

#include "translation.h"
#include "log.h"

using namespace mu::inspector;
using namespace mu::notation;

ScoreAccessibilitySettingsModel::ScoreAccessibilitySettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setSectionType(InspectorSectionType::SECTION_SCORE_ACCESSIBILITY);
    setTitle(muse::qtrc("inspector", "Accessibility"));
    createProperties();
    connect(m_scoreStylePreset, &PropertyItem::valueChanged, this, [this]() {
        if (!m_ignoreStyleChange) {
            setScoreStylePreset(m_scoreStylePreset);
        }
    });

    connect(m_accessibleNoteHead, &PropertyItem::valueChanged, this, [this]() {
        setAccessibleNoteHead(m_accessibleNoteHead);
    });

    connect(m_accessibleNoteHeadColor, &PropertyItem::valueChanged, this, [this]() {
        setAccessibleNoteHeadColor(m_accessibleNoteHeadColor);
    });

    globalContext()->currentNotation()->style()->styleChanged().onNotify(this, [this]() {
        if (!m_ignoreStyleChange && m_scoreStylePreset->value().toInt() != static_cast<int>(mu::engraving::ScoreStylePreset::CUSTOM)) {
            m_scoreStylePreset->setValue(static_cast<int>(mu::engraving::ScoreStylePreset::CUSTOM));
            emit scoreStylePresetChanged();
        }
    });
}

PropertyItem* ScoreAccessibilitySettingsModel::scoreStylePreset() const
{
    return m_scoreStylePreset;
}

PropertyItem* ScoreAccessibilitySettingsModel::accessibleNoteHead() const
{
    return m_accessibleNoteHead;
}

PropertyItem* ScoreAccessibilitySettingsModel::accessibleNoteHeadColor() const
{
    return m_accessibleNoteHeadColor;
}

void ScoreAccessibilitySettingsModel::setScoreStylePreset(PropertyItem* preset)
{
    m_ignoreStyleChange = true;
    m_scoreStylePreset = preset;
    loadStyle(preset);
    emit scoreStylePresetChanged();
    m_ignoreStyleChange = false;
}

void ScoreAccessibilitySettingsModel::setAccessibleNoteHead(PropertyItem* headSystem)
{
    m_accessibleNoteHead = headSystem;
    loadAccessibleNoteHead(headSystem);
    emit accessibleNoteHeadChanged();
}

void ScoreAccessibilitySettingsModel::setAccessibleNoteHeadColor(PropertyItem* headColor)
{
    m_accessibleNoteHeadColor = headColor;
    loadAccessibleNoteHeadColor(headColor);
    emit accessibleNoteHeadColorChanged();
}

void ScoreAccessibilitySettingsModel::createProperties()
{
    m_scoreStylePreset = buildPropertyItem(mu::engraving::Pid::SCORE_STYLE_PRESET);
    m_accessibleNoteHead = buildPropertyItem(mu::engraving::Pid::HEAD_SCHEME);
    m_accessibleNoteHeadColor = buildPropertyItem(mu::engraving::Pid::HEAD_COLOR);
    m_scoreStylePreset->setDefaultValue(static_cast<int>(mu::engraving::ScoreStylePreset::DEFAULT));
    m_scoreStylePreset->setValue(static_cast<int>(mu::engraving::ScoreStylePreset::DEFAULT));
    m_accessibleNoteHead->setDefaultValue(static_cast<int>(mu::engraving::NoteHeadScheme::HEAD_NORMAL));
    m_accessibleNoteHead->setValue(static_cast<int>(mu::engraving::NoteHeadScheme::HEAD_NORMAL));
    m_accessibleNoteHeadColor->setDefaultValue(static_cast<int>(mu::engraving::NoteHeadColor::COLOR_DEFAULT));
    m_accessibleNoteHeadColor->setValue(static_cast<int>(mu::engraving::NoteHeadColor::COLOR_DEFAULT));
}

void ScoreAccessibilitySettingsModel::requestElements()
{
    // Placeholder
}

void ScoreAccessibilitySettingsModel::loadProperties()
{
    static PropertyIdSet propertyIdSet {
        Pid::SCORE_STYLE_PRESET,
        Pid::HEAD_SCHEME,
        Pid::HEAD_COLOR
    };

    loadProperties(propertyIdSet);
}

void ScoreAccessibilitySettingsModel::loadProperties(const mu::engraving::PropertyIdSet& propertyIdSet)
{
    if (muse::contains(propertyIdSet, Pid::SCORE_STYLE_PRESET)) {
        loadPropertyItem(m_scoreStylePreset);
    }

    if (muse::contains(propertyIdSet, Pid::HEAD_SCHEME)) {
        loadPropertyItem(m_accessibleNoteHead);
    }

    if (muse::contains(propertyIdSet, Pid::HEAD_COLOR)) {
        loadPropertyItem(m_accessibleNoteHeadColor);
    }
}

void ScoreAccessibilitySettingsModel::resetProperties()
{
    m_scoreStylePreset->resetToDefault();
    m_accessibleNoteHead->resetToDefault();
    m_accessibleNoteHeadColor->resetToDefault();
    emit scoreStylePresetChanged();
    emit accessibleNoteHeadChanged();
    emit accessibleNoteHeadColorChanged();
}

QVariantList ScoreAccessibilitySettingsModel::possibleScoreStylePreset() const
{
    QMap<mu::engraving::ScoreStylePreset, QString> types {
        { mu::engraving::ScoreStylePreset::DEFAULT, muse::qtrc("inspector", "Default") },
        { mu::engraving::ScoreStylePreset::MSN_16MM, muse::qtrc("inspector", "16mm MSN") },
        { mu::engraving::ScoreStylePreset::MSN_18MM, muse::qtrc("inspector", "18mm MSN") },
        { mu::engraving::ScoreStylePreset::MSN_20MM, muse::qtrc("inspector", "20mm MSN") },
        { mu::engraving::ScoreStylePreset::MSN_24MM, muse::qtrc("inspector", "24mm MSN") },
        { mu::engraving::ScoreStylePreset::MSN_25MM, muse::qtrc("inspector", "25mm MSN") }
    };

    QVariantList result;

    if (m_scoreStylePreset->value().toInt() == static_cast<int>(mu::engraving::ScoreStylePreset::CUSTOM)) {
        QVariantMap customObj;
        customObj["text"] = muse::qtrc("inspector", "Custom");
        customObj["value"] = static_cast<int>(mu::engraving::ScoreStylePreset::CUSTOM);
        result << customObj;
    }

    for (mu::engraving::ScoreStylePreset type : types.keys()) {
        QVariantMap obj;
        obj["text"] = types[type];
        obj["value"] = static_cast<int>(type);
        result << obj;
    }

    return result;
}

QVariantList ScoreAccessibilitySettingsModel::possibleAccessibleNoteHeadTypes() const
{
    QMap<mu::engraving::NoteHeadScheme, QString> types {
        { mu::engraving::NoteHeadScheme::HEAD_NORMAL,                  muse::qtrc("inspector", "Normal") },
        { mu::engraving::NoteHeadScheme::HEAD_PITCHNAME,               muse::qtrc("inspector", "Pitch names") },
        { mu::engraving::NoteHeadScheme::HEAD_PITCHNAME_GERMAN,        muse::qtrc("inspector", "German pitch names") },
        { mu::engraving::NoteHeadScheme::HEAD_SOLFEGE,                 muse::qtrc("inspector", "Solfège movable do") },
        { mu::engraving::NoteHeadScheme::HEAD_SOLFEGE_FIXED,           muse::qtrc("inspector", "Solfège fixed do") },
        { mu::engraving::NoteHeadScheme::HEAD_FIGURENOTES_STAGE_3,     muse::qtrc("inspector", "Figurenotes (stage 3)") },
        { mu::engraving::NoteHeadScheme::HEAD_SHAPE_NOTE_4,            muse::qtrc("inspector", "4-shape (Walker)") },
        { mu::engraving::NoteHeadScheme::HEAD_SHAPE_NOTE_7_AIKIN,      muse::qtrc("inspector", "7-shape (Aikin)") },
        { mu::engraving::NoteHeadScheme::HEAD_SHAPE_NOTE_7_FUNK,       muse::qtrc("inspector", "7-shape (Funk)") },
        { mu::engraving::NoteHeadScheme::HEAD_SHAPE_NOTE_7_WALKER,     muse::qtrc("inspector", "7-shape (Walker)") },
    };

    QVariantList result;

    for (mu::engraving::NoteHeadScheme type : types.keys()) {
        QVariantMap obj;

        obj["text"] = types[type];
        obj["value"] = static_cast<int>(type);

        result << obj;
    }

    return result;
}

QVariantList ScoreAccessibilitySettingsModel::possibleAccessibleNoteHeadColorTypes() const
{
    QMap<mu::engraving::NoteHeadColor, QString> types {
        { mu::engraving::NoteHeadColor::COLOR_DEFAULT,                  muse::qtrc("inspector", "Default") },
        { mu::engraving::NoteHeadColor::COLOR_FIGURENOTES_STAGE_3,               muse::qtrc("inspector", "Figurenotes (stage 3)") },
        { mu::engraving::NoteHeadColor::COLOR_BOOMWHACKERS,     muse::qtrc("inspector", "Boomwhackers") },
    };

    QVariantList result;

    for (mu::engraving::NoteHeadColor type : types.keys()) {
        QVariantMap obj;

        obj["text"] = types[type];
        obj["value"] = static_cast<int>(type);

        result << obj;
    }

    return result;
}

void ScoreAccessibilitySettingsModel::loadStyle(PropertyItem* preset)
{
    int presetValue = preset->value().toInt();

    mu::engraving::ScoreStylePreset stylePreset = static_cast<mu::engraving::ScoreStylePreset>(presetValue);

    mu::engraving::ScoreStylePresetHelper scoreStylePresetHelper;
    muse::io::path_t filePath = scoreStylePresetHelper.getStyleFilePath(stylePreset);

    if (stylePreset == mu::engraving::ScoreStylePreset::DEFAULT) {
        if (filePath.empty()) {
            mu::engraving::StyleIdSet emptySet;
            globalContext()->currentNotation()->style()->resetAllStyleValues(emptySet);
        } else {
            globalContext()->currentNotation()->style()->loadStyle(filePath, true);
        }
    } else if (!filePath.empty()) {
        LOGI() << "Loading style from filePath:" << filePath;
        globalContext()->currentNotation()->style()->loadStyle(filePath, true);
    } else {
        LOGI() << "filePath is empty";
    }
}

void ScoreAccessibilitySettingsModel::loadAccessibleNoteHead(PropertyItem* noteHeadScheme)
{
    int schemeValue = noteHeadScheme->value().toInt();
    mu::engraving::NoteHeadScheme scheme = static_cast<mu::engraving::NoteHeadScheme>(schemeValue);
    globalContext()->currentNotation()->style()->setStyleValue(mu::engraving::Sid::noteHeadScheme, static_cast<int>(scheme));
    emit accessibleNoteHeadChanged();
}

void ScoreAccessibilitySettingsModel::loadAccessibleNoteHeadColor(PropertyItem* noteHeadColor)
{
    int colorValue = noteHeadColor->value().toInt();
    mu::engraving::NoteHeadColor colorScheme = static_cast<mu::engraving::NoteHeadColor>(colorValue);
    globalContext()->currentNotation()->style()->setStyleValue(mu::engraving::Sid::noteHeadColor, static_cast<int>(colorScheme));
    emit accessibleNoteHeadColorChanged();
}
