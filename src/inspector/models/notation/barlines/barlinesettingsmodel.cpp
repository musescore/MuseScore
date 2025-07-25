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
#include "barlinesettingsmodel.h"

#include "translation.h"
#include "types/barlinetypes.h"
#include "engraving/dom/barline.h"

using namespace mu::inspector;
using namespace mu::engraving;

BarlineSettingsModel::BarlineSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_BARLINE);
    setTitle(muse::qtrc("inspector", "Barline"));
    setIcon(muse::ui::IconCode::Code::SECTION_BREAK);
    createProperties();
}

void BarlineSettingsModel::createProperties()
{
    m_type = buildPropertyItem(Pid::BARLINE_TYPE);
    m_playCount = buildPropertyItem(Pid::REPEAT_COUNT, [this](const mu::engraving::Pid propertyId, const QVariant& newValue) {
        onPropertyValueChanged(propertyId, newValue);
        emit requestReloadInspectorListModel();
    });
    m_playCountText = buildPropertyItem(Pid::PLAY_COUNT_TEXT);
    m_playCountTextSetting = buildPropertyItem(Pid::PLAY_COUNT_TEXT_SETTING);
    m_isSpanToNextStaff = buildPropertyItem(Pid::BARLINE_SPAN);
    m_spanFrom = buildPropertyItem(Pid::BARLINE_SPAN_FROM);
    m_spanTo = buildPropertyItem(Pid::BARLINE_SPAN_TO);
    m_hasToShowTips = buildPropertyItem(Pid::BARLINE_SHOW_TIPS);

    connect(m_type, &PropertyItem::valueChanged, this, &BarlineSettingsModel::isRepeatStyleChangingAllowedChanged);
    connect(m_type, &PropertyItem::valueChanged, this, &BarlineSettingsModel::showPlayCountSettingsChanged);
}

void BarlineSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(ElementType::BAR_LINE);
}

void BarlineSettingsModel::loadProperties()
{
    static const PropertyIdSet propertyIdSet {
        Pid::BARLINE_TYPE,
        Pid::REPEAT_COUNT,
        Pid::PLAY_COUNT_TEXT,
        Pid::PLAY_COUNT_TEXT_SETTING,
        Pid::BARLINE_SPAN,
        Pid::BARLINE_SPAN_FROM,
        Pid::BARLINE_SPAN_TO,
        Pid::BARLINE_SHOW_TIPS,
    };

    loadProperties(propertyIdSet);
}

void BarlineSettingsModel::resetProperties()
{
    m_type->resetToDefault();
    m_playCount->resetToDefault();
    m_playCountText->resetToDefault();
    m_playCountTextSetting->resetToDefault();
    m_isSpanToNextStaff->resetToDefault();
    m_spanFrom->resetToDefault();
    m_spanTo->resetToDefault();
    m_hasToShowTips->resetToDefault();
}

void BarlineSettingsModel::onNotationChanged(const mu::engraving::PropertyIdSet& changedPropertyIdSet,
                                             const mu::engraving::StyleIdSet&)
{
    loadProperties(changedPropertyIdSet);
}

void BarlineSettingsModel::loadProperties(const mu::engraving::PropertyIdSet& propertyIdSet)
{
    if (muse::contains(propertyIdSet, Pid::BARLINE_TYPE)) {
        loadPropertyItem(m_type, [](const QVariant& elementPropertyValue) -> QVariant {
            return elementPropertyValue.toInt();
        });
    }

    if (muse::contains(propertyIdSet, Pid::REPEAT_COUNT)) {
        loadPropertyItem(m_playCount, [](const QVariant& elementPropertyValue) -> QVariant {
            return elementPropertyValue.toInt();
        });
    }
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

    if (muse::contains(propertyIdSet, Pid::BARLINE_SPAN)) {
        loadPropertyItem(m_isSpanToNextStaff, [](const QVariant& elementPropertyValue) -> QVariant {
            return elementPropertyValue.toBool();
        });
    }

    if (muse::contains(propertyIdSet, Pid::BARLINE_SPAN_FROM)) {
        loadPropertyItem(m_spanFrom, [](const QVariant& elementPropertyValue) -> QVariant {
            return elementPropertyValue.toInt();
        });
    }

    if (muse::contains(propertyIdSet, Pid::BARLINE_SPAN_TO)) {
        loadPropertyItem(m_spanTo, [](const QVariant& elementPropertyValue) -> QVariant {
            return elementPropertyValue.toInt();
        });
    }

    if (muse::contains(propertyIdSet, Pid::BARLINE_SHOW_TIPS)) {
        loadPropertyItem(m_hasToShowTips);
    }
}

