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
#include "framework/global/defer.h"

using namespace mu::inspector;
using namespace mu::engraving;

ScoreAccessibilitySettingsModel::ScoreAccessibilitySettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setSectionType(InspectorSectionType::SECTION_SCORE_ACCESSIBILITY);
    setTitle(muse::qtrc("inspector", "Accessibility"));

    globalContext()->currentNotation()->style()->styleChanged().onNotify(this, [this]() {
        if (!m_ignoreStyleChange && !m_scoreStylePresetEdited) {
            m_scoreStylePresetEdited = true;
            globalContext()->currentNotation()->elements()->msScore()->score()->style().setPresetEdited(true);
            emit possibleScoreStylePresetsChanged();
            emit scoreStylePresetIndexChanged();
        }
    });
}

void ScoreAccessibilitySettingsModel::createProperties()
{
    // Placeholder
}

void ScoreAccessibilitySettingsModel::requestElements()
{
    // Placeholder
}

void ScoreAccessibilitySettingsModel::loadProperties()
{
    // Placeholder
}

void ScoreAccessibilitySettingsModel::resetProperties()
{
    // Placeholder
}

muse::io::path_t ScoreAccessibilitySettingsModel::getStyleFilePath(ScoreStylePreset preset) const
{
    muse::io::path_t basePath = globalConfiguration()->appDataPath() + "styles/MSN/";
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

QVariantList ScoreAccessibilitySettingsModel::possibleScoreStylePresets() const
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

        if (i == static_cast<int>(m_scoreStylePreset) && m_scoreStylePresetEdited) {
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

void ScoreAccessibilitySettingsModel::setScoreStylePresetIndex(int index)
{
    int oldIndex = scoreStylePresetIndex();

    if (index == oldIndex) {
        return;
    }

    if (m_scoreStylePresetEdited && index > oldIndex) {
        --index;
    }

    auto selectedPreset = static_cast<ScoreStylePreset>(index);

    bool presetChanged = (selectedPreset != m_scoreStylePreset);
    bool editedChanged = (m_scoreStylePresetEdited != false);

    MStyle& style = globalContext()->currentNotation()->elements()->msScore()->score()->style();

    if (presetChanged) {
        loadStyle(selectedPreset);
        m_scoreStylePreset = selectedPreset;
        style.setPreset(selectedPreset);

        emit scoreStylePresetIndexChanged();
    }

    if (editedChanged) {
        m_scoreStylePresetEdited = false;
        style.setPresetEdited(false);

        emit possibleScoreStylePresetsChanged();
        emit scoreStylePresetIndexChanged();
    }
}

void ScoreAccessibilitySettingsModel::loadStyle(ScoreStylePreset preset)
{
    muse::io::path_t filePath = getStyleFilePath(preset);
    m_ignoreStyleChange = true;
    DEFER {
        m_ignoreStyleChange = false;
    };

    if (preset == ScoreStylePreset::DEFAULT && filePath.empty()) {
        StyleIdSet emptySet;
        globalContext()->currentNotation()->style()->resetAllStyleValues(emptySet);
        return;
    }

    IF_ASSERT_FAILED(!filePath.empty()) {
        return;
    }

    LOGI() << "Loading style from filePath: " << filePath;
    globalContext()->currentNotation()->style()->loadStyle(filePath, true);
}

int ScoreAccessibilitySettingsModel::scoreStylePresetIndex() const
{
    return static_cast<int>(m_scoreStylePreset) + (m_scoreStylePresetEdited ? 1 : 0);
}

void ScoreAccessibilitySettingsModel::updateScoreStylePreset()
{
    MStyle style = globalContext()->currentNotation()->elements()->msScore()->score()->style();
    ScoreStylePreset stylePreset = style.preset();
    bool stylePresetEdited = style.presetEdited();
    bool indexChanged = false;

    if (m_scoreStylePreset != stylePreset) {
        m_scoreStylePreset = stylePreset;
        indexChanged = true;
    }

    if (m_scoreStylePresetEdited != stylePresetEdited) {
        m_scoreStylePresetEdited = stylePresetEdited;
        emit possibleScoreStylePresetsChanged();
        indexChanged = true;
    }

    if (indexChanged) {
        emit scoreStylePresetIndexChanged();
    }
}
