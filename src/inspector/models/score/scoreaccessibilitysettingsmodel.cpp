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
#include "engraving/types/typesconv.h"

using namespace mu::engraving;
using namespace mu::inspector;
using namespace mu::notation;

ScoreAccessibilitySettingsModel::ScoreAccessibilitySettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setSectionType(InspectorSectionType::SECTION_SCORE_ACCESSIBILITY);
    setTitle(muse::qtrc("inspector", "Accessibility"));
    setIsExpanded(projectConfiguration()->inspectorExpandAccessibilitySection());

    notationStyle()->styleChanged().onNotify(this, [this]() {
        if (!m_ignoreStyleChanges && !m_currentPresetEdited) {
            m_currentPresetEdited = true;
            engravingStyle().setPresetEdited(m_currentPresetEdited);
            emit possibleStylePresetsChanged();
            emit currentStylePresetIndexChanged();
        }
    });

    connect(this, &ScoreAccessibilitySettingsModel::isExpandedChanged, [this]() {
        projectConfiguration()->setInspectorExpandAccessibilitySection(isExpanded());

        // if (isExpanded()) {
        //     QTimer::singleShot(2000, this, [this]() {
        //         setIsExpanded(false);
        //         LOGW() << "Accessibility Timer Expired";
        //     });
        // }
    });
}

void ScoreAccessibilitySettingsModel::createProperties()
{
    NOT_IMPLEMENTED;
}

void ScoreAccessibilitySettingsModel::loadProperties()
{
    NOT_IMPLEMENTED;
}

void ScoreAccessibilitySettingsModel::resetProperties()
{
    NOT_IMPLEMENTED;
}

void ScoreAccessibilitySettingsModel::requestElements()
{
    // Avoid calling virtual ancestor.
}

QVariantList ScoreAccessibilitySettingsModel::possibleStylePresets() const
{
    QVariantList presets;

    QString text = "text";
    QString value = "value";
    QString preset = "preset";
    QString edited = "edited";

    for (int i = 0; i < static_cast<int>(ScoreStylePreset::MAX_PRESET); ++i) {
        ScoreStylePreset presetEnum = static_cast<ScoreStylePreset>(i);
        QString presetName = TConv::translatedUserName(presetEnum);

        presets.append(QVariantMap {
            { text, presetName },
            {
                value,
                QVariantMap {
                    { preset, i },
                    { edited, false }
                }
            }
        });

        if (i == static_cast<int>(m_currentPreset) && m_currentPresetEdited) {
            QString editedPresetName = muse::qtrc("inspector", "%1 (edited)").arg(presetName);
            presets.append(QVariantMap {
                { text, editedPresetName },
                {
                    value,
                    QVariantMap {
                        { preset, i },
                        { edited, true }
                    }
                }
            });
        }
    }

    return presets;
}

int ScoreAccessibilitySettingsModel::currentStylePresetIndex() const
{
    return static_cast<int>(m_currentPreset) + (m_currentPresetEdited ? 1 : 0);
}

void ScoreAccessibilitySettingsModel::setCurrentStylePresetIndex(int index)
{
    int currentIndex = currentStylePresetIndex();

    if (index == currentIndex) {
        return;
    }

    if (m_currentPresetEdited && index > currentIndex) {
        --index; // enum's values don't include the edited option
    }

    IF_ASSERT_FAILED(0 <= index && index < static_cast<int>(ScoreStylePreset::MAX_PRESET)) {
        return;
    }

    auto preset = static_cast<ScoreStylePreset>(index);

    m_ignoreStyleChanges = true;
    bool success = applyStylePreset(preset);
    m_ignoreStyleChanges = false;

    IF_ASSERT_FAILED(success) {
        return;
    }

    MStyle& style = engravingStyle();
    style.setPreset(preset);
    style.setPresetEdited(false);
    m_currentPreset = preset;

    if (m_currentPresetEdited) {
        m_currentPresetEdited = false;
        emit possibleStylePresetsChanged(); // "(edited)" option removed
    }

    emit currentStylePresetIndexChanged();
}

void ScoreAccessibilitySettingsModel::updateCurrentStylePreset()
{
    int oldIndex = currentStylePresetIndex();
    MStyle& style = engravingStyle();
    m_currentPreset = style.preset();
    bool presetEdited = style.presetEdited();

    if (presetEdited != m_currentPresetEdited) {
        m_currentPresetEdited = presetEdited;
        emit possibleStylePresetsChanged(); // "(edited)" option added or removed
    }

    if (currentStylePresetIndex() != oldIndex) {
        emit currentStylePresetIndexChanged();
    }
}

bool ScoreAccessibilitySettingsModel::applyStylePreset(ScoreStylePreset preset) const
{
    muse::io::path_t filePath = styleFilePath(preset);

    if (preset == ScoreStylePreset::DEFAULT && filePath.empty()) {
        LOGI() << "Loading default style values";
        StyleIdSet emptySet;
        notationStyle()->resetAllStyleValues(emptySet);
        return true;
    }

    IF_ASSERT_FAILED(!filePath.empty()) {
        return false;
    }

    LOGI() << "Loading style from file: " << filePath;
    return notationStyle()->loadStyle(filePath, true);
}

muse::io::path_t ScoreAccessibilitySettingsModel::styleFilePath(ScoreStylePreset preset) const
{
    const muse::io::path_t basePath = globalConfiguration()->appDataPath() + "styles/MSN/";

    switch (preset) {
    case ScoreStylePreset::DEFAULT:
        return engravingConfiguration()->defaultStyleFilePath();
    case ScoreStylePreset::MSN_16MM:
        return basePath + "16mm_MSN.mss";
    case ScoreStylePreset::MSN_18MM:
        return basePath + "18mm_MSN.mss";
    case ScoreStylePreset::MSN_20MM:
        return basePath + "20mm_MSN.mss";
    case ScoreStylePreset::MSN_22MM:
        return basePath + "22mm_MSN.mss";
    case ScoreStylePreset::MSN_25MM:
        return basePath + "25mm_MSN.mss";
    default:
        return muse::io::path_t();
    }
}

MStyle& ScoreAccessibilitySettingsModel::engravingStyle() const
{
    return currentNotation()->elements()->msScore()->score()->style();
}

INotationStylePtr ScoreAccessibilitySettingsModel::notationStyle() const
{
    return currentNotation()->style();
}