void BarlineSettingsModel::applySpanPreset(const int presetType)
{
    BarlineTypes::SpanPreset type = static_cast<BarlineTypes::SpanPreset>(presetType);
    switch (type) {
    case BarlineTypes::SpanPreset::PRESET_DEFAULT:
        m_isSpanToNextStaff->resetToDefault();
        m_spanFrom->resetToDefault();
        m_spanTo->resetToDefault();
        break;
    case BarlineTypes::SpanPreset::PRESET_TICK_1:
        m_isSpanToNextStaff->setValue(false);
        m_spanFrom->setValue(mu::engraving::BARLINE_SPAN_TICK1_FROM);
        m_spanTo->setValue(mu::engraving::BARLINE_SPAN_TICK1_TO);
        break;
    case BarlineTypes::SpanPreset::PRESET_TICK_2:
        m_isSpanToNextStaff->setValue(false);
        m_spanFrom->setValue(mu::engraving::BARLINE_SPAN_TICK2_FROM);
        m_spanTo->setValue(mu::engraving::BARLINE_SPAN_TICK2_TO);
        break;
    case BarlineTypes::SpanPreset::PRESET_SHORT_1:
        m_isSpanToNextStaff->setValue(false);
        m_spanFrom->setValue(mu::engraving::BARLINE_SPAN_SHORT1_FROM);
        m_spanTo->setValue(mu::engraving::BARLINE_SPAN_SHORT1_TO);
        break;
    case BarlineTypes::SpanPreset::PRESET_SHORT_2:
        m_isSpanToNextStaff->setValue(false);
        m_spanFrom->setValue(mu::engraving::BARLINE_SPAN_SHORT2_FROM);
        m_spanTo->setValue(mu::engraving::BARLINE_SPAN_SHORT2_TO);
        break;
    default:
        break;
    }
}

void BarlineSettingsModel::setSpanIntervalAsStaffDefault()
{
    undoStack()->prepareChanges(muse::TranslatableString("undoableAction", "Set barline span interval as staff default"));

    std::vector<mu::engraving::EngravingItem*> staves;

    auto undoChangeProperty = [](mu::engraving::EngravingObject* o, mu::engraving::Pid pid, const QVariant& val)
    {
        o->undoChangeProperty(pid, PropertyValue::fromQVariant(val, mu::engraving::propertyType(pid)));
    };

    for (mu::engraving::EngravingItem* item : m_elementList) {
        if (!item->isBarLine()) {
            continue;
        }

        mu::engraving::BarLine* barline = mu::engraving::toBarLine(item);
        mu::engraving::Staff* staff = barline->staff();

        if (std::find(staves.cbegin(), staves.cend(), staff) == staves.cend()) {
            undoChangeProperty(staff, mu::engraving::Pid::STAFF_BARLINE_SPAN, m_isSpanToNextStaff->value());
            undoChangeProperty(staff, mu::engraving::Pid::STAFF_BARLINE_SPAN_FROM, m_spanFrom->value());
            undoChangeProperty(staff, mu::engraving::Pid::STAFF_BARLINE_SPAN_TO, m_spanTo->value());
            staves.push_back(staff);
        }

        if (barline->barLineType() == mu::engraving::BarLineType::NORMAL) {
            barline->setGenerated(true);
        }
    }

    undoStack()->commitChanges();
    updateNotation();
}

PropertyItem* BarlineSettingsModel::type() const
{
    return m_type;
}

PropertyItem* BarlineSettingsModel::playCount() const
{
    return m_playCount;
}

PropertyItem* BarlineSettingsModel::playCountText() const
{
    return m_playCountText;
}

PropertyItem* BarlineSettingsModel::playCountTextSetting() const
{
    return m_playCountTextSetting;
}

PropertyItem* BarlineSettingsModel::isSpanToNextStaff() const
{
    return m_isSpanToNextStaff;
}

PropertyItem* BarlineSettingsModel::spanFrom() const
{
    return m_spanFrom;
}

PropertyItem* BarlineSettingsModel::spanTo() const
{
    return m_spanTo;
}

PropertyItem* BarlineSettingsModel::hasToShowTips() const
{
    return m_hasToShowTips;
}

bool BarlineSettingsModel::isRepeatStyleChangingAllowed() const
{
    using LineType = BarlineTypes::LineType;

    static const QList<LineType> allowedLineTypes {
        LineType::TYPE_START_REPEAT,
        LineType::TYPE_END_REPEAT,
        LineType::TYPE_END_START_REPEAT
    };

    LineType currentType = static_cast<LineType>(m_type->value().toInt());
    return allowedLineTypes.contains(currentType);
}

bool BarlineSettingsModel::showPlayCountSettings() const
{
    using LineType = BarlineTypes::LineType;

    static const QList<LineType> allowedLineTypes {
        LineType::TYPE_END_REPEAT,
        LineType::TYPE_END_START_REPEAT
    };

    LineType currentType = static_cast<LineType>(m_type->value().toInt());
    return allowedLineTypes.contains(currentType);
}
