/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
#include "playcounttextsettingsmodel.h"

#include "dom/playcounttext.h"
#include "engraving/dom/barline.h"
#include "engraving/types/typesconv.h"
#include "translation.h"

using namespace mu::inspector;
using namespace mu::engraving;

PlayCountTextSettingsModel::PlayCountTextSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_PLAY_COUNT_TEXT);
    setTitle(muse::qtrc("inspector", "Play count text"));
    setIcon(muse::ui::IconCode::Code::SECTION_BREAK);
    createProperties();
}

void PlayCountTextSettingsModel::createProperties()
{
    m_playCountText = buildPropertyItem(Pid::PLAY_COUNT_TEXT);
    m_playCountTextSetting = buildPropertyItem(Pid::PLAY_COUNT_TEXT_SETTING);

    connect(m_playCountTextSetting, &PropertyItem::valueChanged, this, [this]() {
        if (!m_playCountText->value().toString().isEmpty()) {
            return;
        }

        // HACK - fill with auto text
        const PlayCountText* pc = toPlayCountText(m_elementList.front());
        const BarLine* bl = pc->barline();
        const Measure* m = bl->measure();
        const int repeatCount = m->repeatCount();
        if (repeatCount == 2 && !m->style().styleV(Sid::repeatPlayCountShowSingleRepeats).toBool()) {
            return;
        }
        String text = TConv::translatedUserName(m->style().styleV(Sid::repeatPlayCountPreset).value<RepeatPlayCountPreset>()).arg(
            repeatCount);

        m_playCountText->setValue(text.toQString());
    });
}

void PlayCountTextSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(ElementType::PLAY_COUNT_TEXT);
}

void PlayCountTextSettingsModel::loadProperties()
{
    static const PropertyIdSet propertyIdSet {
        Pid::PLAY_COUNT_TEXT,
        Pid::PLAY_COUNT_TEXT_SETTING,
    };

    loadProperties(propertyIdSet);
}

void PlayCountTextSettingsModel::resetProperties()
{
    m_playCountText->resetToDefault();
    m_playCountTextSetting->resetToDefault();
}

void PlayCountTextSettingsModel::onNotationChanged(const mu::engraving::PropertyIdSet& changedPropertyIdSet,
                                                   const mu::engraving::StyleIdSet&)
{
    loadProperties(changedPropertyIdSet);
}

void PlayCountTextSettingsModel::loadProperties(const mu::engraving::PropertyIdSet& propertyIdSet)
{
    if (muse::contains(propertyIdSet, Pid::PLAY_COUNT_TEXT)) {
        loadPropertyItem(m_playCountText, [](const QVariant& elementPropertyValue) -> QVariant {
            return elementPropertyValue.toString();
        });
    }
    if (muse::contains(propertyIdSet, Pid::PLAY_COUNT_TEXT_SETTING)) {
        loadPropertyItem(m_playCountTextSetting, [](const QVariant& elementPropertyValue) -> QVariant {
            return elementPropertyValue.toInt();
        });
    }
}

PropertyItem* PlayCountTextSettingsModel::playCountText() const
{
    return m_playCountText;
}

PropertyItem* PlayCountTextSettingsModel::playCountTextSetting() const
{
    return m_playCountTextSetting;
}
