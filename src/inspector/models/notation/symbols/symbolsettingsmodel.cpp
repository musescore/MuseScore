/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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
#include "symbolsettingsmodel.h"

#include "engraving/types/symnames.h"

#include "translation.h"

using namespace mu::inspector;

SymbolSettingsModel::SymbolSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_SYMBOL);
    setTitle(muse::qtrc("inspector", "Symbol"));
    setIcon(muse::ui::IconCode::Code::TRIANGLE_SYMBOL);
    createProperties();
}

void SymbolSettingsModel::createProperties()
{
    m_sym = buildPropertyItem(mu::engraving::Pid::SYMBOL);
    m_scoreFont = buildPropertyItem(mu::engraving::Pid::SCORE_FONT);
    m_symbolSize = buildPropertyItem(mu::engraving::Pid::SYMBOLS_SIZE,
                                     [this](const mu::engraving::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue.toDouble() / 100);
    },
                                     [this](const mu::engraving::Sid sid, const QVariant& newValue) {
        updateStyleValue(sid, newValue.toDouble() / 100);
        emit requestReloadPropertyItems();
    });
    m_symAngle = buildPropertyItem(mu::engraving::Pid::SYMBOL_ANGLE);
}

void SymbolSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::SYMBOL);
}

void SymbolSettingsModel::loadProperties()
{
    loadPropertyItem(m_sym);
    loadPropertyItem(m_scoreFont);
    loadPropertyItem(m_symbolSize, [](const QVariant& elementPropertyValue) -> QVariant {
        return muse::DataFormatter::roundDouble(elementPropertyValue.toDouble()) * 100;
    });
    loadPropertyItem(m_symAngle);
}

void SymbolSettingsModel::resetProperties()
{
    m_sym->resetToDefault();
    m_scoreFont->resetToDefault();
    m_symbolSize->resetToDefault();
    m_symAngle->resetToDefault();
}

PropertyItem* SymbolSettingsModel::sym() const
{
    return m_sym;
}

PropertyItem* SymbolSettingsModel::scoreFont() const
{
    return m_scoreFont;
}

PropertyItem* SymbolSettingsModel::symbolSize() const
{
    return m_symbolSize;
}

PropertyItem* SymbolSettingsModel::symAngle() const
{
    return m_symAngle;
}

QVariantList SymbolSettingsModel::symFonts()
{
    if (m_symFonts.empty()) {
        for (const engraving::IEngravingFontPtr& f : engravingFonts()->fonts()) {
            QVariantMap style;
            style["text"] = QString::fromStdString(f->name());
            style["value"] = QString::fromStdString(f->name());
            m_symFonts << style;
        }
    }
    return m_symFonts;
}
