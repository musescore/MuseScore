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
using namespace mu::engraving;

ScoreAccessibilitySettingsModel::ScoreAccessibilitySettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setSectionType(InspectorSectionType::SECTION_SCORE_ACCESSIBILITY);
    setTitle(muse::qtrc("inspector", "Accessibility"));

    globalContext()->currentNotation()->style()->styleChanged().onNotify(this, [this]() {
        if (!m_ignoreStyleChange) {
            isAccessibilityStyleEdited = true;
            emit accessibilityStyleEditedChanged();
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

muse::io::path_t ScoreAccessibilitySettingsModel::getStyleFilePath(int preset) const
{
    muse::io::path_t basePath = globalConfiguration()->appDataPath() + "styles/MSN/";
    switch (static_cast<AccessibilityStylePreset>(preset)) {
    case AccessibilityStylePreset::DEFAULT:
        return engravingConfiguration()->defaultStyleFilePath();
    case AccessibilityStylePreset::MSN_16MM:
        return basePath + "16mm_MSN.mss";
    case AccessibilityStylePreset::MSN_18MM:
        return basePath + "18mm_MSN.mss";
    case AccessibilityStylePreset::MSN_20MM:
        return basePath + "20mm_MSN.mss";
    case AccessibilityStylePreset::MSN_22MM:
        return basePath + "22mm_MSN.mss";
    case AccessibilityStylePreset::MSN_25MM:
        return basePath + "25mm_MSN.mss";
    default:
        return muse::io::path_t();
    }
}

QVariantList ScoreAccessibilitySettingsModel::possibleScoreStylePreset() const
{
    QString text = "text";
    QString value = "value";
    QString edited = "edited";

    QVariantList presets = {
        QVariantMap {
            { text, muse::qtrc("inspector", "Default") },
            { value, static_cast<int>(AccessibilityStylePreset::DEFAULT) },
            { edited, false }
        },
        QVariantMap {
            { text, muse::qtrc("inspector", "16mm MSN") },
            { value, static_cast<int>(AccessibilityStylePreset::MSN_16MM) },
            { edited, false }
        },
        QVariantMap {
            { text, muse::qtrc("inspector", "18mm MSN") },
            { value, static_cast<int>(AccessibilityStylePreset::MSN_18MM) },
            { edited, false }
        },
        QVariantMap {
            { text, muse::qtrc("inspector", "20mm MSN") },
            { value, static_cast<int>(AccessibilityStylePreset::MSN_20MM) },
            { edited, false }
        },
        QVariantMap {
            { text, muse::qtrc("inspector", "22mm MSN") },
            { value, static_cast<int>(AccessibilityStylePreset::MSN_22MM) },
            { edited, false }
        },
        QVariantMap {
            { text, muse::qtrc("inspector", "25mm MSN") },
            { value, static_cast<int>(AccessibilityStylePreset::MSN_25MM) },
            { edited, false }
        }
    };

    AccessibilityStylePreset currentPreset = globalContext()->currentNotation()->elements()->msScore()->score()->style().stylePreset();

    for (int i = 0; i < presets.size(); ++i) {
        QVariantMap preset = presets[i].toMap();
        int presetValue = preset.value(value).toInt();
        if (presetValue == static_cast<int>(currentPreset) && isAccessibilityStyleEdited) {
            preset[edited] = true;
            globalContext()->currentNotation()->elements()->msScore()->score()->style().setStylePresetEdited(true);
        }
        presets[i] = preset;
    }

    return presets;
}

void ScoreAccessibilitySettingsModel::loadStyle(int preset)
{
    muse::io::path_t filePath = getStyleFilePath(preset);
    AccessibilityStylePreset presetName = static_cast<AccessibilityStylePreset>(preset);
    m_ignoreStyleChange = true;

    if (presetName == AccessibilityStylePreset::DEFAULT) {
        if (filePath.empty()) {
            StyleIdSet emptySet;
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
    globalContext()->currentNotation()->elements()->msScore()->score()->style().setStylePreset(presetName);

    isAccessibilityStyleEdited = false;
    globalContext()->currentNotation()->elements()->msScore()->score()->style().setStylePresetEdited(false);
    emit accessibilityStyleEditedChanged();
    m_ignoreStyleChange = false;
}

int ScoreAccessibilitySettingsModel::scoreStylePreset() const
{
    int presetIndex = static_cast<int>(globalContext()->currentNotation()->elements()->msScore()->score()->style().stylePreset());
    if (presetIndex == 0) {
        return (!isAccessibilityStyleEdited) ? 0 : 1;
    } else if (presetIndex == 1) {
        return (!isAccessibilityStyleEdited) ? 1 : 2;
    } else if (presetIndex == 2) {
        return (!isAccessibilityStyleEdited) ? 2 : 3;
    } else if (presetIndex == 3) {
        return (!isAccessibilityStyleEdited) ? 3 : 4;
    } else if (presetIndex == 4) {
        return (!isAccessibilityStyleEdited) ? 4 : 5;
    } else if (presetIndex == 5) {
        return (!isAccessibilityStyleEdited) ? 5 : 6;
    }
}

void ScoreAccessibilitySettingsModel::readStyleFileAccessibilityStyleEdited()
{
    isAccessibilityStyleEdited = globalContext()->currentNotation()->elements()->msScore()->score()->style().stylePresetEdited();
    emit accessibilityStyleEditedChanged();
}
