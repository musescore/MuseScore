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

    globalContext()->currentNotation()->style()->styleChanged().onNotify(this, [this]() {
        if (!m_ignoreStyleChange && m_scoreStylePreset->value().toInt() != static_cast<int>(mu::engraving::ScoreStylePreset::CUSTOM)) {
            m_scoreStylePreset->setValue(static_cast<int>(mu::engraving::ScoreStylePreset::CUSTOM));
            emit scoreStylePresetChanged();
        }
    });

    //connect(m_headSystem, &PropertyItem::valueChanged, this, &ScoreAccessibilitySettingsModel::onHeadSystemChanged);
}

PropertyItem* ScoreAccessibilitySettingsModel::scoreStylePreset() const
{
    return m_scoreStylePreset;
}

PropertyItem* ScoreAccessibilitySettingsModel::headSystem() const
{
    return m_headSystem;
}

void ScoreAccessibilitySettingsModel::setScoreStylePreset(PropertyItem* preset)
{
    m_ignoreStyleChange = true;
    m_scoreStylePreset = preset;
    loadStyle(preset);
    emit scoreStylePresetChanged();
    m_ignoreStyleChange = false;
}

void ScoreAccessibilitySettingsModel::setHeadSystem(PropertyItem* headSystem)
{
    m_headSystem = headSystem;
    //onHeadSystemChanged();
    emit headSystemChanged();
}

void ScoreAccessibilitySettingsModel::createProperties()
{
    m_scoreStylePreset = buildPropertyItem(mu::engraving::Pid::SCORE_STYLE_PRESET);
    m_headSystem = buildPropertyItem(mu::engraving::Pid::HEAD_SCHEME);
    m_scoreStylePreset->setDefaultValue(static_cast<int>(mu::engraving::ScoreStylePreset::DEFAULT));
    m_scoreStylePreset->setValue(static_cast<int>(mu::engraving::ScoreStylePreset::DEFAULT));
    m_headSystem->setDefaultValue(static_cast<int>(mu::engraving::NoteHeadScheme::HEAD_AUTO));
    m_headSystem->setValue(static_cast<int>(mu::engraving::NoteHeadScheme::HEAD_AUTO));
}

void ScoreAccessibilitySettingsModel::requestElements()
{
    // Placeholder
}

void ScoreAccessibilitySettingsModel::loadProperties()
{
    static PropertyIdSet propertyIdSet {
        Pid::SCORE_STYLE_PRESET,
        Pid::HEAD_SCHEME
    };

    loadProperties(propertyIdSet);
}

void ScoreAccessibilitySettingsModel::loadProperties(const mu::engraving::PropertyIdSet& propertyIdSet)
{
    if (muse::contains(propertyIdSet, Pid::SCORE_STYLE_PRESET)) {
        loadPropertyItem(m_scoreStylePreset);
    }

    if (muse::contains(propertyIdSet, Pid::HEAD_SCHEME)) {
        loadPropertyItem(m_headSystem);
    }
}

void ScoreAccessibilitySettingsModel::resetProperties()
{
    m_scoreStylePreset->resetToDefault();
    m_headSystem->resetToDefault();
    emit scoreStylePresetChanged();
    emit headSystemChanged();
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

QVariantList ScoreAccessibilitySettingsModel::possibleHeadSystemTypes() const
{
    QMap<mu::engraving::NoteHeadScheme, QString> types {
        { mu::engraving::NoteHeadScheme::HEAD_AUTO,                    muse::qtrc("inspector", "Auto") },
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

/*
void ScoreAccessibilitySettingsModel::onHeadSystemChanged()
{
    auto headScheme = static_cast<mu::engraving::NoteHeadScheme>(m_headSystem->value().toInt());
    LOGI() << "Changing NoteHeadScheme to " << m_headSystem->value().toInt();

    INotationPtr notation = currentNotation();
    if (!notation) {
        LOGE() << "No current notation found!";
        return;
    }

    INotationElementsPtr elementsService = notation->elements();
    if (!elementsService) {
        LOGE() << "No elements service found!";
        return;
    }

    auto elements = elementsService->elements();
    LOGI() << elements.size();
    for (auto* element : elements) {
        if (auto* note = dynamic_cast<Note*>(element)) {
            note->setHeadScheme(headScheme);
        }
    }

    notation->notationChanged().notify();
}
*/
