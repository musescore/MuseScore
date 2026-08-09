/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#include "organpedalmarksettingsmodel.h"

#include "engraving/dom/score.h"
#include "engraving/types/typesconv.h"

#include "notation/inotationelements.h"

#include "translation.h"

using namespace mu::propertiespanel;
using namespace mu::engraving;

OrganPedalMarkSettingsModel::OrganPedalMarkSettingsModel(QObject* parent, const muse::modularity::ContextPtr& iocCtx,
                                                     IElementRepositoryService* repository, PropertiesPanelModelType type)
    : PropertiesPanelAbstractModel(parent, iocCtx, repository)
{
    setModelType(type);
    setTitle(muse::qtrc("propertiespanel", "Organ pedal mark"));

    setIcon(muse::ui::IconCode::Code::PEDAL_MARKING);

    createProperties();
}

void OrganPedalMarkSettingsModel::createProperties()
{
    m_placement = buildPropertyItem(mu::engraving::Pid::PLACEMENT);

    m_size = buildPropertyItem(mu::engraving::Pid::MUSIC_SYMBOL_SIZE);

    m_style = buildPropertyItem(mu::engraving::Pid::TEXT_STYLE, [this](const mu::engraving::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue);
        emit requestReloadPropertyItems();
    });
}

void OrganPedalMarkSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::ORGAN_PEDAL_MARK);
}

void OrganPedalMarkSettingsModel::loadProperties()
{
    static const PropertyIdSet symbolPropertyIdSet {
        Pid::PLACEMENT,
        Pid::MUSIC_SYMBOL_SIZE,
        Pid::TEXT_STYLE,
    };

    loadProperties(symbolPropertyIdSet);
}

void OrganPedalMarkSettingsModel::onNotationChanged(const PropertyIdSet& changedProperyIds, const StyleIdSet& changedStyleIds)
{
    loadProperties(changedProperyIds);

    for (Sid s : {
             Sid::user1Name,
             Sid::user2Name,
             Sid::user3Name,
             Sid::user4Name,
             Sid::user5Name,
             Sid::user6Name,
             Sid::user7Name,
             Sid::user8Name,
             Sid::user9Name,
             Sid::user10Name,
             Sid::user11Name,
             Sid::user12Name
         }) {
        if (changedStyleIds.find(s) != changedStyleIds.cend()) {
            m_textStyles.clear();
            emit textStylesChanged();
            break;
        }
    }
}

void OrganPedalMarkSettingsModel::loadProperties(const PropertyIdSet& propertyIdSet)
{
    if (muse::contains(propertyIdSet, Pid::PLACEMENT)) {
        loadPropertyItem(m_placement);
    }

    if (muse::contains(propertyIdSet, Pid::MUSIC_SYMBOL_SIZE)) {
        loadPropertyItem(m_size);
    }

    if (muse::contains(propertyIdSet, Pid::TEXT_STYLE)) {
        loadPropertyItem(m_style);
    }
}

PropertyItem* OrganPedalMarkSettingsModel::placement() const
{
    return m_placement;
}

PropertyItem* OrganPedalMarkSettingsModel::size() const
{
    return m_size;
}

PropertyItem* OrganPedalMarkSettingsModel::style() const
{
    return m_style;
}

QVariantList OrganPedalMarkSettingsModel::textStyles()
{
    if (m_textStyles.empty()) {
        m_textStyles.reserve(int(TextStyleType::TEXT_TYPES));

        auto notation = currentNotation();
        Score* score = notation ? notation->elements()->msScore() : nullptr;

        for (int t = int(TextStyleType::DEFAULT) + 1; t < int(TextStyleType::TEXT_TYPES); ++t) {
            QVariantMap style;
            style["text"] = (score
                                 ? score->getTextStyleUserName(static_cast<TextStyleType>(t))
                                 : TConv::userName(static_cast<TextStyleType>(t)))
                                .qTranslated();
            style["value"] = t;
            m_textStyles << style;
        }
    }

    return m_textStyles;
}