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
}

PropertyItem* ScoreAccessibilitySettingsModel::scoreStylePreset() const
{
    return m_scoreStylePreset;
}

void ScoreAccessibilitySettingsModel::setScoreStylePreset(PropertyItem* preset)
{
    m_ignoreStyleChange = true;
    m_scoreStylePreset = preset;
    loadStyle(preset);
    emit scoreStylePresetChanged();
    m_ignoreStyleChange = false;
}

void ScoreAccessibilitySettingsModel::createProperties()
{
    m_scoreStylePreset = buildPropertyItem(mu::engraving::Pid::SCORE_STYLE_PRESET);
    m_scoreStylePreset->setDefaultValue(static_cast<int>(mu::engraving::ScoreStylePreset::DEFAULT));
    m_scoreStylePreset->setValue(static_cast<int>(mu::engraving::ScoreStylePreset::DEFAULT));
}

void ScoreAccessibilitySettingsModel::requestElements()
{
    // Placeholder
}

void ScoreAccessibilitySettingsModel::loadProperties()
{
    static PropertyIdSet propertyIdSet {
        Pid::SCORE_STYLE_PRESET
    };

    loadProperties(propertyIdSet);
}

void ScoreAccessibilitySettingsModel::loadProperties(const mu::engraving::PropertyIdSet& propertyIdSet)
{
    if (muse::contains(propertyIdSet, Pid::SCORE_STYLE_PRESET)) {
        loadPropertyItem(m_scoreStylePreset);
    }
}

void ScoreAccessibilitySettingsModel::resetProperties()
{
    m_scoreStylePreset->resetToDefault();
    emit scoreStylePresetChanged();
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
